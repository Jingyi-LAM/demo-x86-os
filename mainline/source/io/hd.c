#include "hd.h"
#include "interrupt.h"
#include "io.h"
#include "ipc.h"
#include "memory.h"
#include "string.h"
#include "tty.h"

#define MAKE_DEVICE_REG(mode, drv, lba_top4)    \
        (((mode) << 6) | ((drv) << 4) | (lba_top4 & 0xf) | 0xa0)

/******************************************************************
* Hard disk Registers
*******************************************************************/
#define HD_REG_DATA             (0x1f0)
#define HD_REG_FEATURES         (0x1f1)
#define HD_REG_ERROR            (HD_REG_FEATURES)
#define HD_REG_NSECTOR          (0x1f2)
#define HD_REG_LBA_BOT          (0x1f3)
#define HD_REG_LBA_MID          (0x1f4)
#define HD_REG_LBA_TOP          (0x1f5)
#define HD_REG_DEVICE           (0x1f6)
#define HD_REG_STATUS           (0x1f7)
#define HD_REG_CMD              (HD_REG_STATUS)
#define HD_REG_DEV_CTRL         (0x3f6)

/******************************************************************
* Hard disk Commands
*******************************************************************/
#define HD_CMD_IDENTIFY         (0xec)
#define HD_CMD_READ             (0x20)
#define HD_CMD_WRITE            (0x30)
#define HD_CMD_OPEN             (0x11)

/******************************************************************
* Hard disk Common Configures
*******************************************************************/
#define NUM_MAX_DRIVER          (2)
#define NUM_PRIMARY_PER_DRIVER  (4)
#define NUM_LOGICAL_PER_EXTEND  (16)
#define NUM_LOGICAL_PER_DRIVER  (NUM_PRIMARY_PER_DRIVER * NUM_LOGICAL_PER_EXTEND)

#define SECTOR_SIZE             (512)
#define PARTITION_TABLE_OFFSET  (0x1be)

#define SYSID_NO_PART           (0x00)
#define SYSID_EXT_PART          (0x05)

#define STATUS_ERR              (-1)
#define STATUS_OK               (0)

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
        uint32_t  num_sector;
} pinfo_entry_t;

typedef struct hd_device {
        struct hd_info {
                uint8_t serial_string[20];
                uint8_t model_string[40];
        } hd_info;

        struct partition_info {
                uint32_t base_sector;
                uint32_t num_sector;
                uint32_t type;
        } primary_info[NUM_PRIMARY_PER_DRIVER], logical_info[NUM_LOGICAL_PER_DRIVER];
} hd_dev_t;

static hd_dev_t hd_dev[NUM_MAX_DRIVER] = {0};
static volatile uint8_t g_is_busy = 0;

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
        uint8_t zero_padding[SECTOR_SIZE] = {0};
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
                        io_read(HD_REG_DATA, zero_padding, SECTOR_SIZE - cnt_left);
                        break;
                }
        }
}

static void hd_write_sector(uint32_t driver,
                            uint32_t sector,
                            uint8_t *buffer,
                            uint32_t length)
{
        uint8_t zero_padding[SECTOR_SIZE] = {0};
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
                        memset(zero_padding, 0, SECTOR_SIZE - cnt_left);
                        io_write(HD_REG_DATA, zero_padding, SECTOR_SIZE - cnt_left);
                        hd_wait_for_interrupt();
                        break;
                }
        }
}

