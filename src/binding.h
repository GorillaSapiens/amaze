#ifndef _INCLUDE_BINDING_H_
#define _INCLUDE_BINDING_H_

#include "object.h"

#define ACTION_NONE    0
#define ACTION_APPLY   'a'
#define ACTION_EAT     'e'
#define ACTION_QUAFF   'q'
#define ACTION_READ    'r'
#define ACTION_WEAR    'w'
#define ACTION_UNWEAR  'W'

#define VERB_APPLY   "apply"
#define VERB_EAT     "eat"
#define VERB_QUAFF   "quaff"
#define VERB_READ    "read"
#define VERB_WEAR    "wear"
#define VERB_UNWEAR  "take off"

typedef struct Object Object;
typedef struct Binding Binding;

typedef struct Binding {
   char action;           // 'a'pply, 'r'ead, 'q'uaff, etc...
   const char *verb;      // "apply", "read", "quaff", etc..
   Object *obj;
   void (*fn)(Binding *binding);
   struct Binding *next;
} Binding;

const char *action2verb(int action);

#endif
