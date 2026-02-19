#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <float.h>
#include <signal.h>
#include <stdarg.h>

#include "sparse_array.h"

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

bool raw = false;
bool see = false;
bool phase = false;
int screen_w = 80;
int screen_h = 21;
int xor = 0; //0xdeadbeef;

SparseArray *samem = NULL;

// in screen help
const char *help[12] = {
   "Procedural maze demo.",
   "",
   "movement:",
   "y k u",
   "h * l",
   "b j n",
   "",
   "s toggle sightlines",
   "p toggle phase",
   "r toggle raw",
   "a/z inc/dec xor",
   "q to quit",
};

// very loosely based on Telengard

// we compute a slightly larger map to avoid artifacts
// at view portal edges

#define SIZE 128     // size of computed map

// player visibility into map
int portal_x = 64;
int portal_y = 20;

// a struct to hold the map
typedef struct Map {
   int str[SIZE][SIZE+1];
   int offset_x;
   int offset_y;
} Map;

// obstruction extent
typedef struct ObstExt {
   double ctheta; // center angle from observer
   double dtheta; // the +/- obscured angle
} ObstExt;

ObstExt obstructs[SIZE][SIZE];

#define OBST_R (1.00 / 2.0) // obstruction radius

// initialize obstruction maps
void init_obstructs(void) {
   int cx = SIZE / 2;
   int cy = SIZE / 2;

   for (int y = 0; y < SIZE; y++) {
      for (int x = 0; x < SIZE; x++) {
         int dx = x - cx;
         int dy = y - cy;
         double d = sqrt(dx * dx + dy * dy);
         if (d != 0.0) {
            obstructs[y][x].ctheta = atan2((double) dy, (double) dx);
            obstructs[y][x].dtheta = asin(OBST_R / d);
         }
      }
   }
}

typedef struct { double lo, hi; } Range;

int cmp_range(const void *a, const void *b)
{
   const Range *A = a, *B = b;
   return (A->lo < B->lo) ? -1 : (A->lo > B->lo);
}

size_t merge_ranges(Range *ranges, size_t n)
{
   if (n == 0) return 0;
   qsort(ranges, n, sizeof *ranges, cmp_range);

   size_t write = 0;
   for (size_t read = 1; read < n; read++) {
      if (ranges[write].hi >= ranges[read].lo) {
         if (ranges[read].hi > ranges[write].hi) {
            ranges[write].hi = ranges[read].hi;
         }
      } else {
         ranges[++write] = ranges[read];
      }
   }
   return write + 1; // new count
}

typedef struct Order {
   int d2;
   int u;
   int v;
} Order;

#define ORDERSIZE ((SIZE / 2) * ((SIZE / 2) + 1) / 2)
Order orders[ORDERSIZE];

int cmp_orders(const void *a, const void *b) {
   const Order *oa = a;
   const Order *ob = b;

   if (oa->d2 < ob->d2) return -1;
   if (oa->d2 > ob->d2) return  1;
   return 0;
}

void init_orders(void) {
   int n = 0;
   for (int u = 0; u < SIZE / 2; u++) {
      for (int v = 0; v <= u; v++) {
         orders[n].d2 = u * u + v * v;
         orders[n].u = u;
         orders[n].v = v;
         n++;
      }
   }
   qsort(orders, n, sizeof(*orders), cmp_orders);
}

int posrand(int y, int x) {
   // Double, double toil and trouble...
   int seed = x & 0xFFFF;
   seed <<= 16;
   seed += y & 0xFFFF;
   seed ^= xor;
   srand(seed);

   return rand();
}

