#ifndef __DESCRIPTOR_H_
#define __DESCRIPTOR_H_

#include "typedef.h"

#define RPL_SYS                 0
#define RPL_USER                3

#define SEGTYPE_LDT             0x2
#define SEGTYPE_386TSS          0x9
#define SEGTYPE_386INT          0xe

#define DESC_TYPE_SYS           0
#define DESC_TYPE_CODE_DATA     1

#define PRESENT_NOT_MEMORY      0
#define PRESENT_IN_MEMORY       1


typedef struct sw_descriptor {
        uint32_t base_address;
        uint32_t segment_limit;
        uint8_t  segment_type;
        uint8_t  descriptor_type;
        uint8_t  dpl;
        uint8_t  present;
        uint8_t  avl;
        uint8_t  ia32e_mode;
        uint8_t  default_operation_size;
        uint8_t  granularity;
} sw_desc_t;

typedef struct sw_gate {
        uint16_t selector;
        uint32_t handler_entry_offset;
        uint8_t  present;
        uint8_t  dpl;
        uint8_t  segment_type;
        uint8_t  param_count;
} sw_gate_t;

typedef struct hw_descriptor {
        uint16_t limit_low;
        uint16_t seg_base_low;
        uint8_t  seg_base_mid;
        uint8_t  attr1;
        uint8_t  attr2;
        uint8_t  seg_base_high;
} hw_desc_t;

typedef struct hw_gate {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t  dcount;
        uint8_t  attr;
        uint16_t offset_high;
} hw_gate_t;

void write_desc(hw_desc_t *ptr_hw_desc, const sw_desc_t *ptr_sw_desc);
void write_gate(hw_gate_t *ptr_hw_gate, const sw_gate_t *ptr_sw_gate);
#endif
