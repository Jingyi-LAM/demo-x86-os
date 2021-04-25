#ifndef __TTY_H_
#define __TTY_H_

#include "common.h"

#define MAX_CMD_SIZE    128

typedef struct tty_command_handler {
        char    cmd[MAX_CMD_SIZE];
        void    (*handler)(void);
        struct tty_command_handler *next;
} tty_cmd_handler_t;

typedef struct tty_command_buffer {
        u8      buffer[MAX_CMD_SIZE];
        u32     current_pos;
} tty_cmd_buf_t;


void tty_register_command(const char *cmd, void (*handler)(void));
void tty_task(void);
#endif
