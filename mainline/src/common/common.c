#include "common.h"


void write_desc(struct hw_descriptor *ptr_hw_desc, const struct sw_descriptor *ptr_sw_desc)
{
        unsigned int p = 0;
        
        p = ((ptr_sw_desc->base_address & 0xffff) << 16) | (ptr_sw_desc->segment_limit & 0xffff);
        *((unsigned int *)ptr_hw_desc) = p;
        
        p = 0;
        p |= (ptr_sw_desc->base_address & 0xff000000) | ((ptr_sw_desc->base_address >> 16) & 0xff);
        p |= (ptr_sw_desc->segment_type & 0xf)             <<  8;
        p |= (ptr_sw_desc->descriptor_type & 0x1)          << 12;
        p |= (ptr_sw_desc->dpl & 0x3)                      << 13;
        p |= (ptr_sw_desc->present & 0x1)                  << 15;
        p |= (ptr_sw_desc->segment_limit & 0x000f0000);
        p |= (ptr_sw_desc->default_operation_size & 0x1)   << 22;
        p |= (ptr_sw_desc->granularity & 0x1)              << 23;
        
        *((unsigned int *)ptr_hw_desc + 1) = p;
}


void write_gate(struct hw_gate *ptr_hw_gate, const struct sw_gate *ptr_sw_gate)
{
        unsigned int p = 0;
        
        p |= ((ptr_sw_gate->selector & 0xffff) << 16) | (ptr_sw_gate->handler_entry_offset & 0xffff);
        *((unsigned int *)ptr_hw_gate) = p;
        
        p = 0;
        p |= ptr_sw_gate->handler_entry_offset & 0xffff0000;
        p |= (ptr_sw_gate->present & 0x1)       << 15;
        p |= (ptr_sw_gate->dpl & 0x3)           << 13;
        p |= (ptr_sw_gate->segment_type & 0xf)  << 8;
        p |= (ptr_sw_gate->param_count & 0xf);
        
        *((unsigned int *)ptr_hw_gate + 1) = p;
}

void mem_cpy(void *dest, void *src, u32 size)
{
        u32 i = 0;

        for (i = 0; i < size; i++)
                *((u8 *)dest + i) = *((u8 *)src + i);
}

void mem_set(void *dest, u8 data, u32 size)
{
        u32 i = 0;

        for (i = 0; i < size; i++)
                *((u8 *)dest + i) = data;
}
