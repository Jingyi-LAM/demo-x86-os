#include "memory.h"


void memcpy(void *dest, void *src, uint32_t size)
{
        uint32_t i = 0;

        for (i = 0; i < size; i++)
                *((uint8_t *)dest + i) = *((uint8_t *)src + i);
}

void memset(void *dest, uint8_t data, uint32_t size)
{
        uint32_t i = 0;

        for (i = 0; i < size; i++)
                *((uint8_t *)dest + i) = data;
}
