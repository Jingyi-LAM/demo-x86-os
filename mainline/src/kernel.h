#ifndef __KERNEL_H_
#define __KERNEL_H_


#include "interrupt.h"
#include "common.h"
#include "process.h"
#include "debug.h"

hw_desc_t *get_available_desc(void);
u32 get_desc_selector(hw_desc_t *ptr_desc);
void set_tss_esp0(u32 esp0);

#endif