Map repair_voids(Map ret) {
   Map visited;

   memset(&visited, 0, sizeof(visited));

   // populate edges
   for (int i = 0; i < SIZE; i++) {
      if (ret.str[0][i] == ' ') visited.str[0][i] = ' ';
      if (ret.str[i][0] == ' ') visited.str[i][0] = ' ';
      if (ret.str[SIZE-1][i] == ' ') visited.str[SIZE-1][i] = ' ';
      if (ret.str[i][SIZE-1] == ' ') visited.str[i][SIZE-1] = ' ';
   }

   bool repaired = true;
   while (repaired) {
      repaired = false;

      bool changed = true;
      while (changed) {
         changed = false;
         for (int iy = 1; iy < SIZE - 1; iy++) {
            for (int ix = 1; ix < SIZE - 1; ix++) {
               if (!visited.str[iy][ix] && ret.str[iy][ix] == ' ') {
                  if (visited.str[iy-1][ix] ||
                        visited.str[iy+1][ix] ||
                        visited.str[iy][ix-1] ||
                        visited.str[iy][ix+1]) {
                     visited.str[iy][ix] = ' ';
                     changed = true;
                  }
               }
            }
         }
      }

      // repair one unvisited void per loop
      for (int iy = 1; !repaired && iy < SIZE - 1; iy++) {
         for (int ix = 1; !repaired && ix < SIZE - 1; ix++) {
            int typ = (ret.offset_x + ix) & 1;
            typ <<= 1;
            typ |= (ret.offset_y + iy) & 1;

            if (typ == 3 && ret.str[iy][ix] == ' ' && !visited.str[iy][ix]) {
               // we need a repair here!
               int tmp = posrand(ret.offset_y + iy, ret.offset_x + ix);

               // unsure that at least one repair bit is set
               while (tmp && !(tmp & (4|8))) {
                  tmp >>= 2;
               }

               if ((tmp & 4) && (iy > 0)) {
                  // space up
                  if (ret.str[iy - 1][ix] != ' ') {
                     ret.str[iy - 1][ix] = ' ';
                     repaired = true;
                  }
               }
               if ((tmp & 8) && (ix > 0)) {
                  // space left
                  if (ret.str[iy][ix - 1] != ' ') {
                     ret.str[iy][ix - 1] = ' ';
                     repaired = true;
                  }
               }
            }
         }
      }
   }

#if 0
   for (int iy = 1; iy < SIZE - 1; iy++) {
      for (int ix = 1; ix < SIZE - 1; ix++) {
         if (ret.str[iy][ix] == ' ' && !visited.str[iy][ix]) {
            ret.str[iy][ix] = 'X';
         }
      }
   }
#endif

   return ret;
}

Map repair_pillars(Map ret) {
   Map pillars;
   int count = -1;

   memset(&pillars, 0, sizeof(pillars));
   count = 0;

   // find 'em
   for (int iy = 1; iy < SIZE - 1; iy++) {
      for (int ix = 1; ix < SIZE - 1; ix++) {
         if (ret.str[iy][ix] == '*' &&
               ret.str[iy-1][ix] == ' ' &&
               ret.str[iy+1][ix] == ' ' &&
               ret.str[iy][ix-1] == ' ' &&
               ret.str[iy][ix+1] == ' ') {
            pillars.str[iy][ix] = ' ';
            count++;
         }
      }
   }

   // bridge adjacent pillars
   for (int iy = 2; iy < SIZE; iy++) {
      for (int ix = 2; ix < SIZE; ix++) {
         if (pillars.str[iy][ix]) {
            bool fix = false;
            if (pillars.str[iy-2][ix]) {
               ret.str[iy-1][ix] = '*';
               pillars.str[iy-2][ix] = 0;
               count--;
               fix = true;
            }
            if (pillars.str[iy][ix-2]) {
               ret.str[iy][ix-1] = '*';
               pillars.str[iy][ix-2] = 0;
               count--;
               fix = true;
            }
            if (fix) {
               pillars.str[iy][ix] = 0;
               count--;
            }
         }
      }
   }

   // connect lone pillars randomly
   for (int iy = 2; iy < SIZE - 2; iy++) {
      for (int ix = 2; ix < SIZE - 2; ix++) {
         if (pillars.str[iy][ix]) {
            // TODO FIX make this random
            if (ret.str[iy-2][ix] == '*') {
               ret.str[iy-1][ix] = '*';
            }
            else if (ret.str[iy+2][ix] == '*') {
               ret.str[iy+1][ix] = '*';
            }
            else if (ret.str[iy][ix-2] == '*') {
               ret.str[iy][ix-1] = '*';
            }
            else if (ret.str[iy][ix+2] == '*') {
               ret.str[iy][ix+1] = '*';
            }

            // adjust count
            pillars.str[iy][ix] = 0;
            count--;
         }
      }
   }

   return ret;
}

