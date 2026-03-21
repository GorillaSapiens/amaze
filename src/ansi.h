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

#define U_EMPTY 0x00B7

// line drawing characters
//           udlr
#define WALL_0000 0x25AA // a square
#define WALL_0001 0x257A // 0x2501
#define WALL_0010 0x2578 // 0x2501
#define WALL_0011 0x2501
#define WALL_0100 0x257B // 0x2503
#define WALL_0101 0x250F
#define WALL_0110 0x2513
#define WALL_0111 0x2533
#define WALL_1000 0x2579 // 0x2503
#define WALL_1001 0x2517
#define WALL_1010 0x251B
#define WALL_1011 0x253B
#define WALL_1100 0x2503
#define WALL_1101 0x2523
#define WALL_1110 0x252B
#define WALL_1111 0x254B

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
