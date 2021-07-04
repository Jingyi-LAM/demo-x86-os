#ifndef __FS_H_
#define __FS_H_

#include "common.h"

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

typedef inode {
        uint32_t mode;
        uint32_t size;
        uint32_t start_sector;
        uint32_t num_sectors;
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

#define GET_OFFSET_FROM_STRUCT(obj, member)     \
        (uint32)((uint8_t *)&(((obj *)0)->member) - (uint8_t *)0)
#endif
