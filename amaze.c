#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

// in screen help
const char *help[9] = {
   "Procedural maze demo.",
   "",
   "movement:",
   "y k u",
   "h * l",
   "b j n",
   "",
   "s toggle sightlines",
   "q to quit",
};

// maze is generated in 16x16 chunks

// this algorithm is a variation of the "Tessellation algorithm" at
// https://en.wikipedia.org/wiki/Maze_generation_algorithm

// this is the chunk template
const char *template[16] = {
   "V***************",
   "* * * * * * * * ",
   "**A***B***C***D*",
   "* * * * * * * * ",
   "****Q*******R***",
   "* * * * * * * * ",
   "**E***F***G***H*",
   "* * * * * * * * ",
   "********U*******",
   "* * * * * * * * ",
   "**I***J***K***L*",
   "* * * * * * * * ",
   "****S*******T***",
   "* * * * * * * * ",
   "**M***N***O***P*",
   "* * * * * * * * "
};

// a struct to hold the chunk map
typedef struct Chunk {
   int chunk[16][17];
} Chunk;

// to keep things random, we dole out
// random numbers a bit at a time
int mrand_bits;
long mrand_data;
int mrand(int x) {
   if (mrand_bits == 0) {
      mrand_bits = sizeof(int) * 8;
      mrand_data = rand();
   }
   int ret = 0;
   int bits = 0;
   switch(x) {
      case 8:
         bits = 3;
         break;
      case 4:
         bits = 2;
         break;
      case 2:
         bits = 1;
         break;
   }
   for (int i = 0; i < bits; i++) {
      ret <<= 1;
      ret |= mrand_data & 1;
      mrand_data >>= 1;
      mrand_bits--;
      if (mrand_bits == 0) {
         mrand_bits = sizeof(int) * 8;
         mrand_data = rand();
      }
   }
   return ret;
}

// call this to seed the random number generator
void smrand(int seed) {
   mrand_bits = 0;
   mrand_data = 0;
   srand(seed);
}

// generate a chunk based on x and y position in the maze
Chunk do_chunk(unsigned int x, unsigned int y, bool at) {
   Chunk ret;

   unsigned int bx = x / 16;
   unsigned int by = y / 16;

   // here's the secret sauce
   // seed the random number generator based on location
   // throw some prime numbers in there too why don'tcha
   smrand(bx * 7 + by * 5);

   for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 16; j++) {
         ret.chunk[i][j] = template[i][j];
      }
      ret.chunk[i][16] = 0;
   }

   for (int u = 0; u < 16; u++) {
      for (int v = 0; v < 16; v++) {
         if (ret.chunk[v][u] >= 'A' && ret.chunk[v][u] <= 'Z') {
            if (ret.chunk[v][u] < 'Q') {
               switch (mrand(4)) {
                  case 0:
                     ret.chunk[v-1][u] = ' ';
                     ret.chunk[v+1][u] = ' ';
                     ret.chunk[v][u+1] = ' ';
                     break;
                  case 1:
                     ret.chunk[v-1][u] = ' ';
                     ret.chunk[v+1][u] = ' ';
                     ret.chunk[v][u-1] = ' ';
                     break;
                  case 2:
                     ret.chunk[v][u+1] = ' ';
                     ret.chunk[v][u-1] = ' ';
                     ret.chunk[v-1][u] = ' ';
                     break;
                  case 3:
                     ret.chunk[v][u+1] = ' ';
                     ret.chunk[v][u-1] = ' ';
                     ret.chunk[v+1][u] = ' ';
                     break;
               }
            }
            else if (ret.chunk[v][u] < 'U') {
               switch (mrand(4)) {
                  case 0:
                     ret.chunk[v-(1+2*mrand(2))][u] = ' ';
                     ret.chunk[v+(1+2*mrand(2))][u] = ' ';
                     ret.chunk[v][u+(1+2*mrand(2))] = ' ';
                     break;
                  case 1:
                     ret.chunk[v-(1+2*mrand(2))][u] = ' ';
                     ret.chunk[v+(1+2*mrand(2))][u] = ' ';
                     ret.chunk[v][u-(1+2*mrand(2))] = ' ';
                     break;
                  case 2:
                     ret.chunk[v][u+(1+2*mrand(2))] = ' ';
                     ret.chunk[v][u-(1+2*mrand(2))] = ' ';
                     ret.chunk[v-(1+2*mrand(2))][u] = ' ';
                     break;
                  case 3:
                     ret.chunk[v][u+(1+2*mrand(2))] = ' ';
                     ret.chunk[v][u-(1+2*mrand(2))] = ' ';
                     ret.chunk[v+(1+2*mrand(2))][u] = ' ';
                     break;
               }
            }
            else if (ret.chunk[v][u] < 'V') {
               switch (mrand(4)) {
                  case 0:
                     ret.chunk[v-(1+2*mrand(4))][u] = ' ';
                     ret.chunk[v+(1+2*mrand(4))][u] = ' ';
                     ret.chunk[v][u+(1+2*mrand(4))] = ' ';
                     break;
                  case 1:
                     ret.chunk[v-(1+2*mrand(4))][u] = ' ';
                     ret.chunk[v+(1+2*mrand(4))][u] = ' ';
                     ret.chunk[v][u-(1+2*mrand(4))] = ' ';
                     break;
                  case 2:
                     ret.chunk[v][u+(1+2*mrand(4))] = ' ';
                     ret.chunk[v][u-(1+2*mrand(4))] = ' ';
                     ret.chunk[v-(1+2*mrand(4))][u] = ' ';
                     break;
                  case 3:
                     ret.chunk[v][u+(1+2*mrand(4))] = ' ';
                     ret.chunk[v][u-(1+2*mrand(4))] = ' ';
                     ret.chunk[v+(1+2*mrand(4))][u] = ' ';
                     break;
               }
            }
            else { // 'V'
               ret.chunk[v][u+(1+2*mrand(8))] = ' ';
               ret.chunk[v+(1+2*mrand(8))][u] = ' ';
            }
            ret.chunk[v][u] = '*';
         }
      }
   }
   if (at) {
      ret.chunk[(y % 16) & 0x0F][(x % 16) & 0x0F] = '@';
   }
   return ret;
}

