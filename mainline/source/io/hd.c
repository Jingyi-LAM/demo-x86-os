#include "hd.h"
#include "interrupt.h"
#include "io.h"
#include "ipc.h"
#include "memory.h"
#include "string.h"
#include "tty.h"

static uint8_t g_buf[SECTOR_SIZE * 4] = {0};
static int8_t g_zero_padding[SECTOR_SIZE] = {0};
static volatile uint8_t g_is_busy = 0;
static partition_info_t g_partition_info[NUM_MAX_DRIVER] = {0};

void user_tty_print(uint8_t *buf,
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


/******************************************************************
* Hard disk I/O Workspace
*******************************************************************/
static void hd_write_cmd(hd_cmd_t *ptr_cmd)
{
        while (io_in8(HD_REG_STATUS) & 0x80);

        io_out8(HD_REG_DEV_CTRL,   0);
        io_out8(HD_REG_DEVICE,     ptr_cmd->device);
        io_out8(HD_REG_FEATURES,   ptr_cmd->features);
        io_out8(HD_REG_NSECTOR,    ptr_cmd->count);
        io_out8(HD_REG_LBA_BOT,    ptr_cmd->lba_bot);
        io_out8(HD_REG_LBA_MID,    ptr_cmd->lba_mid);
        io_out8(HD_REG_LBA_TOP,    ptr_cmd->lba_top);
        io_out8(HD_REG_CMD,        ptr_cmd->command);
}

static void hd_handler(void)
{
        io_in8(HD_REG_STATUS);
        g_is_busy = 0;
}

static inline void hd_set_busy_state(void)
{
        g_is_busy = 1;
}

static inline void hd_wait_for_interrupt(void)
{
        while (g_is_busy);
}

static void hd_read_sector(uint32_t driver,
                           uint32_t sector,
                           uint8_t *buffer,
                           uint32_t length)
{
        uint8_t *current = buffer;
        int32_t cnt_left = length;
        hd_cmd_t cmd = {
                .features = 0,
                .count    = length / SECTOR_SIZE +
                            (length % SECTOR_SIZE == 0 ? 0 : 1),
                .lba_bot  = sector & 0xFF,
                .lba_mid  = (sector >>  8) & 0xFF,
                .lba_top  = (sector >> 16) & 0xFF,
                .device   = MAKE_DEVICE_REG(1, driver, (sector >> 24) & 0xF),
                .command  = HD_CMD_READ,
        };

        hd_set_busy_state();
        hd_write_cmd(&cmd);
        while (cnt_left > 0) {
                hd_wait_for_interrupt();
                while ((io_in8(HD_REG_STATUS) & 0x08) != 0x08);
                if (cnt_left > SECTOR_SIZE) {
                        io_read(HD_REG_DATA, current, SECTOR_SIZE);
                        current += SECTOR_SIZE;
                        cnt_left -= SECTOR_SIZE;
                } else {
                        io_read(HD_REG_DATA, current, cnt_left);
                        io_read(HD_REG_DATA,
                                g_zero_padding,
                                SECTOR_SIZE - cnt_left);
                        break;
                }
        }
}

static void hd_write_sector(uint32_t driver,
                            uint32_t sector,
                            uint8_t *buffer,
                            uint32_t length)
{
        int32_t cnt_left = length;
        uint8_t *current = buffer;
        int i = 0;

        hd_cmd_t cmd = {
                .features = 0,
                .count    = length / SECTOR_SIZE +
                            (length % SECTOR_SIZE == 0 ? 0 : 1),
                .lba_bot  = sector & 0xFF,
                .lba_mid  = (sector >>  8) & 0xFF,
                .lba_top  = (sector >> 16) & 0xFF,
                .device   = MAKE_DEVICE_REG(1, driver, (sector >> 24) & 0xF),
                .command  = HD_CMD_WRITE,
        };

        hd_write_cmd(&cmd);
        while ((io_in8(HD_REG_STATUS) & 0x40) != 0x40);
        while (cnt_left > 0) {
                while ((io_in8(HD_REG_STATUS) & 0x08) != 0x08);
                hd_set_busy_state();
                if (cnt_left > SECTOR_SIZE) {
                        io_write(HD_REG_DATA, current, SECTOR_SIZE);
                        cnt_left -= SECTOR_SIZE;
                        current += SECTOR_SIZE;
                        hd_wait_for_interrupt();
                } else {
                        io_write(HD_REG_DATA, current, cnt_left);
                        memset(g_zero_padding, 0, SECTOR_SIZE - cnt_left);
                        io_write(HD_REG_DATA,
                                 g_zero_padding,
                                 SECTOR_SIZE - cnt_left);
                        hd_wait_for_interrupt();
                        break;
                }
        }
}

/******************************************************************
* Hard disk information Workspace
*******************************************************************/
static void hd_identify(int drv_no)
{
        hd_cmd_t cmd = {
                .device  = MAKE_DEVICE_REG(0, drv_no, 0),
                .command = HD_CMD_IDENTIFY,
        };

        hd_set_busy_state();
        hd_write_cmd(&cmd);
        hd_wait_for_interrupt();
        io_read(HD_REG_DATA, g_buf, SECTOR_SIZE);
}

static void hd_dump_info(uint8_t *ptr_hd_info)
{
        uint8_t serial_number[20] = {0};
        uint8_t model_string[40] = {0};
        int32_t i = 0;
        uint8_t tmp = 0;
        uint8_t buf[128] = {0};

        memcpy(serial_number, &ptr_hd_info[14], 20);
        memcpy(model_string, &ptr_hd_info[28], 39);

        for (i = 0; i < 20; i += 2) {
                tmp = serial_number[i];
                serial_number[i] = serial_number[i + 1];
                serial_number[i + 1] = tmp;
        }
        serial_number[15] = 0;

        for (i = 0; i < 40; i += 2) {
                tmp = model_string[i];
                model_string[i] = model_string[i + 1];
                model_string[i + 1] = tmp; 
        }
        model_string[i] = 0;

        vsprint(buf, "Serial Number: %s, Model: %s",
                &serial_number[6],
                &model_string[26]);
        user_tty_print(buf, 160, 128, -1);
}

static void hd_get_logical_info(uint8_t drv_no, uint8_t primary_no)
{
        pinfo_entry_t pinfo_tbl[NUM_LOGICAL_PER_EXTEND];
        int8_t sector_buf[SECTOR_SIZE] = {0};
        int32_t i = 0;
        int32_t index_first_logical = 0;
        int32_t current_sect = 0;
        int32_t logical_base_sect = 0;
        partition_info_t *ptr_hdinfo = &g_partition_info[drv_no];

        logical_base_sect = ptr_hdinfo->primary[primary_no % NUM_PRIMARY_PER_DRIVER].base_sect;
        current_sect = logical_base_sect;
        index_first_logical = (primary_no % NUM_PRIMARY_PER_DRIVER) * NUM_LOGICAL_PER_EXTEND;

        for (i = 0; i < NUM_LOGICAL_PER_EXTEND; i++) {
                hd_read_sector(drv_no, current_sect, sector_buf, SECTOR_SIZE);
                memcpy(pinfo_tbl,
                        sector_buf + PARTITION_TABLE_OFFSET,
                        sizeof(pinfo_entry_t) * 2);

                ptr_hdinfo->logical[index_first_logical + i].base_sect = current_sect +
                                                        pinfo_tbl[0].start_sector_count;
                ptr_hdinfo->logical[index_first_logical + i].size = pinfo_tbl[0].num_section;

                current_sect = logical_base_sect + pinfo_tbl[1].start_sector_count; 
                if (pinfo_tbl[1].sys_id == SYSID_NO_PART)
                        break;
        }
}

static void hd_get_partition_info(uint8_t drv_no)
{
        pinfo_entry_t pinfo_tbl[NUM_PRIMARY_PER_DRIVER];
        int8_t sector_buf[SECTOR_SIZE] = {0};
        int32_t i = 0;
        partition_info_t *ptr_hdinfo = &g_partition_info[drv_no];

        hd_read_sector(drv_no, 0, sector_buf, SECTOR_SIZE);
        memcpy(pinfo_tbl,
                sector_buf + PARTITION_TABLE_OFFSET,
                sizeof(pinfo_entry_t) * NUM_PRIMARY_PER_DRIVER);

        for (i = 0; i < NUM_PRIMARY_PER_DRIVER; i++) {
                if (pinfo_tbl[i].sys_id == SYSID_NO_PART)
                        continue;

                ptr_hdinfo->primary[i].base_sect = pinfo_tbl[i].start_sector_count;
                ptr_hdinfo->primary[i].size = pinfo_tbl[i].num_section;

                if (pinfo_tbl[i].sys_id == SYSID_EXT_PART)
                        hd_get_logical_info(drv_no, i);
        }
}

static void hd_dump_partition_info(partition_info_t *ptr_hdinfo)
{
        int32_t i = 0;
        uint8_t buf[128] = {0};
        static int32_t line = 2;

        for (i = 0; i < NUM_PRIMARY_PER_DRIVER; i++) {
                if (ptr_hdinfo->primary[i].size == 0)
                        continue;
                
                memset(buf, 0, 128);
                vsprint(buf, "Found primary partition: id=%d, base_sector=%d, size=%d",
                        i,
                        ptr_hdinfo->primary[i].base_sect,
                        ptr_hdinfo->primary[i].size);
                user_tty_print(buf, 160 * line, 128, -1);
                line += 1;
        }

        for (i = 0; i < NUM_LOGICAL_PER_DRIVER; i++) {
                if (ptr_hdinfo->logical[i].size == 0)
                        continue;

                memset(buf, 0, 128);
                vsprint(buf, "Found logical partition: id=%d, base_sector=%d, size=%d",
                        i,
                        ptr_hdinfo->logical[i].base_sect,
                        ptr_hdinfo->logical[i].size);
                user_tty_print(buf, 160 * line, 128, -1);
                line += 1;
        }
}

/******************************************************************
* Hard disk initialization and main function Workspace
*******************************************************************/
#define HD_DUMP_PARTITION_INFO

static void hd_init(void)
{
        register_interrupt_handler(46, hd_handler);
        enable_irq_master(2);
        enable_irq_slave(6);

#ifdef HD_DUMP_PARTITION_INFO
        //hd_identify(0);
        //hd_dump_info(g_buf);
        hd_identify(1);
        hd_dump_info(g_buf);

        //hd_get_partition_info(0);
        //hd_dump_partition_info(&g_partition_info[0]);
        hd_get_partition_info(1);
        hd_dump_partition_info(&g_partition_info[1]);
#endif
}

static void hd_test(void)
{
        memset(g_buf, 0, SECTOR_SIZE);
        vsprint(g_buf, "Test for hd write/read done"); 
        hd_write_sector(0, 1000, g_buf, SECTOR_SIZE);

        memset(g_buf, 0, SECTOR_SIZE);
        hd_read_sector(0, 1000, g_buf, SECTOR_SIZE);
        user_tty_print(g_buf, 160 * 24, SECTOR_SIZE, -1);
}

void hd_task(void)
{
        hd_msg_t msg = {0};
        uint64_t sector_index = 0;
        uint64_t sector_offset = 0;
        int32_t count = 0;

        hd_init();
        hd_test();

        for ( ;; ) {
                memset(&msg, 0, sizeof(msg));
                sync_receive(PID_ANY, (uint8_t *)&msg, sizeof(msg));
                switch (msg.operation) {
                case HD_OP_WRITE:
                        memset(g_buf, 0, SECTOR_SIZE);
                        // Handle the first sector
                        if (msg.addr % SECTOR_SIZE) {
                                sector_index = msg.addr / SECTOR_SIZE;
                                sector_offset = msg.addr % SECTOR_SIZE;
                                hd_read_sector(msg.drv_no,
                                               sector_index,
                                               g_buf,
                                               SECTOR_SIZE);

                                if (sector_offset + msg.length > SECTOR_SIZE)
                                        count = SECTOR_SIZE - sector_offset;
                                else
                                        count = msg.length;

                                memcpy(g_buf + sector_offset,
                                        msg.message_buffer,
                                        count);
                                hd_write_sector(msg.drv_no,
                                                sector_index,
                                                g_buf,
                                                SECTOR_SIZE);

                                msg.message_buffer += count;
                                msg.addr += count;
                                msg.length -= count;
                                if (msg.length <= 0)
                                        break;
                        }

                        while (msg.length / SECTOR_SIZE) {
                                memset(g_buf, 0, SECTOR_SIZE);
                                memcpy(g_buf, msg.message_buffer, SECTOR_SIZE);
                                sector_index = msg.addr / SECTOR_SIZE;
                                hd_write_sector(msg.drv_no,
                                                sector_index,
                                                g_buf,
                                                SECTOR_SIZE);

                                msg.message_buffer += SECTOR_SIZE;
                                msg.addr += SECTOR_SIZE;
                                msg.length -= SECTOR_SIZE;
                        }

                        // Handle the last sector
                        if (msg.length) {
                                sector_index = msg.addr / SECTOR_SIZE;
                                memset(g_buf, 0, SECTOR_SIZE);
                                hd_read_sector(msg.drv_no,
                                               sector_index,
                                               g_buf,
                                               SECTOR_SIZE);
                                memcpy(g_buf, msg.message_buffer, msg.length);
                                hd_write_sector(msg.drv_no,
                                                sector_index,
                                                g_buf,
                                                SECTOR_SIZE);
                        }
                        sync_send(msg.pid_src,
                                  HD_ACK_WRITE_DONE,
                                  strlen(HD_ACK_WRITE_DONE));
                        break;
                case HD_OP_READ:
                        while (msg.length > 0) {
                                memset(g_buf, 0, SECTOR_SIZE);
                                sector_index = msg.addr / SECTOR_SIZE;
                                sector_offset = msg.addr % SECTOR_SIZE;
                                hd_read_sector(msg.drv_no,
                                               sector_index,
                                               g_buf,
                                               SECTOR_SIZE);

                                if (sector_offset + msg.length > SECTOR_SIZE)
                                        count = SECTOR_SIZE - sector_offset;
                                else
                                        count = msg.length;
                                memcpy(msg.message_buffer,
                                        g_buf + sector_offset,
                                        count);

                                msg.message_buffer += count;
                                msg.addr += count;
                                msg.length -= count;
                        }
                        sync_send(msg.pid_src,
                                  HD_ACK_READ_DONE,
                                  strlen(HD_ACK_READ_DONE));
                        break;
                default:
                        sync_send(msg.pid_src,
                                  HD_ACK_UNSUPPORT_CMD,
                                  strlen(HD_ACK_UNSUPPORT_CMD));
                        break;
                }
        }
}