// generate a map based on player position
Map do_map(int x, int y) {
   Map ret;
   ret.offset_x = x - SIZE/2;
   ret.offset_y = y - SIZE/2;

   // tabla rasa
   for (int iy = 0; iy < SIZE; iy++) {
      for (int ix = 0; ix < SIZE; ix++) {
         ret.str[iy][ix] = ' ';
      }
      ret.str[iy][SIZE] = 0;
   }

   // create framework and walls
   for (int iy = 0; iy < SIZE; iy++) {
      for (int ix = 0; ix < SIZE; ix++) {
         int typ = (ret.offset_x + ix) & 1;
         typ <<= 1;
         typ |= (ret.offset_y + iy) & 1;

         switch (typ) {
            case 0:
               // corner always filled
               ret.str[iy][ix] = '*';
               break;
            case 1:
            case 2:
               // possible wall or open
               ret.str[iy][ix] = ' ';
               break;
            case 3:
               {
                  int tmp = posrand(ret.offset_y + iy, ret.offset_x + ix) % 0x10;

#define WALL_UP   do { if (iy > 0) ret.str[iy - 1][ix] = '*'; } while(0)
#define WALL_LEFT do { if (ix > 0) ret.str[iy][ix - 1] = '*'; } while(0)

                  if (tmp < 3) { // 3/16 chance
                     WALL_UP;
                     WALL_LEFT;
                  }
                  else if (tmp < 6) { // 3/16 chance
                     WALL_UP;
                  }
                  else if (tmp < 9) { // 3/16 chance
                     WALL_LEFT;
                  }
               }
               break;
         }
      }
   }

   // force reachability
   ret = repair_voids(ret);

   // join pillars
   ret = repair_pillars(ret);

   // place @
   ret.str[SIZE/2][SIZE/2] = '@';

   return ret;
}

// line drawing characters
int linechars[16] = {
   // udlr
   0x25CB,  // 0000 // a circle
   0x2501,  // 0001
   0x2501,  // 0010
   0x2501,  // 0011
   0x2503,  // 0100
   0x250F,  // 0101
   0x2513,  // 0110
   0x2533,  // 0111
   0x2503,  // 1000
   0x2517,  // 1001
   0x251B,  // 1010
   0x253B,  // 1011
   0x2503,  // 1100
   0x2523,  // 1101
   0x252B,  // 1110
   0x254B,  // 1111
};

// clear the screen
void clear(void) {
   printf("\033[2J\033[H");
}

// set a color
void color(int color) {
   printf("\033[%dm", color);
}