static void hd_write(uint8_t drv_no, uint64_t addr, int64_t length, const uint8_t *buffer_in)
{
        uint8_t buf[SECTOR_SIZE] = {0};
        uint64_t sector_index = 0;
        uint64_t sector_offset = 0;
        int32_t count = 0;

        memset(buf, 0, SECTOR_SIZE);
        // Handle the first sector
        if (addr % SECTOR_SIZE) {
                sector_index = addr / SECTOR_SIZE;
                sector_offset = addr % SECTOR_SIZE;
                hd_read_sector(drv_no, sector_index, buf, SECTOR_SIZE);

                if (sector_offset + length > SECTOR_SIZE)
                        count = SECTOR_SIZE - sector_offset;
                else
                        count = length;

                memcpy(buf + sector_offset, buffer_in, count);
                hd_write_sector(drv_no, sector_index, buf, SECTOR_SIZE);

                buffer_in += count;
                addr += count;
                length -= count;
                if (length <= 0)
                        return;
        }

        while (length / SECTOR_SIZE) {
                memset(buf, 0, SECTOR_SIZE);
                memcpy(buf, buffer_in, SECTOR_SIZE);
                sector_index = addr / SECTOR_SIZE;
                hd_write_sector(drv_no, sector_index, buf, SECTOR_SIZE);
                buffer_in += SECTOR_SIZE;
                addr += SECTOR_SIZE;
                length -= SECTOR_SIZE;
        }

        // Handle the last sector
        if (length) {
                sector_index = addr / SECTOR_SIZE;
                memset(buf, 0, SECTOR_SIZE);
                hd_read_sector(drv_no, sector_index, buf, SECTOR_SIZE);
                memcpy(buf, buffer_in, length);
                hd_write_sector(drv_no, sector_index, buf, SECTOR_SIZE);
        }
}

static void hd_read(uint8_t drv_no, uint64_t addr, int64_t length, uint8_t *buffer_out)
{
        uint8_t buf[SECTOR_SIZE] = {0};
        uint64_t sector_index = 0;
        uint64_t sector_offset = 0;
        int32_t count = 0;

        while (length > 0) {
                memset(buf, 0, SECTOR_SIZE);
                sector_index = addr / SECTOR_SIZE;
                sector_offset = addr % SECTOR_SIZE;
                hd_read_sector(drv_no, sector_index, buf, SECTOR_SIZE);

                if (sector_offset + length > SECTOR_SIZE)
                        count = SECTOR_SIZE - sector_offset;
                else
                        count = length;
                memcpy(buffer_out, buf + sector_offset, count);
                buffer_out += count;
                addr += count;
                length -= count;
        }
}

static int partition_boundary_check(uint8_t drv_no, uint32_t part_index, uint64_t offset, uint64_t length)
{
        uint32_t p_index = PRIMARY(part_index);
        uint32_t l_index = LOGICAL(part_index);

        if (drv_no >= NUM_MAX_DRIVER)
                return STATUS_ERR;
        if (p_index >= NUM_PRIMARY_PER_DRIVER || l_index >= NUM_LOGICAL_PER_DRIVER)
                return STATUS_ERR;

        if (hd_dev[drv_no].primary_info[p_index].type == SYSID_EXT_PART) {
                if (offset + length >= hd_dev[drv_no].logical_info[l_index].num_sector * SECTOR_SIZE)
                        return STATUS_ERR;
        } else {
                if (offset + length >= hd_dev[drv_no].primary_info[p_index].num_sector * SECTOR_SIZE)
                        return STATUS_ERR;
        }

        return STATUS_OK;
}

static int hd_write_partition(uint8_t drv_no, uint32_t part_index, uint64_t addr, uint64_t length, uint8_t *buffer_in)
{
        uint32_t p_index = PRIMARY(part_index);
        uint32_t l_index = LOGICAL(part_index);
        uint64_t partition_base_addr = 0;
        uint64_t partition_total_length = 0;

        if (partition_boundary_check(drv_no, part_index, addr, length))
                return STATUS_ERR;

        if (hd_dev[drv_no].primary_info[p_index].type == SYSID_EXT_PART) {
                partition_base_addr = hd_dev[drv_no].logical_info[l_index].base_sector * SECTOR_SIZE;
                partition_total_length = hd_dev[drv_no].logical_info[l_index].num_sector * SECTOR_SIZE;
        } else {
                partition_base_addr = hd_dev[drv_no].primary_info[p_index].base_sector * SECTOR_SIZE;
                partition_total_length = hd_dev[drv_no].primary_info[p_index].num_sector * SECTOR_SIZE;
        }

        hd_write(drv_no, partition_base_addr + addr, length, buffer_in);

        return STATUS_OK;
}

