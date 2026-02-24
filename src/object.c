#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "ansi.h"

const char obj_types[] = {
   OBJ_COIN,
   OBJ_AMULET,
   OBJ_WEAPON,
   OBJ_ARMOR,
   OBJ_FOOD,
   OBJ_SCROLL,
   OBJ_BOOK,
   OBJ_POTION,
   OBJ_RING,
   OBJ_WAND,
   OBJ_TOOL,
   OBJ_GEM,
   OBJ_BOULDER,
   OBJ_BALL,
   OBJ_CHAIN,
   OBJ_VENOM,
   0 };

const char *obj_type_names[] = {
   "Coins",
   "Amulets",
   "Weapons",
   "Armor",
   "Food",
   "Scrolls",
   "Books",
   "Potions",
   "Rings",
   "Wands",
   "Tools",
   "Gems",
   "Boulders",
   "Balls",
   "Chains",
   "Venom"
};

#define NUM_TYPES (sizeof(obj_types) - 1)

static Object *hashtable[65536] = { NULL };
static Object *inventory = NULL;
static int inventory_count = 0;
static uint64_t inv_assignments = 0;

static uint16_t obj_hash(int16_t y, int16_t x) {
   return y ^ (((x & 0xFF) << 8) | ((x >> 8) & 0xFF));
}

static void obj_insert(Object *obj) {
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

static void obj_remove(Object *obj) {
   uint16_t hash = obj_hash(obj->y, obj->x);

   if (hashtable[hash] == obj) {
      hashtable[hash] = obj->hnext;
   }
   else {
      for (Object *ptr = hashtable[hash]; ptr->hnext; ptr = ptr->hnext) {
         if (ptr->hnext == obj) {
            ptr->hnext = obj->hnext;
            break;
         }
      }
   }

   obj->hnext = NULL;

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
           // bg is NEVER == fg
         } while (obj->bg == obj->fg ||
                  (obj->fg == COLOR_WHITE && obj->bg == COLOR_BLACK));
         obj->fg += ANSI_BRIGHT;

         // glyph selection
         obj->unicode = (rand() % 94) + ' ' + 1; // TODO FIX add more!

         // position selection
         obj->x = ((rand() & 0x7F) - 64) * 2 + 1;
         obj->y = ((rand() & 0x7F) - 64) * 2 + 1;

         obj->hnext = NULL;
         obj->lnext = NULL;
         obj->lprev = NULL;

         obj->in_inventory = false;
         obj->tag = 0;
         obj->type = rand() % NUM_TYPES;
         obj->unicode = obj_types[(int)obj->type]; // TODO come to a decision here!

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

Object *obj_get_inv(void) {
   return inventory;
}

int obj_get_inv_count(void) {
   return inventory_count;
}

// moves object to inventory
void obj_take(Object *obj) {
   if (!obj->in_inventory) {
      obj_remove(obj);

      obj->hnext = inventory;
      inventory = obj;

      obj->in_inventory = true;
      inventory_count++;

      if (!(inv_assignments & (1LL << obj->tag))) {
         inv_assignments |= (1LL << obj->tag);
      }
      else {
         for (int i = 0; i < (sizeof(uint64_t) * 8); i++) {
            if (!(inv_assignments & (1LL << i))) {
               obj->tag = i;
               inv_assignments |= (1LL << i);
               break;
            }
         }
      }
   }
   else {
      // TODO FIX error handling
   }
}

// drops object from inventory
void obj_drop(Object *obj, int16_t y, int16_t x) {
   if (obj->in_inventory) {
      if (inventory == obj) {
         inventory = obj->hnext;
      }
      else {
         for (Object *ptr = inventory; ptr && ptr->hnext; ptr = ptr->hnext) {
            if (ptr->hnext == obj) {
               ptr->hnext = obj->hnext;
               break;
            }
         }
      }
      obj->hnext = NULL;
      obj->in_inventory = false;
      inventory_count--;
      inv_assignments &= ~(1LL << obj->tag);
      obj->y = y;
      obj->x = x;
      obj_insert(obj);
   }
   else {
      // TODO FIX error handling
   }
}
