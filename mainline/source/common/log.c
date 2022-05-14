#include "hd.h"
#include "ipc.h"
#include "log.h"
#include "memory.h"
#include "process.h"
#include "string.h"

#define LOG_BUFFER_SIZE         (0xA000)
#define LOG_PARTITION_DEV       (0)
#define LOG_PARTITION_PRIMARY   (1)
#define LOG_PARTITION_LOGICAL   (17)

static uint32_t current_log_pos = 0;

void log_buffer_init(void)
{
        uint64_t i = 0;
        uint8_t buf[512] = {0};
        hd_msg_t receive_buf = {0};
        hd_msg_t msg = {
                .operation = HD_OP_WRITE_PARTITION,
                .message_buffer = buf,
                .drv_no = LOG_PARTITION_DEV,
                .primary_index = LOG_PARTITION_PRIMARY,
                .logical_index = LOG_PARTITION_LOGICAL,
                .addr = 0,
                .length = 512,
                .pid_src = get_current_pid(),
        };

        memset(buf, 0, 512);
        for (i = 0; i < LOG_BUFFER_SIZE; i += 512) {
                msg.addr = i;
                hd_send_message(&msg);
        }
        current_log_pos = 0;
}

void add_log(const char *fmt, ...)
{
        void *ptr_arg = (char *)&fmt + 4;
        char buf[512] = { 0 };
        int i = 0;
        int j = 0;
        int ret = 0;
        int flag64 = 0;
        hd_msg_t msg = {
                .operation = HD_OP_WRITE_PARTITION,
                .message_buffer = buf,
                .drv_no = LOG_PARTITION_DEV,
                .primary_index = LOG_PARTITION_PRIMARY,
                .logical_index = LOG_PARTITION_LOGICAL,
                .addr = 0,
                .length = 0,
                .pid_src = get_current_pid(),
        };

        while (*fmt != '\0') {
                if (*fmt != '%') {
                        buf[i] = *fmt;
                        i += 1;
                        fmt += 1;
                        continue;
                }

                fmt += 1;
                if (*fmt == 'l') {      // Support %lx only now
                        flag64 = 1;
                        fmt += 1;
                }

                switch (*fmt) {
                case 'x':
                        if (flag64) {
                                ret = hex2string(buf + i, *((int *)ptr_arg + 1), 0);
                                i += ret;
                                ret = hex2string(buf + i, *((int *)ptr_arg), 1);
                                i += ret;
                                ptr_arg = (char *)ptr_arg + 8;
                        } else {
                                ret = hex2string(buf + i, *((int*)ptr_arg), 0);
                                ptr_arg = (char *)ptr_arg + 4;
                                i += ret;
                        }
                        break;
                case 's':
                        ret = strcpy(buf + i, *((char **)ptr_arg));
                        ptr_arg = (char *)ptr_arg + 4;
                        i += ret;
                        break;
                case 'd':
                        ret = dec2string(buf + i, *((int *)ptr_arg));
                        ptr_arg = (char *)ptr_arg + 4;
                        i += ret;
                        break;
                default:
                        break;
                }
                fmt += 1;
        }
        buf[i] = '\0';

        msg.addr = current_log_pos;
        msg.length = strlen(buf);
        hd_send_message(&msg);

        current_log_pos += msg.length;
        if (current_log_pos >= LOG_BUFFER_SIZE)
                current_log_pos = 0;
}
