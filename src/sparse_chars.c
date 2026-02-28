#include "sparse_chars.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct SCEntry {
   int16_t x, y;
   uint32_t unicode;
   struct SCEntry *next;
} SCEntry;

static SCEntry *hashtable[65536] = { NULL };

static uint16_t ent_hash(int16_t y, int16_t x) {
   uint16_t ret = y ^ (((x & 0xFF) << 8) | ((x >> 8) & 0xFF));
   return ret;
}

void sc_set(int16_t y, int16_t x, uint32_t unicode) {
   uint16_t hash = ent_hash(y, x);

   for (SCEntry *ptr = hashtable[hash]; ptr; ptr = ptr->next) {
      if (ptr->y == y && ptr->x == x) {
         ptr->unicode = unicode;
         return;
      }
   }

   SCEntry *ent = (SCEntry *) malloc(sizeof(SCEntry));
   ent->y = y;
   ent->x = x;
   ent->unicode = unicode;
   ent->next = hashtable[hash];
   hashtable[hash] = ent;
}

void sc_clr(int16_t y, int16_t x) {
   uint16_t hash = ent_hash(y, x);

   if (hashtable[hash] &&
       hashtable[hash]->y == y &&
       hashtable[hash]->x == x) {
      SCEntry *freeme = hashtable[hash];
      hashtable[hash] = hashtable[hash]->next;
      free(freeme);
      return;
   }

   for (SCEntry *ptr = hashtable[hash]; ptr && ptr->next; ptr = ptr->next) {
      if (ptr->next->y == y && ptr->next->x == x) {
         SCEntry *nptr = ptr->next->next;
         free(ptr->next);
         ptr->next = nptr;
         return;
      }
   }
}

uint32_t sc_get(int16_t y, int16_t x) {
   uint16_t hash = ent_hash(y, x);

   for (SCEntry *ptr = hashtable[hash]; ptr; ptr = ptr->next) {
      if (ptr->y == y && ptr->x == x) {
         return ptr->unicode;
      }
   }
   return 0;
}

void sc_debug(void) {
   for (int i = 0; i < 65536; i++) {
      if (hashtable[i]) {
         for (SCEntry *ptr = hashtable[i]; ptr; ptr = ptr->next) {
            printf("%d,%d %08x\n", ptr->x, ptr->y, ptr->unicode);
         }
      }
   }
}
