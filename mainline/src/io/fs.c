#include "fs.h"

static uint8_t g_fs_buf[SECTOR_SIZE] = {0};

#define FS_SECTOR_BASE  45056
#define FS_SECTOR_SIZE  40960

#define FS_BOOTSEC_OFFSET       0
#define FS_SUPEERBLOCK_OFFSET   1
#define FS_INODEMAP_OFFSET      2
#define FS_SECTMAP_OFFSET       3

#define I_TYPE_MASK     0170000
#define I_REGULAR       0100000
#define I_BLOCK_SPECIAL 0060000
#define I_DIRECTORY     0040000
#define I_CHAR_SPECIAL  0020000
#define I_NAMED_PIPE	0010000

#define INODE_TYPE_CHAR 0020000
#define INODE_TYPE_DIR  0040000

// Boot image: start: 45056, size: 40960
void mkfs(void)
{
        super_block_t sb = {
                .magic = 0x3344,
                .num_inodes = SECTOR_SIZE * 8,
                .num_inode_sectors = sb.num_inodes * INODE_SIZE, 
                .num_sectors = 40960,
                .num_imap_sectors = 1,
                .num_smap_sectors = 40960,
                .first_data_sector_index = 1 + 1 + 
                                sb.num_imap_sectors +
                                sb.num_smap_sectors +
                                num_inode_sectors, 
                .root_inode_index = 1,
                .inode_size = INODE_SIZE,
                .isize_offset = GET_OFFSET_FROM_STRUCT(inode_t, size),
                .istart_sect_offset = GET_OFFSET_FROM_STRUCT(inode_t, start_sector),
                .dir_entry_size = sizeof(dir_entry_t),
                .dir_entry_inode_offset = GET_OFFSET_FROM_STRUCT(dir_entry_t, inode_index),
                .dir_entry_name_offset = GET_OFFSET_FROM_STRUCT(dir_entry_t, name),
        };
        uint32_t num_sectors = 2048 + 1;
        uint32_t i = 0, j = 0;
        inode_t *ptr_inode = 0;
        dir_entry_t *ptr_dir = 0;

        // Sector 1: Super Block
        mem_set(g_fs_buf, 0x90, SECTOR_SIZE);
        mem_cpy(g_fs_buf, &sb, SUPER_BLOCK_SIZE);
        hd_write_sector(0, FS_SECTOR_BASE + FS_SUPEERBLOCK_OFFSET, g_fs_buf, SECTOR_SIZE); 

        // Sector 2: inode map
        mem_set(g_fs_buf, 0, SECTOR_SIZE);
        g_fs_buf[0] = 0x7;
        hd_write_sector(0, FS_SECTOR_BASE + FS_INODEMAP_OFFSET, g_fs_buf, SECTOR_SIZE);

        // Sector 3~N: Sector map
        mem_set(g_fs_buf, 0, SECTOR_SIZE);
        for (i = 0; i < num_sectors / 8; i++)
                g_fs_buf[i] = 0xff;
        for (j = 0; j < num_sectors % 8; j++)
                g_fs_buf[i] |= (1 << j); 
        hd_write_sector(0, 2 + sb.num_inode_sectors + i, g_fs_buf, SECTOR_SIZE);

        mem_set(g_fs_buf, 0, SECTOR_SIZE);
        for (i = 1; i < sb.num_smap_sectors; i++)
                hd_write_sector(0, 2 + sb.num_inode_sectors + i, g_fs_buf, SECTOR_SIZE);

        // Sector N+1~M: inode array
        mem_set(g_fs_buf, 0, SECTOR_SIZE);
        ptr_inode = (inode_t *)g_fs_buf;
        ptr_inode->mode = INODE_TYPE_DIR;
        ptr_inode->size = sizeof(dir_entry_t) * 2;      // 2 for one tty and a '.'
        ptr_inode->start_sector = sb.first_data_sector_index;
        ptr_inode->num_sectors = num_sectors - 1;

        // inode of tty
        for (i = 0; i < 1; i++) {
                ptr_inode = (inode_t *)(g_fs_buf + INODE_SIZE * (i + 1));
                ptr_inode->mode = INODE_TYPE_CHAR;
                ptr_inode->size = 0;
                ptr_inode->start_sector = 0;    // Need to check
                ptr_inode->num_sectors = 0;     // Need to check
        }
        hd_write_sector(0, 2 + sb.num_inode_sectors + sb.num_smap_sectors, g_fs_buf, SECTOR_SIZE);
       
        // inode of '/'
        mem_set(g_fs_buf, 0, SECTOR_SIZE); 
        ptr_dir = (dir_entry_t *)g_fs_buf;
        ptr_dir->inode_index = 1;
        vsprint(ptr_dir->name, "tty0");
        hd_write_sector(0, sb.first_data_sector_index, g_fs_buf, SECTOR_SIZE);
}

