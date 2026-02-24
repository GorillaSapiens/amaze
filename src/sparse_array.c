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

static Chunk *data[65536 / CHUNK_BITS][65536 / CHUNK_BITS] = { { 0 } };

#define BIN data[y / CHUNK_BITS][x / CHUNK_BITS]
#define BASE base[y % CHUNK_BITS]
#define BIT (((ChunkBase) 1) << (x % CHUNK_BITS))

void sa_reset(void) {
   for (int i = 0; i < 65536 / CHUNK_BITS; i++) {
      for (int j = 0; j < 65536 / CHUNK_BITS; j++) {
         if (data[i][j]) {
            if (data[i][j] != FULL_CHUNK) {
               free(data[i][j]);
            }
            data[i][j] = EMPTY_CHUNK;
         }
      }
   }
}

void sa_set(uint16_t y, uint16_t x) {
   if (BIN == FULL_CHUNK) {
      return; // already set
   }

   if (BIN == EMPTY_CHUNK) {
      BIN = calloc(1, sizeof(Chunk));
   }

   BIN->BASE |= BIT;

   for (int i = 0; i < CHUNK_BITS; i++) {
      if (BIN->base[i] != (ChunkBase) -1) {
         return;
      }
   }

   free(BIN);
   BIN = FULL_CHUNK;
}

void sa_clr(uint16_t y, uint16_t x) {
   if (BIN == EMPTY_CHUNK) {
      return; // already clear
   }

   if (BIN == FULL_CHUNK) {
      BIN = malloc(sizeof(Chunk));
      memset(BIN, 0xFF, sizeof(Chunk));
   }

   BIN->BASE &= ~BIT;

   for (int i = 0; i < CHUNK_BITS; i++) {
      if (BIN->base[i] != 0) {
         return;
      }
   }

   free(BIN);
   BIN = EMPTY_CHUNK;
}

bool sa_get(uint16_t y, uint16_t x) {
   if (BIN == EMPTY_CHUNK) {
      return false;
   }
   else if (BIN == FULL_CHUNK) {
      return true;
   }
   else {
      return (BIN->BASE & BIT) ? true : false;
   }
}

