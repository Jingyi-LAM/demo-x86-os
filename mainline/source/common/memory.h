#ifndef __MEMORY_H_
#define __MEMORY_H_

#include "typedef.h"

void memcpy(void *dest, const void *src, uint32_t size);
void memset(void *dest, uint8_t data, uint32_t size);

#endif
