#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>

#include "ansi.h"

int screen_w = 80;
int screen_h = 21;

// SIGWINCH handler
static void sigwinch_handler(int) {
   printf("\033[18t");
}

// input handler for window changes
void getwinch(void) {
   int mode = 0;
   int rows = 0;
   int cols = 0;

   clear();

   // ESC [ 8 ; rows ; cols t
   while (1) {
      int u = getchar();

      switch(mode) {
         case 0:
            if (u == '[') {
               mode++;
            }
            else {
               // some other input
               return;
            }
            break;
         case 1:
            if (u == '8') {
               mode++;
            }
            else {
               // some other input
               return;
            }
            break;
         case 2:
            if (u == ';') {
               mode++;
            }
            else {
               // some other input
               return;
            }
            break;
         case 3:
            if (u == ';') {
               mode++;
            }
            else if (u >= '0' && u <= '9') {
               rows *= 10;
               rows += u - '0';
            }
            else {
               // some other input
               return;
            }
            break;
         case 4:
            if (u == 't') {
               screen_w = cols;
               screen_h = rows;
               return;
            }
            else if (u >= '0' && u <= '9') {
               cols *= 10;
               cols += u - '0';
            }
            else {
               // some other input
               return;
            }
            break;
      }
   }
}

// initializer
void ansi_init(void) {
   signal(SIGWINCH, sigwinch_handler);
   unbuffer();
   printf("\033[18t"); // get window size
}

// set a color
void color(int color) {
   printf("\033[%dm", color);
}

// print a utf8 character
void utf8printchar(uint8_t fg, uint8_t bg, uint32_t x) {
   static uint8_t _fg = -1;
   static uint8_t _bg = -1;

   if (fg != _fg) {
      _fg = fg;
      color(ANSI_FORE + fg);
   }

   if (bg != _bg) {
      _bg = bg;
      color(ANSI_BACK + bg);
   }

   if (x <= 0x7F) {
      printf("%c", x);
   }
   else if (x < 0x800) {
      printf("%c%c",
             0xC0 | (x >> 6),
             0x80 | (x & 0x3F));
   }
   else if (x < 0x10000) {
      printf("%c%c%c",
             0xE0 | (x >> 12),
             0x80 | ((x >> 6) & 0x3F),
             0x80 | (x & 0x3F));
   }
   else {
      printf("%c%c%c%c",
             0xF0 | (x >> 18),
             0x80 | ((x >> 12) & 0x3F),
             0x80 | ((x >> 6) & 0x3F),
             0x80 | (x & 0x3F));
   }
}

int cprintf(unsigned char fg, unsigned char bg, const char *fmt, ...) {
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
   buffer = tmp = (char *) malloc(len + 1);
   if (!buffer)
   {
      va_end(ap_copy);
      return 0;
   }

   // Second pass: actually format
   vsnprintf(buffer, len + 1, fmt, ap_copy);
   va_end(ap_copy);

   while (*tmp) {
      utf8printchar(fg, bg, *tmp);
      tmp++;
   }

   free(buffer);

   return len;
}

// clear the screen
void clear(void) {
   cprintf(COLOR_BRIGHT_WHITE, COLOR_BLACK, "\033[2J\033[H");
}

// set stdin/stdout as unbuffered
void unbuffer(void) {
   static struct termios oldt, newt;

   /*tcgetattr gets the parameters of the current terminal
     STDIN_FILENO will tell tcgetattr that it should write the settings
     of stdin to oldt*/
   tcgetattr( STDIN_FILENO, &oldt);
   /*now the settings will be copied*/
   newt = oldt;

   /*ICANON normally takes care that one line at a time will be processed
     that means it will return if it sees a "\n" or an EOF or an EOL*/
   newt.c_lflag &= ~(ICANON);

   /*Those new settings will be set to STDIN
     TCSANOW tells tcsetattr to change attributes immediately. */
   tcsetattr( STDIN_FILENO, TCSANOW, &newt);

   //stdout also
   setvbuf(stdout, NULL, _IONBF, 0);   // or _IOLBF for line buffered
}
