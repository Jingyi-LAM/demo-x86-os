#include "tty.h"
#include "keyboard.h"
#include "process.h"
#include "common.h"
#include "heap.h"
#include "debug.h"

tty_cmd_handler_t *g_tty_cmd_list;
tty_cmd_buf_t g_tty_cmd_buffer;

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

void tty_cmd_execute(void)
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

static void screen_clear(void)
{
        u8 *p = (u8 *)0xb8000;
        int i = 0;

        for (i = 0; i < 160 * 20; i++) {
                *(p + i) = 0; 
        }
        log_reset();
}

void tty_task(void)
{
        char ch = 0;
        char buf[50] = {0};
        
        keyboard_init();
        screen_clear();
        log_enter();
        g_tty_cmd_buffer.current_pos = 0;
        log_string("Jingyi@PCM-X01:# ");


        tty_register_command("clear", screen_clear);

        for ( ;; ) {
                ch = keyboard_readbyte();
                switch (ch) {
                case ENTER:
                        g_tty_cmd_buffer.buffer[g_tty_cmd_buffer.current_pos] = '\0';
                        g_tty_cmd_buffer.current_pos = 0;
                        tty_cmd_execute();
                        log_enter();

                        vsprint(buf, "Jingyi@PCM-X01:# ");
                        log_string(buf);
                        break;
                default:                       
                        g_tty_cmd_buffer.buffer[g_tty_cmd_buffer.current_pos] = ch;
                        g_tty_cmd_buffer.current_pos += 1;
                        if (g_tty_cmd_buffer.current_pos > MAX_CMD_SIZE)
                                g_tty_cmd_buffer.current_pos = 0;
                        screen_show_char(ch);
                        break;
                }
        }
}

