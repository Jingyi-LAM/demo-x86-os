#include "fs.h"
#include "hd.h"
#include "interrupt.h"
#include "ipc.h"
#include "memory.h"
#include "process.h"
#include "string.h"
#include "tty.h"

#define SECTOR_SIZE     (512)

static uint8_t g_fs_buf[SECTOR_SIZE * 30] = {0};

static void user_tty_print(uint8_t *buf,
                           uint32_t position,
                           uint32_t len,
                           uint8_t color)
{
        __asm__ __volatile__(
                "movl %0,       %%eax   \n\t"
                "movl %1,       %%ebx   \n\t"
                "movl %2,       %%ecx   \n\t"
                "movl %3,       %%edx   \n\t"
                "movl %4,       %%edi   \n\t"
                "int  $100              \n\t"
                :
                :"g"(buf), "g"(position), "g"(len),
                 "g"(color), "g"(g_syscall_tty_write_index)
                :"eax", "ebx", "ecx", "edx", "edi"
        );
}

static void dump_sb_info(super_block_t *ptr_sb)
{
        memset(g_fs_buf, 0, 512);
        vsprint((uint8_t *)g_fs_buf, "\
magic = 0x%x, \
num_inodes = %d, \
num_sectors = %d, \
num_imap_sectors = %d, \
num_smap_sectors = %d, \
first_data_sector_index = %d, \
num_inode_sectors = %d, \
root_inode_index = %d, \
inode_size = %d, \
isize_offset = %d, \
istart_sect_offset = %d, \
dir_entry_size = %d, \
dir_entry_inode_offset = %d, \
dir_entry_name_offset = %d",
                ptr_sb->magic,
                ptr_sb->num_inodes,
                ptr_sb->num_sectors,
                ptr_sb->num_imap_sectors,
                ptr_sb->num_smap_sectors,
                ptr_sb->first_data_sector_index,
                ptr_sb->num_inode_sectors,
                ptr_sb->root_inode_index,
                ptr_sb->inode_size,
                ptr_sb->isize_offset,
                ptr_sb->istart_sect_offset,
                ptr_sb->dir_entry_size,
                ptr_sb->dir_entry_inode_offset,
                ptr_sb->dir_entry_name_offset);
        user_tty_print((uint8_t *)g_fs_buf, 160 * 5, 512, -1);
}

static void hd_write_read(hd_op_t type,
                          uint8_t drv_no,
                          uint8_t *buf,
                          uint64_t addr,
                          int64_t length)
{
        hd_msg_t msg = {
                .operation = type,
                .message_buffer = buf,
                .drv_no = drv_no,
                .addr = addr,
                .length = length,
                .pid_src = get_current_pid(),
        };

        sync_send(1, (uint8_t *)&msg, sizeof(hd_msg_t));
        for ( ;; ) {
                sync_receive(PID_ANY,
                             (uint8_t *)&msg,
                             sizeof(hd_msg_t));  // Wait for task done

                if (strcmp((uint8_t *)&msg,
                            HD_ACK_CMD_DONE,
                            strlen(HD_ACK_CMD_DONE)))
                        break;
        }

}

