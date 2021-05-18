#include "io.h"

void io_out8(u16 port, u8 val)
{
        __asm__ __volatile__(
                "outb   %0,     %1      \n\t"
                :
                :"r"(val), "r"(port)
                :
        );	
}

u8 io_in8(u16 port)
{
        u8 ret = 0;
        
        __asm__ __volatile__(
                "inb    %1,     %%al    \n\t"
                "movb   %%al,   %0      \n\t"
                :"=r"(ret)
                :"r"(port)
                :"al"
        );	
}
