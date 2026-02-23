#ifndef _INCLUDE_OBJECT_H_
#define _INCLUDE_OBJECT_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct Object {
   bool in_inventory;
   int16_t x, y;
   uint8_t fg;
   uint8_t bg;
   uint16_t unicode;
   const char *name;

   struct Object *hnext; // next in hash bucket, do not touch

   struct Object *lnext; // next in location
   struct Object *lprev; // prev in location

} Object;

void obj_init(void);
Object *obj_get(int16_t y, int16_t x);
Object *obj_get_inv(void);
int obj_get_inv_count(void);
void obj_take(Object *obj); // moves object to inventory
void obj_drop(Object *obj, int16_t y, int16_t x); // drops object from inventory

#endif
