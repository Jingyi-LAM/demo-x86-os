#include "io.h"

void io_out8(uint16_t port, uint8_t val)
{
        __asm__ __volatile__(
                "outb   %0,     %1      \n\t"
                :
                :"r"(val), "r"(port)
                :
        );	
}

uint8_t io_in8(uint16_t port)
{
        uint8_t ret = 0;
        
        __asm__ __volatile__(
                "inb    %1,     %%al    \n\t"
                "movb   %%al,   %0      \n\t"
                :"=r"(ret)
                :"r"(port)
                :"al"
        );	
}

void io_read(uint16_t port, uint8_t *buf, uint32_t len)
{
        __asm__ __volatile__(
                "movl   %0,     %%edx   \n\t"
                "movl   %1,     %%edi   \n\t"
                "movl   %2,     %%ecx   \n\t"
                "shrl   $1,     %%ecx   \n\t"
                "cld                    \n\t"
                "rep    insw"
                :
                :"g"(port), "g"(buf), "g"(len)
                :"ecx", "edx", "edi"
        );
}

void io_write(uint16_t port, uint8_t *buf, uint32_t len)
{
        __asm__ __volatile__(
                "movl   %0,     %%edx   \n\t"
                "movl   %1,     %%esi   \n\t"
                "movl   %2,     %%ecx   \n\t"
                "shrl   $1,     %%ecx   \n\t"
                "cld                    \n\t"
                "rep    outsw"
                :
                :"g"(port), "g"(buf), "g"(len)
                :"ecx", "edx", "esi"
        );
}
