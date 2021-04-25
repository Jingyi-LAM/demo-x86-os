#include "heap.h"

static char g_heap[HEAP_TOTAL_SIZE];
static heap_chunk_t chunk_start, chunk_end;
static unsigned int available_bytes = 0;

static void rheap_init(void)
{
        chunk_start.next = (heap_chunk_t *)g_heap;
        chunk_start.chunk_size = 0;
        chunk_start.prev = 0;

        chunk_end.prev = (heap_chunk_t*)g_heap;
        chunk_end.next = 0;
        chunk_end.chunk_size = HEAP_TOTAL_SIZE;

        ((heap_chunk_t*)g_heap)->prev = &chunk_start;
        ((heap_chunk_t*)g_heap)->next = &chunk_end;
        ((heap_chunk_t*)g_heap)->chunk_size = HEAP_TOTAL_SIZE;

        available_bytes = HEAP_TOTAL_SIZE;
}

char* rheap_malloc(unsigned int alloc_size)
{
        static char is_heap_init = 0;
        heap_chunk_t* ptr_chunk = 0;
        heap_chunk_t* ptr_new_chunk = 0;
        unsigned char* return_address = 0;
        unsigned int requset_chunk_size = sizeof(heap_chunk_t) + alloc_size;

        enter_critical_area();
        if (!is_heap_init) {
                rheap_init();
                is_heap_init = 1;
        }

        if (requset_chunk_size < alloc_size)         // overflow
                goto ALLOC_FAIL;
        if (requset_chunk_size > available_bytes)
                goto ALLOC_FAIL;
        if (!requset_chunk_size)
                goto ALLOC_FAIL;

        ptr_chunk = &chunk_start;
        do {
                ptr_chunk = ptr_chunk->next;
        } while (ptr_chunk->next && ptr_chunk->chunk_size < requset_chunk_size);

        if (ptr_chunk == &chunk_end)
                goto ALLOC_FAIL;

        return_address = (unsigned char*)ptr_chunk + sizeof(heap_chunk_t);
        mem_set(return_address, 0, alloc_size);
        ptr_chunk->prev->next = ptr_chunk->next;
        ptr_chunk->next->prev = ptr_chunk->prev;

        if (ptr_chunk->chunk_size - requset_chunk_size >= sizeof(heap_chunk_t)) {
                ptr_new_chunk = (heap_chunk_t*)((unsigned char*)ptr_chunk + requset_chunk_size);
                ptr_new_chunk->chunk_size = ptr_chunk->chunk_size - requset_chunk_size;


                ptr_chunk = &chunk_start;
                while (ptr_new_chunk->chunk_size > ptr_chunk->next->chunk_size) {
                        ptr_chunk = ptr_chunk->next;
                }

                ptr_new_chunk->prev = ptr_chunk;
                ptr_new_chunk->next = ptr_chunk->next;
                ptr_chunk->next->prev = ptr_new_chunk;
                ptr_chunk->next = ptr_new_chunk;
        }
        available_bytes -= requset_chunk_size;
        exit_critical_area();
        return (char *)return_address;

ALLOC_FAIL:
        exit_critical_area();
        return 0;
}

void rheap_free(void* pointer)
{
        heap_chunk_t* ptr_chunk = 0;
        heap_chunk_t* ptr_free_chunk = 0;
        
        if (!pointer)
                return;

        enter_critical_area();
        ptr_free_chunk = (heap_chunk_t *)((unsigned char*)pointer - sizeof(heap_chunk_t));

        ptr_chunk = &chunk_start;
        while (ptr_free_chunk->chunk_size > ptr_chunk->next->chunk_size) {
                ptr_chunk = ptr_chunk->next;
        }
        ptr_free_chunk->prev = ptr_chunk;
        ptr_free_chunk->next = ptr_chunk->next;
        ptr_chunk->next->prev = ptr_free_chunk;
        ptr_chunk->next = ptr_free_chunk;

        available_bytes += ptr_free_chunk->chunk_size;
        exit_critical_area();
}

