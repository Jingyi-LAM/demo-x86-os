#ifndef __IPC_H_
#define __IPC_H_

#include "typedef.h"

#define MAX_MCB_COUNT                   128
#define PID_ANY                         -0xff

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
        uint8_t                         *user_msg;
        int32_t                         user_msg_size;
        int32_t                         receive_from;
        int32_t                         send_to;
        struct message_control_block    *receive_queue;
        struct message_control_block    *send_queue;
        int32_t                         binding_pid;
        int8_t                          mcb_status;
} mcb_t;


void ipc_init(void);
void sync_send(uint32_t pid_target, uint8_t *buffer, uint32_t size);
void sync_receive(uint32_t pid_src, uint8_t *buffer, uint32_t size);
#endif
