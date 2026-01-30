#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

bool raw = false;
bool see = false;
bool phase = false;

// in screen help
const char *help[11] = {
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
   "q to quit",
};

// based on Telengard

// we compute a slightly larger map to avoid artifacts
// at view portal edges

#define SIZE 80     // size of computed map
#define PORTAL_X 64 // player visibility into map
#define PORTAL_Y 20 // player visibility into map

// a struct to hold the map
typedef struct Map {
   int str[SIZE][SIZE+1];
} Map;

Map repair_voids(Map ret, int offset_x, int offset_y) {
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
            int typ = (offset_x + ix) & 1;
            typ <<= 1;
            typ |= (offset_y + iy) & 1;

            if (typ == 3 && ret.str[iy][ix] == ' ' && !visited.str[iy][ix]) {
               // we need a repair here!

               int seed = (offset_x + ix) & 0xFFFF;
               seed <<= 16;
               seed += (offset_y + iy) & 0xFFFF;
               seed ^= 0xdeadbeef;
               srand(seed);

               int tmp = rand();

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

Map repair_pillars(Map ret, int offset_x, int offset_y) {
   Map pillars;
   int count = -1;
   
   if(1) { //while (count != 0) {
      memset(&pillars, 0, sizeof(pillars));
      count = 0;

#define FIND \
      for (int iy = 1; iy < SIZE - 1; iy++) { \
         for (int ix = 1; ix < SIZE - 1; ix++) { \
            if (ret.str[iy][ix] == '*' && \
                  ret.str[iy-1][ix] == ' ' && \
                  ret.str[iy+1][ix] == ' ' && \
                  ret.str[iy][ix-1] == ' ' && \
                  ret.str[iy][ix+1] == ' ') { \
               pillars.str[iy][ix] = ' '; \
               count++; \
            } \
         } \
      }

      // find em
      FIND;

      // bridge adjacent pillars
      for (int iy = 2; iy < SIZE; iy++) {
         for (int ix = 2; ix < SIZE; ix++) {
            if (pillars.str[iy][ix] && pillars.str[iy-2][ix]) {
               ret.str[iy-1][ix] = '*';
            }
            if (pillars.str[iy][ix] && pillars.str[iy][ix-2]) {
               ret.str[iy][ix-1] = '*';
            }
         }
      }

      // find em
      FIND;

      // connect randomly
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
            }
         }
      }

      // find em
      FIND;
   }

   return ret;
}

// generate a map based on player position
Map do_map(int x, int y) {
   Map ret;
   int offset_x = x - SIZE/2;
   int offset_y = y - SIZE/2;

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
         int typ = (offset_x + ix) & 1;
         typ <<= 1;
         typ |= (offset_y + iy) & 1;

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
                  // Double, double toil and trouble...
                  int seed = (offset_x + ix) & 0xFFFF;
                  seed <<= 16;
                  seed += (offset_y + iy) & 0xFFFF;
                  seed ^= 0xdeadbeef;
                  srand(seed);

                  int tmp = rand();
                  if ((tmp & 1) && (iy > 0)) {
                     // wall up
                     ret.str[iy - 1][ix] = '*';
                  }
                  if ((tmp & 2) && (ix > 0)) {
                     // wall left
                     ret.str[iy][ix - 1] = '*';
                  }
               }
               break;
         }
      }
   }

   // join pillars
   ret = repair_pillars(ret, offset_x, offset_y);

   // force reachability
   ret = repair_voids(ret, offset_x, offset_y);

   ret = repair_pillars(ret, offset_x, offset_y);
   ret = repair_voids(ret, offset_x, offset_y);

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

// print a utf8 character
void utf8print(unsigned int x) {
   if (x <= 0x7F) {
      printf("%c", x);
   }
   else if (x < 0x800) {
      printf("%c%c", 0xC0 | (x >> 6), 0x80 | (x & 0x3F));
   }
   else if (x < 0x10000) {
      printf("%c%c%c", 0xE0 | (x >> 12), 0x80 | ((x >> 6) & 0x3F), 0x80 | (x & 0x3F));
   }
   else {
      printf("%c%c%c%c", 0xF0 | (x >> 18), 0x80 | ((x >> 12) & 0x3F), 0x80 | ((x >> 6) & 0x3F), 0x80 | (x & 0x3F));
   }
}

// clear the screen
void clear(void) {
   printf("\033[2J\033[H");
}

// set stdin as unbuffered
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
}

static inline int sign(int x) {
   return (x > 0) - (x < 0);
}

typedef struct Obst {
   int x;
   int y;
   int d2;
   double theta;
   bool visited;
   int hidden;
} Obst;

#define ORDERED(a,b,c) ((a) <= (b) && (b) <= (c))

