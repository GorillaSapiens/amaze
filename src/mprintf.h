#ifndef _INCLUDE_MPRINTF_H_
#define _INCLUDE_MPRINTF_H_

#include <stdarg.h>

#include "mprintf.h"

// returns malloc'd pointer, caller MUST free
char *mprintf(const char *fmt, ...);

#endif
