#include "descriptor.h"

void write_desc(struct hw_descriptor *ptr_hw_desc, const struct sw_descriptor *ptr_sw_desc)
{
        uint32_t p = 0;
        
        p = ((ptr_sw_desc->base_address & 0xffff) << 16) | (ptr_sw_desc->segment_limit & 0xffff);
        *((uint32_t *)ptr_hw_desc) = p;
        
        p = 0;
        p |= (ptr_sw_desc->base_address & 0xff000000) | ((ptr_sw_desc->base_address >> 16) & 0xff);
        p |= (ptr_sw_desc->segment_type & 0xf)             <<  8;
        p |= (ptr_sw_desc->descriptor_type & 0x1)          << 12;
        p |= (ptr_sw_desc->dpl & 0x3)                      << 13;
        p |= (ptr_sw_desc->present & 0x1)                  << 15;
        p |= (ptr_sw_desc->segment_limit & 0x000f0000);
        p |= (ptr_sw_desc->default_operation_size & 0x1)   << 22;
        p |= (ptr_sw_desc->granularity & 0x1)              << 23;
        
        *((uint32_t *)ptr_hw_desc + 1) = p;
}


void write_gate(struct hw_gate *ptr_hw_gate, const struct sw_gate *ptr_sw_gate)
{
        uint32_t p = 0;
        
        p |= ((ptr_sw_gate->selector & 0xffff) << 16) | (ptr_sw_gate->handler_entry_offset & 0xffff);
        *((uint32_t *)ptr_hw_gate) = p;
        
        p = 0;
        p |= ptr_sw_gate->handler_entry_offset & 0xffff0000;
        p |= (ptr_sw_gate->present & 0x1)       << 15;
        p |= (ptr_sw_gate->dpl & 0x3)           << 13;
        p |= (ptr_sw_gate->segment_type & 0xf)  << 8;
        p |= (ptr_sw_gate->param_count & 0xf);
        
        *((uint32_t *)ptr_hw_gate + 1) = p;
}
