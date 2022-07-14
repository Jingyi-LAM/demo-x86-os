#ifndef __KERNEL_H_
#define __KERNEL_H_

#include "descriptor.h"

void *get_available_desc(void);
int32_t get_desc_selector(void *ptr_desc);
void set_tss_esp0(uint32_t esp0);

#endif