static int hd_read_partition(uint8_t drv_no, uint32_t part_index, uint64_t addr, uint64_t length, uint8_t *buffer_out)
{
        uint32_t p_index = PRIMARY(part_index);
        uint32_t l_index = LOGICAL(part_index);
        uint64_t partition_base_addr = 0;
        uint64_t partition_total_length = 0;

        if (partition_boundary_check(drv_no, part_index, addr, length))
                return STATUS_ERR;

        if (hd_dev[drv_no].primary_info[p_index].type == SYSID_EXT_PART) {
                partition_base_addr = hd_dev[drv_no].logical_info[l_index].base_sector * SECTOR_SIZE;
                partition_total_length = hd_dev[drv_no].logical_info[l_index].num_sector * SECTOR_SIZE;
        } else {
                partition_base_addr = hd_dev[drv_no].primary_info[p_index].base_sector * SECTOR_SIZE;
                partition_total_length = hd_dev[drv_no].primary_info[p_index].num_sector * SECTOR_SIZE;
        }

        hd_read(drv_no, partition_base_addr + addr, length, buffer_out);

        return STATUS_OK;
}

/******************************************************************
* Hard disk information Workspace
*******************************************************************/
static void hd_get_info(uint32_t drv_no, hd_dev_t *dev)
{
        uint8_t buffer[SECTOR_SIZE] = {0};
        uint8_t tmp = 0;
        uint32_t i = 0, j = 0;
        uint32_t index_first_logical = 0;
        uint32_t current_sect = 0;
        uint32_t logical_base_sect = 0;
        pinfo_entry_t primary_tbl[NUM_PRIMARY_PER_DRIVER] = {0};
        pinfo_entry_t logical_tbl[NUM_LOGICAL_PER_EXTEND] = {0};

        /* IDENTIFY Information */
        hd_cmd_t cmd = {
                .device  = MAKE_DEVICE_REG(0, drv_no, 0),
                .command = HD_CMD_IDENTIFY,
        };

        hd_set_busy_state();
        hd_write_cmd(&cmd);
        hd_wait_for_interrupt();
        io_read(HD_REG_DATA, buffer, SECTOR_SIZE);

        memcpy(dev->hd_info.serial_string, &buffer[20], 10);
        memcpy(dev->hd_info.model_string, &buffer[54], 14);

        for (i = 0; i < 20; i += 2) {
                tmp = dev->hd_info.serial_string[i];
                dev->hd_info.serial_string[i] = dev->hd_info.serial_string[i + 1];
                dev->hd_info.serial_string[i + 1] = tmp;
        }

        for (i = 0; i < 40; i += 2) {
                tmp = dev->hd_info.model_string[i];
                dev->hd_info.model_string[i] = dev->hd_info.model_string[i + 1];
                dev->hd_info.model_string[i + 1] = tmp;
        }

        /* Partition Information*/
        memset(buffer, 0, SECTOR_SIZE);
        hd_read_sector(drv_no, 0, buffer, SECTOR_SIZE);
        memcpy(primary_tbl, buffer + PARTITION_TABLE_OFFSET, sizeof(pinfo_entry_t) * NUM_PRIMARY_PER_DRIVER);

        for (i = 0; i < NUM_PRIMARY_PER_DRIVER; i++) {
                if (primary_tbl[i].sys_id == SYSID_NO_PART)
                        continue;

                dev->primary_info[i].base_sector = primary_tbl[i].start_sector_count;
                dev->primary_info[i].num_sector = primary_tbl[i].num_sector;
                dev->primary_info[i].type = primary_tbl[i].sys_id;

                if (primary_tbl[i].sys_id == SYSID_EXT_PART) {
                        logical_base_sect = dev->primary_info[i % NUM_PRIMARY_PER_DRIVER].base_sector;
                        current_sect = logical_base_sect;
                        index_first_logical = (i % NUM_PRIMARY_PER_DRIVER) * NUM_LOGICAL_PER_EXTEND;

                        for (j = 0; j < NUM_LOGICAL_PER_EXTEND; j++) {
                                hd_read_sector(drv_no, current_sect, buffer, SECTOR_SIZE);
                                memcpy(logical_tbl,
                                       buffer + PARTITION_TABLE_OFFSET,
                                       sizeof(pinfo_entry_t) * 2);

                                dev->logical_info[index_first_logical + j].base_sector = current_sect +
                                                                        logical_tbl[0].start_sector_count;
                                dev->logical_info[index_first_logical + j].num_sector = logical_tbl[0].num_sector;
                                dev->logical_info[index_first_logical + j].type = logical_tbl[0].sys_id;

                                current_sect = logical_base_sect + logical_tbl[1].start_sector_count;
                                if (logical_tbl[1].sys_id == SYSID_NO_PART)
                                        break;
                        }
                }
        }
}

