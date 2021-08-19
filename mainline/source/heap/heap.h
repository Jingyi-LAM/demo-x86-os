#ifndef __HEAP_H_
#define __HEAP_H_

#include "typedef.h"

#define HEAP_TOTAL_SIZE         1024 * 1024 * 10

typedef struct heap_chunk {
        struct   heap_chunk* prev;
        struct   heap_chunk* next;
        uint32_t chunk_size;
} heap_chunk_t;

int8_t *rheap_malloc(uint32_t alloc_size);
void rheap_free(void* pointer);
#endif
