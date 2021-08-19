#include "debug.h"
#include "interrupt.h"
#include "kernel.h"
#include "process.h"

static hw_desc_t gdt[256] = {0};
uint8_t gdt_base_addr[6] = {0};
static volatile uint32_t current_gdt_index = 3;

void gdt_init(void)
{
        int len_gdt = *((uint16_t *)gdt_base_addr) + 1;
        int cnt = 0;
        
        for (cnt = 0; cnt < len_gdt; cnt++) {
        	*((uint8_t *)gdt + cnt) = *((uint8_t *)(*((uint32_t *)(&gdt_base_addr[2]))) + cnt);
        }
        
        *((uint16_t *)gdt_base_addr) = sizeof(hw_desc_t) * 256;
        *((uint32_t *)&gdt_base_addr[2]) = (uint32_t)&gdt;        
}

hw_desc_t *get_available_desc(void)
{
        current_gdt_index += 1;
        return (hw_desc_t *)&gdt[current_gdt_index - 1];
}

uint32_t get_desc_selector(hw_desc_t *ptr_desc)
{
        if (!ptr_desc || ptr_desc < gdt)
                return -1;
        
        return ((uint32_t)ptr_desc - (uint32_t)gdt);
}


volatile hw_tss_t tss = {0};
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

void set_tss_esp0(uint32_t esp0)
{
        tss.esp0 = esp0;
}


static hw_desc_t ldts[2] = {0};
uint32_t ldt_init(void)
{
        sw_desc_t ldt_desc = {
                .base_address           = (uint32_t)ldts,
                .segment_limit          = 2 * sizeof(hw_desc_t),
                .segment_type           = 0x2,
                .descriptor_type        = 0,
                .dpl                    = 0,
                .present                = 1, 
        };
        hw_desc_t *ptr_gdt_desc = get_available_desc();
        write_desc(ptr_gdt_desc, &ldt_desc);

        *((uint32_t *)ldts + 0) = 0x0000ffff;
        *((uint32_t *)ldts + 1) = 0x00cffa00;
        *((uint32_t *)ldts + 2) = 0x0000ffff;
        *((uint32_t *)ldts + 3) = 0x00cff200;

        return get_desc_selector(ptr_gdt_desc);
}

