#ifndef _INCLUDE_OBJECT_H_
#define _INCLUDE_OBJECT_H_

#include <stdint.h>
#include <stdbool.h>

#define OBJ_COIN    0
#define OBJ_AMULET  1
#define OBJ_WEAPON  2
#define OBJ_ARMOR   3
#define OBJ_FOOD    4
#define OBJ_SCROLL  5
#define OBJ_BOOK    6
#define OBJ_POTION  7
#define OBJ_RING    8
#define OBJ_WAND    9
#define OBJ_TOOL    10
#define OBJ_GEM     11
#define OBJ_BOULDER 12
#define OBJ_BALL    13
#define OBJ_CHAIN   14
#define OBJ_VENOM   15

#define OBJ_COIN_GLYPH    '$'
#define OBJ_AMULET_GLYPH  '"'
#define OBJ_WEAPON_GLYPH  ')'
#define OBJ_ARMOR_GLYPH   '['
#define OBJ_FOOD_GLYPH    '%'
#define OBJ_SCROLL_GLYPH  '?'
#define OBJ_BOOK_GLYPH    '+'
#define OBJ_POTION_GLYPH  '!'
#define OBJ_RING_GLYPH    '='
#define OBJ_WAND_GLYPH    '/'
#define OBJ_TOOL_GLYPH    '('
#define OBJ_GEM_GLYPH     '*'
#define OBJ_BOULDER_GLYPH '`'
#define OBJ_BALL_GLYPH    '0'
#define OBJ_CHAIN_GLYPH   '_'
#define OBJ_VENOM_GLYPH   '.'

#define OBJ_COIN_MASK    (1 << OBJ_COIN)
#define OBJ_AMULET_MASK  (1 << OBJ_AMULET)
#define OBJ_WEAPON_MASK  (1 << OBJ_WEAPON)
#define OBJ_ARMOR_MASK   (1 << OBJ_ARMOR)
#define OBJ_FOOD_MASK    (1 << OBJ_FOOD)
#define OBJ_SCROLL_MASK  (1 << OBJ_SCROLL)
#define OBJ_BOOK_MASK    (1 << OBJ_BOOK)
#define OBJ_POTION_MASK  (1 << OBJ_POTION)
#define OBJ_RING_MASK    (1 << OBJ_RING)
#define OBJ_WAND_MASK    (1 << OBJ_WAND)
#define OBJ_TOOL_MASK    (1 << OBJ_TOOL)
#define OBJ_GEM_MASK     (1 << OBJ_GEM)
#define OBJ_BOULDER_MASK (1 << OBJ_BOULDER)
#define OBJ_BALL_MASK    (1 << OBJ_BALL)
#define OBJ_CHAIN_MASK   (1 << OBJ_CHAIN)
#define OBJ_VENOM_MASK   (1 << OBJ_VENOM)
#define OBJ_ANY_MASK     (-1)

extern const char obj_type_glyphs[];
extern const char *obj_type_names[];

typedef struct Object {
   bool in_inventory;
   int16_t x, y;
   uint8_t fg;
   uint8_t bg;
   uint16_t unicode;
   const char *name;

   char tag;
   char type;

   struct Object *link; // next in hash bucket, do not touch

   // next/prev pointers for location.  NULL if in main inventory.
   // this *could* be same x,y location, **OR** inside containers
   struct Object *next;
   struct Object *prev;

} Object;

void obj_init(void);
Object *obj_get(int16_t y, int16_t x);
Object *obj_get_inv(void);
int obj_get_inv_count(void);
void obj_take(Object *obj); // moves object to inventory
void obj_drop(Object *obj, int16_t y, int16_t x); // drops object from inventory

#endif
