#include "debug.h"
#include "heap.h"
#include "interrupt.h"
#include "io.h"
#include "keyboard.h"
#include "memory.h"
#include "process.h"
#include "string.h"
#include "tty.h"

tty_cmd_handler_t *g_tty_cmd_list;
tty_cmd_buf_t g_tty_cmd_buffer;

static uint32_t current_row = 0;
static uint32_t current_column = 0;
int32_t g_syscall_tty_write_index = 0;

static void tty_set_cursor(uint8_t color)
{
        uint32_t pos = current_row * TTY_WIN_WIDTH * 2 + current_column;

        __asm__ __volatile__("cli":::);
        io_out8(0x3d4, 0xe);
        io_out8(0x3d5, ((pos / 2) >> 8) & 0xff);
        io_out8(0x3d4, 0xf);
        io_out8(0x3d5, (pos / 2) & 0xff);
        __asm__ __volatile__("sti":::);

        *((uint8_t *)(VRAM_BASE + pos + 1)) = color;
}


static void tty_show_string(const int8_t *str, uint8_t color)
{
        const int8_t *ptr_str = str;
        uint16_t *p = (uint16_t *)(VRAM_BASE + TTY_WIN_WIDTH * 2 * current_row
                         + current_column);

        while (*ptr_str != '\0') {
                *p = *ptr_str | (color << 8);
                p += 1;
                ptr_str += 1;

                current_column += 2;
                if (current_column > TTY_WIN_WIDTH * 2) {
                        current_column = 0;
                        current_row += 1;
                }
        }
        tty_set_cursor(TTY_BG_GRAY | TTY_FG_LIGHTCYAN);
}

static void tty_show_char(int8_t ch, uint8_t color)
{
        uint16_t *p = (uint16_t *)(VRAM_BASE + TTY_WIN_WIDTH * 2 * current_row
                        + current_column);

        *p = ch | ((TTY_BG_GRAY | TTY_FG_LIGHTCYAN) << 8);

        current_column += 2;
        if (current_column >= TTY_WIN_WIDTH * 2) {
                current_column = 0;
                current_row += 1;
        }
        tty_set_cursor(TTY_BG_GRAY | TTY_FG_LIGHTCYAN);
}

static void tty_newline(void)
{
        current_row += 1;
        current_column = 0;
}

static void tty_clear_screen(void)
{
        uint16_t *p = (uint16_t *)VRAM_BASE;
        uint32_t i = 0;

        for (i = 0; i < TTY_WIN_WIDTH * TTY_WIN_HEIGHT; i++) {
                *(p + i) = TTY_BG_GRAY << 8;
        }

        current_column = 0;
        current_row = -1;
}

static void tty_write(uint8_t *buf, int32_t offset, uint32_t length, int8_t color)
{
        int8_t *ptr_str = buf;
        uint16_t *p = 0;
        uint32_t i = 0;

        if (offset < 0)
                p = (uint16_t *)(VRAM_BASE + TTY_WIN_WIDTH * 2 * current_row
                        + current_column);
        else
                p = (uint16_t *)(VRAM_BASE + offset);

        if (color < 0)
                color = TTY_DEFAULT_COLOR;

        if ((uint32_t)p + length > 0xbffff || (uint32_t)p + length < VRAM_BASE)
                return;

        for (i = 0; i < length; i++) {
                *p = *ptr_str | (color << 8);
                p += 1;
                ptr_str += 1;
        }
}


void tty_register_command(const int8_t *cmd, void (*handler)(void))
{
        static int8_t is_inited = 0;
        tty_cmd_handler_t *ptr_prev = 0;
        tty_cmd_handler_t *ptr_new = 0;
        uint32_t i = 0;

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
                if (strcmp(cmd_handler->cmd,
                            g_tty_cmd_buffer.buffer,
                            MAX_CMD_SIZE)) {
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
        memset(&g_tty_cmd_buffer, 0, sizeof(g_tty_cmd_buffer));

        tty_clear_screen();
        tty_newline();
        tty_show_string("Jingyi@PCM-X01:# ", TTY_BG_GRAY | TTY_FG_YELLOW);
}

static void tty_input_handler(int8_t ch)
{
        switch (ch) {
        case ENTER:
                g_tty_cmd_buffer.buffer[g_tty_cmd_buffer.current_pos] = '\0';
                g_tty_cmd_buffer.current_pos = 0;
                tty_cmd_execute();
                tty_newline();
                tty_show_string("Jingyi@PCM-X01:# ",
                                TTY_BG_GRAY | TTY_FG_YELLOW);
                break;
        default:
                g_tty_cmd_buffer.buffer[g_tty_cmd_buffer.current_pos] = ch;
                g_tty_cmd_buffer.current_pos += 1;
                if (g_tty_cmd_buffer.current_pos > MAX_CMD_SIZE)
                        g_tty_cmd_buffer.current_pos = 0;
                tty_show_char(ch, TTY_BG_GRAY | TTY_FG_LIGHTCYAN);
                break;
        }
}

void tty_task(void)
{
        int8_t ch = 0;
        
        keyboard_init();
        tty_init_screen();
        g_syscall_tty_write_index = register_syscall_handler((void (*)(void))tty_write);
        tty_register_command("clear", tty_clear_screen);

        for ( ;; ) {
                ch = keyboard_readbyte();
                tty_input_handler(ch);
        }
}

void tty_display(uint32_t offset, uint32_t length, uint8_t *buf, uint8_t color)
{
        __asm__ __volatile__(
                "movl %0,       %%eax   \n\t"
                "movl %1,       %%ebx   \n\t"
                "movl %2,       %%ecx   \n\t"
                "movl %3,       %%edx   \n\t"
                "movl %4,       %%edi   \n\t"
                "int  $100              \n\t"
                :
                :"g"(buf), "g"(offset), "g"(length), "g"(color), "g"(g_syscall_tty_write_index)
                :"eax", "ebx", "ecx", "edx", "edi"
        );
}
