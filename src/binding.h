#ifndef _INCLUDE_BINDING_H_
#define _INCLUDE_BINDING_H_

#include "object.h"

typedef struct Object Object;

typedef struct Binding {
   char action;           // 'a'pply, 'r'ead, 'q'uaff, etc...
   void (*fn)(Object *obj);
   struct Binding *next;
} Binding;

#endif
