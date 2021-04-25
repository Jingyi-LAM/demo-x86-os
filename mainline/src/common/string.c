#include "string.h"

int str_cmp(unsigned char *base, unsigned char *obj, unsigned int limit)
{
        int i = 0;

        for (i = 0; i < limit; i++) {
                if (base[i] != obj[i])
                        return 0;

                if (base[i] == '\0')
                        return 1;
        }
        
        return 1;
}


int strcpy(char* dest, char* src)
{
        int i = 0;

        while (*src != '\0') {
                *dest = *src;
                dest += 1;
                src += 1;
                i += 1;
        }

        return i;
}

static int hex2string(char* dest, int data)
{
        int i = 0, j = 0;
        unsigned char tmp = 0;

        for (i = 0; i < sizeof(int) * 2; i++) {
                tmp = data >> (28 - i * 4) & 0xf;
                if (!tmp)
                        continue;

                *dest = tmp >= 10 ? tmp - 10 + 'A' : tmp + '0';
                dest += 1;
                j += 1;
        }

        return j;
}

static int dec2string(char* dest, int data)
{
        int input = data;
        int i = 0, j = 0;
        char tmp = 0;

        if (input < 0) {
                *dest = '-';
                dest++;
                input = -input;
        }

        while (input) {
                *(dest + i) = input % 10 + '0';
                input /= 10;
                i += 1;
        }

        for (j = 0; j < i / 2; j++) {
                tmp = dest[j];
                dest[j] = dest[i - j - 1];
                dest[i - j - 1] = tmp;
        }

        return data < 0 ? i + 1 : i;
}

int vsprint(char *dest, const char* fmt, ...)
{
        void *ptr_arg = (char*)&fmt + 4;
        char buf[512] = { 0 };
        int i = 0;
        int ret = 0;

        while (*fmt != '\0') {
                if (*fmt != '%') {
                        buf[i] = *fmt;
                        i += 1;
                        fmt += 1;
                        continue;
                }

                fmt += 1;
                switch (*fmt) {
                case 'x':
                        ret = hex2string(buf + i, *((int*)ptr_arg));
                        ptr_arg = (char*)ptr_arg + 4;
                        i += ret;
                        break;
                case 's':
                        ret = strcpy(buf + i, *((char **)ptr_arg));
                        ptr_arg = (char*)ptr_arg + 4;
                        i += ret;
                        break;
                case 'd':
                        ret = dec2string(buf + i, *((int *)ptr_arg));
                        ptr_arg = (char*)ptr_arg + 4;
                        i += ret;
                        break;
                default:
                        break;
                }
                fmt += 1;
        }
        
        buf[i] = '\0';
        strcpy(dest, buf);
        
        return i;
}

