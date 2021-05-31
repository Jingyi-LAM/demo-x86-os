#include "ipc.h"

static mcb_t mcb_table[MAX_MCB_COUNT] = {0};

mcb_t *get_available_mcb(void)
{
        int i = 0, j = 0;

        for (i = 0; i < MAX_MCB_COUNT; i++) {
                if (!mcb_table[i].mcb_status) {
                        for (j = 0; j < sizeof(mcb_t); j++)
                                *((uint8_t *)&mcb_table[i] + j) = 0;
                        return &mcb_table[i];
                }
        }

        return 0;
}

mcb_t *get_target_mcb(uint32_t pid)
{
        int i = 0;

        for (i = 0; i < MAX_MCB_COUNT; i++) {
                if (MCB_AVAILABLE != mcb_table[i].mcb_status &&
                    pid == mcb_table[i].binding_pid)
                        return &mcb_table[i];
        }
        
        return 0;
}

static int check_deadlock(uint32_t pid_from, uint32_t pid_to)
{
        mcb_t *ptr_to = get_target_mcb(pid_to);

        if (!ptr_to)
                return 0;

        while (ptr_to->send_to) {
                if (ptr_to->send_to == pid_from)
                        return -1;
                ptr_to = get_target_mcb(ptr_to->send_to);
        }

        return 0;
}


void sync_send(uint32_t pid_target, uint8_t *buffer, uint32_t size)
{
        mcb_t *ptr_mcb = get_available_mcb();

        if (weak_assert(ptr_mcb))
                return;

        ptr_mcb->user_msg = buffer;
        ptr_mcb->user_msg_size = size; 
        ptr_mcb->send_to = pid_target;
        ptr_mcb->binding_pid = get_current_pid();
        ptr_mcb->mcb_status = MCB_SENDING;

        __asm__ __volatile__(
                "movl %0,       %%eax   \n\t"
                "movl $1,       %%ebx   \n\t"
                "movl %1,       %%edi   \n\t"
                "int  $100              \n\t"
                :
                :"g"(ptr_mcb), "g"(SYSCALL_SYNC_IPC)
                :"eax", "ebx", "edi"
        );
}

void sync_receive(uint32_t pid_src, uint8_t *buffer, uint32_t size)
{
        uint32_t pid_curr = get_current_pid();
        mcb_t *ptr_mcb = 0;
        int i = 0;

        for (i = 0; i < MAX_MCB_COUNT; i++) {
                if (pid_curr == mcb_table[i].binding_pid &&
                    MCB_BUSY == mcb_table[i].mcb_status) {
                        ptr_mcb = &mcb_table[i];
                        break;
                }
        }

        if (!ptr_mcb)
                ptr_mcb = get_available_mcb();

        if (weak_assert(ptr_mcb))
                return;

        ptr_mcb->user_msg = buffer;
        ptr_mcb->user_msg_size = size; 
        ptr_mcb->receive_from = pid_src;
        ptr_mcb->binding_pid = get_current_pid();
        ptr_mcb->mcb_status = MCB_RECEIVING;

        __asm__ __volatile__(
                "movl %0,       %%eax   \n\t"
                "movl $2,       %%ebx   \n\t"
                "movl %1,       %%edi   \n\t"
                "int  $100              \n\t"
                :
                :"g"(ptr_mcb), "g"(SYSCALL_SYNC_IPC)
                :"eax", "ebx", "edi"
        );
}

