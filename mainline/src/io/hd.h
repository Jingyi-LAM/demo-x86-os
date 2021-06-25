#ifndef __HD_H_
#define __HD_H_

#include "io.h"
#include "common.h"
#include "string.h"
#include "interrupt.h"
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
#define NUM_MAX_DRIVER          1
#define NUM_PART_PER_DRIVE      4
#define NUM_SUB_PER_PART        16
#define NUM_SUB_PER_DRIVE       (NUM_PART_PER_DRIVE * NUM_SUB_PER_PART)
#define NUM_PRIM_PER_DRIVE      (NUM_PART_PER_DRIVE + 1)
#define NUM_MAX_PRIM            (NUM_MAX_DRIVER * NUM_PRIM_PER_DRIVE - 1)

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
        pdata_info_t primary[NUM_PRIM_PER_DRIVE];
        pdata_info_t logical[NUM_SUB_PER_DRIVE];
} partition_info_t;

void hd_task(void);
#endif