#ifdef DUMP_PARTITION_INFO
void dump_partitions(uint8_t drv_no)
{
        int32_t i = 0;
        uint8_t buf[128] = {0};

        if (drv_no >= NUM_MAX_DRIVER)
                return;

        for (i = 0; i < NUM_PRIMARY_PER_DRIVER; i++) {
                if (hd_dev[drv_no].primary_info[i].num_sector == 0)
                        continue;
                
                memset(buf, 0, 128);
                vsprint(buf, "* Primary Partition: id: %d, Offset: 0x%x, Length: 0x%x, Type: 0x%x",
                        i,
                        hd_dev[drv_no].primary_info[i].base_sector * SECTOR_SIZE,
                        hd_dev[drv_no].primary_info[i].num_sector * SECTOR_SIZE,
                        hd_dev[drv_no].primary_info[i].type);
                tty_display(-1, 128, buf, -1);
                tty_newline();
        }

        for (i = 0; i < NUM_LOGICAL_PER_DRIVER; i++) {
                if (hd_dev[drv_no].logical_info[i].num_sector == 0)
                        continue;

                memset(buf, 0, 128);
                vsprint(buf, "* Logical Partition: id: %d, Offset: 0x%x, Length: 0x%x, Type: 0x%x",
                        i,
                        hd_dev[drv_no].logical_info[i].base_sector * SECTOR_SIZE,
                        hd_dev[drv_no].logical_info[i].num_sector * SECTOR_SIZE,
                        hd_dev[drv_no].logical_info[i].type);
                tty_display(-1, 128, buf, -1);
                tty_newline();
        }
}
#endif

/******************************************************************
* Hard disk initialization and main function Workspace
*******************************************************************/
static void hd_init(void)
{
        register_interrupt_handler(46, hd_handler);
        enable_irq_master(2);
        enable_irq_slave(6);

        hd_get_info(0, &hd_dev[0]);
        hd_get_info(1, &hd_dev[1]);
}

