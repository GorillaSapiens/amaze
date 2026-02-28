#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "ansi.h"

const char obj_type_glyphs[] = {
   OBJ_COIN_GLYPH,
   OBJ_AMULET_GLYPH,
   OBJ_WEAPON_GLYPH,
   OBJ_ARMOR_GLYPH,
   OBJ_FOOD_GLYPH,
   OBJ_SCROLL_GLYPH,
   OBJ_BOOK_GLYPH,
   OBJ_POTION_GLYPH,
   OBJ_RING_GLYPH,
   OBJ_WAND_GLYPH,
   OBJ_TOOL_GLYPH,
   OBJ_GEM_GLYPH,
   OBJ_BOULDER_GLYPH,
   OBJ_BALL_GLYPH,
   OBJ_CHAIN_GLYPH,
   OBJ_VENOM_GLYPH,
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

#define NUM_TYPES (sizeof(obj_type_glyphs) - 1)

static Object *hashtable[65536] = { NULL };
static Object *inventory = NULL;
static int inventory_count = 0;
static uint64_t inv_assignments = 0;

static uint16_t obj_hash(int16_t y, int16_t x) {
   return y ^ (((x & 0xFF) << 8) | ((x >> 8) & 0xFF));
}

static void remove_link(Object **ptr, Object *removee) {
   if (!ptr) return;

   while (*ptr && *ptr != removee) {
      ptr = &((*ptr)->link);
   }
   if (*ptr && *ptr == removee) {
      *ptr = removee->link;
      removee->link = NULL;
   }
}

static void obj_insert(Object *obj) {
   uint16_t hash = obj_hash(obj->y, obj->x);
   bool found = false;

   for (Object *ptr = hashtable[hash]; ptr; ptr = ptr->link) {
      if (ptr->x == obj->x && ptr->y == obj->y) {
         obj->prev = ptr->prev;
         obj->next = ptr;
         ptr->prev = obj;
         obj->prev->next = obj;
         found = true;
         break;
      }
   }
   if (!found) {
      obj->prev = obj->next = obj;
   }
   obj->link = hashtable[hash];
   hashtable[hash] = obj;
}

static void obj_remove(Object *obj) {
   uint16_t hash = obj_hash(obj->y, obj->x);

   remove_link(hashtable + hash, obj);

   obj->prev->next = obj->next;
   obj->next->prev = obj->prev;
   obj->next = obj->prev = NULL;
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

         obj->link = NULL;
         obj->next = NULL;
         obj->prev = NULL;

         obj->in_inventory = false;
         obj->tag = 0;
         obj->type = rand() % NUM_TYPES;
         obj->unicode = obj_type_glyphs[(int)obj->type]; // TODO come to a decision here!

         obj_insert(obj);
      }
      fclose(f);
   }
}

Object *obj_get(int16_t y, int16_t x) {
   uint16_t hash = obj_hash(y, x);

   for (Object *ptr = hashtable[hash]; ptr; ptr = ptr->link) {
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

      obj->link = inventory;
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
         inventory = obj->link;
      }
      else {
         for (Object *ptr = inventory; ptr && ptr->link; ptr = ptr->link) {
            if (ptr->link == obj) {
               ptr->link = obj->link;
               break;
            }
         }
      }
      obj->link = NULL;
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

Binding *obj_bind(Object *obj, char action, void (*fn)(Binding *), size_t size) {
   Binding *ret = NULL;
   if (!obj) {
      return NULL;
   }
   if (size < sizeof(Binding)) {
      size = sizeof(Binding);
   }
   ret = (Binding *) malloc(size);
   if (ret) {
      ret->action   = action;
      ret->obj      = obj;
      ret->fn       = fn;
      ret->next     = obj->bindings;
      obj->bindings = ret;
   }
   return ret;
}