// print a utf8 character
void utf8printchar(unsigned char fg, unsigned char bg, unsigned int x) {
   static unsigned char _fg = -1;
   static unsigned char _bg = -1;

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
   buffer = tmp = malloc(len + 1);
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

static inline int sign(int x) {
   return (x > 0) - (x < 0);
}

bool obscured(Map *mask, Map *in, int y, int x, int *count, Range **ranges) {
   double ctheta = obstructs[y][x].ctheta;
   double dtheta = obstructs[y][x].dtheta;

   for (int i = 0; i < *count; i++) {
      if ((*ranges)[i].lo < ctheta - dtheta && (*ranges)[i].hi > ctheta + dtheta) {
         return true;
      }
   }

   // it is visible !!!
   mask->str[y][x] = ' ';

   if (in->str[y][x] != ' ') {
      // add it to obscured ranges
      *ranges = (Range *) realloc(*ranges, sizeof(Range) * ((*count) + 1));
      (*ranges)[*count].lo = ctheta - dtheta;
      (*ranges)[*count].hi = ctheta + dtheta;
      (*count)++;

      // to accomodate wrap...
      if (ctheta + dtheta > M_PI) {
         *ranges = (Range *) realloc(*ranges, sizeof(Range) * ((*count) + 1));
         (*ranges)[*count].lo = ctheta - dtheta - 2.0 * M_PI;
         (*ranges)[*count].hi = ctheta + dtheta - 2.0 * M_PI;
         (*count)++;
      }
      else if (ctheta - dtheta < M_PI) {
         *ranges = (Range *) realloc(*ranges, sizeof(Range) * ((*count) + 1));
         (*ranges)[*count].lo = ctheta - dtheta + 2.0 * M_PI;
         (*ranges)[*count].hi = ctheta + dtheta + 2.0 * M_PI;
         (*count)++;
      }

      // merge and adjust count
      *count = merge_ranges(*ranges, *count);
   }

   return false;
}

Map sight(Map in) {
   int atx = SIZE / 2;
   int aty = SIZE / 2;

   Map mask;
   memset(&mask, 0, sizeof(mask));

   mask.str[aty][atx] = ' ';
   mask.offset_x = in.offset_x;
   mask.offset_y = in.offset_y;

   int count = 0;
   Range *ranges = NULL;
   
   for (int i = 1; i < ORDERSIZE; i++) {
      obscured(&mask, &in, aty + orders[i].u, atx + orders[i].v, &count, &ranges);
      obscured(&mask, &in, aty + orders[i].u, atx - orders[i].v, &count, &ranges);
      obscured(&mask, &in, aty - orders[i].u, atx + orders[i].v, &count, &ranges);
      obscured(&mask, &in, aty - orders[i].u, atx - orders[i].v, &count, &ranges);
      obscured(&mask, &in, aty + orders[i].v, atx + orders[i].u, &count, &ranges);
      obscured(&mask, &in, aty + orders[i].v, atx - orders[i].u, &count, &ranges);
      obscured(&mask, &in, aty - orders[i].v, atx + orders[i].u, &count, &ranges);
      obscured(&mask, &in, aty - orders[i].v, atx - orders[i].u, &count, &ranges);
   }

   for (int y = 0; y < SIZE; y++) {
      for (int x = 0; x < SIZE; x++) {
         if (!mask.str[y][x]) {
            in.str[y][x] = ' '; // replacement chars
         }
         else {
            sa_set(samem, mask.offset_y + y, mask.offset_x + x);
         }
      }
   }

   free(ranges);

   return in;
}

Map wallify(Map c) {
   Map ret;
   for (int j = 0; j < SIZE; j++) {
      for (int i = 0; i < SIZE; i++) {
         if (c.str[j][i] != '*') {
            ret.str[j][i] = c.str[j][i];
         }
         else {
            int index = 0;
            if (j >  0 && c.str[j-1][i] == '*') index |= 8; // up
            if (j < (SIZE-1) && c.str[j+1][i] == '*') index |= 4; // down
            if (i >  0 && c.str[j][i-1] == '*') index |= 2; // left
            if (i < (SIZE-1) && c.str[j][i+1] == '*') index |= 1; // right

            if (index == 0 && (i == 0 || i == (SIZE-1))) index |= 3;
            if (index == 0 && (j == 0 || j == (SIZE-1))) index |= 12;

            ret.str[j][i] = linechars[index];
         }
      }
   }

   return ret;
}

Map fixsingles(Map a, Map b) {
   for (int y = 0; y < SIZE; y++) {
      for (int x = 0; x < SIZE; x++) {
         if (a.str[y][x] == linechars[0]) {
            a.str[y][x] = b.str[y][x];
         }
      }
   }
   return a;
}

// SIGWINCH handler
void sigwinch_handler(int) {
   printf("\033[18t");
}

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

               portal_x = screen_w * 3 / 4;
               portal_y = screen_h - 3;
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

// our entry point
int main(int argc, char **argv) {
   int16_t x = argc > 1 ? atoi(argv[1]) : 1;
   int16_t y = argc > 2 ? atoi(argv[2]) : 1;

   // assure we don't start in a wall
   x = (x & ~1) + 1;
   y = (y & ~1) + 1;

   // initialize our obstruction maps
   init_orders();
   init_obstructs();

   samem = sa_new();

   signal(SIGWINCH, sigwinch_handler);
   unbuffer();
   printf("\033[18t"); // get window size

   while (1) {
      clear();
      cprintf(COLOR_BLACK, COLOR_BRIGHT_CYAN,
              "%d,%d [%d,%d] %08x %s\n",
              x, y, screen_w, screen_h, xor, see ? "see" : "!see");

      Map drawme = do_map(x,y);
      Map singles = wallify(drawme);

      if (!raw) {
         if (see) {
            drawme = sight(drawme);
         }

         drawme = wallify(drawme);
         drawme = fixsingles(drawme, singles);
      }

      int nx;
      int ny;

      for (int j = 0; j < portal_y; j++) {
         int y = j + (SIZE - portal_y) / 2;
         for (int i = 0; i < portal_x; i++) {
            int x = i + (SIZE - portal_x) / 2;

            if (drawme.str[y][x] == '@') {
               nx = x;
               ny = y;
               if (phase) {
                  drawme.str[y][x] = 'X';
               }
            }
            utf8printchar(COLOR_BRIGHT_WHITE, COLOR_BLACK, drawme.str[y][x]);
         }
         if (j < sizeof(help) / sizeof(help[0])) {
            cprintf(COLOR_WHITE, COLOR_BLACK,"   %s\n", help[j]);
         }
         else {
            printf("\n");
         }
      }

      int dx = 0;
      int dy = 0;

      int u = getchar();
      switch(u) {
         case 'k': dy--; break;
         case 'j': dy++; break;
         case 'h': dx--; break;
         case 'l': dx++; break;
         case 'y': dy--; dx--; break;
         case 'u': dy--; dx++; break;
         case 'b': dy++; dx--; break;
         case 'n': dy++; dx++; break;

         case 's': see = !see; break;
         case 'p': phase = !phase; break;
         case 'r': raw = !raw; break;

         case 'a': xor++; break;
         case 'z': xor--; break;

         case 'q': exit(0); break;

         // escape
         case 0x1B: getwinch(); break;
      }

      if (drawme.str[ny+dy][nx+dx] == ' ' || phase) {
         x += dx;
         y += dy;
      }
   }
}
