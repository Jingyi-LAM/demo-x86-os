#include "debug.h"
#include "interrupt.h"
#include "kernel.h"
#include "process.h"

#define MAX_GLOBAL_DESCRIPTORS  256

static hw_desc_t gdt[MAX_GLOBAL_DESCRIPTORS] = {0};
static hw_desc_t ldt[2] = {0};
volatile hw_tss_t tss = {0};
uint8_t gdt_meta[6] = {0};
static volatile uint32_t available_desc_index = 2;

hw_desc_t *get_available_desc(void)
{
        if (available_desc_index >= MAX_GLOBAL_DESCRIPTORS)
                return 0;

        available_desc_index += 1;
        return (hw_desc_t *)&gdt[available_desc_index];
}

int32_t get_desc_selector(hw_desc_t *ptr_desc)
{
        if (!ptr_desc || ptr_desc < gdt || ptr_desc >= gdt + MAX_GLOBAL_DESCRIPTORS)
                return -1;
        
        return ((uint32_t)ptr_desc - (uint32_t)gdt);
}

void set_tss_esp0(uint32_t esp0)
{
        tss.esp0 = esp0;
}

void reorganize_gdt(void)
{
        int len_gdt = 0, i = 0;

        // The following structures cannot be used.
        // Otherwise, it will cause problems due to memory alignment.
        //
        // struct gdt_meta {
        //     uint16_t size;
        //     uint32_t base_addr;
        // };
        len_gdt = *((uint16_t *)gdt_meta) + 1;
        for (i = 0; i < len_gdt; i++) {
                *((uint8_t *)gdt + i) = *((uint8_t *)(*((uint32_t *)(&gdt_meta[2]))) + i);
        }

        *((uint16_t *)gdt_meta) = sizeof(hw_desc_t) * MAX_GLOBAL_DESCRIPTORS;
        *((uint32_t *)&gdt_meta[2]) = (uint32_t)&gdt;
}

uint32_t tss_init(uint32_t ss0, uint32_t esp0)
{
        sw_desc_t tss_desc = {
                .base_address           = (unsigned int)&tss,
                .segment_limit          = sizeof(struct hw_tss),
                .segment_type           = SEGTYPE_386TSS,
                .descriptor_type        = DESC_TYPE_SYS,
                .dpl                    = 0,
                .present                = PRESENT_IN_MEMORY,
        };
        hw_desc_t *ptr_gdt_desc = get_available_desc();
        
        tss.ss0 = ss0;
        tss.esp0 = esp0;
        tss.iobase = sizeof(hw_tss_t);
        write_desc(ptr_gdt_desc, &tss_desc);

        return get_desc_selector(ptr_gdt_desc);
}

uint32_t ldt_init(void)
{
        sw_desc_t ldt_desc = {
                .base_address           = (uint32_t)ldt,
                .segment_limit          = 2 * sizeof(hw_desc_t),
                .segment_type           = 0x2,
                .descriptor_type        = 0,
                .dpl                    = 0,
                .present                = 1, 
        };
        hw_desc_t *ptr_gdt_desc = get_available_desc();
        write_desc(ptr_gdt_desc, &ldt_desc);

        *((uint32_t *)ldt + 0) = 0x0000ffff;
        *((uint32_t *)ldt + 1) = 0x00cffa00;
        *((uint32_t *)ldt + 2) = 0x0000ffff;
        *((uint32_t *)ldt + 3) = 0x00cff200;

        return get_desc_selector(ptr_gdt_desc);
}

