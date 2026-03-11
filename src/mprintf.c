#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "mprintf.h"

char *mprintf(const char *fmt, ...) {
   char *buffer;
   int len;

   va_list ap;
   va_list ap_copy;

   va_start(ap, fmt);
   va_copy(ap_copy, ap);

   // get required length
   len = vsnprintf(NULL, 0, fmt, ap);
   va_end(ap);

   if (len < 0)
   {
      va_end(ap_copy);
      return NULL;
   }

   // Allocate (+1 for null terminator)
   buffer = malloc((size_t) len + 1);
   if (!buffer)
   {
      va_end(ap_copy);
      return NULL;
   }

   // Second pass: actually format
   vsnprintf(buffer, (size_t) len + 1, fmt, ap_copy);
   va_end(ap_copy);

   return buffer;
}
