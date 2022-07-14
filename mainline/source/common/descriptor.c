#include "descriptor.h"

void write_desc(void *ptr_desc_entry, const desc_t *ptr_desc)
{
        uint32_t p = 0;
        
        p = ((ptr_desc->base_address & 0xffff) << 16) | (ptr_desc->segment_limit & 0xffff);
        *((uint32_t *)ptr_desc_entry) = p;
        
        p = 0;
        p |= (ptr_desc->base_address & 0xff000000) | ((ptr_desc->base_address >> 16) & 0xff);
        p |= (ptr_desc->segment_type & 0xf)             <<  8;
        p |= (ptr_desc->descriptor_type & 0x1)          << 12;
        p |= (ptr_desc->dpl & 0x3)                      << 13;
        p |= (ptr_desc->present & 0x1)                  << 15;
        p |= (ptr_desc->segment_limit & 0x000f0000);
        p |= (ptr_desc->default_operation_size & 0x1)   << 22;
        p |= (ptr_desc->granularity & 0x1)              << 23;
        
        *((uint32_t *)ptr_desc_entry + 1) = p;
}

void write_gate(void *ptr_gate_entry, const gate_t *ptr_gate)
{
        uint32_t p = 0;
        
        p |= ((ptr_gate->selector & 0xffff) << 16) | (ptr_gate->handler_entry_offset & 0xffff);
        *((uint32_t *)ptr_gate_entry) = p;
        
        p = 0;
        p |= ptr_gate->handler_entry_offset & 0xffff0000;
        p |= (ptr_gate->present & 0x1)       << 15;
        p |= (ptr_gate->dpl & 0x3)           << 13;
        p |= (ptr_gate->segment_type & 0xf)  << 8;
        p |= (ptr_gate->param_count & 0xf);
        
        *((uint32_t *)ptr_gate_entry + 1) = p;
}
