/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992, 1993  William B. Ackerman.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    This is for version 28. */

/* This defines the following functions:
   mirror_this
   do_stability
   basic_move
*/

#include "sd.h"



/* This file uses a few bogus setups.  They are never allowed to escape:
   8x8 =
     0   1   2   3  28  24  20  16 
     4   5   6   7  29  25  21  17
     8   9  10  11  30  26  22  18
    12  13  14  15  31  27  23  19
    51  55  59  63  47  46  45  44
    50  54  58  62  43  42  41  40
    49  53  57  61  39  38  37  36
    48  52  56  60  35  34  33  32

   x4dmd =
      4        8       15
   7     5     9    14    12
      6       10       13
              11
   0  1  2  3    19 18 17 16
              27
     29       26       22
  28    30    25    21    23
     31       24       20

   x1x6 =
            3
            4
            5
   0  1  2     8  7  6
           11
           10
            9
*/


extern void mirror_this(setup *s)

{
   unsigned int l, r, z, n, t;
   coordrec *cptr;
   int i, x, y, place, limit;

   setup temp = *s;

   cptr = nice_setup_coords[s->kind];
   if (!cptr) fail("Don't recognize ending setup for this call; not able to do it mirror.");
   limit = setup_limits[s->kind];

   if (s->rotation & 1) {
      for (i=0; i<=limit; i++) {
         x = cptr->xca[i];
         y = - cptr->yca[i];
         place = cptr->diagram[28 - ((y >> 2) << cptr->xfactor) + (x >> 2)];

         if ((place < 0) || (cptr->xca[place] != x) || (cptr->yca[place] != y))
            fail("Don't recognize ending setup for this call, not able to do it mirror.");

         n = temp.people[i].id1;
         t = (- (n & (STABLE_VBIT*3))) & (STABLE_VBIT*3);
         l = (n & ROLLBITL) >> 2;
         r = (n & ROLLBITR) << 2;
         z = (n & ~(ROLLBITL | ROLLBITR | (STABLE_VBIT*3))) | l | r | t;
         s->people[place].id1 = (z & 010) ? (z ^ 2) : z;
         s->people[place].id2 = temp.people[i].id2;
      }
   }
   else {
      for (i=0; i<=limit; i++) {
         x = - cptr->xca[i];
         y = cptr->yca[i];
         place = cptr->diagram[28 - ((y >> 2) << cptr->xfactor) + (x >> 2)];

         if ((place < 0) || (cptr->xca[place] != x) || (cptr->yca[place] != y))
            fail("Don't recognize ending setup for this call, not able to do it mirror.");

         n = temp.people[i].id1;
         t = (- (n & (STABLE_VBIT*3))) & (STABLE_VBIT*3);
         l = (n & ROLLBITL) >> 2;
         r = (n & ROLLBITR) << 2;
         z = (n & ~(ROLLBITL | ROLLBITR | (STABLE_VBIT*3))) | l | r | t;
         s->people[place].id1 = (z & 1) ? (z ^ 2) : z;
         s->people[place].id2 = temp.people[i].id2;
      }
   }
}



typedef struct {
   int size;                 /* how many people in the maps */
   int lmask;                /* which people are facing E-W in original double-length setup */
   int rmask;                /* where in original setup can people be found */
   int cmask;                /* where in original setup have people collided */
   int source[12];           /* where to get the people */
   int map0[12];             /* where to put the north (or east)-facer */
   int map1[12];             /* where to put the south (or west)-facer */
   setup_kind initial_kind;  /* what setup they are collided in */
   setup_kind final_kind;    /* what setup to change it to */
   int rot;                  /* whether to rotate final setup CW */
   int warning;              /* if >= 0, a warning to give */
   } collision_map;

Private collision_map collision_map_table[] = {
   /* These items handle various types of "1/2 circulate" calls from 2x4's. */
   {4, 0x0CC0CC, 0xCC, 0xCC, {2, 3, 6, 7},         {0, 3, 5, 6},          {1, 2, 4, 7},           s_crosswave, s1x8,        1, -1},   /* from lines in */
   {4, 0x000000, 0x33, 0x33, {0, 1, 4, 5},         {0, 3, 5, 6},          {1, 2, 4, 7},           s_crosswave, s1x8,        0, -1},   /* from lines out */
   {4, 0x022022, 0xAA, 0xAA, {1, 3, 5, 7},         {2, 5, 7, 0},          {3, 4, 6, 1},           s_spindle,   s_crosswave, 0, -1},   /* from trade by */
   {6, 0x0880CC, 0xDD, 0x88, {0, 2, 3, 4, 6, 7},   {7, 0, 1, 3, 4, 6},    {7, 0, 2, 3, 4, 5},     s_crosswave, s_3x1dmd,    1, -1},   /* from 3&1 lines w/ centers in */
   {6, 0x000044, 0x77, 0x22, {0, 1, 2, 4, 5, 6},   {0, 1, 3, 4, 6, 7},    {0, 2, 3, 4, 5, 7},     s_crosswave, s_3x1dmd,    0, -1},   /* from 3&1 lines w/ centers out */
   {6, 0x0440CC, 0xEE, 0x44, {1, 2, 3, 5, 6, 7},   {7, 0, 2, 3, 5, 6},    {7, 1, 2, 3, 4, 6},     s_crosswave, s_3x1dmd,    1, -1},   /* from 3&1 lines w/ ends in */
   {4, 0x088088, 0x99, 0x99, {0, 3, 4, 7},         {0, 2, 5, 7},          {1, 3, 4, 6},           s_crosswave, s_crosswave, 0, -1},   /* from inverted lines w/ centers in */
   {4, 0x044044, 0x66, 0x66, {1, 2, 5, 6},         {6, 0, 3, 5},          {7, 1, 2, 4},           s_crosswave, s_crosswave, 1, warn__ctrs_stay_in_ctr},   /* from inverted lines w/ centers out */
   {4, 0x044044, 0x55, 0x55, {0, 2, 4, 6},         {0, 2, 5, 7},          {1, 3, 4, 6},           s_crosswave, s_crosswave, 0, warn__ctrs_stay_in_ctr},   /* from trade-by w/ ctrs 1/4 out */

   /* These items handle parallel lines with people wedged on one end, and hence handle flip or cut the hourglass. */
   {6, 0x000000, 0x77, 0x11, {0, 1, 2, 4, 5, 6},   {0, 2, 3, 7, 8, 9},    {1, 2, 3, 6, 8, 9},     s2x4,        s2x6,        0, -1},
   {6, 0x000000, 0xEE, 0x88, {1, 2, 3, 5, 6, 7},   {2, 3, 4, 8, 9, 11},   {2, 3, 5, 8, 9, 10},    s2x4,        s2x6,        0, -1},
   /* These items handle single lines with people wedged on one end, and hence handle flip or cut the diamond. */
   {3, 0x000000, 0x0B, 0x01, {0, 1, 3},            {0, 2, 5},             {1, 2, 5},              s1x4,        s_1x6,       0, -1},
   {3, 0x000000, 0x0E, 0x04, {1, 2, 3},            {2, 4, 5},             {2, 3, 5},              s1x4,        s_1x6,       0, -1},
   /* These items handle columns with people wedged everywhere, and hence handle unwraps of facing diamonds etc. */
   {4, 0x055055, 0x55, 0x55, {0, 2, 4, 6},         {12, 14, 2, 11},       {10, 3, 4, 6},          s2x4,        s4x4,        0, -1},
   {4, 0x0AA0AA, 0xAA, 0xAA, {1, 3, 5, 7},         {13, 0, 7, 9},         {15, 1, 5, 8},          s2x4,        s4x4,        0, -1},

   /* These items handle various types of 8-person "1/2 circulate" calls from T-boned 2x4's.  They can wind up in
      collided 1x6's and crosswaves. */
   {6, 0x000000,  077, 011,  {0, 1, 2, 3, 4, 5},   {0, 3, 2, 5, 7, 6},    {1, 3, 2, 4, 7, 6},     s_1x6,       s1x8,        0, -1},
   {6, 0x000000,  077, 022,  {0, 1, 2, 3, 4, 5},   {0, 1, 2, 4, 7, 6},    {0, 3, 2, 4, 5, 6},     s_1x6,       s1x8,        0, -1},
   {6, 0x000000,  077, 044,  {0, 1, 2, 3, 4, 5},   {0, 1, 3, 4, 5, 6},    {0, 1, 2, 4, 5, 7},     s_1x6,       s1x8,        0, -1},
   {6, 0x088088, 0xBB, 0x88, {0, 1, 3, 4, 5, 7},   {0, 1, 2, 4, 5, 7},    {0, 1, 3, 4, 5, 6},     s_crosswave, s_crosswave, 0, -1},
   {6, 0x0000CC, 0xDD, 0x11, {0, 2, 3, 4, 6, 7},   {0, 2, 3, 5, 6, 7},    {1, 2, 3, 4, 6, 7},     s_crosswave, s_crosswave, 0, -1},

   /* These items handle the situation from a 2FL/intlkd box circ/split box trade circ. */
/* Not sure what that comment meant, but we are trying to handle colliding 2FL-type circulates */
   {2, 0x000000, 0x03, 0x03, {0, 1},               {0, 2},                {1, 3},                 s2x4,        s2x8,        0, -1},
   {2, 0x000000, 0x0C, 0x0C, {2, 3},               {4, 6},                {5, 7},                 s2x4,        s2x8,        0, -1},
   {2, 0x000000, 0x30, 0x30, {5, 4},               {11, 9},               {10, 8},                s2x4,        s2x8,        0, -1},
   {2, 0x000000, 0xC0, 0xC0, {7, 6},               {15, 13},              {14, 12},               s2x4,        s2x8,        0, -1},
/* The preceding lines used to be these, which seems quite wrong:
   {2, 0x000000, 0x03, 0x03, {0, 1},               {0, 2},                {1, 3},                 s2x4,        s2x4,        0, -1},
   {2, 0x000000, 0x0C, 0x0C, {2, 3},               {0, 2},                {1, 3},                 s2x4,        s2x4,        0, -1},
   {2, 0x000000, 0x30, 0x30, {5, 4},               {7, 5},                {6, 4},                 s2x4,        s2x4,        0, -1},
   {2, 0x000000, 0xC0, 0xC0, {7, 6},               {7, 5},                {6, 4},                 s2x4,        s2x4,        0, -1},
*/

   /* These items handle various types of "1/2 circulate" calls from 2x2's. */
   {2, 0x000000, 0x05, 0x05, {0, 2},               {0, 3},                {1, 2},                 sdmd,        s1x4,        0, -1},   /* from couples out if it went to diamond */
   {2, 0x000000, 0x05, 0x05, {0, 2},               {0, 3},                {1, 2},                 s1x4,        s1x4,        0, -1},   /* from couples out if it went to line */
   {2, 0x00A00A, 0x0A, 0x0A, {1, 3},               {0, 3},                {1, 2},                 sdmd,        s1x4,        1, -1},   /* from couples in if it went to diamond */
   {2, 0x000000, 0x0A, 0x0A, {1, 3},               {0, 3},                {1, 2},                 s1x4,        s1x4,        0, -1},   /* from couples in if it went to line */
   {2, 0x000000, 0x06, 0x06, {1, 2},               {0, 3},                {1, 2},                 s1x4,        s1x4,        0, -1},   /* from "head pass thru, all split circulate" */
   {2, 0x000000, 0x09, 0x09, {0, 3},               {0, 3},                {1, 2},                 s1x4,        s1x4,        0, -1},   /* from "head pass thru, all split circulate" */
   /* These items handle "1/2 split trade circulate" from 2x2's. */
   {3, 0x008008, 0x0D, 0x08, {0, 2, 3},            {0, 2, 1},             {0, 2, 3},              sdmd,        sdmd,        0, -1},
   {3, 0x002002, 0x07, 0x02, {0, 2, 1},            {0, 2, 1},             {0, 2, 3},              sdmd,        sdmd,        0, -1},
   /* These items handle various types of "circulate" calls from 2x2's. */
   {2, 0x009009, 0x09, 0x09, {0, 3},               {7, 5},                {6, 4},                 s2x2,        s2x4,        1, -1},   /* from box facing all one way */
   {2, 0x006006, 0x06, 0x06, {1, 2},               {0, 2},                {1, 3},                 s2x2,        s2x4,        1, -1},   /* we need all four cases */
   {2, 0x000000, 0x0C, 0x0C, {2, 3},               {5, 7},                {4, 6},                 s2x2,        s2x4,        0, -1},   /* sigh */
   {2, 0x000000, 0x03, 0x03, {0, 1},               {0, 2},                {1, 3},                 s2x2,        s2x4,        0, -1},   /* sigh */
   /* These items handle more 2x2 stuff, including the "special drop in" that makes chain reaction/motivate etc. work. */
   {2, 0x005005, 0x05, 0x05, {0, 2},               {7, 2},                {6, 3},                 s2x2,        s2x4,        1, -1},
   {2, 0x00A00A, 0x0A, 0x0A, {1, 3},               {0, 5},                {1, 4},                 s2x2,        s2x4,        1, -1},
   {2, 0x000000, 0x05, 0x05, {0, 2},               {0, 5},                {1, 4},                 s2x2,        s2x4,        1, -1},
   {2, 0x000000, 0x0A, 0x0A, {1, 3},               {2, 7},                {3, 6},                 s2x2,        s2x4,        1, -1},
   {-1}};


Private void fix_collision(
   int collision_mask,
   int collision_index,
   int result_mask,
   setup *result)

