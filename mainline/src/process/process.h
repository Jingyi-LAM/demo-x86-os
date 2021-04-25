#ifndef __PROCESS_H_
#define __PROCESS_H_

#include "common.h"
#include "debug.h"
#include "kernel.h"

#define MAX_PROCESS     32

enum proc_run_state {
        PS_NULL = 0,
        PS_READY_TO_RUN,
        PS_PENDING,
};

typedef struct stack_frame {
        u32     gs;
        u32     fs;
        u32     es;
        u32     ds;
        u32     edi;
        u32     esi;
        u32     ebp;
        u32     kernel_esp;
        u32     ebx;
        u32     edx;
        u32     ecx;
        u32     eax;
        u32     ret_addr;
        u32     eip;
        u32     cs;
        u32     eflags;
        u32     esp;
        u32     ss;
} stack_frame_t;

typedef struct hw_tss {
        u32     backlink;
        u32     esp0;
        u32     ss0;
        u32     esp1;
        u32     ss1;
        u32     esp2;
        u32     ss2;
        u32     cr3;
        u32     eip;
        u32     flags;
        u32     eax;
        u32     ecx;
        u32     edx;
        u32     ebx;
        u32     esp;
        u32     ebp;
        u32     esi;
        u32     edi;
        u32     es;
        u32     cs;
        u32     ss;
        u32     ds;
        u32     fs;
        u32     gs;
        u32     ldt;
        u16     trap;
        u16     iobase;
} hw_tss_t;

typedef struct process_info {
        void    (*f_entry)(void);
        u8      *stack;
        u32     stack_size;
        u8      priviledge;
} proc_info_t;

typedef struct schedule_information {
        int     time_slice; 
        int     default_ts;
} schedule_info_t;


typedef struct process {
        stack_frame_t   regs;
        u16             selector_ldt;
        hw_desc_t       ldts[2];
        proc_info_t     proc_info;
        schedule_info_t schedule_info;
        u8              process_previous_status;
        u8              process_status;
        u32             pid;
        struct process  *next;
} process_t;


process_t *get_available_process_struct(void);
int create_process(proc_info_t *ptr_proc_info);
s32 get_current_pid(void);
process_t *pid2proc(u32 pid);
void block(u32 pid);
void unblock(u32 pid);
void enter_critical_area(void);
void exit_critical_area(void);

#endif
