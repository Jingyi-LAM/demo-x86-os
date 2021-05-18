#ifndef __IO_H_
#define __IO_H_

#include "common.h"


void io_out8(u16 port, u8 val);
u8 io_in8(u16 port);
#endif