{
   int temprot, lowbitmask;
   int i;
   collision_map *c_map_ptr;
   setup spare_setup;

   spare_setup = *result;
   clear_people(result);

   lowbitmask = 0;
   for (i=0; i<24; i++) {
      lowbitmask |= ((spare_setup.people[i].id1) & 1) << i;
   }

   c_map_ptr = collision_map_table;
   for (;;) {
      if (c_map_ptr->size < 0) break;
      if ((result->kind == c_map_ptr->initial_kind) && ((lowbitmask == c_map_ptr->lmask)) && (result_mask == c_map_ptr->rmask) && (collision_mask == c_map_ptr->cmask))
         goto win;
      c_map_ptr++;
   }

   /* Don't recognize the pattern, report this as normal collision. */
   collision_person1 = spare_setup.people[collision_index].id1;
   collision_person2 = spare_setup.people[collision_index+12].id1;
   error_message1[0] = '\0';
   error_message2[0] = '\0';
   longjmp(longjmp_ptr->the_buf, 3);

   win:

   warn(warn__take_right_hands);
   if (c_map_ptr->warning >= 0) warn(c_map_ptr->warning);
   temprot = ((-c_map_ptr->rot) & 3) * 011;
   result->kind = c_map_ptr->final_kind;
   result->rotation += c_map_ptr->rot;

   for (i=0; i<c_map_ptr->size; i++) {
      int oldperson;   

      oldperson = spare_setup.people[c_map_ptr->source[i]].id1;
      if (oldperson & 2)
         install_rot(result, c_map_ptr->map1[i], &spare_setup, c_map_ptr->source[i], temprot);
      else
         install_rot(result, c_map_ptr->map0[i], &spare_setup, c_map_ptr->source[i], temprot);
      oldperson = spare_setup.people[c_map_ptr->source[i]+12].id1;
      if (oldperson & 2)
         install_rot(result, c_map_ptr->map1[i], &spare_setup, c_map_ptr->source[i]+12, temprot);
      else
         install_rot(result, c_map_ptr->map0[i], &spare_setup, c_map_ptr->source[i]+12, temprot);
   }
}


Private int identity[24] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
Private int ftc2x4[8] = {10, 15, 3, 1, 2, 7, 11, 9};
Private int ftl2x4[12] = {6, 11, 15, 13, 14, 3, 7, 5, 6, 11, 15, 13};
Private int ftcspn[8] = {-1, 5, -1, 6, -1, 11, -1, 0};
Private int ftlcwv[12] = {9, 10, 1, 2, 3, 4, 7, 8, 9, 10, 1, 2};
Private int galtranslateh[16] = {0, 3, 4, 2, 0, 0, 0, 5, 0, 7, 0, 6, 0, 0, 0, 1};
Private int galtranslatev[16] = {0, 0, 0, 1, 0, 3, 4, 2, 0, 0, 0, 5, 0, 7, 0, 6};
Private int s1x6translateh[12] = {0, 1, 2, 0, 0, 0, 3, 4, 5, 0, 0, 0};
Private int s1x6translatev[12] = {0, 0, 0, 0, 1, 2, 0, 0, 0, 3, 4, 5};
Private int sxwvtranslateh[12] = {0, 1, 0, 0, 2, 3, 4, 5, 0, 0, 6, 7};
Private int sxwvtranslatev[12] = {0, 6, 7, 0, 1, 0, 0, 2, 3, 4, 5, 0};
Private int s3dmftranslateh[12] = {9, 10, 11, 1, 0, 0, 3, 4, 5, 7, 0, 0};
Private int s3dmftranslatev[12] = {7, 0, 0, 9, 10, 11, 1, 0, 0, 3, 4, 5};
Private int s3dmntranslateh[12] = {9, 10, 11, 0, 1, 0, 3, 4, 5, 0, 7, 0};
Private int s3dmntranslatev[12] = {0, 7, 0, 9, 10, 11, 0, 1, 0, 3, 4, 5};



Private int octtranslateh[64] = {
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,
   0,  0,  0,  7,  0,  0,  0,  6,  0,  0,  0,  5,  0,  0,  0,  4,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  9, 10, 11,
   0,  0,  0, 15,  0,  0,  0, 14,  0,  0,  0, 13,  0,  0,  0, 12};

Private int octtranslatev[64] = {
   0,  0,  0, 15,  0,  0,  0, 14,  0,  0,  0, 13,  0,  0,  0, 12,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,
   0,  0,  0,  7,  0,  0,  0,  6,  0,  0,  0,  5,  0,  0,  0,  4,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  9, 10, 11};

Private int qdmtranslateh[32] = {
   12, 13, 14, 15,  0,  1,  0,  0,  0,   0,  0,  0,  3,  0,  2,  0,
   4,   5,  6,  7,  0,  9,  0,  8,  0,   0,  0,  0, 11,  0, 10,  0};

Private int qdmtranslatev[32] = {
   0,   0,  0,  0, 11,  0, 10,  0, 12, 13, 14, 15,  0,  1,  0,  0,
   0,   0,  0,  0,  3,  0,  2,  0,  4,   5,  6,  7,  0,  9,  0,  8};

Private int dmdhyperh[12] = {0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 3, 0};
Private int linehyperh[12] = {0, 1, 0, 0, 0, 0, 2, 3, 0, 0, 0, 0};
Private int galhyperh[12] = {6, 0, 0, 0, 3, 1, 2, 0, 4, 0, 7, 5};
Private int dmdhyperv[12] = {0, 3, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0};
Private int linehyperv[12] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 3, 0};
Private int galhyperv[12] = {0, 7, 5, 6, 0, 0, 0, 3, 1, 2, 0, 4};
Private int starhyperh[12] =  {0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 3, 0};
Private int fstarhyperh[12] = {0, 0, 0, 1, 0, 0, 2, 0, 0, 3, 0, 0};



extern void do_stability(unsigned int *personp, unsigned int def_word, int turning)
{
   int t, at, st, atr;

   t = turning & 3;

   switch ((stability) ((def_word/DBSTAB_BIT) & 0xF)) {
      case stb_z:
         if (t != 0)
            fail("Person turned but was marked 'Z' stability.");
         break;
      case stb_a:
         t -= 4;
         break;
      case stb_c:
         if (t == 0) t = 4;
         break;
      default:
         *personp &= ~STABLE_MASK;
         break;
   }

   st = (t < 0) ? -1 : 1;
   at = t * st;
   atr = at - ((*personp & (STABLE_RBIT*7)) / STABLE_RBIT);

   if (atr <= 0) {
      *personp -= at*STABLE_RBIT;
   }
   else {
      *personp =
         (*personp & ~(STABLE_RBIT*7|STABLE_VBIT*3)) |
         ((*personp + (STABLE_VBIT * atr * st)) & (STABLE_VBIT*3));
   }
}



Private void check_line_restriction(setup *ss, call_restriction restr, unsigned int flags)
{
   int q0, q1, q2, q3, q4, q5, q6, q7;
   int i, j, k, z, t;

   if (restr == cr_alwaysfail) goto ldef_failed;

   switch (ss->kind) {
      case s2x2:
         switch (restr) {
            case cr_wave_only:
               /* check for a "real" (walk-and-dodge type) box */
               k = 0; j = 0; i = 3; z = 3;
               if (t = ss->people[0].id1) { k |= t; i &= t; j |= (t^2); z &= (t^2); }
               if (t = ss->people[1].id1) { j |= t; i &= t; k |= (t^2); z &= (t^2); }
               if (t = ss->people[2].id1) { j |= t; z &= t; k |= (t^2); i &= (t^2); }
               if (t = ss->people[3].id1) { k |= t; z &= t; j |= (t^2); i &= (t^2); }
               if ((k&3) && ((~i)&3) && (j&3) && ((~z)&3))
                  goto ldef_failed;
               break;
            case cr_2fl_only: case cr_all_facing_same:
               /* check for a "one-faced" (reverse-the-pass type) box */
               k = 0; j = 0; i = 3; z = 3;
               if (t = ss->people[0].id1) { k |= t; i &= t; j |= (t^2); z &= (t^2); }
               if (t = ss->people[1].id1) { k |= t; i &= t; j |= (t^2); z &= (t^2); }
               if (t = ss->people[2].id1) { k |= t; i &= t; j |= (t^2); z &= (t^2); }
               if (t = ss->people[3].id1) { k |= t; i &= t; j |= (t^2); z &= (t^2); }
               if ((k&3) && ((~i)&3) && (j&3) && ((~z)&3))
                  goto ldef_failed;
               break;
            case cr_magic_only:
               /* check for a "magic" (split-trade-circulate type) box */
               k = 0; j = 0; i = 3; z = 3;
               if (t = ss->people[0].id1) { k |= t; i &= t; j |= (t^2); z &= (t^2); }
               if (t = ss->people[1].id1) { j |= t; z &= t; k |= (t^2); i &= (t^2); }
               if (t = ss->people[2].id1) { k |= t; i &= t; j |= (t^2); z &= (t^2); }
               if (t = ss->people[3].id1) { j |= t; z &= t; k |= (t^2); i &= (t^2); }
               if ((k&3) && ((~i)&3) && (j&3) && ((~z)&3))
                  goto ldef_failed;
               break;
            case cr_couples_only:
               /* check for everyone as a couple */
               q0 = 0; q1 = 0; q2 = 3; q3 = 3;
               q4 = 0; q5 = 0; q7 = 3; q6 = 3;
               if (t = ss->people[0].id1) { q0 |= t; q2 &= t; q1 |= (t^2); q3 &= (t^2); }
               if (t = ss->people[1].id1) { q0 |= t; q7 &= t; q1 |= (t^2); q6 &= (t^2); }
               if (t = ss->people[2].id1) { q5 |= t; q7 &= t; q4 |= (t^2); q6 &= (t^2); }
               if (t = ss->people[3].id1) { q5 |= t; q2 &= t; q4 |= (t^2); q3 &= (t^2); }
               if (((q0&3) && ((~q2)&3) && (q1&3) && ((~q3)&3)) ||
                   ((q5&3) && ((~q7)&3) && (q4&3) && ((~q6)&3)))
                  goto ldef_failed;
               break;
            case cr_peelable_box:
               /* check for a "peelable" (everyone in genuine tandem somehow) box */
               q0 = 0; q1 = 0; q2 = 3; q3 = 3;
               q4 = 0; q5 = 0; q7 = 3; q6 = 3;
               if (t = ss->people[0].id1) { q0 |= t; q2 &= t; q1 |= (t^2); q3 &= (t^2); }
               if (t = ss->people[1].id1) { q5 |= t; q2 &= t; q4 |= (t^2); q3 &= (t^2); }
               if (t = ss->people[2].id1) { q5 |= t; q7 &= t; q4 |= (t^2); q6 &= (t^2); }
               if (t = ss->people[3].id1) { q0 |= t; q7 &= t; q1 |= (t^2); q6 &= (t^2); }
               if (((q0&3) && ((~q2)&3) && (q1&3) && ((~q3)&3)) ||
                   ((q5&3) && ((~q7)&3) && (q4&3) && ((~q6)&3)))
                  goto ldef_failed;
               break;
            case cr_not_tboned:
               /* check for a box that is not T-boned */
               if (((ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1) & 011) == 011)
                  goto ldef_failed;
               break;
         }
         break;
      case s2x4:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for parallel waves */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[2].id1) { k |=  t; i &=  t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[4].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[5].id1) { k |=  t; i &=  t; }
               if (t = ss->people[6].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[7].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_all_facing_same:
               q0 = 0; q1 = 0;  /* check for all 8 people in a "one-faced" setup */
               if (t = ss->people[0].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[1].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[2].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[3].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[4].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[5].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[6].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[7].id1) { q0 |= t; q1 |= ~t; }
               if (q0&q1&2)
                  goto ldef_failed;
               break;
            case cr_1fl_only:
               q0 = 0; q1 = 0; /* check for parallel one-faced lines */
               q2 = 0; q3 = 0;
               if (t = ss->people[0].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[1].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[2].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[3].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[4].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[5].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[6].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[7].id1) { q2 |= t; q3 |= ~t; }
               if ((q0&q1&2) || (q2&q3&2))
                  goto ldef_failed;
               break;
            case cr_2fl_only:
               k = 0;         /* check for parallel two-faced lines */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |=  t; i &=  t; }
               if (t = ss->people[2].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[4].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[5].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1) { k |=  t; i &=  t; }
               if (t = ss->people[7].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_ends_are_peelable:
               /* check for ends in a "peelable" (everyone in genuine tandem somehow) box */
               q1 = 0; q0 = 0; q5 = 0; q4 = 0;
               if (t = ss->people[0].id1) { q1 |= t; q0 |= (t^2); }
               if (t = ss->people[3].id1) { q5 |= t; q4 |= (t^2); }
               if (t = ss->people[4].id1) { q5 |= t; q4 |= (t^2); }
               if (t = ss->people[7].id1) { q1 |= t; q0 |= (t^2); }
               if (((q1&3) && (q0&3)) || ((q5&3) && (q4&3)))
                  goto ldef_failed;
               break;
            case cr_couples_only:
               /* check for everyone as a couple */
               q0 = 0; q1 = 0;
               q4 = 0; q5 = 0;
               q2 = 0; q3 = 0;
               q6 = 0; q7 = 0;
               if (t = ss->people[0].id1) { q0 |= t; q1 |= (t^2); }
               if (t = ss->people[1].id1) { q0 |= t; q1 |= (t^2); }
               if (t = ss->people[2].id1) { q5 |= t; q4 |= (t^2); }
               if (t = ss->people[3].id1) { q5 |= t; q4 |= (t^2); }
               if (t = ss->people[4].id1) { q2 |= t; q3 |= (t^2); }
               if (t = ss->people[5].id1) { q2 |= t; q3 |= (t^2); }
               if (t = ss->people[6].id1) { q6 |= t; q7 |= (t^2); }
               if (t = ss->people[7].id1) { q6 |= t; q7 |= (t^2); }
               if (((q0&3) && (q1&3)) ||
                   ((q2&3) && (q3&3)) ||
                   ((q6&3) && (q7&3)) ||
                   ((q5&3) && (q4&3)))
                  goto ldef_failed;
               break;
         }
         break;
      case s_2x3:
         switch (restr) {
            case cr_all_facing_same:
               q0 = 0; q1 = 0;  /* check for all 6 people in a "one-faced" setup */
               if (t = ss->people[0].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[1].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[2].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[3].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[4].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[5].id1) { q0 |= t; q1 |= ~t; }
               if (q0&q1&2)
                  goto ldef_failed;
               break;
            case cr_1fl_only:
               q0 = 0; q1 = 0; /* check for parallel one-faced lines of 3 */
               q2 = 0; q3 = 0;
               if (t = ss->people[0].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[1].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[2].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[3].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[4].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[5].id1) { q2 |= t; q3 |= ~t; }
               if ((q0&q1&2) || (q2&q3&2))
                  goto ldef_failed;
               break;
         }
         break;
      case s2x6:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for parallel 12-matrix waves */
               i = 2;
               if (t = ss->people[0].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[1].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[2].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[3].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[4].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[5].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[7].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[8].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[9].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[10].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[11].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
         }
         break;
      case s2x8:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for parallel 16-matrix waves */
               i = 2;
               if (t = ss->people[0].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[1].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[2].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[3].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[4].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[5].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[7].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[8].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[9].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[10].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[11].id1) { k |=  t; i &=  t; }
               if (t = ss->people[12].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[13].id1) { k |=  t; i &=  t; }
               if (t = ss->people[14].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[15].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
         }
         break;
      case s_qtag:
         switch (restr) {
            case cr_nice_diamonds:
               k = 0;         /* check for consistent diamonds, so can do "diamond swing thru" */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t+1; i &=  t+1; }
               if (t = ss->people[1].id1) { k |=  t+1; i &=  t+1; }
               if (t = ss->people[4].id1) { k |=  t-1; i &=  t-1; }
               if (t = ss->people[5].id1) { k |=  t-1; i &=  t-1; }
               if (t = ss->people[2].id1) { k |=  t; i &=  t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[7].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_wave_only:
               k = 0;         /* check for wave across the center */
               i = 2;
               if (t = ss->people[2].id1) { k |=  t; i &=  t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[7].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_2fl_only:
               k = 0;         /* check for two-faced line across the center */
               i = 2;
               if (t = ss->people[2].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1) { k |=  t; i &=  t; }
               if (t = ss->people[7].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
         }
         break;
      case s_ptpd:
         switch (restr) {
            case cr_nice_diamonds:
               k = 0;         /* check for consistent diamonds, so can do "diamond swing thru" */
               i = 2;
               if (t = ss->people[1].id1) { k |=  t+1; i &=  t+1; }
               if (t = ss->people[7].id1) { k |=  t+1; i &=  t+1; }
               if (t = ss->people[5].id1) { k |=  t-1; i &=  t-1; }
               if (t = ss->people[3].id1) { k |=  t-1; i &=  t-1; }
               if (t = ss->people[4].id1) { k |=  t; i &=  t; }
               if (t = ss->people[6].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[0].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[2].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
         }
         break;
      case s1x8:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for grand wave */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[2].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[3].id1) { k |=  t; i &=  t; }
               if (t = ss->people[4].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[5].id1) { k |=  t; i &=  t; }
               if (t = ss->people[6].id1) { k |=  t; i &=  t; }
               if (t = ss->people[7].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_2fl_only:
               k = 0;         /* check for grand two-faced line */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |=  t; i &=  t; }
               if (t = ss->people[2].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[4].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[5].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1) { k |=  t; i &=  t; }
               if (t = ss->people[7].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_4x4_2fl_only:
               k = 0;         /* check for 4x4 two-faced line -- 4 up and 4 down */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |=  t; i &=  t; }
               if (t = ss->people[2].id1) { k |=  t; i &=  t; }
               if (t = ss->people[3].id1) { k |=  t; i &=  t; }
               if (t = ss->people[4].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[5].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[7].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_4x4couples_only:
               /* check for each four people facing same way */
               q0 = 0; q1 = 0; q2 = 0; q3 = 0;
               if (t = ss->people[0].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[1].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[2].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[3].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[4].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[5].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[6].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[7].id1) { q2 |= t; q3 |= ~t; }
               if (((q0&q1&2)) || ((q2&q3&2)))
                  goto ldef_failed;
               break;
         }
         break;
      case s_1x6:
         switch (restr) {
            case cr_3x3_2fl_only:
               k = 0;         /* check for 3x3 two-faced line -- 3 up and 3 down */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |=  t; i &=  t; }
               if (t = ss->people[2].id1) { k |=  t; i &=  t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[4].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[5].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_3x3couples_only:
               /* check for each three people facing same way */
               /* check for everyone as a couple */
               q0 = 0; q1 = 0; q2 = 0; q3 = 0;
               if (t = ss->people[0].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[1].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[2].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[3].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[4].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[5].id1) { q2 |= t; q3 |= ~t; }
               if (((q0&q1&2)) || ((q2&q3&2)))
                  goto ldef_failed;
               break;
         }
         break;
      case s1x4:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for a wave */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[2].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[3].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_2fl_only:
               k = 0;         /* check for a 2-faced line */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |=  t; i &=  t; }
               if (t = ss->people[2].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_1fl_only:
               k = 0;         /* check for a 1-faced line */
               i = 0;
               if (t = ss->people[0].id1) { k |= t; i |= ~t; }
               if (t = ss->people[1].id1) { k |= t; i |= ~t; }
               if (t = ss->people[2].id1) { k |= t; i |= ~t; }
               if (t = ss->people[3].id1) { k |= t; i |= ~t; }
               if (k & i & 2)
                  goto ldef_failed;
               break;
            case cr_couples_only:
               /* check for everyone as a couple */
               q0 = 0; q1 = 0; q2 = 0; q3 = 0;
               if (t = ss->people[0].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[1].id1) { q0 |= t; q1 |= ~t; }
               if (t = ss->people[2].id1) { q2 |= t; q3 |= ~t; }
               if (t = ss->people[3].id1) { q2 |= t; q3 |= ~t; }
               if (((q0&q1&2)) || ((q2&q3&2)))
                  goto ldef_failed;
               break;
            case cr_awkward_centers:
               k = 2;         /* check for centers not having left hands */
               i = 2;
               if (t = ss->people[1].id1) k &= ~t;
               if (t = ss->people[3].id1) i &= t;
               if (!((k | i) & 2)) warn(warn__awkward_centers);
               break;
         }
         break;
      case s_1x2:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for a miniwave */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_2fl_only: case cr_couples_only:
               k = 0;         /* check for a couple */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto ldef_failed;
               break;
            case cr_awkward_centers:
               k = 2;         /* check for people not having left hands */
               i = 2;
               if (t = ss->people[0].id1) k &= ~t;
               if (t = ss->people[1].id1) i &= t;
               if (!((k | i) & 2)) warn(warn__awkward_centers);
               break;
         }
         break;
   }

   return;

   ldef_failed:

   switch (flags & CAF__RESTR_MASK) {
      case CAF__RESTR_UNUSUAL:
         warn(warn__unusual);
         break;
      case CAF__RESTR_FORBID:
         fail("This call is not legal from this formation.");
         break;
      case CAF__RESTR_RESOLVE_OK:
         warn(warn__dyp_resolve_ok);
         break;
      default:
         warn(warn__do_your_part);
         break;
   }
}




