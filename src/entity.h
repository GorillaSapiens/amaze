#ifndef _INCLUDE_ENTITY_H_
#define _INCLUDE_ENTITY_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct Entity {
   int16_t x, y;
   uint8_t fg;
   uint8_t bg;
   uint16_t unicode;
   const char *name;
   bool remembered;
} Entity;

void ent_init(void);
Entity *ent_get(int16_t y, int16_t x);

#endif
