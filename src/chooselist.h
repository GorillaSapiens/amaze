#ifndef _INCLUDE_CHOOSELIST_H_
#define _INCLUDE_CHOOSELIST_H_

#include "object.h"

typedef struct Chooselist Chooselist;

Chooselist *cl_new(const char *prompt, void (*fn)(Object *obj, char tag));
void cl_add_obj(Chooselist *cl, Object *obj, char tag);
void cl_add_text(Chooselist *cl, const char *text, char tag);
void cl_display(Chooselist *cl, int height, int width);
void cl_delete(Chooselist *cl);

#endif