void mkfs(void)
{
        super_block_t sb = {0};
        uint32_t num_root_file_sectors = 2047 + 1;      // 1 for reserved
        uint32_t i = 0, j = 0;
        inode_t *ptr_inode = 0;
        dir_entry_t *ptr_dir = 0;

        // Sector 1: Super Block
        memset(&sb, 0, sizeof(super_block_t));
        sb.magic                   = 0x5346594A,
        sb.num_imap_sectors        = 1,
        sb.num_inodes              = sb.num_imap_sectors * SECTOR_SIZE * 8,
        sb.num_sectors             = FS_SECTOR_SIZE,
        sb.num_smap_sectors        = sb.num_sectors / SECTOR_SIZE / 8 + 1,
        sb.num_inode_sectors       = sb.num_inodes * INODE_SIZE / SECTOR_SIZE,
        sb.first_data_sector_index = FS_SECTOR_BASE + FS_SECTMAP_OFFSET +
                                     sb.num_smap_sectors + sb.num_inode_sectors,
        sb.root_inode_index        = 1,
        sb.inode_size              = INODE_SIZE,
        sb.isize_offset            = OF_OFFSET(inode_t, file_size),
        sb.istart_sect_offset      = OF_OFFSET(inode_t, start_sector),
        sb.dir_entry_size          = sizeof(dir_entry_t),
        sb.dir_entry_inode_offset  = OF_OFFSET(dir_entry_t, inode_index),
        sb.dir_entry_name_offset   = OF_OFFSET(dir_entry_t, name),

        //dump_sb_info(&sb);
        memset(g_fs_buf, 0x90, SECTOR_SIZE);
        memcpy(g_fs_buf, &sb, SUPER_BLOCK_SIZE);
        hd_write_read(HD_OP_WRITE,
                      1,
                      g_fs_buf,
                      (FS_SECTOR_BASE + FS_SUPEERBLOCK_OFFSET) * SECTOR_SIZE,
                      SECTOR_SIZE);


        // Sector 2: inode map
        memset(g_fs_buf, 0, SECTOR_SIZE);
        g_fs_buf[0] = 0x7;      // Pre-defined 3 inodes: reserved, '/', tty
        hd_write_read(HD_OP_WRITE,
                      1,
                      g_fs_buf,
                      (FS_SECTOR_BASE + FS_INODEMAP_OFFSET) * SECTOR_SIZE,
                      SECTOR_SIZE);


        // Sector 3~N: Sector map
        memset(g_fs_buf, 0, SECTOR_SIZE);
        for (i = 0; i < num_root_file_sectors / 8; i++)
                g_fs_buf[i] = 0xff;
        for (j = 0; j < num_root_file_sectors % 8; j++)
                g_fs_buf[i] |= (1 << j); 
        hd_write_read(HD_OP_WRITE,
                      1,
                      g_fs_buf,
                      (FS_SECTOR_BASE + FS_SECTMAP_OFFSET) * SECTOR_SIZE,
                      SECTOR_SIZE);

        memset(g_fs_buf, 0xcc, SECTOR_SIZE * 30);
        hd_write_read(HD_OP_WRITE,
                      1,
                      g_fs_buf,
                      (FS_SECTOR_BASE + FS_SECTMAP_OFFSET + 1) * SECTOR_SIZE,
                      (sb.num_smap_sectors - 1) * SECTOR_SIZE);


        // Sector N+1~M: inode array
        memset(g_fs_buf, 0, SECTOR_SIZE);
        ptr_inode = (inode_t *)g_fs_buf;
        ptr_inode->mode = INODE_TYPE_DIR;
        ptr_inode->file_size = sizeof(dir_entry_t);     // 1 file: '.'
        ptr_inode->start_sector = sb.first_data_sector_index;
        ptr_inode->max_sectors = num_root_file_sectors;
        hd_write_read(HD_OP_WRITE,
                      1,
                      g_fs_buf,
                      SECTOR_SIZE *
                      (FS_SECTOR_BASE + FS_SECTMAP_OFFSET + sb.num_smap_sectors),
                      SECTOR_SIZE);
       
        // content of '/' file
        memset(g_fs_buf, 0, SECTOR_SIZE); 
        ptr_dir = (dir_entry_t *)g_fs_buf;
        ptr_dir->inode_index = 1;
        vsprint(ptr_dir->name, ".");
        hd_write_read(HD_OP_WRITE,
                      1,
                      g_fs_buf,
                      sb.first_data_sector_index * SECTOR_SIZE,
                      SECTOR_SIZE);
}

void fs_task(void)
{
        mkfs();

        for ( ;; ) {
        }
}
