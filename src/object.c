#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

static Object *hashtable[65536] = { NULL };

uint16_t obj_hash(int16_t y, int16_t x) {
   return y ^ (((x & 0xFF) << 16) | ((x >>16) & 0xFF));
}

void obj_insert(Object *obj) {
   uint16_t hash = obj_hash(obj->y, obj->x);
   bool found = false;

   for (Object *ptr = hashtable[hash]; ptr; ptr = ptr->hnext) {
      if (ptr->x == obj->x && ptr->y == obj->y) {
         obj->lprev = ptr->lprev;
         obj->lnext = ptr;
         ptr->lprev = obj;
         obj->lprev->lnext = obj;
         found = true;
         break;
      }
   }
   if (!found) {
      obj->lprev = obj->lnext = obj;
   }
   obj->hnext = hashtable[hash];
   hashtable[hash] = obj;
}

void obj_remove(Object *obj) {
   uint16_t hash = obj_hash(obj->y, obj->x);

   if (hashtable[hash] == obj) {
      hashtable[hash] = obj->hnext;
   }
   else {
      for (Object *ptr = hashtable[hash]; ptr->hnext; ptr = ptr->hnext) {
         if (ptr->hnext == obj) {
            ptr->hnext = obj->hnext;
            obj->hnext = NULL;
            break;
         }
      }
   }

   obj->lprev->lnext = obj->lnext;
   obj->lnext->lprev = obj->lprev;
   obj->lnext = obj->lprev = NULL;
}

void obj_init(void) {
   FILE *f = fopen("things.txt", "r");
   if (f) {
      char buf[1024];
      while (fgets(buf, sizeof(buf) - 1, f)) {
         Object *obj = (Object *) malloc (sizeof(Object));
         // TODO FIX handle OOM

         // trim newline
         for (char *p = buf; *p; p++) {
            if (*p < ' ') {
               *p = 0;
               break;
            }
         }

         obj->name = strdup(buf);
         // TODO FIX handle OOM

         // color selection
         obj->fg = (rand() % 7) + 1; // fg is NEVER 0 (black)
         do {
            obj->bg = rand() % 7; // bg is NEVER 7 (white)
         } while (obj->bg == obj->fg); // bg is NEVER == fg
         obj->fg += 60; // ANSI_BRIGHT

         // glyph selection
         obj->unicode = (rand() % 94) + ' ' + 1; // TODO FIX add more!

         // position selection
         obj->x = ((rand() & 0x7F) - 64) * 2 + 1;
         obj->y = ((rand() & 0x7F) - 64) * 2 + 1;

         // TODO FIX this will get removed later
         obj->remembered = false;

         obj->hnext = NULL;
         obj->lnext = NULL;
         obj->lprev = NULL;

         obj_insert(obj);
      }
      fclose(f);
   }
}

Object *obj_get(int16_t y, int16_t x) {
   uint16_t hash = obj_hash(y, x);

   for (Object *ptr = hashtable[hash]; ptr; ptr = ptr->hnext) {
      if (ptr->x == x && ptr->y == y) {
         return ptr;
      }
   }
   return NULL;
}
