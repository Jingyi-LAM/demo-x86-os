#ifndef __PROCESS_H_
#define __PROCESS_H_

#include "common.h"

#define MAX_PROCESS     32
#define MAX_NAME_LEN    16

enum proc_run_state {
        PS_NULL = 0,
        PS_READY_TO_RUN,
        PS_PENDING,
};

typedef struct stack_frame {
        uint32_t        gs;
        uint32_t        fs;
        uint32_t        es;
        uint32_t        ds;
        uint32_t        edi;
        uint32_t        esi;
        uint32_t        ebp;
        uint32_t        kernel_esp;
        uint32_t        ebx;
        uint32_t        edx;
        uint32_t        ecx;
        uint32_t        eax;
        uint32_t        ret_addr;
        uint32_t        eip;
        uint32_t        cs;
        uint32_t        eflags;
        uint32_t        esp;
        uint32_t        ss;
} stack_frame_t;

typedef struct hw_tss {
        uint32_t        backlink;
        uint32_t        esp0;
        uint32_t        ss0;
        uint32_t        esp1;
        uint32_t        ss1;
        uint32_t        esp2;
        uint32_t        ss2;
        uint32_t        cr3;
        uint32_t        eip;
        uint32_t        flags;
        uint32_t        eax;
        uint32_t        ecx;
        uint32_t        edx;
        uint32_t        ebx;
        uint32_t        esp;
        uint32_t        ebp;
        uint32_t        esi;
        uint32_t        edi;
        uint32_t        es;
        uint32_t        cs;
        uint32_t        ss;
        uint32_t        ds;
        uint32_t        fs;
        uint32_t        gs;
        uint32_t        ldt;
        uint16_t        trap;
        uint16_t        iobase;
} hw_tss_t;

typedef struct process_info {
        void            (*f_entry)(void);
        uint8_t         *stack;
        uint8_t         name[16];
        uint32_t        stack_size;
        uint8_t         priviledge;
} proc_info_t;

typedef struct schedule_information {
        int32_t         time_slice;
        int32_t         default_ts;
} schedule_info_t;


typedef struct process {
        stack_frame_t   regs;
        uint16_t        selector_ldt;
        hw_desc_t       ldts[2];
        proc_info_t     proc_info;
        schedule_info_t schedule_info;
        uint8_t         process_previous_status;
        uint8_t         process_status;
        uint32_t        pid;
        uint8_t         name[16];
        struct process  *next;
} process_t;


void block(uint32_t pid);
void unblock(uint32_t pid);

void enter_critical_area(void);
void exit_critical_area(void);

process_t *get_available_process_struct(void);
process_t *pid2proc(uint32_t pid);
int32_t get_current_pid(void);
int32_t get_pid_by_name(uint8_t *name);

int32_t create_process(proc_info_t *ptr_proc_info);
#endif
