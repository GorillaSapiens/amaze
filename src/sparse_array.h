#ifndef _INCLUDE_SPARSE_ARRAY_H_
#define _INCLUDE_SPARSE_ARRAY_H_

#include <stdbool.h>
#include <stdint.h>

void sa_reset(void);
void sa_set(uint16_t y, uint16_t x);
void sa_clr(uint16_t y, uint16_t x);
bool sa_get(uint16_t y, uint16_t x);

#endif
