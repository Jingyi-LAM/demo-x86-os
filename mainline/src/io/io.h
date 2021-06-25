#ifndef __IO_H_
#define __IO_H_

#include "common.h"

void io_out8(uint16_t port, uint8_t val);
uint8_t io_in8(uint16_t port);
void io_read(uint16_t port, uint8_t *buf, uint32_t len);
void io_write(uint16_t port, uint8_t *buf, uint32_t len);
#endif
