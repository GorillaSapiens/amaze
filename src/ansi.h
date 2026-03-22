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

// line drawing characters
//           abcd
#define BLOCK_UH  0x2580
#define BLOCK_LoH 0x2584
#define BLOCK_RH  0x2590
#define BLOCK_LeH 0x258C
#define BLOCK_FUL 0x2588
#define BLOCK_LL  0x2596
#define BLOCK_LR  0x2597
#define BLOCK_UL  0x2598
#define BLOCK_UR  0x259D
#define BLOCK_nLL 0x259C
#define BLOCK_nLR 0x259B
#define BLOCK_nUL 0x259F
#define BLOCK_nUR 0x2599
#define BLOCK_SL  0x259E
#define BLOCK_nSL 0x259A

#if 0
#define GRAY_0000 0x0020
#define GRAY_0001 0x2598
#define GRAY_0010 0x259D
#define GRAY_0011 0x2580
#define GRAY_0100 0x2596
#define GRAY_0101 0x25 B
#define GRAY_0110 0x25 B
#define GRAY_0111 0x25 B
#define GRAY_1000 0x25 B
#define GRAY_1001 0x25 B
#define GRAY_1010 0x25 B
#define GRAY_1011 0x25 B
#define GRAY_1100 0x25 B
#define GRAY_1101 0x25 B
#define GRAY_1110 0x25 B
#define GRAY_1111 0x25 B
#endif

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
