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
   bool remembered;
} Object;

void obj_init(void);
Object *obj_get(int16_t y, int16_t x);

#endif
