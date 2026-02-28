#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "object.h"
#include "sparse_array.h"
#include "sparse_chars.h"
#include "ansi.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool show_raw = false;
bool use_sightlines = true;
int sight_dist2 = 8; // sight distance squared
bool phase = false;
int xor = 0; //0xdeadbeef;

// in screen help
static const char *help[16] = {
   "Procedural maze demo.",
   "",
   "movement:",
   "y k u",
   "h * l",
   "b j n",
   "",
   ", take an object",
   "d drop an object",
   "i list inventory",
   "",
   "s toggle sightlines",
   "p toggle phase",
   "r toggle raw",
   "a/z inc/dec xor",
   "q to quit",
};
static int help_width = -1;

static char message[80] = { 0 };

static void help_init(void) {
   for (size_t i = 0; i < sizeof(help) / sizeof(help[0]); i++) {
      int len = strlen(help[i]);
      if (len > help_width) {
         help_width = len;
      }
   }
}

// very loosely based on Telengard

// we compute a slightly larger map to avoid artifacts
// at view portal edges

#define SIZE 128     // size of computed map

// player visibility into map
#define portal_x (screen_w - help_width - 3)
#define portal_y (screen_h - 4)

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
static void init_obstructs(void) {
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

static int cmp_range(const void *a, const void *b)
{
   const Range *A = a, *B = b;
   return (A->lo < B->lo) ? -1 : (A->lo > B->lo);
}

static size_t merge_ranges(Range *ranges, size_t n)
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

static int cmp_orders(const void *a, const void *b) {
   const Order *oa = a;
   const Order *ob = b;

   if (oa->d2 < ob->d2) return -1;
   if (oa->d2 > ob->d2) return  1;
   return 0;
}

static void init_orders(void) {
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

static int posrand(int y, int x) {
   // Double, double toil and trouble...
   int seed = x & 0xFFFF;
   seed <<= 16;
   seed += y & 0xFFFF;
   seed ^= xor;
   srand(seed);

   return rand();
}

static Map repair_voids(Map ret) {
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

static Map repair_pillars(Map ret) {
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

// generate a raw map based on player position
static Map do_raw(int x, int y) {
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

static Map do_mem(int x, int y) {
   Map ret;
   memset(&ret, 0, sizeof(ret));
   ret.offset_x = x - SIZE/2;
   ret.offset_y = y - SIZE/2;

   for (y = 0; y < SIZE; y++) {
      for (x = 0; x < SIZE; x++) {
         if (sa_get(ret.offset_y + y, ret.offset_x + x)) {
            ret.str[y][x] = ' ';
         }
      }
   }
   return ret;
}

// line drawing characters
static int linechars[16] = {
   // udlr
   0x25AA,  // 0000 // a square
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

static inline int sign(int x) {
   return (x > 0) - (x < 0);
}

static bool obscured(Map *mask, Map *in, int y, int x, int *count, Range **ranges) {
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

static Map do_sight(Map in) {
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
      int dy = abs(y - aty);
      for (int x = 0; x < SIZE; x++) {
         int dx = abs(x - atx);

         if ((dy*dy+dx*dx) > (sight_dist2)) {
            mask.str[y][x] = 0;
         }

         if (mask.str[y][x]) {
            sa_set(mask.offset_y + y, mask.offset_x + x);
         }
      }
   }

   free(ranges);

   return mask;
}

static Map do_walls(Map raw, Map mem) {
   Map ret;
   ret.offset_y = raw.offset_y;
   ret.offset_x = raw.offset_x;
   for (int j = 0; j < SIZE; j++) {
      for (int i = 0; i < SIZE; i++) {
         if (raw.str[j][i] != '*') {
            ret.str[j][i] = raw.str[j][i];
         }
         else {
            int index = 0;
            if (j >  0       && raw.str[j-1][i] == '*' && mem.str[j-1][i]) index |= 8; // up
            if (j < (SIZE-1) && raw.str[j+1][i] == '*' && mem.str[j+1][i]) index |= 4; // down
            if (i >  0       && raw.str[j][i-1] == '*' && mem.str[j][i-1]) index |= 2; // left
            if (i < (SIZE-1) && raw.str[j][i+1] == '*' && mem.str[j][i+1]) index |= 1; // right

            if (index == 0 && (i == 0 || i == (SIZE-1))) index |= 3;
            if (index == 0 && (j == 0 || j == (SIZE-1))) index |= 12;

            ret.str[j][i] = linechars[index];
         }
      }
   }

   return ret;
}

static char tag2char(uint8_t tag) {
   if (tag < 26) {
      return 'a' + tag;
   }
   else if (tag < 52) {
      return 'A' + (tag - 26);
   }
   else {
      return '#' + (tag - 52);
   }
}

static uint8_t char2tag(char c) {
   if (c >= 'a' && c <= 'z') {
      return c - 'a';
   }
   else if (c >= 'A' && c <= 'Z') {
      return c - 'A' + 26;
   }
   else if (c >= '#' && c <= '&') {
      return c - '#' + 52;
   }
   else {
      return -1;
   }
}

static int inv_cmp(const void *a, const void *b) {
   const Object * const *oa = (const Object * const *) a;
   const Object * const *ob = (const Object * const *) b;

   if ((*oa)->type < (*ob)->type) {
      return -1;
   }
   else if ((*oa)->type > (*ob)->type) {
      return 1;
   }
   else if ((*oa)->tag < (*ob)->tag) {
      return -1;
   }
   else if ((*oa)->tag > (*ob)->tag) {
      return 1;
   }
   else {
      return 0;
   }
}

static Object *inventory(const char *prompt, int mask, bool is_inv, Object *start) {
   Object *inv[64 + 16] = { 0 };
   int8_t spottags[64 + 16];
   int type = -1;
   int end = 0;

   // populate inv array, so we can sort it later
   Object *tmp = start;
   if (is_inv) {
      while (tmp) {
         if (mask & (1 << tmp->type)) {
            inv[end++] = tmp;
         }
         tmp = tmp->link;
      }
   }
   else if (start) {
      do {
         if (mask & (1 << tmp->type)) {
            inv[end++] = tmp;
            // TODO FIX handle cases with too many objects
         }
         tmp = tmp->next;
      } while (tmp != start);
   }

   // nothing added ???
   if (end == 0) {
      if (is_inv) {
         sprintf(message, "No inventory!");
      }
      else {
         sprintf(message, "Nothing here!");
      }
      return NULL;
   }

   // sort it
   qsort(inv, end, sizeof(Object *), inv_cmp);

   // add gaps...
   for (int i = 0; i < end; i++) {
      if (inv[i]->type != type) {
         type = inv[i]->type;
         memmove(inv + i + 1, inv + i, sizeof(Object *) * (end - i));
         inv[i] = NULL;
         end++;
      }
   }

   // for !is_inv, populate spottags
   if (!is_inv) {
      int8_t spottag = 0;
      for (int i = 0; i < end; i++) {
         if (inv[i] != NULL) {
            spottags[i] = spottag;
            spottag++;
            // TODO FIX handle cases with too many objects
         }
         else {
            spottags[i] = -1;
         }
      }
   }

   // draw selection screen
   clear();
   cprintf(COLOR_BRIGHT_WHITE, COLOR_BLACK, "%s >>\n\n", prompt);

   int cols;

   if (end < screen_h - 3) {
      cols = 1;
   }
   else if (end / 2 < screen_h - 3) {
      cols = 2;
   }
   else {
      cols = 3;
   }

   for (int i = 0; i < (end + cols - 1) / cols; i++) {
      for (int j = 0; j < cols; j++) {
         int spot = j * (end + cols - 1) / cols + i;
         if (inv[spot] == NULL && inv[spot + 1] != NULL) {
            cprintf(COLOR_BRIGHT_WHITE, COLOR_BLACK, " %-*.*s",
                    screen_w / cols - 2,
                    screen_w / cols - 2,
                    obj_type_names[(int)inv[spot + 1]->type]);
         }
         else if (inv[spot]) {
            cprintf(COLOR_BRIGHT_WHITE, COLOR_BLACK, "  %c) ",
                    is_inv ? tag2char(inv[spot]->tag) : tag2char(spottags[spot]));
            utf8printchar(inv[spot]->fg, inv[spot]->bg, inv[spot]->unicode);
            cprintf(COLOR_BRIGHT_WHITE, COLOR_BLACK, " %-*.*s",
                    screen_w / cols - 8,
                    screen_w / cols - 8,
                    inv[spot]->name);
         }
         else {
            cprintf(COLOR_BRIGHT_WHITE, COLOR_BLACK, "%-*.*s",
                    screen_w / cols - 1,
                    screen_w / cols - 1,
                    "");
         }
      }
      cprintf(COLOR_BRIGHT_WHITE, COLOR_BLACK, "\n");
   }

   int u = getchar();

   int tag = char2tag(u);

   if (tag != -1) {
      for (int i = 0; i < end; i++) {
         if (inv[i] && ((is_inv && inv[i]->tag == tag) || (!is_inv && spottags[i] == tag))) {
            return inv[i];
         }
      }
   }

   return NULL;
}

static void drop(int16_t y, int16_t x) {
   Object *obj = obj_get_inv();

   message[0] = 0;
   obj = inventory("Drop what?", OBJ_ANY_MASK, true, obj_get_inv());

   if (obj) {
      obj_drop(obj, y, x);
   }
   else {
      if (!message[0]) {
         sprintf(message, "drop cancelled");
      }
   }
}

static void take(int16_t y, int16_t x) {
   message[0] = 0;
   Object *obj = obj_get(y,x);

   if (obj->next != obj) {
      obj = inventory("Take what?", OBJ_ANY_MASK, false, obj);
   }

   if (obj) {
      obj_take(obj);
      sprintf(message, "taken (%c) %s", tag2char(obj->tag), obj->name);
   }
   else {
      if (!message[0]) {
         sprintf(message, "Nothing here!");
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

   sa_reset();

   obj_init();

   help_init();

   ansi_init();

   while (1) {
      clear();
      cprintf(COLOR_BLACK, COLOR_BRIGHT_CYAN,
              "%d,%d [%d,%d] %08x %s\n",
              x, y, screen_w, screen_h, xor, use_sightlines ? "sightlines" : "!sightlines");

      Map raw    = do_raw(x,y);
      Map seen   = do_sight(raw);
      Map memory = do_mem(x,y);
      Map drawme = do_walls(raw, memory);

      if (show_raw) {
         memcpy(&drawme, &raw, sizeof(raw));
      }

      int nx;
      int ny;
      Object *nobj = NULL;

      for (int j = 0; j < portal_y; j++) {
         int sy = j + (SIZE - portal_y) / 2;
         for (int i = 0; i < portal_x; i++) {
            int sx = i + (SIZE - portal_x) / 2;

            if (drawme.str[sy][sx] == '@') {
               nx = sx;
               ny = sy;
               if (phase) {
                  drawme.str[sy][sx] = 'X';
               }
               nobj = obj_get(drawme.offset_y + sy, drawme.offset_x + sx);
            }

            if (seen.str[sy][sx]) {
               sc_clr(drawme.offset_y + sy, drawme.offset_x + sx);
               if (drawme.str[sy][sx] != ' ') {
                  utf8printchar(COLOR_BRIGHT_WHITE, COLOR_BLACK, drawme.str[sy][sx]);
               }
               else {
                  Object *obj = obj_get(drawme.offset_y + sy, drawme.offset_x + sx);
                  if (obj) {
                     utf8printchar(obj->fg, obj->bg, obj->unicode);
                     sc_set(drawme.offset_y + sy, drawme.offset_x + sx, obj->unicode);
                  }
                  else {
                     utf8printchar(COLOR_BRIGHT_WHITE, COLOR_BLACK, 0xB7);
                  }
               }
            }
            else if (memory.str[sy][sx]) {
               if (drawme.str[sy][sx] != ' ') {
                  utf8printchar(COLOR_BRIGHT_BLACK, COLOR_BLACK, drawme.str[sy][sx]);
               }
               else {
                  uint32_t unicode = sc_get(drawme.offset_y + sy, drawme.offset_x + sx);
                  if (unicode) {
                     utf8printchar(COLOR_BLACK, COLOR_BRIGHT_BLACK, unicode);
                  }
                  else {
                     utf8printchar(COLOR_BRIGHT_BLACK, COLOR_BLACK, 0xB7);
                  }
               }
            }
            else {
               if (use_sightlines) {
                  utf8printchar(COLOR_WHITE, COLOR_BLACK, ' ');
               }
               else {
                  Object *obj = obj_get(drawme.offset_y + sy, drawme.offset_x + sx);
                  if (obj) {
                     utf8printchar(obj->fg, obj->bg, obj->unicode);
                  }
                  else {
                     utf8printchar(COLOR_WHITE, COLOR_BLACK, drawme.str[sy][sx]);
                  }
               }
            }
         }

         // do "help"
         if ((size_t) j < sizeof(help) / sizeof(help[0])) {
            cprintf(COLOR_WHITE, COLOR_BLACK,"  %s\n", help[j]);
         }
         else {
            printf("\n");
         }
      }
      if (nobj) {
         Object *ptr = nobj;
         if (nobj->next != nobj) {
            cprintf(COLOR_WHITE, COLOR_BLACK, "There are many objects here.\n");
         }
         else {
            cprintf(ptr->fg, ptr->bg, "%s", ptr->name);
            cprintf(COLOR_WHITE, COLOR_BLACK, "\n");
         }
      }
      if (message[0]) {
         cprintf(COLOR_WHITE, COLOR_BLACK, "%s\n", message);
      }
      //sc_debug();

      int dx = 0;
      int dy = 0;

      int u = getchar();
      switch(u) {
         case 'k': dy--; message[0] = 0; break;
         case 'j': dy++; message[0] = 0; break;
         case 'h': dx--; message[0] = 0; break;
         case 'l': dx++; message[0] = 0; break;
         case 'y': dy--; dx--; message[0] = 0; break;
         case 'u': dy--; dx++; message[0] = 0; break;
         case 'b': dy++; dx--; message[0] = 0; break;
         case 'n': dy++; dx++; message[0] = 0; break;

         case 'i': inventory("inventory", OBJ_ANY_MASK, true, obj_get_inv()); break;

         case ',': take(y, x); break;
         case 'd': drop(y, x); break;

         case 's': use_sightlines = !use_sightlines; break;
         case 'p': phase = !phase; break;
         case 'r': show_raw = !show_raw; break;

         case 'a': xor++; sa_reset(); break;
         case 'z': xor--; sa_reset(); break;

         case 'q': clear(); exit(0); break;

         // escape
         case 0x1B: getwinch(); break;
      }

      if (drawme.str[ny+dy][nx+dx] == ' ' || phase) {
         x += dx;
         y += dy;
      }
   }
}
