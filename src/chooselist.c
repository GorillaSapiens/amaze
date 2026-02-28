#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ansi.h"
#include "chooselist.h"

#define TYPE_TEXT 0
#define TYPE_OBJ 1

typedef struct ChooselistItem {
   int type;
   char tag;
   union {
      Object *obj;
      char *text;
   };
} ChooselistItem;

typedef struct Chooselist {
   char *prompt;
   void (*fn)(Object *obj, char tag);
   int count;
   ChooselistItem *items;
} Chooselist;

Chooselist *cl_new(const char *prompt, void (*fn)(Object *obj, char tag)) {
   Chooselist *ret = (Chooselist *)malloc(sizeof(Chooselist));
   ret->prompt = strdup(prompt);
   ret->fn = fn;
   ret->count = 0;
   ret->items = NULL;
   return ret;
}

void cl_add_obj(Chooselist *cl, Object *obj, char tag) {
   cl->items = realloc(cl->items, sizeof(ChooselistItem) * (cl->count + 1));
   cl->items[cl->count].type = TYPE_OBJ;
   cl->items[cl->count].obj = obj;
   cl->items[cl->count].tag = tag;
   cl->count++;
}

void cl_add_text(Chooselist *cl, const char *text, char tag) {
   cl->items = realloc(cl->items, sizeof(ChooselistItem) * (cl->count + 1));
   cl->items[cl->count].type = TYPE_TEXT;
   cl->items[cl->count].text = strdup(text);
   cl->items[cl->count].tag = tag;
   cl->count++;
}

void cl_display(Chooselist *cl) {
}

void cl_delete(Chooselist *cl) {
   free((void *)cl->prompt);
   for (int i = 0; i < cl->count; i++) {
      if (cl->items[i].type == TYPE_TEXT) {
         free((void *)cl->items[i].text);
      }
   }
}
