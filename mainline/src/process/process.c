#include "process.h"

static process_t process_table[MAX_PROCESS] = {0};
process_t *ptr_proc_run = 0;

process_t *get_available_process_struct(void)
{
        int i = 0, j = 0;

        for (i = 0; i < MAX_PROCESS; i++) {
                if (!process_table[i].process_status) {
                        for (j = 0; j < sizeof(process_t); j++)
                                *((u8 *)&process_table[i] + j) = 0;
                        return &process_table[i];
                }
        }

        return 0;
}

int create_process(proc_info_t *ptr_proc_info)
{
        sw_desc_t ldt_desc = {0}; 
        hw_desc_t *ptr_gdt_desc = get_available_desc();
        process_t *ptr_proc_curr = get_available_process_struct();
        process_t *ptr_proc_prev = 0;

        if (weak_assert(ptr_proc_curr) || weak_assert(ptr_gdt_desc) || weak_assert(ptr_proc_info))
                return -1;

        if (ptr_proc_info->priviledge < 0 || ptr_proc_info->priviledge > 3)
                ptr_proc_info->priviledge = 3;

        ldt_desc.base_address = (u32)ptr_proc_curr->ldts;
        ldt_desc.segment_limit = 2 * sizeof(hw_desc_t);
        ldt_desc.segment_type = 0x2;
        ldt_desc.descriptor_type = 0;
        ldt_desc.dpl = 0;
        ldt_desc.present = 1;
        write_desc(ptr_gdt_desc, &ldt_desc);

        mem_cpy(&ptr_proc_curr->proc_info, ptr_proc_info, sizeof(proc_info_t));
        ptr_proc_curr->regs.gs = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.fs = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.es = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.ds = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.ss = 0x8 | 4 | ptr_proc_info->priviledge;
        ptr_proc_curr->regs.cs = 0x0 | 4 | ptr_proc_info->priviledge;

        ptr_proc_curr->regs.eip = (u32)ptr_proc_info->f_entry;
        ptr_proc_curr->regs.esp = (u32)ptr_proc_info->stack + ptr_proc_info->stack_size;
        ptr_proc_curr->regs.eflags = 0x1202;

        ptr_proc_curr->selector_ldt = get_desc_selector(ptr_gdt_desc);

        *((u32 *)ptr_proc_curr->ldts + 0) = 0x0000ffff;
        *((u32 *)ptr_proc_curr->ldts + 1) = 0x00cf9a00 | ((ptr_proc_info->priviledge & 0x3) << 13);
        *((u32 *)ptr_proc_curr->ldts + 2) = 0x0000ffff;
        *((u32 *)ptr_proc_curr->ldts + 3) = 0x00cf9200 | ((ptr_proc_info->priviledge & 0x3) << 13);

        switch (ptr_proc_info->priviledge) {
        case 0:
        case 1:
        case 2:
                ptr_proc_curr->schedule_info.time_slice = 30;
                ptr_proc_curr->schedule_info.default_ts = 30;
                break;
        case 3:
                ptr_proc_curr->schedule_info.time_slice = 10;
                ptr_proc_curr->schedule_info.default_ts = 10;
                break;
        default:
                ptr_proc_curr->schedule_info.time_slice = 10;
                ptr_proc_curr->schedule_info.default_ts = 10;
                break;
        } 


        if (process_table[0].process_status) {
                ptr_proc_prev = process_table;
                while (ptr_proc_prev->next != process_table)
                        ptr_proc_prev = ptr_proc_prev->next;
                ptr_proc_curr->next = ptr_proc_prev->next;
                ptr_proc_prev->next = ptr_proc_curr;
                ptr_proc_curr->process_status = PS_READY_TO_RUN;
        } else {
                ptr_proc_curr->next = process_table;
                ptr_proc_curr->process_status = PS_READY_TO_RUN;
                ptr_proc_run = ptr_proc_curr;
        }
        ptr_proc_curr->pid = (ptr_proc_curr - process_table);

        return 0;
}

s32 get_current_pid(void)
{
        if (ptr_proc_run != 0)
                return ptr_proc_run->pid;
        else
                return -1;
}

process_t *pid2proc(u32 pid)
{
        process_t *current = process_table;

        while (current) {
                if (pid == current->pid)
                        break;
                else
                        current = current->next;
        }

        return current;
}

// Only for single process
void enter_critical_area(void)
{
        int i = 0;

        for (i = 0; i < MAX_PROCESS; i++) {
                if (process_table[i].process_status == PS_NULL)
                        continue;
                if (ptr_proc_run != (process_table + i)) {
                        process_table[i].process_previous_status = process_table[i].process_status;
                        process_table[i].process_status = PS_PENDING;
                }
        }
}

void exit_critical_area(void)
{
        int i = 0;

        for (i = 0; i < MAX_PROCESS; i++) {
                if (process_table[i].process_status == PS_NULL)
                        continue;
                if (ptr_proc_run != (process_table + i)) {
                        process_table[i].process_status = process_table[i].process_previous_status;
                        process_table[i].process_previous_status = -1;
                }
        }        
}

void unblock(u32 pid)
{
        process_t *ptr_proc = pid2proc(pid);

        ptr_proc->process_status = PS_READY_TO_RUN;
}

void block(u32 pid)
{
        process_t *ptr_proc = pid2proc(pid);
        
        if (weak_assert(ptr_proc->next))
                return;
        

        ptr_proc->process_status = PS_PENDING;

        while (ptr_proc->next) {
                ptr_proc = ptr_proc->next;
                if (ptr_proc->process_status == PS_READY_TO_RUN)
                        break;
        }

        ptr_proc_run = ptr_proc;
        ptr_proc_run->schedule_info.time_slice = ptr_proc_run->schedule_info.default_ts;
        set_tss_esp0((u32)&ptr_proc_run->selector_ldt);
        __asm__ __volatile__(
                "movl   %0,     %%edi   \n\t"
                "lldt   72(%%edi)       \n\t"
                ::"a"(ptr_proc_run):"edi"
        );
}

void process_schedule(void)
{
        ptr_proc_run->schedule_info.time_slice -= 1;
        if (ptr_proc_run->schedule_info.time_slice < 0) {
                ptr_proc_run->schedule_info.time_slice = ptr_proc_run->schedule_info.default_ts;

                while (ptr_proc_run->next) {
                        ptr_proc_run = ptr_proc_run->next;
                        if (ptr_proc_run->process_status == PS_READY_TO_RUN)
                                break;
                }
                set_tss_esp0((u32)&ptr_proc_run->selector_ldt);

                __asm__ __volatile__(
                        "movl   %0,     %%edi   \n\t"
                        "lldt   72(%%edi)       \n\t"
                        ::"a"(ptr_proc_run):"edi"
                );
        }

}

void process_pre_init(void)
{
        int i = 0;
        
        for (i = 0; i < MAX_PROCESS * sizeof(process_t); i++)
                *((u8 *)process_table + i) = 0;

        register_interrupt_handler(0x20, process_schedule);
        enable_irq_master(0);
}


