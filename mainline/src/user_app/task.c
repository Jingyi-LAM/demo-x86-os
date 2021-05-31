#include "task.h"
#include "heap.h"
#include "tty.h"

void print_k(void)
{
        char buf[12] = {"Jingyi"};
        uint32_t offset = 120;
        uint32_t length = 12;
        uint8_t color = TTY_BG_GRAY | TTY_FG_LIGHTCYAN;

        __asm__ __volatile__(
                "movl %0,       %%eax   \n\t"
                "movl %1,       %%ebx   \n\t"
                "movl %2,       %%ecx   \n\t"
                "movl %3,       %%edx   \n\t"
                "movl %4,       %%edi   \n\t"
                "int  $100              \n\t"
                :
                :"g"(buf), "g"(offset), "g"(length), "g"(color), "g"(SYSCALL_TTY_WRITE)
                :"eax", "ebx", "ecx", "edx", "edi"
        );
}

void print_f(void)
{
        char buf[12] = {"Hello"};
        int32_t offset = -1;
        uint32_t length = 12;
        uint8_t color = -1;

        __asm__ __volatile__(
                "movl %0,       %%eax   \n\t"
                "movl %1,       %%ebx   \n\t"
                "movl %2,       %%ecx   \n\t"
                "movl %3,       %%edx   \n\t"
                "movl %4,       %%edi   \n\t"
                "int  $100              \n\t"
                :
                :"g"(buf), "g"(offset), "g"(length), "g"(color), "g"(SYSCALL_TTY_WRITE)
                :"eax", "ebx", "ecx", "edx", "edi"
        );
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
        char *demo1_stack = rheap_malloc(256);
        char *demo2_stack = rheap_malloc(256);

        proc_info_t proc_info = {
                .f_entry = tty_task,
                .stack = tty_stack,
                .stack_size = 512,
                .priviledge = 1,
        };
        create_process(&proc_info);  

        proc_info.f_entry = demo1;
        proc_info.stack = demo1_stack;
        proc_info.stack_size = 256;
        proc_info.priviledge = 3;
        create_process(&proc_info);  

        proc_info.f_entry = demo2;
        proc_info.stack = demo2_stack;
        proc_info.stack_size = 256;
        proc_info.priviledge = 3;
        create_process(&proc_info); 
}

