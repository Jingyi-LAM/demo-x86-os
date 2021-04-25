#include "task.h"
#include "heap.h"
#include "tty.h"

void print_k(void)
{
        u8 *p = (u8 *)0xb8408;

        *p = 'k';
        *(p + 1) = 0xc;
}

void print_f(void)
{
        u8 *p = (u8 *)0xb840a;

        *p = 'f';
        *(p + 1) = 0xc;
}

void demo1(void)
{
        tty_register_command("demo1", print_k);

        for ( ;; ) {

        }
}

void demo2(void)
{
        tty_register_command("demo2", print_f);

        for ( ;; ) {

        }
}


void create_task(void)
{
        char *tty_stack = rheap_malloc(512);
        char *demo1_stack = rheap_malloc(128);
        char *demo2_stack = rheap_malloc(128);

        proc_info_t proc_info = {
                .f_entry = tty_task,
                .stack = tty_stack,
                .stack_size = 512,
                .priviledge = 1,
        };
        create_process(&proc_info);  

        proc_info.f_entry = demo1;
        proc_info.stack = demo1_stack;
        proc_info.stack_size = 128;
        proc_info.priviledge = 3;
        create_process(&proc_info);  

        proc_info.f_entry = demo2;
        proc_info.stack = demo2_stack;
        proc_info.stack_size = 128;
        proc_info.priviledge = 3;
        create_process(&proc_info); 
}