static int hd_test(void)
{
        int i = 0;
        uint8_t buf[SECTOR_SIZE] = {0};
        char disp_buf[SECTOR_SIZE] = {0};

        memset(buf, 0x2f, SECTOR_SIZE);
        hd_write_sector(0, 1000, buf, SECTOR_SIZE);
        memset(buf, 0, SECTOR_SIZE);
        hd_read_sector(0, 1000, buf, SECTOR_SIZE);
        for (i = 0; i < SECTOR_SIZE; i++) {
                if (buf[i] != 0x2f)
                        return STATUS_ERR;
        }

        memset(buf, 0x3b, SECTOR_SIZE);
        hd_write(0, 10000, SECTOR_SIZE, buf);
        memset(buf, 0, SECTOR_SIZE);
        hd_read(0, 10000, SECTOR_SIZE, buf);
        for (i = 0; i < SECTOR_SIZE; i++) {
                if (buf[i] != 0x3b)
                        return STATUS_ERR;
        }

        memset(buf, 0xcc, SECTOR_SIZE);
        if (hd_write_partition(0, MAKE_PARTITION_INDEX(0, 0), 0, SECTOR_SIZE, buf)) {
                tty_display(-1, 128, "hd_write_partition failed", -1);
                tty_newline();
        }
        memset(buf, 0, SECTOR_SIZE);
        if (hd_read_partition(0, MAKE_PARTITION_INDEX(0, 0), 0, SECTOR_SIZE, buf)) {
                tty_display(-1, 128, "hd_read_partition failed", -1);
                tty_newline();
        }
        for (i = 0; i < SECTOR_SIZE; i++) {
                if (buf[i] != 0xcc)
                        return STATUS_ERR;
        }


        memset(buf, 0xdd, SECTOR_SIZE);
        hd_write_partition(0, MAKE_PARTITION_INDEX(0, 0), SECTOR_SIZE, SECTOR_SIZE, buf);
        memset(buf, 0, SECTOR_SIZE);
        hd_read_partition(0, MAKE_PARTITION_INDEX(0, 0), SECTOR_SIZE, SECTOR_SIZE, buf);
        for (i = 0; i < SECTOR_SIZE; i++) {
                if (buf[i] != 0xdd)
                        return STATUS_ERR;
        }

        if (partition_boundary_check(0, MAKE_PARTITION_INDEX(1, 16), 0, 0x9fffff)) {
                tty_display(-1, 128, "bound check failed", -1);
                tty_newline();
        } else {
                tty_display(-1, 128, "bound check pass", -1);
                tty_newline();
        }

        return STATUS_OK;
}


void hd_task(void)
{
        hd_msg_t msg = {0};

        hd_init();
        hd_test();

#ifdef DUMP_PARTITION_INFO
        dump_partitions(0);
        dump_partitions(1);
#endif

        for ( ;; ) {
                memset(&msg, 0, sizeof(msg));
                sync_receive(PID_ANY, (uint8_t *)&msg, sizeof(msg));
                switch (msg.operation) {
                case HD_OP_WRITE:
                        hd_write(msg.drv_no, msg.addr, msg.length, msg.message_buffer);
                        sync_send(msg.pid_src, HD_ACK_WRITE_DONE, strlen(HD_ACK_WRITE_DONE));
                        break;
                case HD_OP_READ:
                        hd_read(msg.drv_no, msg.addr, msg.length, msg.message_buffer);
                        sync_send(msg.pid_src, HD_ACK_READ_DONE, strlen(HD_ACK_READ_DONE));
                        break;
                case HD_OP_WRITE_PARTITION:
                        hd_write_partition(msg.drv_no, MAKE_PARTITION_INDEX(msg.primary_index, msg.logical_index),
                                                       msg.addr,
                                                       msg.length,
                                                       msg.message_buffer);
                        sync_send(msg.pid_src, HD_ACK_READ_DONE, strlen(HD_ACK_READ_DONE));
                        break;
                case HD_OP_READ_PARTITION:
                        hd_read_partition(msg.drv_no, MAKE_PARTITION_INDEX(msg.primary_index, msg.logical_index),
                                                      msg.addr,
                                                      msg.length,
                                                      msg.message_buffer);
                        sync_send(msg.pid_src, HD_ACK_READ_DONE, strlen(HD_ACK_READ_DONE));
                        break;
                default:
                        sync_send(msg.pid_src, HD_ACK_UNSUPPORT_CMD, strlen(HD_ACK_UNSUPPORT_CMD));
                        break;
                }
        }
}

