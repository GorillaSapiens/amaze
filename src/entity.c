#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "entity.h"

static int count = 0;
static Entity *ents = NULL;

void ent_init(void) {
   FILE *f = fopen("things.txt", "r");
   if (f) {
      char buf[1024];
      while (fgets(buf, sizeof(buf) - 1, f)) {
         ents = realloc(ents, sizeof(Entity) * (count + 1));
         ents[count].name = strdup(buf);
         ents[count].fg = rand() % 16;
         do {
            ents[count].bg = rand() % 16;
         } while (ents[count].bg == ents[count].fg);
         if (ents[count].fg & 8) {
            ents[count].fg = ents[count].fg - 8 + 60;
         }
         if (ents[count].bg & 8) {
            ents[count].bg = ents[count].bg - 8 + 60;
         }
         ents[count].x = ((rand() & 0x7F) - 64) * 2 + 1;
         ents[count].y = ((rand() & 0x7F) - 64) * 2 + 1;
         ents[count].unicode = (rand() % 95) + ' ' + 1; // TODO FIX add more!
         ents[count].remembered = false;
         count++;
      }
      fclose(f);
   }

   ents[0].x = ents[0].y = 3;
}

Entity *ent_get(int16_t y, int16_t x) {
   for (int i = 0; i < count; i++) {
      if (ents[i].x == x && ents[i].y == y) {
         return ents + i; // TODO fix what about multiple matches ???
      }
   }
   return NULL;
}
