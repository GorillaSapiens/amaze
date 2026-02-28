#ifndef _INCLUDE_BINDING_H_
#define _INCLUDE_BINDING_H_

#include "object.h"

typedef struct Object Object;
typedef struct Binding Binding;

typedef struct Binding {
   char action;           // 'a'pply, 'r'ead, 'q'uaff, etc...
   Object *obj;
   void (*fn)(Binding *binding);
   struct Binding *next;
} Binding;

#endif