void sys_sendrecv(mcb_t *ptr_mcb, uint32_t message_type)
{
        mcb_t *ptr_mcb_prev = 0;
        mcb_t *ptr_mcb_target = 0;

        switch (message_type) {
        case MSG_TYPE_SEND:
                if (weak_assert(!check_deadlock(ptr_mcb->binding_pid, ptr_mcb->send_to)))
                        return;
                if (weak_assert(ptr_mcb->send_to != ptr_mcb->binding_pid))
                        return;

                ptr_mcb_target = get_target_mcb(ptr_mcb->send_to);
                if (ptr_mcb_target) {
                        if (MCB_RECEIVING == ptr_mcb_target->mcb_status &&
                           (ptr_mcb->binding_pid == ptr_mcb_target->receive_from ||
                           (ptr_mcb_target->receive_from == PID_ANY &&
                            ptr_mcb_target->receive_queue == 0))) {
                                mem_cpy(ptr_mcb_target->user_msg,
                                        ptr_mcb->user_msg,
                                        ptr_mcb_target->user_msg_size);

                                ptr_mcb_target->receive_queue = ptr_mcb->send_queue;
                                if (ptr_mcb_target->receive_queue == 0)
                                        ptr_mcb_target->mcb_status = MCB_AVAILABLE;
                                else
                                        ptr_mcb_target->mcb_status = MCB_BUSY;
                                ptr_mcb->mcb_status = MCB_AVAILABLE;

                                unblock(ptr_mcb_target->binding_pid);
                                return;
                        }
                } else {
                        ptr_mcb_target = get_available_mcb(); 
                        ptr_mcb_target->binding_pid = ptr_mcb->send_to; 
                        ptr_mcb_target->mcb_status = MCB_BUSY;
                }

                if (ptr_mcb_target->receive_queue) {
                        ptr_mcb_prev = ptr_mcb_target->receive_queue;
                        while (ptr_mcb_prev->send_queue) {
                                ptr_mcb_prev = ptr_mcb_prev->send_queue;
                        }
                        ptr_mcb_prev->send_queue = ptr_mcb;
                } else {
                        ptr_mcb_target->receive_queue = ptr_mcb;
                }
                ptr_mcb->send_queue = 0;
                block(ptr_mcb->binding_pid);
                
                break;
        case MSG_TYPE_RECEIVE:
                if (ptr_mcb->receive_from == PID_ANY) {
                        if (ptr_mcb->receive_queue) {
                                ptr_mcb_target = ptr_mcb->receive_queue;
                                mem_cpy(ptr_mcb->user_msg,
                                        ptr_mcb_target->user_msg,
                                        ptr_mcb->user_msg_size);
                                ptr_mcb->receive_queue = ptr_mcb_target->send_queue;

                                ptr_mcb_target->mcb_status = MCB_AVAILABLE;
                                if (ptr_mcb->receive_queue == 0)
                                        ptr_mcb->mcb_status = MCB_AVAILABLE;
                                else
                                        ptr_mcb->mcb_status = MCB_BUSY;
                                unblock(ptr_mcb_target->binding_pid);
                        } else {
                                block(ptr_mcb->binding_pid);
                        }
                        break;
                }

                ptr_mcb_target = get_target_mcb(ptr_mcb->receive_from);
                if (ptr_mcb_target->mcb_status == MCB_SENDING &&
                    ptr_mcb_target->send_to == ptr_mcb->binding_pid) {
                        mem_cpy(ptr_mcb->user_msg,
                                ptr_mcb_target->user_msg,
                                ptr_mcb->user_msg_size);

                        if (ptr_mcb->receive_queue == ptr_mcb_target) {
                                ptr_mcb->receive_queue = ptr_mcb_target->send_queue;
                        } else {
                                ptr_mcb_prev = ptr_mcb->receive_queue;
                                while (ptr_mcb_prev->send_queue != ptr_mcb_target)
                                        ptr_mcb_prev = ptr_mcb_prev->send_queue;
                                ptr_mcb_prev->send_queue = ptr_mcb_target->send_queue;
                        }

                        ptr_mcb_target->mcb_status = MCB_AVAILABLE;
                        if (ptr_mcb->receive_queue == 0)
                                ptr_mcb->mcb_status = MCB_AVAILABLE;
                        else
                                ptr_mcb->mcb_status = MCB_BUSY;

                        unblock(ptr_mcb_target->binding_pid);
                } else {
                        block(ptr_mcb->binding_pid);
                }
                break;
        default:
                break;
        }
}


void ipc_init(void)
{
        mem_set((uint8_t *)mcb_table, 0, sizeof(mcb_t) * MAX_MCB_COUNT); 
        register_syscall_handler(SYSCALL_SYNC_IPC, sys_sendrecv);
}


