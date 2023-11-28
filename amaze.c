#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

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

struct Chunk {
   int chunk[16][17];
};

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

void smrand(int seed) {
   mrand_bits = 0;
   mrand_data = 0;
   srand(seed);
}

struct Chunk do_chunk(unsigned int x, unsigned int y, bool at) {
   struct Chunk ret;

   unsigned int bx = x / 16;
   unsigned int by = y / 16;

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

struct Chunk four(struct Chunk a, struct Chunk b,
                  struct Chunk c, struct Chunk d,
                  unsigned int x, unsigned int y) {
   struct Chunk ret;

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

struct Chunk draw(unsigned int x, unsigned int y) {
   struct Chunk here = do_chunk(x, y, true);

   if ((x % 16) <= 7) {
      struct Chunk left = do_chunk(x - 16, y, false);
      if ((y % 16) <= 7) {
         struct Chunk up = do_chunk(x, y - 16, false);
         struct Chunk upleft = do_chunk(x - 16, y - 16, false);
         return four(upleft, up, left, here, 16 + (x % 16), 16 + (y % 16));
      }
      else { // (y % 16) >= 8
         struct Chunk down = do_chunk(x, y + 16, false);
         struct Chunk downleft = do_chunk(x - 16, y + 16, false);
         return four(left, here, downleft, down, 16 + (x % 16), (y % 16));
      }
   }
   else { // (x % 16) >= 8
      struct Chunk right = do_chunk(x + 16, y, false);
      if ((y % 16) <= 7) {
         struct Chunk up = do_chunk(x, y - 16, false);
         struct Chunk upright = do_chunk(x + 16, y - 16, false);
         return four(up, upright, here, right, (x % 16), 16 + (y % 16));
      }
      else { // (y % 16) >= 8
         struct Chunk down = do_chunk(x, y + 16, false);
         struct Chunk downright = do_chunk(x + 16, y + 16, false);
         return four(here, right, down, downright, (x % 16), (y % 16));
      }
   }
}

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

void clear(void) {
   printf("\033[2J\033[H");
}

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

int main(int argc, char **argv) {
   unsigned int x = atoi(argv[1]);
   unsigned int y = atoi(argv[2]);

   unbuffer();

   while (1) {
      int nx;
      int ny;

      clear();
      printf("%d,%d\n", x, y);
      struct Chunk drawme = draw(x,y);
      for (int j = 0; j < 16; j++) {
         for (int i = 0; i < 16; i++) {
            if (drawme.chunk[j][i] != '*') {
               printf("%c", drawme.chunk[j][i]);
               if (drawme.chunk[j][i] == '@') {
                  ny = j;
                  nx = i;
               }
            }
            else {
               int index = 0;
               if (j >  0 && drawme.chunk[j-1][i] == '*') index |= 8; // up
               if (j < 15 && drawme.chunk[j+1][i] == '*') index |= 4; // down
               if (i >  0 && drawme.chunk[j][i-1] == '*') index |= 2; // left
               if (i < 15 && drawme.chunk[j][i+1] == '*') index |= 1; // right

               if (index == 0 && (i == 0 || i == 15)) index |= 3;
               if (index == 0 && (j == 0 || j == 15)) index |= 12;
               utf8print(linechars[index]);
            }
         }
         printf("\n");
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

         case 'q': exit(0); break;
      }

      if (drawme.chunk[ny+dy][nx+dx] == ' ') {
         x += dx;
         y += dy;
      }
   }
}
