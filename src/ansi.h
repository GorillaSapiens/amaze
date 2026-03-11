#ifndef _INCLUDE_ANSI_H_
#define _INCLUDE_ANSI_H_

#include <stdint.h>

#define ANSI_FORE    30
#define ANSI_BACK    40
#define ANSI_BRIGHT  60

#define COLOR_BLACK   0
#define COLOR_RED     1
#define COLOR_GREEN   2
#define COLOR_YELLOW  3
#define COLOR_BLUE    4
#define COLOR_MAGENTA 5
#define COLOR_CYAN    6
#define COLOR_WHITE   7

#define COLOR_BRIGHT_BLACK   (ANSI_BRIGHT + COLOR_BLACK)
#define COLOR_BRIGHT_RED     (ANSI_BRIGHT + COLOR_RED)
#define COLOR_BRIGHT_GREEN   (ANSI_BRIGHT + COLOR_GREEN)
#define COLOR_BRIGHT_YELLOW  (ANSI_BRIGHT + COLOR_YELLOW)
#define COLOR_BRIGHT_BLUE    (ANSI_BRIGHT + COLOR_BLUE)
#define COLOR_BRIGHT_MAGENTA (ANSI_BRIGHT + COLOR_MAGENTA)
#define COLOR_BRIGHT_CYAN    (ANSI_BRIGHT + COLOR_CYAN)
#define COLOR_BRIGHT_WHITE   (ANSI_BRIGHT + COLOR_WHITE)

extern int screen_w;
extern int screen_h;

// initializer
void ansi_init(void);

// input handler for window changes
void getwinch(void);

// set a color
void color(int color);

// print a utf8 character
void utf8printchar(uint8_t fg, uint8_t bg, uint32_t x);

// printf with color
int cprintf(unsigned char fg, unsigned char bg, const char *fmt, ...);

// clear the screen
void clear(void);

// set cursor to home
void home(void);

// move cursor to x and y
void mvcurs(int y, int x);

// set stdin/stdout as unbuffered
void unbuffer(void);

#endif
