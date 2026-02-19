#ifndef _INCLUDE_SPARSE_ARRAY_H_
#define _INCLUDE_SPARSE_ARRAY_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct SparseArray SparseArray;

SparseArray *sa_new(void);
void sa_set(SparseArray *sa, int16_t y, int16_t x);
void sa_clr(SparseArray *sa, int16_t y, int16_t x);
bool sa_get(SparseArray *sa, int16_t y, int16_t x);

#endif