Private void check_column_restriction(setup *ss, call_restriction restr, unsigned int flags)
{
   int q0, q1, q2, q3, q4, q5, q6, q7;
   int i, k, t;

   if (restr == cr_alwaysfail) goto cdef_failed;

   switch (ss->kind) {
      case s2x4:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for real columns */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |=  t; i &=  t; }
               if (t = ss->people[2].id1) { k |=  t; i &=  t; }
               if (t = ss->people[3].id1) { k |=  t; i &=  t; }
               if (t = ss->people[4].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[5].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[7].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
            case cr_magic_only:
               k = 0;         /* check for magic columns */
               i = 2;
               if (t = ss->people[0].id1) { k |=  t; i &=  t; }
               if (t = ss->people[1].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[2].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[3].id1) { k |=  t; i &=  t; }
               if (t = ss->people[4].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[5].id1) { k |=  t; i &=  t; }
               if (t = ss->people[6].id1) { k |=  t; i &=  t; }
               if (t = ss->people[7].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
            case cr_couples_only:
               /* check for everyone as a couple */
               q0 = 0; q1 = 0;
               q4 = 0; q5 = 0;
               q2 = 0; q3 = 0;
               q6 = 0; q7 = 0;
               if (t = ss->people[0].id1) { q0 |= (t^1); q1 |= (t^3); }
               if (t = ss->people[7].id1) { q0 |= (t^1); q1 |= (t^3); }
               if (t = ss->people[1].id1) { q5 |= (t^1); q4 |= (t^3); }
               if (t = ss->people[6].id1) { q5 |= (t^1); q4 |= (t^3); }
               if (t = ss->people[2].id1) { q2 |= (t^1); q3 |= (t^3); }
               if (t = ss->people[5].id1) { q2 |= (t^1); q3 |= (t^3); }
               if (t = ss->people[3].id1) { q6 |= (t^1); q7 |= (t^3); }
               if (t = ss->people[4].id1) { q6 |= (t^1); q7 |= (t^3); }
               if (((q0&3) && (q1&3)) ||
                   ((q2&3) && (q3&3)) ||
                   ((q6&3) && (q7&3)) ||
                   ((q5&3) && (q4&3)))
                  goto cdef_failed;
               break;
            case cr_peelable_box:
               /* check for a "peelable" 2x4 column */
               q2 = 3; q3 = 3;
               q7 = 3; q6 = 3;
               if (t = ss->people[0].id1) { q2 &= t; q3 &= (t^2); }
               if (t = ss->people[1].id1) { q2 &= t; q3 &= (t^2); }
               if (t = ss->people[2].id1) { q2 &= t; q3 &= (t^2); }
               if (t = ss->people[3].id1) { q2 &= t; q3 &= (t^2); }
               if (t = ss->people[4].id1) { q7 &= t; q6 &= (t^2); }
               if (t = ss->people[5].id1) { q7 &= t; q6 &= (t^2); }
               if (t = ss->people[6].id1) { q7 &= t; q6 &= (t^2); }
               if (t = ss->people[7].id1) { q7 &= t; q6 &= (t^2); }
               if ((((~q2)&3) && ((~q3)&3)) || (((~q7)&3) && ((~q6)&3)))
                  goto cdef_failed;
               break;
            case cr_quarterbox_or_col:
               k = 0;         /* check for a reasonable "triple cross" setup */
               i = 2;
               if (ss->people[0].id1) { k |=  ss->people[0].id1;                         }
               if (ss->people[1].id1) { k |=  ss->people[1].id1; i &=  ss->people[1].id1; }
               if (ss->people[2].id1) { k |=  ss->people[2].id1; i &=  ss->people[2].id1; }
               if (ss->people[3].id1) {                         i &=  ss->people[3].id1; }
               if (ss->people[4].id1) { k |= ~ss->people[4].id1;                         }
               if (ss->people[5].id1) { k |= ~ss->people[5].id1; i &= ~ss->people[5].id1; }
               if (ss->people[6].id1) { k |= ~ss->people[6].id1; i &= ~ss->people[6].id1; }
               if (ss->people[7].id1) {                         i &= ~ss->people[7].id1; }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
            case cr_quarterbox_or_magic_col:
               k = 0;         /* check for a reasonable "make magic" setup */
               i = 2;
               if (ss->people[0].id1) {                         i &= ~ss->people[0].id1; }
               if (ss->people[1].id1) { k |=  ss->people[1].id1; i &=  ss->people[1].id1; }
               if (ss->people[2].id1) { k |=  ss->people[2].id1; i &=  ss->people[2].id1; }
               if (ss->people[3].id1) { k |= ~ss->people[3].id1;                         }
               if (ss->people[4].id1) {                         i &=  ss->people[4].id1; }
               if (ss->people[5].id1) { k |= ~ss->people[5].id1; i &= ~ss->people[5].id1; }
               if (ss->people[6].id1) { k |= ~ss->people[6].id1; i &= ~ss->people[6].id1; }
               if (ss->people[7].id1) { k |=  ss->people[7].id1;                         }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
         }
         break;
      case s_2x3:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for real columns */
               i = 2;
               if (ss->people[0].id1) { k |=  ss->people[0].id1; i &=  ss->people[0].id1; }
               if (ss->people[1].id1) { k |=  ss->people[1].id1; i &=  ss->people[1].id1; }
               if (ss->people[2].id1) { k |=  ss->people[2].id1; i &=  ss->people[2].id1; }
               if (ss->people[3].id1) { k |= ~ss->people[3].id1; i &= ~ss->people[3].id1; }
               if (ss->people[4].id1) { k |= ~ss->people[4].id1; i &= ~ss->people[4].id1; }
               if (ss->people[5].id1) { k |= ~ss->people[5].id1; i &= ~ss->people[5].id1; }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
            case cr_magic_only:
               k = 0;         /* check for magic columns */
               i = 2;
               if (ss->people[0].id1) { k |=  ss->people[0].id1; i &=  ss->people[0].id1; }
               if (ss->people[1].id1) { k |= ~ss->people[1].id1; i &= ~ss->people[1].id1; }
               if (ss->people[2].id1) { k |=  ss->people[2].id1; i &=  ss->people[2].id1; }
               if (ss->people[3].id1) { k |= ~ss->people[3].id1; i &= ~ss->people[3].id1; }
               if (ss->people[4].id1) { k |=  ss->people[4].id1; i &=  ss->people[4].id1; }
               if (ss->people[5].id1) { k |= ~ss->people[5].id1; i &= ~ss->people[5].id1; }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
            case cr_peelable_box:
               /* check for a "peelable" 2x3 column */
               q2 = 3; q3 = 3;
               q7 = 3; q6 = 3;
               if (t = ss->people[0].id1) { q2 &= t; q3 &= (t^2); }
               if (t = ss->people[1].id1) { q2 &= t; q3 &= (t^2); }
               if (t = ss->people[2].id1) { q2 &= t; q3 &= (t^2); }
               if (t = ss->people[3].id1) { q7 &= t; q6 &= (t^2); }
               if (t = ss->people[4].id1) { q7 &= t; q6 &= (t^2); }
               if (t = ss->people[5].id1) { q7 &= t; q6 &= (t^2); }
               if ((((~q2)&3) && ((~q3)&3)) || (((~q7)&3) && ((~q6)&3)))
                  goto cdef_failed;
               break;
         }
         break;
      case s2x6:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for real 12-matrix columns */
               i = 2;
               if (t = ss->people[0].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[1].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[2].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[3].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[4].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[5].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[6].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[7].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[8].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[9].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[10].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[11].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
         }
         break;
      case s2x8:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for real 16-matrix columns */
               i = 2;
               if (t = ss->people[0].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[1].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[2].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[3].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[4].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[5].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[6].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[7].id1)  { k |=  t; i &=  t; }
               if (t = ss->people[8].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[9].id1)  { k |= ~t; i &= ~t; }
               if (t = ss->people[10].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[11].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[12].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[13].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[14].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[15].id1) { k |= ~t; i &= ~t; }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
         }
         break;
      case s_qtag:
         switch (restr) {
            case cr_wave_only:
               k = 0;         /* check for wave across the center */
               i = 2;
               if (t = ss->people[2].id1) { k |=  t; i &=  t; }
               if (t = ss->people[3].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[6].id1) { k |= ~t; i &= ~t; }
               if (t = ss->people[7].id1) { k |=  t; i &=  t; }
               if (k & ~i & 2)
                  goto cdef_failed;
               break;
         }
         break;
      case s_1x2:
         switch (restr) {
            case cr_opposite_sex:
               i = 0;
               k = 0;
               if (ss->people[0].id1) {
                  i = ss->people[0].id2;
                  if (!(i & (ID2_BOY | ID2_GIRL)))
                     goto cdef_failed;
               }

               if (ss->people[1].id1) {
                  k = ss->people[1].id2;
                  if (!(k & (ID2_BOY | ID2_GIRL)))
                     goto cdef_failed;
               }

               if ((i&k) & (ID2_BOY | ID2_GIRL))
                  goto cdef_failed;
               break;
         }
         break;
   }

   return;

   cdef_failed:

   switch (flags & CAF__RESTR_MASK) {
      case CAF__RESTR_UNUSUAL:
         warn(warn__unusual);
         break;
      case CAF__RESTR_RESOLVE_OK:
         warn(warn__dyp_resolve_ok);
         break;
      case CAF__RESTR_FORBID:
         fail("This call is not legal from this formation.");
      default:
         warn(warn__do_your_part);
         break;
   }
}






