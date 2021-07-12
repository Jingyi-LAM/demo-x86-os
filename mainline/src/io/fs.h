#ifndef __FS_H_
#define __FS_H_

#include "common.h"
#include "hd.h"
#include "ipc.h"

#define FS_SECTOR_BASE          14336
#define FS_SECTOR_SIZE          20480

#define FS_BOOTSEC_OFFSET       0
#define FS_SUPEERBLOCK_OFFSET   1
#define FS_INODEMAP_OFFSET      2
#define FS_SECTMAP_OFFSET       3

#define I_TYPE_MASK             0170000
#define I_REGULAR               0100000
#define I_BLOCK_SPECIAL         0060000
#define I_DIRECTORY             0040000
#define I_CHAR_SPECIAL          0020000
#define I_NAMED_PIPE	        0010000

#define INODE_TYPE_CHAR         0020000
#define INODE_TYPE_DIR          0040000

typedef struct super_block {
        uint32_t magic;
        uint32_t num_inodes;
        uint32_t num_sectors;
        uint32_t num_imap_sectors;
        uint32_t num_smap_sectors;
        uint32_t first_data_sector_index;
        uint32_t num_inode_sectors;
        uint32_t root_inode_index;
        uint32_t inode_size;
        uint32_t isize_offset;
        uint32_t istart_sect_offset;
        uint32_t dir_entry_size;
        uint32_t dir_entry_inode_offset;
        uint32_t dir_entry_name_offset;

        int32_t super_block_device;
} super_block_t; 

#define SUPER_BLOCK_SIZE (sizeof(super_block_t) - sizeof(int32_t))

typedef struct inode {
        uint32_t mode;
        uint32_t file_size;  // in bytes
        uint32_t start_sector;
        uint32_t max_sectors;
        uint8_t  reserved[16];

        int32_t  device;
        int32_t  counter;
        int32_t  index;
} inode_t;

#define INODE_SIZE (sizeof(inode_t) - sizeof(int32_t) * 3)

typedef struct dir_entry {
        int32_t inode_index;
        uint8_t name[12];
} dir_entry_t;

#define OF_OFFSET(obj, member)     \
        (uint32_t)((uint8_t *)&(((obj *)0)->member) - (uint8_t *)0)

void mkfs(void);
void fs_task(void);
#endif
