#ifndef __KERNEL_H_
#define __KERNEL_H_

#include "descriptor.h"

hw_desc_t *get_available_desc(void);
uint32_t get_desc_selector(hw_desc_t *ptr_desc);
void set_tss_esp0(uint32_t esp0);

#endif
