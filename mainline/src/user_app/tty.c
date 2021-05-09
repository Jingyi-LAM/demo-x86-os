#include "tty.h"
#include "keyboard.h"
#include "process.h"
#include "common.h"
#include "heap.h"
#include "debug.h"
#include "interrupt.h"

tty_cmd_handler_t *g_tty_cmd_list;
tty_cmd_buf_t g_tty_cmd_buffer;

static int current_row = 0;
static int current_column = 0;

void tty_show_string(const char *str)
{
        const char *p = str;

        while (*p != '\0') {
                if (current_column > 160) {
                        current_column = 0;
                        current_row += 1;
                }

                *((u8 *)0xb8000 + 160 * current_row + current_column) = *p;
                *((u8 *)0xb8000 + 160 * current_row + current_column + 1) = 0xc;

                current_column += 2;
                p += 1;
        }
}

void tty_show_char(char ch)
{
        *((unsigned char *)0xb8000 + 160 * current_row + current_column) = ch;
        *((unsigned char *)0xb8000 + 160 * current_row + current_column + 1) = 0xc;

        current_column += 2;
        if (current_column >= 160) {
                current_column = 0;
                current_row += 1;
        }
}

void tty_newline(void)
{
        current_row += 1;
        current_column = 0;
}

void tty_clear_screen(void)
{
        u8 *p = (u8 *)0xb8000;
        int i = 0;

        for (i = 0; i < 160 * 20; i++) {
                *(p + i) = 0;
        }
}

void tty_write(u8 *buf, u32 length)
{
        int i = 0;

        for (i = 0; i < length; i++)
                tty_show_char(*(buf + i));
}


void tty_register_command(const char *cmd, void (*handler)(void))
{
        static char is_inited = 0;
        tty_cmd_handler_t *ptr_prev = 0;
        tty_cmd_handler_t *ptr_new = 0;
        int i = 0;

        if (!is_inited) {
                g_tty_cmd_list = 0;
                is_inited = 1;
        }

        ptr_new = (tty_cmd_handler_t *)rheap_malloc(sizeof(tty_cmd_handler_t));
        if (weak_assert(ptr_new))
                return;

        for (i = 0; i < MAX_CMD_SIZE; i++) {
                if (cmd[i] == '\0')
                        break;
                ptr_new->cmd[i] = cmd[i];
        }
        ptr_new->handler = handler; 
        ptr_new->next = 0;

        if (g_tty_cmd_list) {
                ptr_prev = g_tty_cmd_list;
                while (ptr_prev->next)
                        ptr_prev = ptr_prev->next;
                ptr_prev->next = ptr_new;
        } else {
                g_tty_cmd_list = ptr_new;
        }
}

static void tty_cmd_execute(void)
{
        tty_cmd_handler_t *cmd_handler = g_tty_cmd_list;

        while (cmd_handler) {
                if (str_cmp(cmd_handler->cmd, g_tty_cmd_buffer.buffer, MAX_CMD_SIZE)) {
                        cmd_handler->handler();
                        break;
                }
        
                cmd_handler = cmd_handler->next;
        }
}

static void tty_init_screen(void)
{
        current_row = 0;
        current_column = 0;
        mem_set(&g_tty_cmd_buffer, 0, sizeof(g_tty_cmd_buffer));

        tty_clear_screen();
        tty_show_string("Jingyi@PCM-X01:# ");
}

static void tty_input_handler(u8 ch)
{
        switch (ch) {
        case ENTER:
                g_tty_cmd_buffer.buffer[g_tty_cmd_buffer.current_pos] = '\0';
                g_tty_cmd_buffer.current_pos = 0;
                tty_cmd_execute();
                tty_newline();
                tty_show_string("Jingyi@PCM-X01:# ");
                break;
        default:
                g_tty_cmd_buffer.buffer[g_tty_cmd_buffer.current_pos] = ch;
                g_tty_cmd_buffer.current_pos += 1;
                if (g_tty_cmd_buffer.current_pos > MAX_CMD_SIZE)
                        g_tty_cmd_buffer.current_pos = 0;
                tty_show_char(ch);
                break;
        }
}

void tty_task(void)
{
        char ch = 0;
        
        keyboard_init();
        tty_init_screen();
        register_syscall_handler(SYSCALL_TTY_WRITE, tty_write);
        tty_register_command("clear", tty_clear_screen);

        for ( ;; ) {
                ch = keyboard_readbyte();
                tty_input_handler(ch);
        }
}