// paste together 4 chunks to center on x,y
Chunk four(Chunk a, Chunk b,
                  Chunk c, Chunk d,
                  unsigned int x, unsigned int y) {
   Chunk ret;

   for (int j = 0; j < 16; j++) {
      int dy = y - 8 + j;
      for (int i = 0; i < 16; i++) {
         int dx = x - 8 + i;

         if (dx < 16) {
            if (dy < 16) {
               ret.chunk[j][i] = a.chunk[dy][dx];
            }
            else {
               ret.chunk[j][i] = c.chunk[dy - 16][dx];
            }
         }
         else {
            if (dy < 16) {
               ret.chunk[j][i] = b.chunk[dy][dx - 16];
            }
            else {
               ret.chunk[j][i] = d.chunk[dy - 16][dx - 16];
            }
         }
      }
   }

   for (int i = 0; i < 16; i++) {
      ret.chunk[i][16] = 0;
   }

   return ret;
}

// generate a drawable section based on x and y
Chunk draw(unsigned int x, unsigned int y) {
   Chunk here = do_chunk(x, y, true);

   if ((x % 16) <= 7) {
      Chunk left = do_chunk(x - 16, y, false);
      if ((y % 16) <= 7) {
         Chunk up = do_chunk(x, y - 16, false);
         Chunk upleft = do_chunk(x - 16, y - 16, false);
         return four(upleft, up, left, here, 16 + (x % 16), 16 + (y % 16));
      }
      else { // (y % 16) >= 8
         Chunk down = do_chunk(x, y + 16, false);
         Chunk downleft = do_chunk(x - 16, y + 16, false);
         return four(left, here, downleft, down, 16 + (x % 16), (y % 16));
      }
   }
   else { // (x % 16) >= 8
      Chunk right = do_chunk(x + 16, y, false);
      if ((y % 16) <= 7) {
         Chunk up = do_chunk(x, y - 16, false);
         Chunk upright = do_chunk(x + 16, y - 16, false);
         return four(up, upright, here, right, (x % 16), 16 + (y % 16));
      }
      else { // (y % 16) >= 8
         Chunk down = do_chunk(x, y + 16, false);
         Chunk downright = do_chunk(x + 16, y + 16, false);
         return four(here, right, down, downright, (x % 16), (y % 16));
      }
   }
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
   int dy;
   int dx;
   int d2;
   bool seen;
   double theta;
} Obst;

#define ORDERED(a,b,c) ((a) <= (b) && (b) <= (c))

