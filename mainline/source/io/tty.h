#ifndef __TTY_H_
#define __TTY_H_

#include "typedef.h"

#define MAX_CMD_SIZE            128
#define VRAM_BASE               0xb8000
#define TTY_WIN_WIDTH           80
#define TTY_WIN_HEIGHT          25

#define TTY_FG_BLACK            0x0
#define TTY_FG_BLUE             0x1
#define TTY_FG_GREEN            0x2
#define TTY_FG_CYAN             0x3
#define TTY_FG_RED              0x4
#define TTY_FG_PURPL            0x5
#define TTY_FG_BROWN            0x6
#define TTY_FG_GRAY             0x7
#define TTY_FG_DARKGRAY         0x8
#define TTY_FG_LIGHTBLUE        0x9
#define TTY_FG_LIGHTGREEN       0xa
#define TTY_FG_LIGHTCYAN        0xb
#define TTY_FG_LIGHTRED         0xc
#define TTY_FG_LIGHTPURPLE      0xd
#define TTY_FG_YELLOW           0xe
#define TTY_FG_WHITE            0xf

#define TTY_BG_BLACK            (0x0 << 4)
#define TTY_BG_BLUE             (0x1 << 4)
#define TTY_BG_GREEN            (0x2 << 4)
#define TTY_BG_CYAN             (0x3 << 4)
#define TTY_BG_RED              (0x4 << 4)
#define TTY_BG_PURPL            (0x5 << 4)
#define TTY_BG_BROWN            (0x6 << 4)
#define TTY_BG_GRAY             (0x7 << 4)
#define TTY_BG_DARKGRAY         (0x8 << 4)
#define TTY_BG_LIGHTBLUE        (0x9 << 4)
#define TTY_BG_LIGHTGREEN       (0xa << 4)
#define TTY_BG_LIGHTCYAN        (0xb << 4)
#define TTY_BG_LIGHTRED         (0xc << 4)
#define TTY_BG_LIGHTPURPLE      (0xd << 4)
#define TTY_BG_YELLOW           (0xe << 4)
#define TTY_BG_WHITE            (0xf << 4)

#define TTY_DEFAULT_COLOR       (TTY_BG_GRAY | TTY_FG_LIGHTCYAN)

typedef struct tty_command_handler {
        int8_t   cmd[MAX_CMD_SIZE];
        void     (*handler)(void);
        struct   tty_command_handler *next;
} tty_cmd_handler_t;

typedef struct tty_command_buffer {
        uint8_t  buffer[MAX_CMD_SIZE];
        uint32_t current_pos;
} tty_cmd_buf_t;

void tty_register_command(const int8_t *cmd, void (*handler)(void));
void tty_task(void);
#endif
