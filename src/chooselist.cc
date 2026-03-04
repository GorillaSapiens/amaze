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
   cl->items = (ChooselistItem *) realloc(cl->items, sizeof(ChooselistItem) * (cl->count + 1));
   cl->items[cl->count].type = TYPE_OBJ;
   cl->items[cl->count].obj = obj;
   cl->items[cl->count].tag = tag;
   cl->count++;
}

void cl_add_text(Chooselist *cl, const char *text, char tag) {
   cl->items = (ChooselistItem *) realloc(cl->items, sizeof(ChooselistItem) * (cl->count + 1));
   cl->items[cl->count].type = TYPE_TEXT;
   cl->items[cl->count].text = strdup(text);
   cl->items[cl->count].tag = tag;
   cl->count++;
}

void cl_display(Chooselist *cl) {
   int lines = screen_h - 3;
   int columns = (cl->count + lines - 1) / lines;
   int width = screen_w / columns;

   clear();
   cprintf(COLOR_WHITE, COLOR_BLACK, "%s >>\n", cl->prompt);

   lines = (cl->count + columns - 1) / columns;
   for (int line = 0; line < lines; line++) {
      for (int column = 0; column < columns; column++) {
         int i = lines * column + line;
         if (i < cl->count) {
            if (cl->items[i].type == TYPE_TEXT) {
               if (cl->items[i].tag != -1) {
                  cprintf(COLOR_WHITE, COLOR_BLACK,
                        "  %c) %-*.*s  ", tag2char(cl->items[i].tag),
                        width - 7, width - 7,
                        cl->items[i].text ? cl->items[i].text : "");
               }
               else {
                  cprintf(COLOR_WHITE, COLOR_BLACK,
                        " %-*.*s  ",
                        width - 3, width - 3,
                        cl->items[i].text ? cl->items[i].text : "");
               }
            }
            else { // TYPE_OBJECT
               cprintf(COLOR_WHITE, COLOR_BLACK,
                     "  %c) %-*.*s  ", tag2char(cl->items[i].tag),
                     width - 7, width - 7,
                     cl->items[i].obj->name);
            }
         }
      }
      printf("\n");
   }

   int u = getchar();
   for (int i = 0; i < cl->count; i++) {
      if (u == tag2char(cl->items[i].tag)) {
         cl->fn(cl->items[i].obj, u);
      }
   }
}

void cl_delete(Chooselist *cl) {
   free((void *)cl->prompt);
   for (int i = 0; i < cl->count; i++) {
      if (cl->items[i].type == TYPE_TEXT) {
         free((void *)cl->items[i].text);
      }
   }
}
