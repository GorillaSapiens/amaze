#include "sparse_array.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef uint64_t ChunkBase;

#define CHUNK_BITS (sizeof(ChunkBase) * 8)

typedef struct Chunk {
   ChunkBase base[CHUNK_BITS];
} Chunk;

#define EMPTY_CHUNK NULL
#define FULL_CHUNK ((Chunk *) -1)

struct SparseArray {
   Chunk *data[65536 / CHUNK_BITS][65536 / CHUNK_BITS];
};

#define BIN data[y / CHUNK_BITS][x / CHUNK_BITS]
#define BASE base[y % CHUNK_BITS]
#define BIT (((ChunkBase) 1) << (x % CHUNK_BITS))

SparseArray *sa_new(void) {
   return calloc(1, sizeof(SparseArray));
}

void sa_set(SparseArray *sa, uint16_t y, uint16_t x) {
   if (sa->BIN == FULL_CHUNK) {
      return; // already set
   }

   if (sa->BIN == EMPTY_CHUNK) {
      sa->BIN = calloc(1, sizeof(Chunk));
   }

   sa->BIN->BASE |= BIT;

   for (int i = 0; i < CHUNK_BITS; i++) {
      if (sa->BIN->base[i] != (ChunkBase) -1) {
         return;
      }
   }

   free(sa->BIN);
   sa->BIN = FULL_CHUNK;
}

void sa_clr(SparseArray *sa, uint16_t y, uint16_t x) {
   if (sa->BIN == EMPTY_CHUNK) {
      return; // already clear
   }

   if (sa->BIN == FULL_CHUNK) {
      sa->BIN = malloc(sizeof(Chunk));
      memset(sa->BIN, 0xFF, sizeof(Chunk));
   }

   sa->BIN->BASE &= ~BIT;

   for (int i = 0; i < CHUNK_BITS; i++) {
      if (sa->BIN->base[i] != 0) {
         return;
      }
   }

   free(sa->BIN);
   sa->BIN = EMPTY_CHUNK;
}

bool sa_get(SparseArray *sa, uint16_t y, uint16_t x) {
   if (sa->BIN == EMPTY_CHUNK) {
      return false;
   }
   else if (sa->BIN == FULL_CHUNK) {
      return true;
   }
   else {
      return (sa->BIN->BASE & BIT) ? true : false;
   }
}

