#ifndef _INCLUDE_BIGBITS_H_
#define _INCLUDE_BIGBITS_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX 256
#define limb uint64_t

typedef struct Bigbits {
   limb bits[(MAX / 8 + (sizeof(limb) - 1)) / sizeof(limb)];
} Bigbits;

static inline void bb_set(Bigbits *bb, unsigned int x) {
   if (x < MAX) {
      bb->bits[x / (sizeof(limb) * 8)] |= ((limb) 1) << (x % (sizeof(limb) * 8));
   }
}

static inline void bb_clr(Bigbits *bb, unsigned int x) {
   if (x < MAX) {
      bb->bits[x / (sizeof(limb) * 8)] &= ~(((limb) 1) << (x % (sizeof(limb) * 8)));
   }
}

static inline bool bb_tst(Bigbits *bb, unsigned int x) {
   if (x < MAX) {
      return (bb->bits[x / (sizeof(limb) * 8)] & ~(((limb) 1) << (x % (sizeof(limb) * 8)))) ? true : false;
   }
}

#undef limb
#endif
