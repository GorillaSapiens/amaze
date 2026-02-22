#ifndef _INCLUDE_SPARSE_CHARS_H_
#define _INCLUDE_SPARSE_CHARS_H_

#include <stdint.h>

void sc_set(uint16_t y, uint16_t x, uint32_t unicode);
void sc_clr(uint16_t y, uint16_t x);
uint32_t sc_get(uint16_t y, uint16_t x);

#endif
