#ifndef _INCLUDE_MESSAGES_H_
#define _INCLUDE_MESSAGES_H_

int msg_printf(const char *fmt, ...); // allocates memory

char *msg_next(void); // caller is responsible for free()ing !!!

#endif
