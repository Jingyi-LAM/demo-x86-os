#ifndef __COMMON_H_
#define __COMMON_H_

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;
typedef char                    s8;
typedef short                   s16;
typedef int                     s32;
typedef long long               s64;

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
        unsigned int  base_address;
        unsigned int  segment_limit;
        unsigned char segment_type;
        unsigned char descriptor_type;
        unsigned char dpl;
        unsigned char present;
        unsigned char avl;
        unsigned char ia32e_mode;
        unsigned char default_operation_size;
        unsigned char granularity;
} sw_desc_t;

typedef struct sw_gate {
        unsigned short  selector;
        unsigned int    handler_entry_offset;
        unsigned char   present;
        unsigned char   dpl;
        unsigned char   segment_type;
        unsigned char   param_count;
} sw_gate_t;

typedef struct hw_descriptor {
        unsigned short  limit_low;
        unsigned short  seg_base_low;
        unsigned char   seg_base_mid;
        unsigned char   attr1;
        unsigned char   attr2;
        unsigned char   seg_base_high;
} hw_desc_t;

typedef struct hw_gate {
        unsigned short  offset_low;
        unsigned short  selector;
        unsigned char   dcount;
        unsigned char   attr;
        unsigned short  offset_high;
} hw_gate_t;

void write_desc(hw_desc_t *ptr_hw_desc, const sw_desc_t *ptr_sw_desc);
void write_gate(hw_gate_t *ptr_hw_gate, const sw_gate_t *ptr_sw_gate);
void mem_cpy(void *dest, void *src, u32 size);
void mem_set(void *dest, u8 data, u32 size);
#endif
