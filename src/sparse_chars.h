#ifndef _INCLUDE_SPARSE_CHARS_H_
#define _INCLUDE_SPARSE_CHARS_H_

#include <stdint.h>

void sc_set(int16_t y, int16_t x, uint32_t unicode);
void sc_clr(int16_t y, int16_t x);
uint32_t sc_get(int16_t y, int16_t x);
void sc_debug(void);

#endif