Chunk sight(Chunk in) {
   Obst obst[256];
   int spot = 0;

   int atx, aty;

   // find @
   for (int y = 0; y < 16; y++) {
      for (int x = 0; x < 16; x++) {
         if (in.chunk[y][x] == '@') {
            atx = x;
            aty = y;
         }
      }
   }

   // populate obst
   for (int y = 0; y < 16; y++) {
      for (int x = 0; x < 16; x++) {
         if (in.chunk[y][x] != ' ' && in.chunk[y][x] != '@') {
            obst[spot].x = x;
            obst[spot].y = y;
            int dx = obst[spot].dx = x - atx;
            int dy = obst[spot].dy = y - aty;
            obst[spot].theta = atan2(dy, dx) + M_PI; // 0 to 2*pi
            obst[spot].d2 = dx * dx + dy * dy;
            obst[spot].seen = true;
            spot++;
         }
      }
   }

   // sort obst
   for (int i = 0; i < spot; i++) {
      for (int j = i + 1; j < spot; j++) {
         if (obst[j].d2 < obst[i].d2) {
            Obst tmp;
            memcpy(&tmp, obst + i, sizeof(tmp));
            memcpy(obst + i, obst + j, sizeof(tmp));
            memcpy(obst + j, &tmp, sizeof(tmp));
         }
      }
   }

   // find adjacent pairs of obst
   for (int i = 0; i < spot; i++) {
      for (int j = i + 1; j < spot; j++) {
         int dx = obst[i].x - obst[j].x;
         int dy = obst[i].y - obst[j].y;
         int d2 = dx * dx + dy * dy;
         if (d2 <= 1) {
            // i and j are adjacent
            for (int k = 0; k < spot; k++) {
               if (k != i && k != j &&
                   obst[k].d2 >= obst[i].d2 &&
                   obst[k].d2 >= obst[j].d2) {
                  double ij = fabs(obst[i].theta - obst[j].theta);
                  if (ij > M_PI) { ij = 2.0 * M_PI - ij; }
                  double ik = fabs(obst[i].theta - obst[k].theta);
                  if (ik > M_PI) { ik = 2.0 * M_PI - ik; }
                  double jk = fabs(obst[j].theta - obst[k].theta);
                  if (jk > M_PI) { jk = 2.0 * M_PI - jk; }

                  ik += jk;

                  if (!((ik - ij) > DBL_EPSILON)) {
#if 0
printf("%d,%d :: %d,%d,%d obscured by %d,%d,%d and %d,%d,%d\n",
atx, aty,
obst[k].x, obst[k].y, obst[k].d2,
obst[i].x, obst[i].y, obst[i].d2,
obst[j].x, obst[j].y, obst[j].d2);
#endif
                     in.chunk[obst[k].y][obst[k].x] = ' ';
                  }
               }
            }
         }
      }
   }

   return in;
}

Chunk wallify(Chunk c) {
   Chunk ret;
   for (int j = 0; j < 16; j++) {
      for (int i = 0; i < 16; i++) {
         if (c.chunk[j][i] != '*') {
            ret.chunk[j][i] = c.chunk[j][i];
         }
         else {
            int index = 0;
            if (j >  0 && c.chunk[j-1][i] == '*') index |= 8; // up
            if (j < 15 && c.chunk[j+1][i] == '*') index |= 4; // down
            if (i >  0 && c.chunk[j][i-1] == '*') index |= 2; // left
            if (i < 15 && c.chunk[j][i+1] == '*') index |= 1; // right

            if (index == 0 && (i == 0 || i == 15)) index |= 3;
            if (index == 0 && (j == 0 || j == 15)) index |= 12;
            ret.chunk[j][i] = linechars[index];
         }
      }
   }

   return ret;
}

// our entry point
int main(int argc, char **argv) {
   bool see = false;
   unsigned int x = argc > 1 ? atoi(argv[1]) : 1;
   unsigned int y = argc > 2 ? atoi(argv[2]) : 1;

   unbuffer();

   while (1) {
      int nx;
      int ny;

      clear();
      printf("%d,%d\n", x, y);

      Chunk drawme = wallify(draw(x,y));

      if (see) {
         drawme = sight(drawme);
      }

      for (int j = 0; j < 16; j++) {
         for (int i = 0; i < 16; i++) {
            if (drawme.chunk[j][i] == '@') {
               nx = i;
               ny = j;
            }
            utf8print(drawme.chunk[j][i]);
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

         case 'q': exit(0); break;
      }

      if (drawme.chunk[ny+dy][nx+dx] == ' ') {
         x += dx;
         y += dy;
      }
   }
}
