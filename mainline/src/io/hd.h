#ifndef __HD_H_
#define __HD_H_

#include "common.h"
#include "fs.h"
#include "interrupt.h"
#include "io.h"
#include "ipc.h"
#include "string.h"
#include "tty.h"

#define MAKE_DEVICE_REG(mode, drv, lba_top4)    \
        (((mode) << 6) | ((drv) << 4) | (lba_top4 & 0xf) | 0xa0)


/******************************************************************
* Hard disk Registers
*******************************************************************/
#define HD_REG_DATA             0x1f0
#define HD_REG_FEATURES         0x1f1
#define HD_REG_ERROR            HD_REG_FEATURES
#define HD_REG_NSECTOR          0x1f2
#define HD_REG_LBA_BOT          0x1f3
#define HD_REG_LBA_MID          0x1f4
#define HD_REG_LBA_TOP          0x1f5
#define HD_REG_DEVICE           0x1f6
#define HD_REG_STATUS           0x1f7
#define HD_REG_CMD              HD_REG_STATUS
#define HD_REG_DEV_CTRL         0x3f6

/******************************************************************
* Hard disk Commands
*******************************************************************/
#define HD_CMD_IDENTIFY         0xec
#define HD_CMD_READ             0x20
#define HD_CMD_WRITE            0x30
#define HD_CMD_OPEN             0x11

/******************************************************************
* Hard disk Common Configures
*******************************************************************/
#define NUM_MAX_DRIVER          2
#define NUM_PRIMARY_PER_DRIVER  4
#define NUM_LOGICAL_PER_EXTEND  16
#define NUM_LOGICAL_PER_DRIVER  (NUM_PRIMARY_PER_DRIVER * NUM_LOGICAL_PER_EXTEND)

#define MAJOR_NO_DEV            0x0
#define MAJOR_FLOPPY            0x1
#define MAJOR_CDROM             0x2
#define MAJOR_HD                0x3
#define MAJOR_TTY               0x4
#define MAJOR_SCSI              0x5

#define SECTOR_SIZE             512
#define PARTITION_TABLE_OFFSET  0x1be

#define PART_TYPE_PRIMARY       0x83
#define PART_TYPE_EXTENDED      0x05

#define SYSID_NO_PART           0x00
#define SYSID_EXT_PART          0x05

#define HD_ACK_WRITE_DONE       "HD WR Done"
#define HD_ACK_READ_DONE        "HD RD Done"
#define HD_ACK_UNSUPPORT_CMD    "Unsupport Message Type"

typedef struct hd_cmd {
        uint8_t features;
        uint8_t count;
        uint8_t lba_bot;
        uint8_t lba_mid;
        uint8_t lba_top;
        uint8_t device;
        uint8_t command;
} hd_cmd_t;

typedef struct partition_info_entry {
        uint8_t boot_indicator;
        uint8_t start_head;
        uint8_t start_sector;
        uint8_t start_cylinder;
        uint8_t sys_id;
        uint8_t end_head;
        uint8_t end_sector;
        uint8_t end_cylinder;
        uint32_t  start_sector_count;
        uint32_t  num_section;
} pinfo_entry_t;

typedef struct partition_data_info {
        uint32_t base_sect;
        uint32_t size;
} pdata_info_t;

typedef struct partition_info {
        pdata_info_t primary[NUM_PRIMARY_PER_DRIVER];
        pdata_info_t logical[NUM_LOGICAL_PER_DRIVER];
} partition_info_t;


/******************************************************************
* IPC
*******************************************************************/
typedef enum hd_operation_type {
        HD_OP_WRITE = 0xfa,
        HD_OP_READ = 0xaf,
        HD_OP_GET_PARTITION_INFO = 0xcc,
} hd_op_t;

typedef struct hd_message {
        hd_op_t                 operation;
        uint8_t                 *message_buffer;
        uint8_t                 drv_no;
        uint64_t                addr;
        int64_t                 length;
        int32_t                 pid_src;
} hd_msg_t;

void hd_task(void);
#endif

