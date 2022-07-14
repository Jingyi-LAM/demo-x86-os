#ifndef __KHEAP_H_
#define __KHEAP_H_

#include "typedef.h"

#define KERNEL_HEAP_SIZE       (1024 * 1024 * 10)

typedef struct heap_chunk {
        struct   heap_chunk* prev;
        struct   heap_chunk* next;
        uint32_t chunk_size;
} heap_chunk_t;

int8_t *kmalloc(uint32_t alloc_size);
void kfree(void* pointer);
#endif
