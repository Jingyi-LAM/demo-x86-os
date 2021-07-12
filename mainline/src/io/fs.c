#include "fs.h"
#include "string.h"

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
                 "g"(color), "g"(SYSCALL_TTY_WRITE)
                :"eax", "ebx", "ecx", "edx", "edi"
        );
}

static void dump_sb_info(super_block_t *ptr_sb)
{
        mem_set(g_fs_buf, 0, 512);
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

#if 0
void mkfs(void)
{
        super_block_t sb = {0};
        uint32_t num_root_file_sectors = 2047 + 1;      // 1 for reserved
        uint32_t i = 0, j = 0;
        inode_t *ptr_inode = 0;
        dir_entry_t *ptr_dir = 0;

        // Sector 1: Super Block
        mem_set(&sb, 0, sizeof(super_block_t));
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

        mem_set(g_fs_buf, 0x90, SECTOR_SIZE);
        mem_cpy(g_fs_buf, &sb, SUPER_BLOCK_SIZE);
        hd_write_sector(0, FS_SECTOR_BASE + FS_SUPEERBLOCK_OFFSET, g_fs_buf, SECTOR_SIZE); 
        dump_sb_info(&sb);

        // Sector 2: inode map
        mem_set(g_fs_buf, 0, SECTOR_SIZE);
        g_fs_buf[0] = 0x7;      // Pre-defined 3 inodes: reserved, '/', tty
        hd_write_sector(0, FS_SECTOR_BASE + FS_INODEMAP_OFFSET, g_fs_buf, SECTOR_SIZE);

        // Sector 3~N: Sector map
        mem_set(g_fs_buf, 0, SECTOR_SIZE);
        for (i = 0; i < num_root_file_sectors / 8; i++)
                g_fs_buf[i] = 0xff;
        for (j = 0; j < num_root_file_sectors % 8; j++)
                g_fs_buf[i] |= (1 << j); 
        hd_write_sector(0, FS_SECTOR_BASE + FS_SECTMAP_OFFSET, g_fs_buf, SECTOR_SIZE);

        mem_set(g_fs_buf, 0, SECTOR_SIZE);
        for (i = 1; i < sb.num_smap_sectors; i++)       // Start with 1, because root file sector
                hd_write_sector(0, FS_SECTOR_BASE + FS_SECTMAP_OFFSET + i, g_fs_buf, SECTOR_SIZE);

        // Sector N+1~M: inode array
        mem_set(g_fs_buf, 0, SECTOR_SIZE);
        ptr_inode = (inode_t *)g_fs_buf;
        ptr_inode->mode = INODE_TYPE_DIR;
        ptr_inode->file_size = sizeof(dir_entry_t);     // 1 file: '.'
        ptr_inode->start_sector = sb.first_data_sector_index;
        ptr_inode->max_sectors = num_root_file_sectors;
        hd_write_sector(0, FS_SECTOR_BASE + FS_SECTMAP_OFFSET + sb.num_smap_sectors, g_fs_buf, SECTOR_SIZE);
       
        // content of '/' file
        mem_set(g_fs_buf, 0, SECTOR_SIZE); 
        ptr_dir = (dir_entry_t *)g_fs_buf;
        ptr_dir->inode_index = 1;
        vsprint(ptr_dir->name, ".");
        hd_write_sector(0, sb.first_data_sector_index, g_fs_buf, SECTOR_SIZE);
}
#endif

void fs_task(void)
{
        hd_msg_t msg = {0};
        uint64_t i = 0, j = 0;

        mem_set(g_fs_buf, 0x4c, SECTOR_SIZE * 30);
        msg.msg_type = HD_MSG_WRITE;
        msg.message_buffer = (uint8_t *)g_fs_buf;
        msg.drv_no = 1;
        msg.addr = 16;
        msg.length = SECTOR_SIZE * 30;
        msg.pid_src = get_current_pid();
        sync_send(1, (uint8_t *)&msg, sizeof(hd_msg_t));
        while (1) {
                sync_receive(PID_ANY, (uint8_t *)&msg, sizeof(hd_msg_t));  // Wait for task done
                if (str_cmp((uint8_t *)&msg, HD_ACK_WRITE_DONE, str_len(HD_ACK_WRITE_DONE)))
                        break;
        }

        mem_set(&msg, 0, sizeof(hd_msg_t));
        mem_set(g_fs_buf, 0, SECTOR_SIZE * 30);
        msg.msg_type = HD_MSG_READ;
        msg.message_buffer = (uint8_t *)g_fs_buf;
        msg.drv_no = 1;
        msg.addr = 16;
        msg.length = SECTOR_SIZE * 30;
        msg.pid_src = get_current_pid();
        sync_send(1, (uint8_t *)&msg, sizeof(hd_msg_t));
        mem_set(&msg, 0, sizeof(hd_msg_t));
        while (1) {
                sync_receive(PID_ANY, (uint8_t *)&msg, 10);  // Wait for task done
                if (str_cmp((uint8_t *)&msg, HD_ACK_READ_DONE, str_len(HD_ACK_READ_DONE)))
                        break;
        }

        mem_set((uint8_t *)&msg, 0, sizeof(hd_msg_t));
        for (i = 0; i < SECTOR_SIZE * 30; i++) {
                if (g_fs_buf[i] != 0x4c) {
                        vsprint((uint8_t *)&msg, "i = %d, val = %d", i, g_fs_buf[i]) ;
                        user_tty_print((uint8_t *)&msg, 160 * 22, 28, -1);
                        while (1);
                }
        }

        user_tty_print("OK Beng", 160, 16, -1);
        for ( ;; ) {
                *((uint8_t *)0xb8000) += 1;
        }
}
