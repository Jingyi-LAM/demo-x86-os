#include "kernel.h"

#define MAX_GLOBAL_DESCRIPTORS  (256)

extern uint8_t desc_gdt;
static volatile uint32_t available_desc_index = 2 * DESCRIPTOR_SIZE;

void *get_available_desc(void)
{
        if (available_desc_index >= DESCRIPTOR_SIZE * MAX_GLOBAL_DESCRIPTORS)
                return 0;

        available_desc_index += DESCRIPTOR_SIZE;
        return (void *)((uint8_t *)&desc_gdt + available_desc_index);
}

int32_t get_desc_selector(void *ptr_desc)
{
        if ((uint8_t *)ptr_desc < (uint8_t *)&desc_gdt ||
            (uint8_t *)ptr_desc >= (uint8_t *)&desc_gdt + DESCRIPTOR_SIZE * MAX_GLOBAL_DESCRIPTORS ||
            ptr_desc == 0)
                return -1;
        
        return ((uint32_t)ptr_desc - (uint32_t)((uint32_t *)&desc_gdt));
}

