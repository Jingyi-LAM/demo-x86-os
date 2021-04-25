#include "debug.h"

static int logging_row = -1;
static int logging_column = 0;

static void debug_string(const char *str, unsigned int row, unsigned column)
{
        const char *p = str;

        while (*p != '\0') {
                if (column > 160) {
                        column = 0;
                        row += 1;
                }

                *((unsigned char *)0xb8000 + 160 * row + column) = *p;
                *((unsigned char *)0xb8000 + 160 * row + column + 1) = 0xc;

                column += 2;
                p += 1;
        }

        logging_row = row;
        logging_column = column;
}

static void debug_hex(unsigned int data, unsigned int row, unsigned column)
{
        unsigned int input = data;
        int i = 0;
        unsigned char tmp = 0;
        
        for (i = 0; i < 8; i++) {
                if (column > 160) {
                        column = 0;
                        row += 1;
                }

                tmp = input >> (28 - i * 4) & 0xf;
                *((unsigned char *)0xb8000 + 160 * row + column) = tmp >= 10 ? tmp - 10 + 'A' : tmp + '0';
                *((unsigned char *)0xb8000 + 160 * row + column + 1) = 0xc;
                column += 2;
        }

        logging_row = row;
        logging_column = column;
}

static void debug_dec(int data, unsigned int row, unsigned column)
{
        int input = data;
        char buf[10] = {0};
        int i = 0, minus_flag = 0;
        
        if (data < 0) {
                minus_flag = 1;
                input = -input;
        }

        while (input) {
                buf[i] = input % 10 + '0';
                input = input / 10;
                i += 1;
        }

        if (minus_flag)
                buf[i] = '-';

        for (i; i >= 0; i--) {
                 if (column > 160) {
                        column = 0;
                        row += 1;
                }
               
                *((unsigned char *)0xb8000 + 160 * row + column) = buf[i];
                *((unsigned char *)0xb8000 + 160 * row + column + 1) = 0xc;
                column += 2;
        }

        logging_row = row;
        logging_column = column;
}


void log_string(const char *str)
{
        debug_string(str, logging_row, logging_column);
}

void log_hex(int data)
{
        debug_hex(data, logging_row, logging_column);
}

void log_dec(int data)
{
        debug_dec(data, logging_row, logging_column);
}

void log_enter(void)
{
        logging_row += 1;
        logging_column = 0;
}

void log_reset(void)
{
        logging_row = -1;
        logging_column = 0;
}

void screen_show_char(char ch)
{
        *((unsigned char *)0xb8000 + 160 * logging_row + logging_column) = ch;
        *((unsigned char *)0xb8000 + 160 * logging_row + logging_column + 1) = 0xc;

        logging_column += 2;
        if (logging_column >= 160) {
                logging_column = 0;
                logging_row += 1;
        }
                
}

int weak_assert_fail(char *exp, char *file, char *base_file, int line)
{
        char buf[50] = {0};

        vsprint(buf, "weak assert fail: (%s) in %s, line %d", exp, file, line);
        log_string(buf);
        log_enter();

        return -1;
}

void assert_fail(char *exp, char *file, char *base_file, int line)
{
        char buf[512] = {0};

        vsprint(buf, "assert fail: (%s) in %s, line %d", exp, file, line);
        log_string(buf);
        log_enter();

        __asm__ __volatile__("ud2":::);
}
