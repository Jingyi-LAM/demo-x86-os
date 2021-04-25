#ifndef __IPC_H_
#define __IPC_H_

#include "process.h"
#include "common.h"

#define MAX_MCB_COUNT                   128

enum message_control_block_status {
        MCB_AVAILABLE   = 0,
        MCB_BUSY,
        MCB_SENDING,
        MCB_RECEIVING,
};

enum message_type {
        MSG_TYPE_UNKNOWED = 0,
        MSG_TYPE_SEND,
        MSG_TYPE_RECEIVE,
};


typedef struct message_control_block {
        u8                              *user_msg;
        s32                             user_msg_size;
        s32                             receive_from;
        s32                             send_to;
        struct message_control_block    *receive_queue;
        struct message_control_block    *send_queue;
        s32                             binding_pid;
        s8                              mcb_status;
} mcb_t;


void ipc_init(void);
void sync_send(u32 pid_target, u8 *buffer, u32 size);
void sync_receive(u32 pid_src, u8 *buffer, u32 size);
#endif