Private void special_4_way_symm(
   callarray *tdef,
   setup *scopy,
   personrec newpersonlist[],
   int newplacelist[],
   int result_mask[],
   setup *result)

{
   static int table_2x4[8] = {10, 15, 3, 1, 2, 7, 11, 9};
   static int table_2x8[16] = {
      12, 13, 14, 15, 31, 27, 23, 19,
      44, 45, 46, 47, 63, 59, 55, 51};
   static int table_4dmd[16] = {
      7, 5, 14, 12, 16, 17, 18, 19,
      23, 21, 30, 28, 0, 1, 2, 3};
   static int line_table[4] = {0, 1, 6, 7};
   static int dmd_table[4] = {0, 4, 6, 10};

   int begin_size;
   int real_index;
   int real_direction, northified_index;
   unsigned int z;
   int k, zzz, result_size, result_quartersize;
   int *the_table;

   switch (result->kind) {
      case s2x2: case s_galaxy: case s_c1phan: case s4x4: case s_hyperglass: case s_star: case s_1x1:
         the_table = 0;
         break;
      case s1x4:
         result->kind = s_hyperglass;
         the_table = line_table;
         break;
      case sdmd:
         result->kind = s_hyperglass;
         the_table = dmd_table;
         break;
      case s2x4:
         result->kind = s4x4;
         the_table = table_2x4;
         break;
      case s2x8:
         result->kind = s_8x8;
         the_table = table_2x8;
         break;
      case s_4dmd:
         result->kind = s_x4dmd;
         the_table = table_4dmd;
         break;
      default:
         fail("Don't recognize ending setup for this call.");
   }

   begin_size = setup_limits[scopy->kind]+1;
   result_size = setup_limits[result->kind]+1;
   result_quartersize = result_size >> 2;
   result_mask[0] = 0;
   result_mask[1] = 0;

   for (real_index=0; real_index<begin_size; real_index++) {
      personrec this_person = scopy->people[real_index];
      newpersonlist[real_index].id1 = 0;
      newpersonlist[real_index].id2 = 0;
      if (this_person.id1) {
         real_direction = this_person.id1 & 3;
         northified_index = (real_index + (((4-real_direction)*begin_size) >> 2)) % begin_size;
         z = find_calldef(tdef, scopy, real_index, real_direction, northified_index);
         k = (z >> 4) & 017;
         if (the_table) k = the_table[k];
         k = (k + real_direction*result_quartersize) % result_size;
         zzz = (z+real_direction*011) & 013;
         newpersonlist[real_index].id1 = (this_person.id1 & ~(ROLL_MASK | 077)) | zzz | ((z * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);

         if (this_person.id1 & STABLE_ENAB) {
            do_stability(&newpersonlist[real_index].id1, z, (z + result->rotation));
         }

         newpersonlist[real_index].id2 = this_person.id2;
         newplacelist[real_index] = k;
         result_mask[k>>5] |= (1 << (k&037));
      }
   }
}



/* This function is internal. */

Private void special_triangle(
   callarray *cdef,
   callarray *ldef,
   setup *scopy,
   personrec newpersonlist[],
   int newplacelist[])

{
   int real_index;
   int real_direction, d2, northified_index;
   unsigned int z;
   int k;

   for (real_index=0; real_index<3; real_index++) {
      personrec this_person = scopy->people[real_index];
      newpersonlist[real_index].id1 = 0;
      newpersonlist[real_index].id2 = 0;
      newplacelist[real_index] = -1;
      if (this_person.id1) {
         real_direction = this_person.id1 & 3;
         d2 = ((this_person.id1 >> 1) & 1) * 3;
         northified_index = (real_index + d2);
         z = find_calldef((real_direction & 1) ? cdef : ldef, scopy, real_index, real_direction, northified_index);
         k = (((z >> 4) & 017) - d2);
         if (k < 0) k += 6;
         newpersonlist[real_index].id1 = (this_person.id1 & ~(ROLL_MASK | 077)) | ((z + real_direction * 011) & 013) | ((z * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);
         newpersonlist[real_index].id1 &= ~STABLE_MASK;   /* For now, can't do fractional stable on this kind of call. */
         newpersonlist[real_index].id2 = this_person.id2;
         newplacelist[real_index] = k;
      }
   }
}


/* For this routine, we know that callspec is a real call, with an array definition schema.
   Also, result->people have been cleared. */

extern void basic_move(
   setup *ss,
   parse_block *parseptr,
   callspec_block *callspec,
   final_set final_concepts,
   int tbonetest,
   long_boolean fudged,
   setup *result)

{
   int i, j, k;
   int livemask;
   int newtb;
   callarray *calldeflist;
   long_boolean funny;
   map_thing *division_maps;
   callarray *linedefinition;
   callarray *coldefinition;
   unsigned int matrix_check_flag = 0;
   callarray *goodies;
   unsigned int search_concepts;
   int real_index, northified_index;
   int num, halfnum, final_numout;
   int collision_mask, collision_index;
   int result_mask;
   personrec newpersonlist[24];
   int newplacelist[24];
   int resultflags, desired_elongation, orig_elongation;
   int inconsistent_rotation, inconsistent_setup;
   long_boolean four_way_startsetup;
   long_boolean funny_ok1 = FALSE;
   long_boolean funny_ok2 = FALSE;
   calldef_block *qq;

   /* We demand that the final concepts that remain be only those in the following list,
      which includes all of the "heritable" concepts. */

   if (final_concepts &
         ~(HERITABLE_FLAG_MASK | FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED | FINAL__SPLIT_SEQ_DONE | FINAL__TRIANGLE))
      fail("This concept not allowed here.");

   newtb = tbonetest;
   if (setup_limits[ss->kind] < 0) fail("Setup is extremely bizarre.");
   resultflags = 0;
   desired_elongation = 0;

   /* Set up the result elongation that we will need if the result is
      a 2x2.  This comes from the original 2x2 elongation, or is set
      perpendicular to the original 1x4/diamond elongation.  If the call has the
      "parallel_conc_end" flag set, invert that elongation.
      We will override all this if we divide a 1x4 or 2x2 into 1x2's. */

   switch (ss->kind) {
      case s2x2:
         desired_elongation = ((ss->setupflags & SETUPFLAG__ELONGATE_MASK) / SETUPFLAG__ELONGATE_BIT) * RESULTFLAG__ELONGATE_BIT;
         break;
      case s1x4: case sdmd:
         desired_elongation = ((2 - (ss->rotation & 1)) * RESULTFLAG__ELONGATE_BIT);
         break;
   }

   orig_elongation = desired_elongation;   /* We may need this later. */

   if (callspec->callflags & cflag__parallel_conc_end) desired_elongation ^= RESULTFLAG__ELONGATE_MASK;
   /* If the flags were zero and we complemented them so that both are set, that's not good. */
   if (desired_elongation == RESULTFLAG__ELONGATE_MASK)
      desired_elongation = 0;

   /* Attend to a few details. */

   switch (ss->kind) {
      case s2x2:
         /* Now we do a special check for split-square-thru or split-dixie-style types of things. */
         
         if (final_concepts & FINAL__SPLIT_SQUARE_APPROVED) {
            unsigned int i1, i2, p1, p2;

            ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

            /* Find out who are facing each other directly and will therefore start. */
   
            if (((ss->people[2].id1 & d_mask) == d_west) && ((ss->people[3].id1 & d_mask) == d_east))
               i1 = 0;
            else if (((ss->people[3].id1 & d_mask) == d_north) && ((ss->people[0].id1 & d_mask) == d_south))
               i1 = 1;
            else if (((ss->people[0].id1 & d_mask) == d_east) && ((ss->people[1].id1 & d_mask) == d_west))
               i1 = 2;
            else if (((ss->people[1].id1 & d_mask) == d_south) && ((ss->people[2].id1 & d_mask) == d_north))
               i1 = 3;
            else
               fail("People are not in correct position for split call.");

            i2 = (i1 + 1) & 3;
            p1 = ss->people[i1].id1;
            p2 = ss->people[i2].id1;

            /* Demand that the other people are facing the shoulders of the people who start. */
            if (((p1 & (BIT_PERSON | 3)) != (i1 ^ (BIT_PERSON | 2))) || ((p2 ^ p1) & d_mask))
               fail("People are not in correct position for split call.");

            ss->people[i1].id1 = rotccw(p2);
            ss->people[i2].id1 = rotcw(p1);
            p1 = ss->people[i1].id2;
            p2 = ss->people[i2].id2;
            ss->people[i1].id2 = p2;
            ss->people[i2].id2 = p1;
            newtb = ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1;
         }
         else if (final_concepts & FINAL__SPLIT_DIXIE_APPROVED) {
            fail("Can't do split dixie stuff -- sorry.");
         }
         break;
      case s_qtag:
         if (fudged && !(callspec->callflags & cflag__fudge_to_q_tag))
            fail("Can't do this call from arbitrary 3x4 setup.");
         break;
   }

   /* Many of the remaining final concepts (all of the heritable ones
      except "funny" and "left", but "left" has been taken care of)
      determine what call definition we will get. */

   search_concepts = final_concepts & HERITABLE_FLAG_MASK & ~FINAL__FUNNY;

   calldeflist = 0;

   for (qq = callspec->stuff.arr.def_list; qq; qq = qq->next) {
      if (qq->modifier_set == search_concepts) {
         if (qq->modifier_level > calling_level)
            fail("Use of this modifier on this call is not allowed at this level.");
         calldeflist = qq->callarray_list;
         break;
      }
   }

   use_this_calldeflist:

   /* Calldeflist ought to have something now.  If not, perhaps the concept
      "magic" or "interlocked" was given, and the call has no special definition for same,
      but wants us to divide the setup magically or interlockedly.  Or a similar thing with 12 matrix. */

   if (!calldeflist) {
      unsigned long int y;
      switch (ss->kind) {
         case s2x4:
            y = search_concepts & ~(FINAL__DIAMOND | FINAL__SINGLE | FINAL__CROSS | FINAL__GRAND);
            if (y == FINAL__MAGIC) {
               /* "Magic" was specified.  Split it into 1x4's in the appropriate magical way. */
               division_maps = &map_2x4_magic;
               final_concepts &= ~y;
               goto divide_us;
            }
            break;
         case s3x4:
            if (search_concepts == FINAL__12_MATRIX && callspec->callflags & cflag__12_16_matrix_means_split) {
               /* "12 matrix" was specified.  Split it into 1x4's in the appropriate way. */
               division_maps = (*map_lists[s1x4][2])[MPKIND__SPLIT][1];
               final_concepts &= ~search_concepts;
               goto divide_us;
            }
            break;
         case s2x6:
            if (search_concepts == FINAL__12_MATRIX && callspec->callflags & cflag__12_16_matrix_means_split) {
               /* "12 matrix" was specified.  Split it into 2x2's in the appropriate way. */
               division_maps = (*map_lists[s2x2][2])[MPKIND__SPLIT][0];
               final_concepts &= ~search_concepts;
               goto divide_us;
            }
            break;
         case s2x8:
            if (search_concepts == FINAL__16_MATRIX && callspec->callflags & cflag__12_16_matrix_means_split) {
               /* "16 matrix" was specified.  Split it into 2x2's in the appropriate way. */
               division_maps = (*map_lists[s2x2][3])[MPKIND__SPLIT][0];
               final_concepts &= ~search_concepts;
               goto divide_us;
            }
            break;
         case s4x4:
            if (search_concepts == FINAL__16_MATRIX && callspec->callflags & cflag__12_16_matrix_means_split) {
               /* "16 matrix" was specified.  Split it into 1x4's in the appropriate way. */
               /* But which way is appropriate?  A 4x4 is ambiguous.  Being too lazy to look at
                  the call definition (the "assoc" stuff), we assume the call wants lines, since
                  it seems that that is true for all calls that have the "12_16_matrix_means_split" property. */
               if (tbonetest & 1) {
                  division_maps = &map_4x4v;
               }
               else {
                  division_maps = (*map_lists[s1x4][3])[MPKIND__SPLIT][1];
               }
               final_concepts &= ~search_concepts;
               goto divide_us;
            }
            break;
         case s_qtag:
            y = search_concepts & ~(FINAL__DIAMOND | FINAL__SINGLE | FINAL__CROSS | FINAL__GRAND);
            if (y == FINAL__MAGIC) {
               /* "Magic" was specified, perhaps with "diamond".  Split it into diamonds in the appropriate magical way. */
               division_maps = &map_qtg_magic;
               final_concepts &= ~y;
               resultflags |= RESULTFLAG__NEED_DIAMOND;      /* Indicate that we have done the division and the concept name needs to be changed. */
               goto divide_us;
            }
            else if (y == FINAL__INTERLOCKED) {
               /* "Interlocked" was specified, perhaps with "diamond".  Split it into diamonds in the appropriate interlocked way. */
               division_maps = &map_qtg_intlk;
               final_concepts &= ~y;
               resultflags |= RESULTFLAG__NEED_DIAMOND;      /* Indicate that we have done the division and the concept name needs to be changed. */
               goto divide_us;
            }
            else if (y == (FINAL__INTERLOCKED | FINAL__MAGIC)) {
               /* "Magic interlocked" was specified, perhaps with "diamond".  Split it into diamonds in the appropriate disgusting way. */
               division_maps = &map_qtg_magic_intlk;
               final_concepts &= ~y;
               resultflags |= RESULTFLAG__NEED_DIAMOND;      /* Indicate that we have done the division and the concept name needs to be changed. */
               goto divide_us;
            }
            break;
         case s_ptpd:
            y = search_concepts & ~(FINAL__DIAMOND | FINAL__SINGLE | FINAL__CROSS | FINAL__GRAND);
            if (y == FINAL__MAGIC) {
               /* "Magic" was specified, perhaps with "diamond".  Split it into diamonds in the appropriate magical way. */
               division_maps = &map_ptp_magic;
               final_concepts &= ~y;
               resultflags |= RESULTFLAG__NEED_DIAMOND;      /* Indicate that we have done the division and the concept name needs to be changed. */
               goto divide_us;
            }
            else if (y == FINAL__INTERLOCKED) {
               /* "Interlocked" was specified, perhaps with "diamond".  Split it into diamonds in the appropriate interlocked way. */
               division_maps = &map_ptp_intlk;
               final_concepts &= ~y;
               resultflags |= RESULTFLAG__NEED_DIAMOND;      /* Indicate that we have done the division and the concept name needs to be changed. */
               goto divide_us;
            }
            else if (y == (FINAL__INTERLOCKED | FINAL__MAGIC)) {
               /* "Magic interlocked" was specified, perhaps with "diamond".  Split it into diamonds in the appropriate disgusting way. */
               division_maps = &map_ptp_magic_intlk;
               final_concepts &= ~y;
               resultflags |= RESULTFLAG__NEED_DIAMOND;      /* Indicate that we have done the division and the concept name needs to be changed. */
               goto divide_us;
            }
            break;
      }

      fail("Can't do this call with this modifier.");
   }
   
   /* We now have an association list (setups ==> definition arrays) in calldeflist.
      Search it for an entry matching the setup we have, or else divide the setup
         until we find something.
      If we can't handle the starting setup, perhaps we need to look for "12 matrix" or
         "16 matrix" call definitions. */

   search_for_call_def:

   linedefinition = (callarray *) 0;
   coldefinition = (callarray *) 0;

   if (calldeflist) {
      begin_kind key1 = keytab[ss->kind][0];
      begin_kind key2 = keytab[ss->kind][1];

      four_way_startsetup = FALSE;
   
      if (key1 != b_nothing && key2 != b_nothing) {
         if (key1 == key2) {     /* This is for things like 2x2 or 1x1. */
            linedefinition = assoc(key1, ss, calldeflist);
            coldefinition = linedefinition;
            four_way_startsetup = TRUE;
         }
         else {
            if (newtb & 010) linedefinition = assoc(key1, ss, calldeflist);
            if (newtb & 1) coldefinition = assoc(key2, ss, calldeflist);
         }
      }
   }

   switch (ss->kind) {
      case s_short6:
      case s_bone6:
      case s_trngl:
         break;
      default:
         if (final_concepts & FINAL__TRIANGLE)
            fail("Triangle concept not allowed here.");
   }

   /* If we found a definition for the setup we are in, we win.
      This is true even if we only found a definition for the lines version
      of the setup and not the columns version, or vice-versa.
      If we need both and don't have both, we will lose. */

   if (linedefinition || coldefinition) {
      /* Raise a warning if the "split" concept was explicitly used but the
         call would have naturally split that way anyway. */

/* ******* we have patched this out, because we aren't convinced that it really
   works.  How do we make it not complain on "split sidetrack" even though some
   parts of the call, like the "zig-zag", would complain?  And how do we account
   for the fact that it is observed not to raise the warning on split sidetrack
   even though we don't understand why?  By the way, uncertainty about this is what
   led us to make this a warning.  It was originally an error.  Which is correct?
   It is probably best to leave it as a warning of the "don't use in resolve" type.

      if (ss->setupflags & SETUPFLAG__SAID_SPLIT) {
         switch (ss->kind) {
            case s2x2:
               if (!assoc(b_2x4, ss, calldeflist) && !assoc(b_4x2, ss, calldeflist))
                  warn(warn__excess_split);
               break;
            case s1x4:
               if (!assoc(b_1x8, ss, calldeflist) && !assoc(b_8x1, ss, calldeflist))
                  warn(warn__excess_split);
               break;
            case sdmd:
               if (!assoc(b_qtag, ss, calldeflist) && !assoc(b_pqtag, ss, calldeflist) &&
                        !assoc(b_ptpd, ss, calldeflist) && !assoc(b_pptpd, ss, calldeflist))
                  warn(warn__excess_split);
               break;
         }
      }
*/

      if (final_concepts & FINAL__TRIANGLE)
         fail("Triangle concept not allowed here.");

      ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;
      goto do_the_call;
   }

   /* We got something in "calldeflist" corresponding to the modifiers on this call,
      but there was nothing listed under that definition matching the starting setup. */

   /* First, see if adding a "12 matrix" or "16 matrix" modifier to the call will help.
      We need to be very careful about this.  The code below will divide, for example,
      a 2x6 into 2x3's if SETUPFLAG__EXPLICIT_MATRIX is on (that is, if the caller
      said "2x6 matrix") and the call has a 2x3 definition.  This is what makes
      "2x6 matrix double play" work correctly from parallelogram columns, while
      just "double play" is not legal.  We take the position that the division of the
      2x6 into 2x3's only occurs if the caller said "2x6 matrix".  But the call
      "circulate" has a 2x3 column definition for the case with no 12matrix modifier
      (so that "center 6, circulate" will work), and a 2x6 definition if the 12matrix
      modifier is given.  We want the 12-person version of the circulate to happen
      if the caller said either "12 matrix" or "2x6 matrix".  If "2x6 matrix" was
      used, we will get here with nothing in coldefinition.  We must notice that
      doing the call search again with the "12 matrix" modifier set will get us the
      12-person call definition.
      
      If we didn't do this check, then a 2x6 parallelogram column, for example,
      would be split into 2x3's in the code below, if a 2x3 definition existed
      for the call.  This would mean that, if we said "2x6 matrix circulate",
      we would split the setup and do the circulate in each 2x3, which is not
      what people want. */
      
   if (matrix_check_flag == 0 && (ss->setupflags & SETUPFLAG__EXPLICIT_MATRIX)) {
      if (ss->kind == s2x6) matrix_check_flag |= FINAL__12_MATRIX;
      else matrix_check_flag |= FINAL__16_MATRIX;

      /* Now search again. */

      for (qq = callspec->stuff.arr.def_list; qq; qq = qq->next) {
         if (qq->modifier_set == matrix_check_flag | search_concepts) {
            if (qq->modifier_level > calling_level)
               fail("Use of this modifier on this call is not allowed at this level.");
            calldeflist = qq->callarray_list;
            goto use_this_calldeflist;
         }
      }
   }

   /* We need to divide the setup. It will be helpful to have a mask of where the
      live people are. */

   for (i=0, j=1, livemask = 0; i<=setup_limits[ss->kind]; i++, j<<=1) {
      if (ss->people[i].id1) livemask |= j;
   }

   switch (ss->kind) {
      case s4x4:
         /* The only way this can be legal is if we can identify
            smaller setups of all real people and can do the call on them.  For
            example, if the outside phantom lines are fully occupied and the inside
            ones empty, we could do a swing-thru.  We also identify Z's from which
            we can do "Z axle". */
   
         switch (livemask) {
            case 0x7171:
               division_maps = &map_4x4_ns;
               warn(warn__each1x4);
               goto divide_us_no_recompute;
            case 0x1717:
               division_maps = &map_4x4_ew;
               warn(warn__each1x4);
               goto divide_us_no_recompute;
            case 0x4E4E: case 0x8B8B:
               division_maps = &map_rh_s2x3_3;
               /* If this changes shape (as it will in the only known case
                  of this -- Z axle), divided_setup_move will give a warning
                  about going to a parallelogram, since we did not start
                  with 50% offset, though common practice says that a
                  parallelogram is the correct result.  If the call turns out
                  not to be a shape-changer, no warning will be given.  If
                  the call is a shape changer that tries to go into a setup
                  other than a parallelogram, divided_setup_move will raise
                  an error. */
               ss->setupflags |= SETUPFLAG__OFFSET_Z;
               if ((callspec->callflags & cflag__split_large_setups) &&
                     (!(newtb & 010) || assoc(b_3x2, ss, calldeflist)) &&
                     (!(newtb & 001) || assoc(b_2x3, ss, calldeflist)))
                  goto divide_us_no_recompute;
               break;
            case 0xA6A6: case 0x9C9C:
               division_maps = &map_lh_s2x3_3;
               ss->setupflags |= SETUPFLAG__OFFSET_Z;
               if ((callspec->callflags & cflag__split_large_setups) &&
                     (!(newtb & 010) || assoc(b_3x2, ss, calldeflist)) &&
                     (!(newtb & 001) || assoc(b_2x3, ss, calldeflist)))
                  goto divide_us_no_recompute;
               break;
            case 0xE4E4: case 0xB8B8:
               division_maps = &map_rh_s2x3_2;
               ss->setupflags |= SETUPFLAG__OFFSET_Z;
               if ((callspec->callflags & cflag__split_large_setups) &&
                     (!(newtb & 010) || assoc(b_2x3, ss, calldeflist)) &&
                     (!(newtb & 001) || assoc(b_3x2, ss, calldeflist)))
                  goto divide_us_no_recompute;
               break;
            case 0x6A6A: case 0xC9C9:
               division_maps = &map_lh_s2x3_2;
               ss->setupflags |= SETUPFLAG__OFFSET_Z;
               if ((callspec->callflags & cflag__split_large_setups) &&
                     (!(newtb & 010) || assoc(b_2x3, ss, calldeflist)) &&
                     (!(newtb & 001) || assoc(b_3x2, ss, calldeflist)))
                  goto divide_us_no_recompute;
               break;
         }

         fail("You must specify a concept.");
      case s2x6:
         /* The call has no applicable 2x6 or 6x2 definition. */

         /* See if this call has applicable 2x8 definitions and matrix expansion is permitted.
            If so, that is what we must do. */

         if ((!(newtb & 010) || assoc(b_2x8, ss, calldeflist)) &&
               (!(newtb & 1) || assoc(b_8x2, ss, calldeflist)) &&
               !(ss->setupflags & SETUPFLAG__NO_EXPAND_MATRIX)) {
            do_matrix_expansion(ss, CONCPROP__NEED_2X8);
            if (ss->kind != s2x8) fail("Failed to expand to 2X8.");  /* Should never fail, but we don't want a loop. */
            goto search_for_call_def;        /* And try again. */
         }

         /* Next, check whether it has 2x3/3x2 definitions, and divide the setup if so,
            and if the call permits it.  This is important for permitting "Z axle" from
            a 2x6 but forbidding "circulate".  We also enable this if the caller explicitly
            said "2x6 matrix". */

         if (((callspec->callflags & cflag__split_large_setups) || (ss->setupflags & SETUPFLAG__EXPLICIT_MATRIX)) &&
                  (!(newtb & 010) || assoc(b_2x3, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_3x2, ss, calldeflist))) {
            division_maps = (*map_lists[s_2x3][1])[MPKIND__SPLIT][0];
            goto divide_us_no_recompute;
         }
   
         /* Otherwise, the only way this can be legal is if we can identify
            smaller setups of all real people and can do the call on them.  For
            example, we will look for 1x4 setups, so we could do things like
            swing thru from a parallelogram. */
   
         switch (livemask) {
            case 07474:    /* a parallelogram */
               division_maps = (*map_lists[s1x4][1])[MPKIND__OFFS_R_HALF][1];
               warn(warn__each1x4);
               goto divide_us_no_recompute;
            case 01717:    /* a parallelogram */
               division_maps = (*map_lists[s1x4][1])[MPKIND__OFFS_L_HALF][1];
               warn(warn__each1x4);
               goto divide_us_no_recompute;
            case 06363:    /* the outer triple boxes */
               division_maps = (*map_lists[s2x2][2])[MPKIND__SPLIT][0];
               warn(warn__each2x2);
               goto divide_us_no_recompute;
         }
   
         fail("You must specify a concept.");
      case s2x8:

         /* Check whether it has 2x4/4x2 definitions, and divide the setup if so,
            and if the caller explicitly said "2x8 matrix". */

         if ((ss->setupflags & SETUPFLAG__EXPLICIT_MATRIX) &&
                  (!(newtb & 010) || assoc(b_2x4, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_4x2, ss, calldeflist))) {
            division_maps = (*map_lists[s2x4][1])[MPKIND__SPLIT][0];
            goto divide_us_no_recompute;
         }
   
         /* Otherwise, the only way this can be legal is if we can identify
            smaller setups of all real people and can do the call on them.  For
            example, we will look for 1x4 setups, so we could do things like
            swing thru from a totally offset parallelogram. */
   
         switch (livemask) {
            case 0xF0F0:    /* a parallelogram */
               division_maps = (*map_lists[s1x4][1])[MPKIND__OFFS_R_FULL][1];
               warn(warn__each1x4);
               break;
            case 0x0F0F:    /* a parallelogram */
               division_maps = (*map_lists[s1x4][1])[MPKIND__OFFS_L_FULL][1];
               warn(warn__each1x4);
               break;
            case 0xC3C3:    /* the outer quadruple boxes */
               division_maps = (*map_lists[s2x2][3])[MPKIND__SPLIT][0];
               warn(warn__each2x2);
               break;
            default:
               fail("You must specify a concept.");
         }
   
         goto divide_us_no_recompute;
      case s_c1phan:
   
         /* The only way this can be legal is if people are in genuine
            C1 phantom spots and the call can be done from 1x2's or 2x1's.
            *** Actually, that isn't so.  We ought to be able to do 1x1 calls
            from any population at all. */
   
         if ((livemask & 0xAAAA) == 0)
            division_maps = (*map_lists[s_1x2][3])[MPKIND__4_QUADRANTS][0];
         else if ((livemask & 0x5555) == 0)
            division_maps = (*map_lists[s_1x2][3])[MPKIND__4_QUADRANTS][1];
         else
            fail("You must specify a concept.");
   
         goto divide_us_no_recompute;
      case s2x2:
         ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

         /* Any 2x2 -> 2x2 call that acts by dividing itself into 1x2's
            is presumed to want the people in each 1x2 to stay near each other.
            We signify that by reverting to the original elongation,
            overriding anything that may have been in the call definition. */

         desired_elongation = orig_elongation;

         /* Tentatively choose a map.  We may change it later to "map_2x2v". */
         division_maps = (*map_lists[s_1x2][1])[MPKIND__SPLIT][1];
   
         if ((newtb & 011) == 011) {
            /* The situation is serious.  If the call has both 1x2 and 2x1 definitions,
               we can do it, by guessing correctly which way to divide the setup.
               Otherwise, if it has either a 1x2 or a 2x1 definition, but not both,
               we lose, because the call presumably wants to use same.
               But if the call has neither 1x2 nor 2x1 definitions, but does have
               a 1x1 definition, we can do it.  Just divide the setup arbitrarily. */
   
            long_boolean losing = FALSE;

            if (assoc(b_2x1, ss, calldeflist)) {
               if (assoc(b_1x2, ss, calldeflist)) {
                  /* The call has both definitions.  We look at the manner in
                     which the setup is T-boned in order to figure out how
                     to divide the setup. */
   
                  if ((((ss->people[0].id1 | ss->people[3].id1) & 011) != 011) &&
                        (((ss->people[1].id1 | ss->people[2].id1) & 011) != 011))
                     division_maps = &map_2x2v;
                  else if ((((ss->people[0].id1 | ss->people[1].id1) & 011) == 011) ||
                        (((ss->people[2].id1 | ss->people[3].id1) & 011) == 011))
                     fail("Can't figure out who should be working with whom.");
   
                  goto divide_us_no_recompute;
               }
               else
                  losing = TRUE;
            }
            else if (assoc(b_1x2, ss, calldeflist))
               losing = TRUE;
            else if (assoc(b_1x1, ss, calldeflist))
               goto divide_us_no_recompute;

            if (losing) {
               if (ss->setupflags & SETUPFLAG__PHANTOMS)
                  fail("Sorry, should have people do their own part, but don't know how.");
               else
                  fail("People are not working with each other in a consistent way.");
            }
         }
         else {
            /* People are not T-boned.  Check for a 2x1 or 1x2 definition.
               If found, use it as a guide.  If both are present, we use
               the setup elongation flag to tell us what to do.  In any
               case, the setup elongation flag, if present, must not be
               inconsistent with our decision. */

            unsigned long int elong = 0;
            unsigned long int foo;

            if (assoc(b_2x1, ss, calldeflist)) {
               elong |= (2 -  (newtb & 1));
            }

            if (assoc(b_1x2, ss, calldeflist)) {
               elong |= (1 + (newtb & 1));
            }

            if (elong == 0) {
               /* Neither 2x1 or 1x2 definition existed.  Check for 1x1.
                  If found, any division axis will work. */
               if (assoc(b_1x1, ss, calldeflist))
                  goto divide_us_no_recompute;
            }
            else {
               elong *= SETUPFLAG__ELONGATE_BIT;
               foo = (ss->setupflags | ~elong) & SETUPFLAG__ELONGATE_MASK;

               if (foo == 0) {
                  fail("Can't figure out who should be working with whom.");
               }
               else if (foo == SETUPFLAG__ELONGATE_MASK) {
                  if (!(ss->setupflags & SETUPFLAG__NO_CHK_ELONG))
                     fail("People are too far away to work with each other on this call.");
                  foo ^= elong;
               }

               if (foo == (1*SETUPFLAG__ELONGATE_BIT))
                  division_maps = &map_2x2v;

               goto divide_us_no_recompute;
            }
         }

         break;
      case s2x4:
         division_maps = (*map_lists[s2x2][1])[MPKIND__SPLIT][0];    /* The map we will probably use. */
   
         /* See if this call is being done "split" as in "split square thru" or "split dixie style",
            in which case split into boxes. */
   
         if (final_concepts & (FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED))
            goto divide_us_no_recompute;
   
         /* See if this call has applicable 2x6 or 2x8 definitions and matrix expansion is permitted.
            If so, that is what we must do. */

         if ((!(newtb & 010) || assoc(b_2x6, ss, calldeflist) || assoc(b_2x8, ss, calldeflist)) &&
               (!(newtb & 1) || assoc(b_6x2, ss, calldeflist) || assoc(b_8x2, ss, calldeflist)) &&
               !(ss->setupflags & SETUPFLAG__NO_EXPAND_MATRIX)) {
            do_matrix_expansion(ss, CONCPROP__NEED_2X6);
            if (ss->kind != s2x6) fail("Failed to expand to 2X6.");  /* Should never fail, but we don't want a loop. */
            goto search_for_call_def;        /* And try again. */
         }

         /* See if this call has applicable 1x4 or 4x1 definitions, in which case split it that way. */
      
         if ((!(newtb & 010) || assoc(b_1x4, ss, calldeflist)) &&
               (!(newtb & 1) || assoc(b_4x1, ss, calldeflist))) {
            division_maps = (*map_lists[s1x4][1])[MPKIND__SPLIT][1];
            goto divide_us_no_recompute;
         }
   
         /* See if this call has applicable 2x2 definition, in which case split into boxes. */
            
         else if (assoc(b_2x2, ss, calldeflist)) goto divide_us_no_recompute;
   
         /* See if this call has applicable 1x2 or 2x1 definitions, (but not 2x2), in a non-T-boned setup.
            If so, split into boxes.  Also, if some phantom concept has been used and there are 1x2 or 2x1
            definitions, we also split it into boxes even if people are T-boned.  This is what makes
            everyone do their part if we say "heads into the middle and heads are standard in split phantom
            lines, partner trade". */
   
         else if (     (((newtb & 011) != 011) || (ss->setupflags & SETUPFLAG__PHANTOMS))
                                                  &&
                       (assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist)))
            goto divide_us_no_recompute;
      
         /* If we are T-boned and have 1x2 or 2x1 definitions, we need to be careful. */
   
         else if ((newtb & 011) == 011) {
            int tbi = ss->people[1].id1 | ss->people[2].id1 | ss->people[5].id1 | ss->people[6].id1;
            int tbo = ss->people[0].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[7].id1;
   
            /* If the centers and ends are separately consistent, we can do the call concentrically *IF*
               the appropriate type of definition exists for the ends to work with the near person rather
               than the far one.  This is what makes "heads into the middle and everbody partner trade" work,
               and forbids "heads into the middle and everbody star thru". */
   
            if (((tbi & 011) != 011) && ((tbo & 011) != 011)) {
               if ((!(tbo & 010) || assoc(b_2x1, ss, calldeflist)) &&
                        (!(tbo & 1) || assoc(b_1x2, ss, calldeflist))) {
                  concentric_move(ss, parseptr, parseptr, callspec, callspec, final_concepts, final_concepts, schema_concentric, 0, 0, result);
                  goto un_mirror;
               }

               /* Or if we have a 1x1 definition, we can divide it.  Otherwise, we lose. */
               else if (assoc(b_1x1, ss, calldeflist)) goto divide_us_no_recompute;
            }
   
            /* If the centers and ends are not separately consistent, we should just split it into 2x2's.
               Perhaps the call has both 1x2 and 2x1 definitions, and will be done sort of siamese in each quadrant.
               Another possibility is that the call has just (say) 1x2 definitions, but everyone can do their
               part and miraculously not hit each other. */
   
            else goto divide_us_no_recompute;
         }
   
         /* We are not T-boned, and there is no 1x2 or 2x1 definition.  The only possibility is that there
            is a 1x1 definition, in which case splitting into boxes will work. */
   
         else if (assoc(b_1x1, ss, calldeflist)) goto divide_us_no_recompute;
         break;
      case s_rigger:
         {
            int tinytb = ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1;
   
            /* See if this call has applicable 1x2 or 2x1 definitions, and the people in the wings are
               facing appropriately.  Then do it concentrically. */
   
            if ((!(tinytb & 010) || assoc(b_1x2, ss, calldeflist)) &&
               (!(tinytb & 1) || assoc(b_2x1, ss, calldeflist))) {
               concentric_move(ss, parseptr, parseptr, callspec, callspec, final_concepts, final_concepts, schema_concentric, 0, 0, result);
               goto un_mirror;
            }
         }
         break;
      case s3x4:
         {
            long_boolean can_do_boxes;
            setup sss;
            long_boolean really_fudged;

            /* The call has no applicable 3x4 or 4x3 definition. */
            /* First, check whether it has 2x3/3x2 definitions, and divide the setup if so,
               and if the call permits it.  This is important for permitting "Z axle" from
               a 3x4 but forbidding "circulate" (unless we give a concept like 12 matrix
               phantom columns.) */
      
            if ((callspec->callflags & cflag__split_large_setups) &&
                  (!(newtb & 010) || assoc(b_3x2, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_2x3, ss, calldeflist))) {
               division_maps = (*map_lists[s_2x3][1])[MPKIND__SPLIT][1];
               goto divide_us_no_recompute;
            }
   
            /* Either we are actually in an offset lines/columns situation and the call has
               a definition for 2x2 but not bigger things, in which case we give a warning,
               remove the offset (turning it into a 2x4) and do the call, or the setup can
               be fudged into a quarter-tag and we can do such a call.  We do not allow the offset
               removal if the call has a definition from a 2x4, for example, so that we can't
               say "circulate" from offset waves (do we mean each box? do we mean 12 matrix?)
               but we can say "cross back", a call which presumably will be perceived as unambiguous. */
               
            /* See if this call has applicable 2x2 definition, but not 2x3/3x2/2x4/4x2, in which case
               try to make an offset 2x4, which will cause the call to be done in each 2x2. */
      
            can_do_boxes =
                  (
                        assoc(b_2x2, ss, calldeflist) ||
                        assoc(b_1x2, ss, calldeflist) ||
                        assoc(b_2x1, ss, calldeflist) ||
                        assoc(b_1x1, ss, calldeflist)) &&
                  (!assoc(b_2x4, ss, calldeflist)) &&
                  (!assoc(b_4x2, ss, calldeflist)) &&
                  (!assoc(b_2x3, ss, calldeflist)) &&
                  (!assoc(b_3x2, ss, calldeflist)) &&
                  (!assoc(b_dmd, ss, calldeflist)) &&
                  (!assoc(b_pmd, ss, calldeflist)) &&
                  (!assoc(b_qtag, ss, calldeflist)) &&
                  (!assoc(b_pqtag, ss, calldeflist));
      
            /* See if offset lines/columns spots are occupied.  If so, the quarter-tag fudge would not
               work.  The only thing to do is check for the legality of 2x2 division and then remove the offset. */
      
            switch (livemask) {
               case 07474:
                  if (!can_do_boxes) fail("Don't know how to do this call in this setup.");
                  warn(warn__each2x2);
                  division_maps = (*map_lists[s2x2][1])[MPKIND__OFFS_L_HALF][0];
                  goto divide_us_no_recompute;
               case 06363:
                  if (!can_do_boxes) fail("Don't know how to do this call in this setup.");
                  warn(warn__each2x2);
                  division_maps = (*map_lists[s2x2][1])[MPKIND__OFFS_R_HALF][0];
                  goto divide_us_no_recompute;
            }
   
            /* Now check for something that can be fudged into a "quarter tag"
               (which includes diamonds).  Note whether we fudged,
               since some calls do not tolerate it. */
   
            really_fudged = FALSE;
   
            (void) copy_person(&sss, 0, ss, 1);
            (void) copy_person(&sss, 1, ss, 2);
            (void) copy_person(&sss, 2, ss, 4);
            (void) copy_person(&sss, 3, ss, 5);
            (void) copy_person(&sss, 4, ss, 7);
            (void) copy_person(&sss, 5, ss, 8);
            (void) copy_person(&sss, 6, ss, 10);
            (void) copy_person(&sss, 7, ss, 11);

            if (ss->people[0].id1) {
               if (ss->people[1].id1) fail("Can't do this call from arbitrary 3x4 setup.");
               else (void) copy_person(&sss, 0, ss, 0);
               really_fudged = TRUE;
            }
            if (ss->people[3].id1) {
               if (ss->people[2].id1) fail("Can't do this call from arbitrary 3x4 setup.");
               else (void) copy_person(&sss, 1, ss, 3);
               really_fudged = TRUE;
            }
            if (ss->people[6].id1) {
               if (ss->people[7].id1) fail("Can't do this call from arbitrary 3x4 setup.");
               else (void) copy_person(&sss, 4, ss, 6);
               really_fudged = TRUE;
            }
            if (ss->people[9].id1) {
               if (ss->people[8].id1) fail("Can't do this call from arbitrary 3x4 setup.");
               else (void) copy_person(&sss, 5, ss, 9);
               really_fudged = TRUE;
            }
         
            sss.kind = s_qtag;
            sss.rotation = ss->rotation;
            sss.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
            ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;
            move(&sss, parseptr, callspec, final_concepts, really_fudged, result);
         }

         goto un_mirror;
      case s_qtag:
         if (assoc(b_dmd, ss, calldeflist) || assoc(b_pmd, ss, calldeflist)) {
            division_maps = (*map_lists[sdmd][1])[MPKIND__SPLIT][1];
            goto divide_us_no_recompute;
         }
         else if ((!(newtb & 010) || assoc(b_1x2, ss, calldeflist)) &&
                  (!(newtb & 1) || assoc(b_2x1, ss, calldeflist))) {
            concentric_move(ss, parseptr, parseptr, callspec, callspec, final_concepts, final_concepts, schema_concentric, 0, 0, result);
            goto un_mirror;
         }
         else if ((livemask & 0x55) == 0) {    /* Check for stuff like "heads pass the ocean; side corners */
            division_maps = &map_qtag_f1;      /* only slide thru". */
            goto divide_us_no_recompute;
         }
         else if ((livemask & 0x66) == 0) {
            division_maps = &map_qtag_f2;
            goto divide_us_no_recompute;
         }
         else if ((livemask & 0x77) == 0) {    /* Check for stuff like "center two slide thru". */
            division_maps = &map_qtag_f0;
            goto divide_us_no_recompute;
         }
         break;
      case s_bone:
         {
            int tbi = ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1;
            int tbo = ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1;

            if ((!((tbi & 010) | (tbo & 001)) || assoc(b_1x2, ss, calldeflist)) &&
                     (!((tbi & 001) | (tbo & 010)) || assoc(b_2x1, ss, calldeflist))) {
               concentric_move(ss, parseptr, parseptr, callspec, callspec, final_concepts, final_concepts, schema_concentric, 0, 0, result);
               goto un_mirror;
            }
         }
         break;
      case s_ptpd:
         if (assoc(b_dmd, ss, calldeflist) || assoc(b_pmd, ss, calldeflist)) {
            division_maps = (*map_lists[sdmd][1])[MPKIND__SPLIT][0];
            goto divide_us_no_recompute;
         }
         break;
      case s_2x3:
         /* See if this call has applicable 1x2 or 2x1 definitions, in which case split it 3 ways. */
            
         if (assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist) || assoc(b_1x1, ss, calldeflist)) {
            division_maps = (*map_lists[s_1x2][2])[MPKIND__SPLIT][1];
            goto divide_us_no_recompute;
         }
         break;
      case s_short6:
         if (assoc(b_trngl, ss, calldeflist) || assoc(b_ptrngl, ss, calldeflist) ||
               assoc(b_1x1, ss, calldeflist) || assoc(b_2x2, ss, calldeflist)) {
            division_maps = &map_s6_trngl;
            goto divide_us_no_recompute;
         }
         break;
      case s_bone6:
         if (assoc(b_trngl, ss, calldeflist) || assoc(b_ptrngl, ss, calldeflist) ||
               assoc(b_1x1, ss, calldeflist) || assoc(b_2x2, ss, calldeflist)) {
            division_maps = &map_b6_trngl;
            goto divide_us_no_recompute;
         }
         break;
      case s_trngl:
         if (assoc(b_2x2, ss, calldeflist)) {
            if (ss->setupflags & SETUPFLAG__SAID_TRIANGLE) {
               if (final_concepts & FINAL__TRIANGLE)
                  fail("'Triangle' concept is redundant.");
            }
            else {
               if (!(final_concepts & FINAL__TRIANGLE))
                  fail("You must give the 'triangle' concept.");
            }

            if ((ss->people[0].id1 & d_mask) == d_east)
               division_maps = &map_trngl_box1;
            else if ((ss->people[0].id1 & d_mask) == d_west)
               division_maps = &map_trngl_box2;
            else
               fail("Can't figure out which way triangle point is facing.");

            final_concepts &= ~FINAL__TRIANGLE;
            divided_setup_move(ss, parseptr, callspec, final_concepts, division_maps, phantest_ok, FALSE, result);

            switch (result->kind) {
               int rot;

               case s1x4:
                  if (result->people[0].id1 && !result->people[2].id1) {
                     (void) copy_person(result, 2, result, 3);
                  }
                  else if (result->people[2].id1 && !result->people[0].id1) {
                     (void) copy_person(result, 0, result, 1);
                     (void) copy_person(result, 1, result, 3);
                  }
                  else
                     fail("Bad ending setup for triangle-become-box.");
                  result->kind = s_1x3;
                  break;
               case s2x2:
                  if      (!result->people[0].id1) rot = 3;
                  else if (!result->people[1].id1) rot = 2;
                  else if (!result->people[2].id1) rot = 1;
                  else if (!result->people[3].id1) rot = 0;
                  else
                     fail("Bad ending setup for triangle-become-box.");

                  result->rotation += rot;
                  canonicalize_rotation(result);
                  /* Now the empty spot is in the lower-left corner. */
                  if ((!result->people[0].id1) || (!result->people[1].id1) || (!result->people[2].id1))
                     fail("Bad ending setup for triangle-become-box.");

                  if (result->people[0].id1 & result->people[2].id1 & 1) {
                     (void) copy_person(result, 3, result, 2);
                  }
                  else if (result->people[0].id1 & result->people[2].id1 & 010) {
                     rot--;
                     result->rotation--;
                     canonicalize_rotation(result);
                  }
                  else
                     fail("Bad ending setup for triangle-become-box.");

                  (void) copy_person(result, 2, result, 1);
                  (void) copy_person(result, 1, result, 0);
                  (void) copy_person(result, 0, result, 3);
                  result->kind = s_trngl;
                  result->rotation -= rot;   /* Put it back. */
                  canonicalize_rotation(result);
                  break;
               case sdmd:
                  if (result->people[0].id1 && !result->people[2].id1) {
                     (void) copy_rot(result, 2, result, 3, 033);
                     (void) copy_rot(result, 0, result, 0, 033);
                     (void) copy_rot(result, 1, result, 1, 033);
                     result->rotation++;
                  }
                  else if (result->people[2].id1 && !result->people[0].id1) {
                     (void) copy_rot(result, 0, result, 2, 011);
                     (void) copy_rot(result, 2, result, 1, 011);
                     (void) copy_rot(result, 1, result, 3, 011);
                     result->rotation--;
                  }
                  else
                     fail("Bad ending setup for triangle-become-box.");
                  result->kind = s_trngl;
                  break;
               default:
                  fail("Bad ending setup for triangle-become-box.");
            }

            goto un_mirror;
         }
         break;
      case s_1x6:
         /* See if this call has applicable 1x2 or 2x1 definitions, in which case split it 3 ways. */
            
         if (assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist) || assoc(b_1x1, ss, calldeflist)) {
            division_maps = (*map_lists[s_1x2][2])[MPKIND__SPLIT][0];
            goto divide_us_no_recompute;
         }
         break;
      case s_1x2:
         if (assoc(b_1x1, ss, calldeflist)) {
            division_maps = (*map_lists[s_1x1][1])[MPKIND__SPLIT][0];
            goto divide_us_no_recompute;
         }
         break;
      case sdmd:
         if (assoc(b_1x1, ss, calldeflist)) {
            division_maps = &map_dmd_1x1;
            goto divide_us_no_recompute;
         }
         break;
      case s1x8:
         /* See if the call has a 1x4, 4x1, 1x2, 2x1, or 1x1 definition, in which case split it and do each part. */
         
         if (    (assoc(b_1x4, ss, calldeflist) || assoc(b_4x1, ss, calldeflist) ||
                  assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist) ||
                  assoc(b_1x1, ss, calldeflist))) {
            division_maps = (*map_lists[s1x4][1])[MPKIND__SPLIT][0];
            goto divide_us_no_recompute;
         }
         break;
      case s1x4:
         /* See if the call has a 1x2, 2x1, or 1x1 definition, in which case split it and do each part. */
         
         if ((assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist) || assoc(b_1x1, ss, calldeflist))) {

            /* The following makes "ends hinge" work from a grand wave.  Any 1x4 -> 2x2 call
               that acts by dividing itself into 1x2's is presumed to want the people in each 1x2
               to stay near each other.  We signify that by flipping the elongation, which we
               had previously set perpendicular to the 1x4 axis, overriding anything that may
               have been in the call definition. */

            desired_elongation = orig_elongation ^ RESULTFLAG__ELONGATE_MASK;
            /* If the flags were zero and we complemented them so that both are set, that's not good. */
            if (desired_elongation == RESULTFLAG__ELONGATE_MASK)
               desired_elongation = 0;

            division_maps = (*map_lists[s_1x2][1])[MPKIND__SPLIT][0];
            goto divide_us_no_recompute;
         }
         break;
   }

   /* We are about to do the call by array! */

   do_the_call:

   ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

   funny = final_concepts & FINAL__FUNNY;
   inconsistent_rotation = 0;
   inconsistent_setup = 0;

   if (funny && (!(calldeflist->callarray_flags & CAF__FACING_FUNNY)))
      fail("Can't do this call 'funny'.");

   /* Check that "linedefinition" has been set up if we will need it. */
   
   goodies = (callarray *) 0;

   if ((newtb & 010) || four_way_startsetup) {
      if (!linedefinition) {
         switch (ss->kind) {
            case sdmd:
               fail("Can't handle people in diamond or PU quarter-tag for this call.");
            case s_trngl:
               fail("Can't handle people in triangle for this call.");
            case s_qtag:
               fail("Can't handle people in quarter tag for this call.");
            case s3x4:
               fail("Can't handle people in triple lines for this call.");
            case s_2x3: case s2x4:
               fail("Can't handle people in lines for this call.");
            case s_short6:
               fail("Can't handle people in tall/short 6 for this call.");
            case s2x2:
               fail("Can't handle people in box of 4 for this call.");
            case s_1x2:
               fail("Can't handle people in line of 2 for this call.");
            case s1x4:
               fail("Can't handle people in line of 4 for this call.");
            case s_1x6:
               fail("Can't handle people in line of 6 for this call.");
            case s1x8:
               fail("Can't handle people in line of 8 for this call.");
            case s_galaxy:
               fail("Can't handle people in galaxy for this call.");
            default:
               fail("Can't do this call.");
         }
      }
   
      if (linedefinition->restriction != cr_none) check_line_restriction(ss, linedefinition->restriction, linedefinition->callarray_flags);
      goodies = linedefinition;
   }
   
   /* Check that "coldefinition" has been set up if we will need it. */
   
   if ((newtb & 1) && (!four_way_startsetup)) {
      if (!coldefinition) {
         switch (ss->kind) {
            case sdmd:
               fail("Can't handle people in diamond or normal quarter-tag for this call.");
            case s_trngl:
               fail("Can't handle people in triangle for this call.");
            case s_qtag: case s3x4:
               fail("Can't handle people in triple columns for this call.");
            case s_2x3: case s2x4:
               fail("Can't handle people in columns for this call.");
            case s_short6:
               fail("Can't handle people in tall/short 6 for this call.");
            case s_1x2:
               fail("Can't handle people in column of 2 for this call.");
            case s1x4:
               fail("Can't handle people in column of 4 for this call.");
            case s_1x6:
               fail("Can't handle people in column of 6 for this call.");
            case s1x8:
               fail("Can't handle people in column of 8 for this call.");
            case s_galaxy:
               fail("Can't handle people in galaxy for this call.");
            default:
               fail("Can't do this call.");
         }
      }

      if (coldefinition->restriction != cr_none) check_column_restriction(ss, coldefinition->restriction, coldefinition->callarray_flags);

      /* If we have linedefinition also, check for consistency. */
   
      if (goodies) {
         /* ***** should also check the other stupid fields! */
         inconsistent_rotation = (goodies->callarray_flags ^ coldefinition->callarray_flags) & CAF__ROT;
         if (goodies->end_setup != coldefinition->end_setup) inconsistent_setup = 1;
      }
   
      goodies = coldefinition;
   }

   result->kind = goodies->end_setup;
   
   if (result->kind == s_normal_concentric) {         /* ***** this requires an 8-person call definition */
      setup inners;
      setup outers;
      int outer_elongation;
      setup p1;

      if (inconsistent_rotation | inconsistent_setup) fail("This call is an inconsistent shape-changer.");
      if (funny) fail("Sorry, can't do this call 'funny'");

      clear_people(&p1);
   
      for (real_index=0; real_index<8; real_index++) {
         personrec this_person = ss->people[real_index];
         if (this_person.id1) {
            unsigned int z;
            int real_direction = this_person.id1 & 3;
            int d2 = (this_person.id1 << 1) & 4;
            northified_index = (real_index ^ d2);
            z = find_calldef((real_direction & 1) ? coldefinition : linedefinition, ss, real_index, real_direction, northified_index);
            k = ((z >> 4) & 017) ^ (d2 >> 1);
            install_person(&p1, k, ss, real_index);
            p1.people[k].id1 = (p1.people[k].id1 & ~(ROLL_MASK | 077)) | ((z + real_direction * 011) & 013) | ((z * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);
            p1.people[k].id1 &= ~STABLE_MASK;   /* For now, can't do fractional stable on this kind of call. */
         }
      }
   
      for (k=0; k<4; k++) {
         (void) copy_person(&inners, k, &p1, k);
         (void) copy_person(&outers, k, &p1, k+4);
      }

      inners.setupflags = 0;
      outers.setupflags = 0;
      inners.kind = goodies->end_setup_in;
      outers.kind = goodies->end_setup_out;
      inners.rotation = goodies->callarray_flags & CAF__ROT;
      outers.rotation = (goodies->callarray_flags & CAF__ROT_OUT) ? 1 : 0;
   
      /* For calls defined by array with concentric end setup, the "parallel_conc_end" flag
         turns on the outer elongation. */
      outer_elongation = outers.rotation;
      if (callspec->callflags & cflag__parallel_conc_end) outer_elongation ^= 1;

      normalize_concentric(schema_concentric, 1, &inners, &outers, outer_elongation, result);

      goto fixup;
   }
   else {   
      int lilresult_mask[2];
      setup_kind tempkind;

      result->rotation = goodies->callarray_flags & CAF__ROT;

      collision_mask = 0;
      result_mask = 0;
      num = setup_limits[ss->kind]+1;
      halfnum = num >> 1;
      tempkind = result->kind;
      lilresult_mask[0] = 0;
      lilresult_mask[1] = 0;

      if (funny) {
         if ((ss->kind != result->kind) || result->rotation || inconsistent_rotation || inconsistent_setup)
            fail("Can't do 'funny' shape-changer.");
      }

      if (four_way_startsetup) {
         special_4_way_symm(linedefinition, ss, newpersonlist, newplacelist, lilresult_mask, result);
      }
      else if (ss->kind == s_trngl) {
         if (inconsistent_rotation | inconsistent_setup) fail("This call is an inconsistent shape-changer.");
         special_triangle(coldefinition, linedefinition, ss, newpersonlist, newplacelist);

         /* Check whether the call went into the other triangle.  If so, it
            must have done so completely. */
         if (newplacelist[0] >=3 || newplacelist[1] >=3 || newplacelist[2] >=3) {
            result->rotation += 2;

            for (i=0; i<3; i++) {
               if (newplacelist[i] >=0) {
                  newplacelist[i] -= 3;
                  if (newplacelist[i] < 0)
                     fail("Call went into other triangle????.");
                  newpersonlist[i].id1 = rotperson(newpersonlist[i].id1, 022);
               }
            }
         }
      }
      else {
         int *final_translatec = identity;
         int *final_translatel = identity;
         int rotfudge = 0;
         int numout;
         int halfnumout;

         numout = setup_limits[result->kind]+1;
         halfnumout = numout >> 1;

         if (inconsistent_setup) {
            if (inconsistent_rotation &&
                     result->kind == s_spindle &&
                     linedefinition->end_setup == s_crosswave) {
               result->kind = s_x1x6;
               tempkind = s_x1x6;
               final_translatec = ftcspn;

               if (goodies->callarray_flags & CAF__ROT) {
                  final_translatel = &ftlcwv[0];
                  rotfudge = 3;
               }
               else {
                  final_translatel = &ftlcwv[4];
                  rotfudge = 1;
               }
            }
            else
               fail("T-bone call went to a weird setup.");
         }
         else if (inconsistent_rotation) {
            if (result->kind == s2x4) {
               result->kind = s4x4;
               tempkind = s4x4;
               final_translatec = ftc2x4;

               if (goodies->callarray_flags & CAF__ROT) {
                  final_translatel = &ftl2x4[0];
                  rotfudge = 3;
               }
               else {
                  final_translatel = &ftl2x4[4];
                  rotfudge = 1;
               }
            }
            else
               fail("This call is an inconsistent shape-changer.");
         }

         for (real_index=0; real_index<num; real_index++) {
            int kt;
            unsigned int z;
            personrec this_person = ss->people[real_index];
            newpersonlist[real_index].id1 = 0;
            newpersonlist[real_index].id2 = 0;
            if (this_person.id1) {
               int final_direction;
               int real_direction = this_person.id1 & 3;
               int d2 = ((this_person.id1 >> 1) & 1) * halfnum;
               int d2out = ((this_person.id1 >> 1) & 1) * halfnumout;
               northified_index = (real_index + d2) % num;
               z = find_calldef((real_direction & 1) ? coldefinition : linedefinition,
                     ss, real_index, real_direction, northified_index);
               k = (((z >> 4) & 017) + d2out) % numout;
               final_direction = real_direction;
               /* Line people are going into wrong rotation. */
               if (!(real_direction & 1)) final_direction = (final_direction + rotfudge) & 3;
               newpersonlist[real_index].id1 = (this_person.id1 & ~(ROLL_MASK | 077)) |
                     ((z + final_direction * 011) & 013) |
                     ((z * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);

               if (this_person.id1 & STABLE_ENAB) {
                  do_stability(&newpersonlist[real_index].id1, z, (z + final_direction - real_direction + result->rotation));
               }

               newpersonlist[real_index].id2 = this_person.id2;
               kt = ((real_direction & 1) ? final_translatec : final_translatel) [k];
               if (kt < 0) fail("T-bone call went to a weird and confused setup.");
               newplacelist[real_index] = kt;
               lilresult_mask[0] |= (1 << kt);
            }
         }
      }

      /* If the result setup size is larger than the starting setup size, we assume that this call
         has a concocted result setup (e.g. squeeze the galaxy, unwrap the galaxy, change your
         image), and we try to compress it.  We claim that, if the result size given
         in the calls database is bigger than the starting size, as it is for those calls, we
         don't really want that big setup, but want to compress it immediately if possible.
         Q: Why don't we just let the natural setup normalization that will occur later do this
         for us?  A: That only happens at the top level.  In this case, we consider the compression
         to be part of doing the call.  If we someday were able to do a reverse flip of split
         phantom galaxies, we would want each galaxy to compress itself to a 2x4 before
         reassembling them.
      If we handle 4x4 start setups in the future, we would NOT want to compress
         a 4x4 to a 2x4 after a 16-matrix circulate!!!!!  This is why we compare the beginning and
         ending setup sizes. */

      if (setup_limits[ss->kind] < setup_limits[result->kind]) {
         int *permuter = (int *) 0;
         int rotator = 0;

         if (result->kind == s4x4) {
            /* See if people landed on 2x4 spots. */
            if ((lilresult_mask[0] & 0x7171) == 0) {
               result->kind = s2x4;
               permuter = galtranslateh;
            }
            else if ((lilresult_mask[0] & 0x1717) == 0) {
               result->kind = s2x4;
               permuter = galtranslatev;
               rotator = 1;
            }
            else {
               /* If fakery occurred and we were not able to compress it, that is an error.  It
                  means the points got confused on a reverse flip the galaxy.  If we were not able
                  to compress a 4x4 but no fakery occurred, we let it pass.  That simply means
                  that the unwrappers were T-boned in an unwrap the galaxy, so they led out
                  in strange directions.  Is this the right thing to do?  Do we want to allow
                  T-boned reverse flip?  Probably not. */
      
               if (tempkind != s4x4) fail("Galaxy call went to improperly-formed setup.");
            }
         }
         else if (result->kind == s_x1x6) {
            if ((lilresult_mask[0] & 07070) == 0) {         /* Check horiz 1x6 spots. */
               result->kind = s_1x6;
               permuter = s1x6translateh;
            }
            else if ((lilresult_mask[0] & 00707) == 0) {    /* Check vert 1x6 spots. */
               result->kind = s_1x6;
               permuter = s1x6translatev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 01414) == 0) {    /* Check horiz xwv spots. */
               result->kind = s_crosswave;
               permuter = sxwvtranslateh;
            }
            else if ((lilresult_mask[0] & 04141) == 0) {    /* Check vert xwv spots. */
               result->kind = s_crosswave;
               permuter = sxwvtranslatev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 06060) == 0) {    /* Check horiz 3dmd spots w/points out far. */
               result->kind = s_3dmd;
               permuter = s3dmftranslateh;
            }
            else if ((lilresult_mask[0] & 00606) == 0) {    /* Check vert 3dmd spots w/points out far. */
               result->kind = s_3dmd;
               permuter = s3dmftranslatev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 05050) == 0) {    /* Check horiz 3dmd spots w/points in close. */
               result->kind = s_3dmd;
               permuter = s3dmntranslateh;
            }
            else if ((lilresult_mask[0] & 00505) == 0) {    /* Check vert 3dmd spots w/points in close. */
               result->kind = s_3dmd;
               permuter = s3dmntranslatev;
               rotator = 1;
            }
            else
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == s_8x8) {
            /* See if people landed on 2x8 spots. */
            if ((lilresult_mask[0] & 0x77770FFF) == 0 && (lilresult_mask[1] & 0x77770FFF) == 0) {
               result->kind = s2x8;
               permuter = octtranslateh;
            }
            else if ((lilresult_mask[0] & 0x0FFF7777) == 0 && (lilresult_mask[1] & 0x0FFF7777) == 0) {
               result->kind = s2x8;
               permuter = octtranslatev;
               rotator = 1;
            }
            else
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == s_x4dmd) {
            /* See if people landed on quad diamond spots. */
            if ((lilresult_mask[0] & 0xAF50AF50) == 0) {
               result->kind = s_4dmd;
               permuter = qdmtranslateh;
            }
            else if ((lilresult_mask[0] & 0x50AF50AF) == 0) {
               result->kind = s2x8;
               permuter = qdmtranslatev;
               rotator = 1;
            }
            else
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == s_hyperglass) {
            /* We check for 1x4's before checking for diamonds. */
            if ((lilresult_mask[0] & 07474) == 0) {
               result->kind = s1x4;
               permuter = linehyperh;
            }
            else if ((lilresult_mask[0] & 04747) == 0) {
               result->kind = s1x4;
               permuter = linehyperv;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 05656) == 0) {
               result->kind = sdmd;
               permuter = dmdhyperh;
            }
            else if ((lilresult_mask[0] & 06565) == 0) {
               result->kind = sdmd;
               permuter = dmdhyperv;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 01212) == 0) {
               result->kind = s_hrglass;
               permuter = galhyperh;
            }
            else if ((lilresult_mask[0] & 02121) == 0) {
               result->kind = s_hrglass;
               permuter = galhyperv;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 05555) == 0) {
               result->kind = s_star;
               permuter = starhyperh;
            }
            else if ((lilresult_mask[0] & 06666) == 0) {
               result->kind = s_star;        /* Actually, this is a star with all people sort of far away
                                             from the center.  We may need to invent a new setup, "farstar". */
               permuter = fstarhyperh;
            }
            else
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == s_2x3) {
            /* This call turned a smaller setup (like a 1x4) into a 2x3.
               It is presumably a call like "pair the line" or "step and slide".
               Flag the result setup so that the appropriate phantom-squashing
               will take place if two of these results are placed end-to-end. */
            resultflags |= RESULTFLAG__EXPAND_TO_2X3;
         }

         if (permuter) {
            if (rotator) {
               for (real_index=0; real_index<num; real_index++) {
                  if (ss->people[real_index].id1) {
                     newplacelist[real_index] = permuter[newplacelist[real_index]];
                     newpersonlist[real_index].id1 = rotccw(newpersonlist[real_index].id1);
                  }
               }
               result->rotation++;
            }
            else {
               for (real_index=0; real_index<num; real_index++) {
                  if (ss->people[real_index].id1) newplacelist[real_index] = permuter[newplacelist[real_index]];
               }
            }
         }
      }
   }

   /* Install all the people. */

   final_numout = setup_limits[result->kind]+1;

   for (real_index=0; real_index<num; real_index++) {
      personrec newperson = newpersonlist[real_index];
      if (newperson.id1) {
         if (funny) {
            if (newperson.id1 != -1) {           /* We only handle people who haven't been erased. */
               k = real_index;
               j = real_index;               /* j will move twice as fast as k, looking for a loop not containing starting point. */
               do {
                  j = newplacelist[j];
                  /* If hit a phantom, we can't proceed. */
                  if (!newpersonlist[j].id1) fail("Can't do 'funny' call with phantoms.");
                  /* If hit an erased person, we have clearly hit a loop not containing starting point. */
                  else if (newpersonlist[j].id1 == -1) break;
                  j = newplacelist[j];
                  if (!newpersonlist[j].id1) fail("Can't do 'funny' call with phantoms.");
                  else if (newpersonlist[j].id1 == -1) break;
                  k = newplacelist[k];
                  if (k == real_index) goto funny_win;
               } while (k != j);

               /* This person can't move, because he moves into a loop not containing his starting point. */
               k = real_index;
               newperson.id1 = (ss->people[real_index].id1 & ~ROLL_MASK) | ROLLBITM;
               newperson.id2 = ss->people[real_index].id2;
               result->people[k] = newperson;
               newpersonlist[k].id1 = -1;
               funny_ok1 = TRUE;    /* Someone decided not to move.  Hilarious. */
               goto funny_end;

               funny_win:
               /* Move the entire loop, replacing people with -1. */
               k = real_index;
               j = 0;      /* See how long the loop is. */
               do {
                  newperson = newpersonlist[k];
                  newpersonlist[k].id1 = -1;
                  k = newplacelist[k];
                  result->people[k] = newperson;
                  j++;
               } while (k != real_index);

               if (j > 2) warn(warn__hard_funny);
               funny_ok2 = TRUE;    /* Someone was able to move.  Hysterical. */
                                    /* Actually, I don't see how this test can fail.
                                       Someone can always move! */

               funny_end: ;
            }
         }
         else {              /* Un-funny move. */
            k = newplacelist[real_index];
            if (result->people[k].id1 == 0) {
               result->people[k] = newperson;
               result_mask |= (1 << k);
            }
            else if ((callspec->callflags & cflag__take_right_hands) && (final_numout <= 12) && (result->people[k+12].id1 == 0)) {
               /* Collisions are legal.  Store the person in the overflow area
                  (12 higher than the main area, which is why we only permit
                  this if the result setup size is <= 12) and record the fact
                  in the collision_mask so we can figure out what to do. */
               result->people[k+12] = newperson;
               collision_mask |= (1 << k);
               collision_index = k;        /* In case we need to report a mundane collision. */
            }
            else {
               collision_person1 = result->people[k].id1;
               collision_person2 = newperson.id1;
               error_message1[0] = '\0';
               error_message2[0] = '\0';
               longjmp(longjmp_ptr->the_buf, 3);
            }
         }
      }
   }

   if (funny) {
      if (!funny_ok1 || !funny_ok2) warn(warn__not_funny);
   }
   else {
      if (collision_mask) fix_collision(collision_mask, collision_index, result_mask, result);
   }
   
   fixup:

   reinstate_rotation(ss, result);
   goto un_mirror;

   divide_us:

   divided_setup_move(ss, parseptr, callspec, final_concepts, division_maps, phantest_ok, TRUE, result);
   goto un_mirror;

   divide_us_no_recompute:

   divided_setup_move(ss, parseptr, callspec, final_concepts, division_maps, phantest_ok, FALSE, result);

   /* If expansion to a 2x3 occurred (because the call was, for example, a "pair the line"),
      and the two 2x3's are end-to-end in a 2x6, see if we can squash phantoms.  We squash both
      internal (the center triple box) and external ones.  The external ones would probably have
      been squashed anyway due to the top level normalization, but we want this to occur
      immediately, not just at the top level, though we can't think of a concrete example
      in which it makes a difference. */
   if ((result->setupflags & RESULTFLAG__EXPAND_TO_2X3) && (result->kind == s2x6)) {
      if (!(result->people[2].id1 | result->people[3].id1 | result->people[8].id1 | result->people[9].id1)) {
         /* Inner spots are empty. */
         result->kind = s2x4;
         (void) copy_person(result, 2, result, 4);            /* careful -- order is important */
         (void) copy_person(result, 3, result, 5);
         (void) copy_person(result, 4, result, 6);
         (void) copy_person(result, 5, result, 7);
         (void) copy_person(result, 6, result, 10);
         (void) copy_person(result, 7, result, 11);
      }
      else if (!(result->people[0].id1 | result->people[5].id1 | result->people[6].id1 | result->people[11].id1)) {
         /* Outer spots are empty. */
         result->kind = s2x4;
         (void) copy_person(result, 0, result, 1);            /* careful -- order is important */
         (void) copy_person(result, 1, result, 2);
         (void) copy_person(result, 2, result, 3);
         (void) copy_person(result, 3, result, 4);
         (void) copy_person(result, 4, result, 7);
         (void) copy_person(result, 5, result, 8);
         (void) copy_person(result, 6, result, 9);
         (void) copy_person(result, 7, result, 10);
      }
   }

   un_mirror:

   /* We take out any elongation info that divided_setup_move may have put in
      and override it with the correct info. */

   result->setupflags &= ~RESULTFLAG__ELONGATE_MASK;
   result->setupflags |= resultflags | desired_elongation;
}
