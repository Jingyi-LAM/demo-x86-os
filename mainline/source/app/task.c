#include "debug.h"
#include "fs.h"
#include "hd.h"
#include "heap.h"
#include "interrupt.h"
#include "ipc.h"
#include "keyboard.h"
#include "log.h"
#include "memory.h"
#include "process.h"
#include "string.h"
#include "tty.h"
#include "typedef.h"
#include "kernel.h"

#define LOCAL_TRACE     (1)

void cmd_say_hello(void)
{
        char show[] = "Hello~";
        int len = strlen(show);

        tty_newline();
        tty_display(-1, len, show, TTY_BG_GRAY | TTY_FG_LIGHTCYAN);

        log_buffer_init();
}

void cmd_say_goodbye(void)
{
        char show[] = "Goodbye~";
        int len = strlen(show);

        tty_newline();
        tty_display(-1, len, show, TTY_BG_GRAY | TTY_FG_LIGHTCYAN);

        LTRACE("Log Test\n");
}

void demo1(void)
{
        tty_register_command("say hello", cmd_say_hello);

        for ( ;; ) {

        }
}

void demo2(void)
{
        tty_register_command("say goodbye", cmd_say_goodbye);

        for ( ;; ) {

        }
}

void create_task(void)
{
        int8_t *tty_stack = rheap_malloc(4096);
        int8_t *hd_stack = rheap_malloc(20480);
        int8_t *fs_stack = rheap_malloc(2048);
        int8_t *demo1_stack = rheap_malloc(1024);
        int8_t *demo2_stack = rheap_malloc(1024);

        proc_info_t proc_info = {
                .f_entry = tty_task,
                .stack = tty_stack,
                .stack_size = 4096,
                .priviledge = 1,
                .name = "tty",
        };
        create_process(&proc_info);

        proc_info.f_entry = hd_task;
        proc_info.stack = hd_stack;
        proc_info.stack_size = 20480;
        proc_info.priviledge = 1;
        strcpy(proc_info.name, "hd");
        create_process(&proc_info);

        proc_info.f_entry = fs_task;
        proc_info.stack = fs_stack;
        proc_info.stack_size = 2048;
        proc_info.priviledge = 1;
        strcpy(proc_info.name, "fs");
        create_process(&proc_info);

        proc_info.f_entry = demo1;
        proc_info.stack = demo1_stack;
        proc_info.stack_size = 1024;
        proc_info.priviledge = 3;
        strcpy(proc_info.name, "demo1");
        create_process(&proc_info);

        proc_info.f_entry = demo2;
        proc_info.stack = demo2_stack;
        proc_info.stack_size = 1024;
        proc_info.priviledge = 3;
        strcpy(proc_info.name, "demo2");
        create_process(&proc_info);
}

