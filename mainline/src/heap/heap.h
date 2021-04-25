#ifndef __HEAP_H_
#define __HEAP_H_

#include "process.h"
#include "common.h"
#define HEAP_TOTAL_SIZE         1024 * 1024 * 10

typedef struct heap_chunk {
        struct heap_chunk* prev;
        struct heap_chunk* next;
        unsigned int chunk_size;
} heap_chunk_t;

char* rheap_malloc(unsigned int alloc_size);
void rheap_free(void* pointer);
#endif
