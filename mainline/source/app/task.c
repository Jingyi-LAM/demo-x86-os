#include "fs.h"
#include "hd.h"
#include "heap.h"
#include "interrupt.h"
#include "ipc.h"
#include "keyboard.h"
#include "process.h"
#include "string.h"
#include "tty.h"
#include "typedef.h"

void print_k(void)
{
        int8_t buf[12] = {"Jingyi"};
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
        int8_t buf[12] = {"Hello"};
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
        int8_t *tty_stack = rheap_malloc(512);
        int8_t *hd_stack = rheap_malloc(4096);
        int8_t *fs_stack = rheap_malloc(2048);
        int8_t *demo1_stack = rheap_malloc(256);
        int8_t *demo2_stack = rheap_malloc(256);

        proc_info_t proc_info = {
                .f_entry = tty_task,
                .stack = tty_stack,
                .stack_size = 512,
                .priviledge = 1,
                .name = "tty",
        };
        create_process(&proc_info);

        proc_info.f_entry = hd_task;
        proc_info.stack = hd_stack;
        proc_info.stack_size = 4096;
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
        proc_info.stack_size = 256;
        proc_info.priviledge = 3;
        strcpy(proc_info.name, "demo1");
        create_process(&proc_info);

        proc_info.f_entry = demo2;
        proc_info.stack = demo2_stack;
        proc_info.stack_size = 256;
        proc_info.priviledge = 3;
        strcpy(proc_info.name, "demo2");
        create_process(&proc_info);

}

