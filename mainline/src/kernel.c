#include "kernel.h"

static hw_desc_t gdt[256] = {0};
u8 gdt_base_addr[6] = {0};
static volatile u32 current_gdt_index = 3;

void gdt_init(void)
{
        int len_gdt = *((u16 *)gdt_base_addr) + 1;
        int cnt = 0;
        
        for (cnt = 0; cnt < len_gdt; cnt++) {
        	*((u8 *)gdt + cnt) = *((u8 *)(*((u32 *)(&gdt_base_addr[2]))) + cnt);
        }
        
        *((u16 *)gdt_base_addr) = sizeof(hw_desc_t) * 256;
        *((u32 *)&gdt_base_addr[2]) = (u32)&gdt;        
}

hw_desc_t *get_available_desc(void)
{
        current_gdt_index += 1;
        return (hw_desc_t *)&gdt[current_gdt_index - 1];
}

u32 get_desc_selector(hw_desc_t *ptr_desc)
{
        if (!ptr_desc || ptr_desc < gdt)
                return -1;
        
        return ((u32)ptr_desc - (u32)gdt);
}


volatile hw_tss_t tss = {0};
u32 tss_init(u32 ss0, u32 esp0)
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

void set_tss_esp0(u32 esp0)
{
        tss.esp0 = esp0;
}


static hw_desc_t ldts[2] = {0};
u32 ldt_init(void)
{
        sw_desc_t ldt_desc = {
                .base_address           = (u32)ldts,
                .segment_limit          = 2 * sizeof(hw_desc_t),
                .segment_type           = 0x2,
                .descriptor_type        = 0,
                .dpl                    = 0,
                .present                = 1, 
        };
        hw_desc_t *ptr_gdt_desc = get_available_desc();
        write_desc(ptr_gdt_desc, &ldt_desc);

        *((u32 *)ldts + 0) = 0x0000ffff;
        *((u32 *)ldts + 1) = 0x00cffa00;
        *((u32 *)ldts + 2) = 0x0000ffff;
        *((u32 *)ldts + 3) = 0x00cff200;

        return get_desc_selector(ptr_gdt_desc);
}

