#ifndef __DESCRIPTOR_H_
#define __DESCRIPTOR_H_

#include "typedef.h"

#define RPL_SYS                 (0x0)
#define RPL_USER                (0x3)

#define SEGTYPE_LDT             (0x2)
#define SEGTYPE_386TSS          (0x9)
#define SEGTYPE_386INT          (0xe)

#define DESC_TYPE_SYS           (0x0)
#define DESC_TYPE_CODE_DATA     (0x1)

#define PRESENT_NOT_MEMORY      (0x0)
#define PRESENT_IN_MEMORY       (0x1)

#define DESCRIPTOR_SIZE         (0x8)
#define GATE_SIZE               (0x8)

typedef struct descriptor {
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
} desc_t;

typedef struct gate {
        uint16_t selector;
        uint32_t handler_entry_offset;
        uint8_t  present;
        uint8_t  dpl;
        uint8_t  segment_type;
        uint8_t  param_count;
} gate_t;

void write_desc(void *ptr_desc_entry, const desc_t *ptr_desc);
void write_gate(void *ptr_gate_entry, const gate_t *ptr_gate);
#endif
