#ifndef _INCLUDE_SPARSE_ARRAY_H_
#define _INCLUDE_SPARSE_ARRAY_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct SparseArray SparseArray;

SparseArray *sa_new(void);
void sa_set(SparseArray *sa, uint16_t y, uint16_t x);
void sa_clr(SparseArray *sa, uint16_t y, uint16_t x);
bool sa_get(SparseArray *sa, uint16_t y, uint16_t x);

#endif
