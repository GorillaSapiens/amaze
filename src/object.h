#ifndef _INCLUDE_OBJECT_H_
#define _INCLUDE_OBJECT_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct Object {
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

#endif
