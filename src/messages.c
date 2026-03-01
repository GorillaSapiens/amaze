#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "messages.h"

static char **queue = NULL;
static int count = 0;

int msg_printf(const char *fmt, ...) {
   char *buffer, *tmp;
   int len;

   va_list ap;
   va_list ap_copy;

   va_start(ap, fmt);
   va_copy(ap_copy, ap);

   // get required length
   len = vsnprintf(NULL, 0, fmt, ap);
   va_end(ap);

   if (len <= 0)
   {
      va_end(ap_copy);
      return 0;
   }

   // Allocate (+1 for null terminator)
   buffer = tmp = malloc(len + 1);
   if (!buffer)
   {
      va_end(ap_copy);
      return 0;
   }

   // Second pass: actually format
   vsnprintf(buffer, len + 1, fmt, ap_copy);
   va_end(ap_copy);

   queue = (char **) realloc(queue, sizeof(char *) * (count + 1)); 
   queue[count++] = buffer;

   return len;
}

char *msg_next(void) {
   if (!queue || !count) {
      return NULL;
   }

   char *ret = queue[0];
   memmove(queue, queue + 1, sizeof(char *) * (count - 1));
   count--;

   return ret;
}
