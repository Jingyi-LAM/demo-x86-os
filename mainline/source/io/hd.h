#ifndef __HD_H_
#define __HD_H_

#include "typedef.h"

#define HD_ACK_WRITE_DONE       "HD WR Done"
#define HD_ACK_READ_DONE        "HD RD Done"
#define HD_ACK_UNSUPPORT_CMD    "Unsupport Message Type"

#define MAKE_PARTITION_INDEX(primary, logical)  \
        ((primary & 0xffff) << 16 | (logical & 0xffff))
#define PRIMARY(partition_index)                \
        ((partition_index >> 16) & 0xffff)
#define LOGICAL(partition_index)                \
        ((partition_index) & 0xffff)

/******************************************************************
* IPC
*******************************************************************/
typedef enum hd_operation_type {
        HD_OP_WRITE = 0xfa,
        HD_OP_READ = 0xaf,
        HD_OP_GET_PARTITION_INFO = 0xcc,
        HD_OP_WRITE_PARTITION = 0xab,
        HD_OP_READ_PARTITION = 0xba,
} hd_op_t;

typedef struct hd_message {
        hd_op_t         operation;
        uint8_t         *message_buffer;
        uint8_t         drv_no;
        uint32_t        primary_index;
        uint32_t        logical_index;
        uint64_t        addr;
        uint64_t        length;
        int32_t         pid_src;
} hd_msg_t;

void hd_task(void);
#endif