void sight_helper(Obst *o, int size, int x, int y) {
   // update d2 and theta
   for (int i = 0; i < size; i++) {
      int dx = o[i].x - x;
      int dy = o[i].y - y;
      o[i].visited = false;
      o[i].d2 = dx * dx + dy * dy;
      o[i].theta = atan2(dy, dx) + M_PI; // range 0 to 2pi
   }
   // sort by d2
   for (int i = 0; i < size; i++) {
      for (int j = i + 1; j < size; j++) {
         if (o[j].d2 < o[i].d2) {
            Obst tmp;
            memcpy(&tmp, o + i, sizeof(tmp));
            memcpy(o + i, o + j, sizeof(tmp));
            memcpy(o + j, &tmp, sizeof(tmp));
         }
      }
   }
   // find adjacent pairs of obst
   for (int i = 0; i < size; i++) {
      for (int j = i + 1; j < size; j++) {
         int dx = o[i].x - o[j].x;
         int dy = o[i].y - o[j].y;
         int d2 = dx * dx + dy * dy;
         if (d2 <= 1) {
            // i and j are adjacent
            for (int k = 0; k < size; k++) {
               if (k != i && k != j &&
                   o[k].d2 >= o[i].d2 &&
                   o[k].d2 >= o[j].d2) {
                  double ij = fabs(o[i].theta - o[j].theta);
                  if (ij > M_PI) { ij = 2.0 * M_PI - ij; }
                  double ik = fabs(o[i].theta - o[k].theta);
                  if (ik > M_PI) { ik = 2.0 * M_PI - ik; }
                  double jk = fabs(o[j].theta - o[k].theta);
                  if (jk > M_PI) { jk = 2.0 * M_PI - jk; }

#ifdef DEBUG_SIGHT
//if (o[k].y == 7 && o[k].x == 0) {
   printf("=== %d(%d,%d)(%g) %d(%d,%d)(%g) %d(%d,%d)(%g)\n",
   i, o[i].x - x, o[i].y - y, o[i].theta,
   j, o[j].x - x, o[j].y - y, o[j].theta,
   k, o[k].x - x, o[k].y - y, o[k].theta);
   printf("ij=%g ik=%g jk=%g\n", ij, ik, jk);
//}
#endif
                  if (ik <= ij && jk <= ij) {
                     if (1) { //!o[k].visited) {
                        o[k].hidden++;
                        o[k].visited = true;
                     }
#ifdef DEBUG_SIGHT
printf("%d,%d :: %d,%d,%d obscured by %d,%d,%d and %d,%d,%d\n",
x, y,
o[k].x, o[k].y, o[k].d2,
o[i].x, o[i].y, o[i].d2,
o[j].x, o[j].y, o[j].d2);
#endif
                  }
               }
            }
         }
      }
   }
}

Map sight(Map in) {
   Obst obst[SIZE*SIZE];
   int spot = 0;

   int atx, aty;

   // find @
   for (int y = 0; y < SIZE; y++) {
      for (int x = 0; x < SIZE; x++) {
         if (in.str[y][x] == '@') {
            atx = x;
            aty = y;
         }
      }
   }

   // populate obst
   for (int y = 0; y < SIZE; y++) {
      for (int x = 0; x < SIZE; x++) {
         if (in.str[y][x] != ' ' && in.str[y][x] != '@') {
            obst[spot].x = x;
            obst[spot].y = y;
            obst[spot].hidden = 0;
            spot++;
         }
      }
   }

   sight_helper(obst, spot, atx, aty);

   // erase anything hidden hcalls times
   for (int i = 0; i < spot; i++) {
      if (obst[i].hidden) {
         in.str[obst[i].y][obst[i].x] = ' ';
      }
   }

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

// our entry point
int main(int argc, char **argv) {
   int x = argc > 1 ? atoi(argv[1]) : 1;
   int y = argc > 2 ? atoi(argv[2]) : 1;

   // assure we don't start in a wall
   x = (x & ~1) + 1;
   y = (y & ~1) + 1;

   unbuffer();

   while (1) {
      int nx;
      int ny;

      clear();
      printf("%d,%d\n", x, y);

      Map drawme = do_map(x,y);
      Map singles = wallify(drawme);

      if (!raw) {
         if (see) {
            drawme = sight(drawme);
         }

         drawme = wallify(drawme);
         drawme = fixsingles(drawme, singles);
      }

      for (int j = 0; j < PORTAL_Y; j++) {
         int y = j + (SIZE - PORTAL_Y) / 2;
         for (int i = 0; i < PORTAL_X; i++) {
            int x = i + (SIZE - PORTAL_X) / 2;

            if (drawme.str[y][x] == '@') {
               nx = x;
               ny = y;
               if (phase) {
                  drawme.str[y][x] = 'X';
               }
            }
            utf8print(drawme.str[y][x]);
         }
         if (j < sizeof(help) / sizeof(help[0])) {
            printf("   %s\n", help[j]);
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

         case 'q': exit(0); break;
      }

      if (drawme.str[ny+dy][nx+dx] == ' ' || phase) {
         x += dx;
         y += dy;
      }
   }
}
