/* SD -- square dance caller's helper.

    Copyright (C) 1990-1997  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 31. */

/* This defines the following functions:
   mirror_this
   fix_collision
   do_stability
   check_restriction
   basic_move
*/

#include "sd.h"



/* This file uses a few bogus setups.  They are never allowed to escape:
   s8x8 =
     0   1   2   3  28  24  20  16
     4   5   6   7  29  25  21  17
     8   9  10  11  30  26  22  18
    12  13  14  15  31  27  23  19
    51  55  59  63  47  46  45  44
    50  54  58  62  43  42  41  40
    49  53  57  61  39  38  37  36
    48  52  56  60  35  34  33  32

   sx4dmd =
      4        8       15
   7     5     9    14    12
      6       10       13
              11
   0  1  2  3    19 18 17 16
              27
     29       26       22
  28    30    25    21    23
     31       24       20

   sx1x6 =
            3
            4
            5
   0  1  2     8  7  6
           11
           10
            9

   sx1x16 =
                     8
                     9
                    ...
                    14
                    15
   0  1  ....  6  7     23 22 ... 17 16
                    31
                    30
                    ...
                    25
                    24
*/



static coordrec tgl4_0 = {s_trngl4, 3,
   {  0,   0,  -2,   2},
   { -4,   0,   4,   4}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  2,  3, -1, -1, -1,
      -1, -1, -1, -1,  1, -1, -1, -1,
      -1, -1, -1, -1,  0, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};


static coordrec tgl4_1 = {s_trngl4, 3,
   { -4,   0,   4,   4},
   {  0,   0,   2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2, -1, -1,
      -1, -1, -1, -1, -1,  3, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};


extern void mirror_this(setup *s)
{
   uint32 l, r, z, n, t;
   coordrec *cptr;
   int i, x, y, place, limit;

   setup temp = *s;

   if (s->kind == nothing) return;

   cptr = setup_attrs[s->kind].nice_setup_coords;

   if (!cptr) {
      if (s->kind == s_trngl4) {
         if (s->rotation & 1) {
            s->rotation += 2;
            for (i=0; i<4; i++) (void) copy_rot(s, i, s, i, 022);
            cptr = &tgl4_1;
         }
         else
            cptr = &tgl4_0;
      }
      else if (s->kind == s_normal_concentric) {
         int i;

         if (     s->inner.skind == s_normal_concentric ||
                  s->outer.skind == s_normal_concentric ||
                  s->inner.skind == s_dead_concentric ||
                  s->outer.skind == s_dead_concentric)
            fail("Recursive concentric?????.");

         s->kind = s->inner.skind;
         s->rotation = s->inner.srotation;
         mirror_this(s);    /* Sorry! */
         s->inner.srotation = s->rotation;

         s->kind = s->outer.skind;
         s->rotation = s->outer.srotation;
         for (i=0 ; i<12 ; i++) swap_people(s, i, i+12);
         mirror_this(s);    /* Sorrier! */
         for (i=0 ; i<12 ; i++) swap_people(s, i, i+12);
         s->outer.srotation = s->rotation;

         s->kind = s_normal_concentric;
         s->rotation = 0;
         return;
      }
      else if (s->kind == s_dead_concentric) {
         if (s->inner.skind == s_normal_concentric || s->inner.skind == s_dead_concentric)
            fail("Recursive concentric?????.");

         s->kind = s->inner.skind;
         s->rotation = s->inner.srotation;
         mirror_this(s);    /* Sorry! */
         s->inner.srotation = s->rotation;

         s->kind = s_dead_concentric;
         s->rotation = 0;
         return;
      }
      else
         fail("Don't recognize ending setup for this call; not able to do it mirror.");
   }

   limit = setup_attrs[s->kind].setup_limits;

   if (s->rotation & 1) {
      for (i=0; i<=limit; i++) {
         x = cptr->xca[i];
         y = - cptr->yca[i];
         place = cptr->diagram[28 - ((y >> 2) << cptr->xfactor) + (x >> 2)];

         if ((place < 0) || (cptr->xca[place] != x) || (cptr->yca[place] != y))
            fail("Don't recognize ending setup for this call; not able to do it mirror.");

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
            fail("Don't recognize ending setup for this call; not able to do it mirror.");

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
   uint32 lmask;             /* which people are facing E-W in original double-length setup */
   uint32 rmask;             /* where in original setup can people be found */
   uint32 cmask;             /* where in original setup have people collided */
   int source[12];           /* where to get the people */
   int map0[12];             /* where to put the north (or east)-facer */
   int map1[12];             /* where to put the south (or west)-facer */
   setup_kind initial_kind;  /* what setup they are collided in */
   setup_kind final_kind;    /* what setup to change it to */
   int rot;                  /* whether to rotate final setup CW */
   warning_index warning;    /* an optional warning to give */
} collision_map;

static collision_map collision_map_table[] = {
   /* These items handle various types of "1/2 circulate" calls from 2x4's. */
   {4, 0x000000, 0x33, 0x33, {0, 1, 4, 5},         {0, 3, 5, 6},          {1, 2, 4, 7},           s_crosswave, s1x8,        0, warn__none},   /* from lines in */
   {4, 0x0CC0CC, 0xCC, 0xCC, {2, 3, 6, 7},         {0, 3, 5, 6},          {1, 2, 4, 7},           s_crosswave, s1x8,        1, warn__none},   /* from lines out */
   {4, 0x000000, 0x0F, 0x0F, {0, 1, 2, 3},         {0, 3, 5, 6},          {1, 2, 4, 7},           s1x4,        s1x8,        0, warn__none},   /* more of same */
   {4, 0x022022, 0xAA, 0xAA, {1, 3, 5, 7},         {2, 5, 7, 0},          {3, 4, 6, 1},           s_spindle,   s_crosswave, 0, warn__none},   /* from trade by */
   {2, 0x022022, 0x22, 0x22, {1, 5},               {2, 7},                {3, 6},                 s_spindle,   s_crosswave, 0, warn__none},   /* from DPT/trade by with no ends */
   {6, 0x0880CC, 0xDD, 0x88, {0, 2, 3, 4, 6, 7},   {7, 0, 1, 3, 4, 6},    {7, 0, 2, 3, 4, 5},     s_crosswave, s3x1dmd,     1, warn__none},   /* from 3&1 lines w/ centers in */
   {6, 0x000044, 0x77, 0x22, {0, 1, 2, 4, 5, 6},   {0, 1, 3, 4, 6, 7},    {0, 2, 3, 4, 5, 7},     s_crosswave, s3x1dmd,     0, warn__none},   /* from 3&1 lines w/ centers out */
   {6, 0x0440CC, 0xEE, 0x44, {1, 2, 3, 5, 6, 7},   {7, 0, 2, 3, 5, 6},    {7, 1, 2, 3, 4, 6},     s_crosswave, s3x1dmd,     1, warn__none},   /* from 3&1 lines w/ ends in */
   {6, 0x000088, 0xBB, 0x11, {0, 1, 3, 4, 5, 7},   {0, 2, 3, 5, 6, 7},    {1, 2, 3, 4, 6, 7},     s_crosswave, s1x3dmd,     0, warn__none},   /* from 3&1 lines w/ ends out */
   {4, 0x088088, 0x99, 0x99, {0, 3, 4, 7},         {0, 2, 5, 7},          {1, 3, 4, 6},           s_crosswave, s_crosswave, 0, warn__none},   /* from inverted lines w/ centers in */
   {4, 0x044044, 0x66, 0x66, {1, 2, 5, 6},         {6, 0, 3, 5},          {7, 1, 2, 4},           s_crosswave, s_crosswave, 1, warn__ctrs_stay_in_ctr}, /* from inverted lines w/ centers out */
   {4, 0x044044, 0x55, 0x55, {0, 2, 4, 6},         {0, 2, 5, 7},          {1, 3, 4, 6},           s_crosswave, s_crosswave, 0, warn__ctrs_stay_in_ctr}, /* from trade-by w/ ctrs 1/4 out */
   /* This was put in so that "1/2 circulate" would work from lines in with centers counter rotated. */
   {4, 0x088088, 0xAA, 0xAA, {1, 3, 5, 7},         {0, 2, 5, 7},          {1, 3, 4, 6},           s_crosswave, s_crosswave, 0, warn__ctrs_stay_in_ctr}, /* from lines in and centers face */
   {6, 0x000082, 0xAAA, 0x820, {1, 3, 5, 7, 9, 11},{3, 4, 6, 7, 0, 1},    {3, 4, 5, 7, 0, 2},     s3dmd,       s3x1dmd,     0, warn__none},   /* from trade by with some outside quartered out */
   {6, 0x000088, 0x0DD, 0x044, {3, 4, 6, 7, 0, 2}, {3, 4, 6, 7, 0, 1},    {3, 4, 5, 7, 0, 2},     s3x1dmd,     s3x1dmd,     0, warn__none},   /* Same. */
   {6, 0x088088, 0xBB, 0x88, {0, 1, 3, 4, 5, 7},   {0, 1, 2, 4, 5, 7},    {0, 1, 3, 4, 5, 6},     s_crosswave, s_crosswave, 0, warn__none},
   {6, 0x0000CC, 0xDD, 0x11, {0, 2, 3, 4, 6, 7},   {0, 2, 3, 5, 6, 7},    {1, 2, 3, 4, 6, 7},     s_crosswave, s_crosswave, 0, warn__none},
   {6, 0x0000CC, 0xEE, 0x22, {1, 2, 3, 5, 6, 7},   {0, 2, 3, 5, 6, 7},    {1, 2, 3, 4, 6, 7},     s_crosswave, s_crosswave, 0, warn__none},
   {6, 0x000000,  077, 011,  {0, 1, 2, 3, 4, 5},   {0, 3, 2, 5, 7, 6},    {1, 3, 2, 4, 7, 6},     s1x6,        s1x8,        0, warn__none},
   {6, 0x000000,  077, 022,  {0, 1, 2, 3, 4, 5},   {0, 1, 2, 4, 7, 6},    {0, 3, 2, 4, 5, 6},     s1x6,        s1x8,        0, warn__none},
   {6, 0x000000,  077, 044,  {0, 1, 2, 3, 4, 5},   {0, 1, 3, 4, 5, 6},    {0, 1, 2, 4, 5, 7},     s1x6,        s1x8,        0, warn__none},
   {4, 0x000000,  055, 055,  {0, 2, 3, 5},         {0, 3, 5, 6},          {1, 2, 4, 7},           s1x6,        s1x8,        0, warn__none},

   /* These items handle parallel lines with people wedged on one end, and hence handle flip or cut the hourglass. */
   {6, 0x000000, 0x77, 0x11, {0, 1, 2, 4, 5, 6},   {0, 2, 3, 7, 8, 9},    {1, 2, 3, 6, 8, 9},     s2x4,        s2x6,        0, warn__none},
   {6, 0x000000, 0xEE, 0x88, {1, 2, 3, 5, 6, 7},   {2, 3, 4, 8, 9, 11},   {2, 3, 5, 8, 9, 10},    s2x4,        s2x6,        0, warn__none},
   /* These items handle single lines with people wedged on one end, and hence handle flip or cut the diamond. */
   {3, 0x000000, 0x0B, 0x01, {0, 1, 3},            {0, 2, 5},             {1, 2, 5},              s1x4,        s1x6,        0, warn__none},
   {3, 0x000000, 0x0E, 0x04, {1, 2, 3},            {2, 4, 5},             {2, 3, 5},              s1x4,        s1x6,        0, warn__none},
   /* These items handle single diamonds with people wedged on one end, and hence handle diamond circulate. */
   {3, 0x00000A, 0x0E, 0x04, {1, 2, 3},            {2, 4, 5},             {2, 3, 5},              sdmd,    s_1x2dmd,        0, warn__none},
   {3, 0x00000A, 0x0B, 0x01, {0, 1, 3},            {0, 2, 5},             {1, 2, 5},              sdmd,    s_1x2dmd,        0, warn__none},
   /* These items handle columns with people wedged everywhere, and hence handle unwraps of facing diamonds etc. */
   {4, 0x055055, 0x55, 0x55, {0, 2, 4, 6},         {12, 14, 2, 11},       {10, 3, 4, 6},          s2x4,        s4x4,        0, warn__none},
   {4, 0x0AA0AA, 0xAA, 0xAA, {1, 3, 5, 7},         {13, 0, 7, 9},         {15, 1, 5, 8},          s2x4,        s4x4,        0, warn__none},
   /* These items handle columns with people wedged in clumps, and hence handle gravitate from lefties etc. */
   {4, 0x033033, 0x33, 0x33, {0, 1, 4, 5},         {12, 13, 2, 7},        {10, 15, 4, 5},         s2x4,        s4x4,        0, warn__none},
   {4, 0x0CC0CC, 0xCC, 0xCC, {2, 3, 6, 7},         {14, 0, 11, 9},        {3, 1, 6, 8},           s2x4,        s4x4,        0, warn__none},
   /* Collision on ends of an "erase". */
   {1, 0x000000, 0x02, 0x02, {1},                  {3},                   {2},                    s1x2,        s1x4,        0, warn__none},
   {1, 0x000000, 0x01, 0x01, {0},                  {0},                   {1},                    s1x2,        s1x4,        0, warn__none},

   /* These items handle the situation from a 2FL/intlkd box circ/split box trade circ. */
/* Not sure what that comment meant, but we are trying to handle colliding 2FL-type circulates */
   {2, 0x000000, 0x03, 0x03, {0, 1},               {0, 2},                {1, 3},                 s2x4,        s2x8,        0, warn__none},
   {2, 0x000000, 0x0C, 0x0C, {2, 3},               {4, 6},                {5, 7},                 s2x4,        s2x8,        0, warn__none},
   {2, 0x000000, 0x30, 0x30, {5, 4},               {11, 9},               {10, 8},                s2x4,        s2x8,        0, warn__none},
   {2, 0x000000, 0xC0, 0xC0, {7, 6},               {15, 13},              {14, 12},               s2x4,        s2x8,        0, warn__none},
/* The preceding lines used to be these, which seems quite wrong:
   {2, 0x000000, 0x03, 0x03, {0, 1},               {0, 2},                {1, 3},                 s2x4,        s2x4,        0, warn__none},
   {2, 0x000000, 0x0C, 0x0C, {2, 3},               {0, 2},                {1, 3},                 s2x4,        s2x4,        0, warn__none},
   {2, 0x000000, 0x30, 0x30, {5, 4},               {7, 5},                {6, 4},                 s2x4,        s2x4,        0, warn__none},
   {2, 0x000000, 0xC0, 0xC0, {7, 6},               {7, 5},                {6, 4},                 s2x4,        s2x4,        0, warn__none},
*/

                  /* The warning "warn_bad_collision" in the warning field is special -- it means give that warning if it appears illegal.
                           If it doesn't appear illegal, don't say anything at all (other than the usual "take right hands".)
                           If anything appears illegal but does not have "warn_bad_collision" in this field, it is an ERROR. */
   {4, 0x000000, 0xAA, 0x0AA, {1, 3, 5, 7},        {2, 6, 11, 15},        {3, 7, 10, 14},         s2x4,        s2x8,        0, warn_bad_collision},
   {4, 0x000000, 0x55, 0x055, {0, 2, 4, 6},        {0, 4, 9, 13},         {1, 5, 8, 12},          s2x4,        s2x8,        0, warn_bad_collision},
   {4, 0x000000, 0x33, 0x033, {0, 1, 4, 5},        {0, 2, 9, 11},         {1, 3, 8, 10},          s2x4,        s2x8,        0, warn_bad_collision},
   {4, 0x000000, 0xCC, 0x0CC, {2, 3, 6, 7},        {4, 6, 13, 15},        {5, 7, 12, 14},         s2x4,        s2x8,        0, warn_bad_collision},

   /* These items handle various types of "1/2 circulate" calls from 2x2's. */
   {2, 0x000000, 0x05, 0x05, {0, 2},               {0, 3},                {1, 2},                 sdmd,        s1x4,        0, warn__none},   /* from couples out if it went to diamond */
   {1, 0x000000, 0x01, 0x01, {0},                  {0},                   {1},                    sdmd,        s1x4,        0, warn__none},   /* same, but with missing people */
   {1, 0x000000, 0x04, 0x04, {2},                  {3},                   {2},                    sdmd,        s1x4,        0, warn__none},   /* same, but with missing people */
   {2, 0x000000, 0x05, 0x05, {0, 2},               {0, 3},                {1, 2},                 s1x4,        s1x4,        0, warn__none},   /* from couples out if it went to line */
   {1, 0x000000, 0x01, 0x01, {0},                  {0},                   {1},                    s1x4,        s1x4,        0, warn__none},   /* same, but with missing people */
   {1, 0x000000, 0x04, 0x04, {2},                  {3},                   {2},                    s1x4,        s1x4,        0, warn__none},   /* same, but with missing people */
   {2, 0x00A00A, 0x0A, 0x0A, {1, 3},               {0, 3},                {1, 2},                 sdmd,        s1x4,        1, warn__none},   /* from couples in if it went to diamond */
   {2, 0x000000, 0x0A, 0x0A, {1, 3},               {0, 3},                {1, 2},                 s1x4,        s1x4,        0, warn__none},   /* from couples in if it went to line */
   {2, 0x000000, 0x06, 0x06, {1, 2},               {0, 3},                {1, 2},                 s1x4,        s1x4,        0, warn__none},   /* from "head pass thru, all split circulate" */
   {2, 0x000000, 0x09, 0x09, {0, 3},               {0, 3},                {1, 2},                 s1x4,        s1x4,        0, warn__none},   /* from "head pass thru, all split circulate" */
   /* These items handle "1/2 split trade circulate" from 2x2's.  They also do "switch to a diamond" when the ends come to the same spot in the center. */
   {3, 0x008008, 0x0D, 0x08, {0, 2, 3},            {0, 2, 1},             {0, 2, 3},              sdmd,        sdmd,        0, warn_bad_collision},
   {3, 0x002002, 0x07, 0x02, {0, 2, 1},            {0, 2, 1},             {0, 2, 3},              sdmd,        sdmd,        0, warn_bad_collision},
   /* These items handle various types of "circulate" calls from 2x2's. */
   {2, 0x009009, 0x09, 0x09, {0, 3},               {7, 5},                {6, 4},                 s2x2,        s2x4,        1, warn_bad_collision},   /* from box facing all one way */
   {2, 0x006006, 0x06, 0x06, {1, 2},               {0, 2},                {1, 3},                 s2x2,        s2x4,        1, warn_bad_collision},   /* we need all four cases */
   {2, 0x000000, 0x0C, 0x0C, {3, 2},               {7, 5},                {6, 4},                 s2x2,        s2x4,        0, warn_bad_collision},   /* sigh */
   {2, 0x000000, 0x03, 0x03, {0, 1},               {0, 2},                {1, 3},                 s2x2,        s2x4,        0, warn_bad_collision},   /* sigh */
   {2, 0x000000, 0x09, 0x09, {0, 3},               {0, 7},                {1, 6},                 s2x2,        s2x4,        0, warn_bad_collision},   /* from "inverted" box */
   {2, 0x000000, 0x06, 0x06, {1, 2},               {2, 5},                {3, 4},                 s2x2,        s2x4,        0, warn_bad_collision},   /* we need all four cases */
   {2, 0x003003, 0x03, 0x03, {1, 0},               {0, 7},                {1, 6},                 s2x2,        s2x4,        1, warn_bad_collision},   /* sigh */
   {2, 0x00C00C, 0x0C, 0x0C, {2, 3},               {2, 5},                {3, 4},                 s2x2,        s2x4,        1, warn_bad_collision},   /* sigh */





   /* These items handle horrible lockit collisions in the middle (from inverted lines, for example). */
   {2, 0x000000, 0x06, 0x06, {1, 2},               {3, 5},                {2, 4},                 s1x4,        s1x8,        0, warn_bad_collision},
   {2, 0x000000, 0x09, 0x09, {0, 3},               {0, 6},                {1, 7},                 s1x4,        s1x8,        0, warn_bad_collision},
/* Some new ones. */
   {2, 0x000000, 0x03, 0x03, {0, 1},               {0, 3},                {1, 2},                 s1x4,        s1x8,        0, warn_bad_collision},
   {2, 0x000000, 0x0C, 0x0C, {3, 2},               {6, 5},                {7, 4},                 s1x4,        s1x8,        0, warn_bad_collision},



   /* These items handle circulate in a short6, and hence handle collisions in 6X2 acey deucey. */
   {4, 0x12, 0x1B, 0x09, {0, 1, 3, 4},             {0, 2, 7, 8},           {1, 2, 6, 8},          s_short6,    sbigdmd,     0, warn__none},
   {4, 0x12, 0x36, 0x24, {1, 2, 4, 5},             {2, 4, 8, 11},          {2, 5, 8, 10},         s_short6,    sbigdmd,     0, warn__none},

   /* These items handle more 2x2 stuff, including the "special drop in" that makes chain reaction/motivate etc. work. */
   {2, 0x005005, 0x05, 0x05, {0, 2},               {7, 2},                {6, 3},                 s2x2,        s2x4,        1, warn__none},
   {2, 0x00A00A, 0x0A, 0x0A, {1, 3},               {0, 5},                {1, 4},                 s2x2,        s2x4,        1, warn__none},
   {2, 0x000000, 0x05, 0x05, {0, 2},               {0, 5},                {1, 4},                 s2x2,        s2x4,        0, warn__none},
   {2, 0x000000, 0x0A, 0x0A, {1, 3},               {2, 7},                {3, 6},                 s2x2,        s2x4,        0, warn__none},
   /* Same spot as points of diamonds. */
   {6, 0x022022, 0xEE, 0x22, {1, 2, 3, 5, 6, 7},   {0, 2, 3, 7, 8, 9},    {1, 2, 3, 6, 8, 9},     s_qtag,      sbigdmd,     1, warn__none},
   {6, 0x011011, 0xDD, 0x11, {0, 2, 3, 4, 6, 7},   {11, 2, 3, 4, 8, 9},   {10, 2, 3, 5, 8, 9},    s_qtag,      sbigdmd,     1, warn__none},
   {-1}};


extern void fix_collision(
   uint32 explicit_mirror_flag,
   int collision_mask,
   int collision_index,
   int result_mask,
   long_boolean appears_illegal,
   long_boolean mirror,
   setup *result)
{
   uint32 lowbitmask, flip;
   int i, temprot;
   collision_map *c_map_ptr;
   setup spare_setup = *result;

   clear_people(result);

   lowbitmask = 0;
   for (i=0; i<MAX_PEOPLE; i++) lowbitmask |= ((spare_setup.people[i].id1) & 1) << i;

   for (c_map_ptr = collision_map_table ; c_map_ptr->size >= 0 ; c_map_ptr++) {
      if ((result->kind == c_map_ptr->initial_kind) && ((lowbitmask == c_map_ptr->lmask)) && (result_mask == c_map_ptr->rmask) && (collision_mask == c_map_ptr->cmask)) {
         if (!appears_illegal || c_map_ptr->warning == warn_bad_collision)
            goto win;
      }
   }

   /* Don't recognize the pattern, report this as normal collision. */
   longjmp(longjmp_ptr->the_buf, 3);

   win:

   if (explicit_mirror_flag & CMD_MISC__EXPLICIT_MIRROR)
      warn(warn__take_left_hands);
   else
      warn(warn__take_right_hands);

   if (c_map_ptr->warning != warn_bad_collision || appears_illegal)
      warn(c_map_ptr->warning);

   temprot = ((-c_map_ptr->rot) & 3) * 011;
   result->kind = c_map_ptr->final_kind;
   result->rotation += c_map_ptr->rot;

   /* If this is under an implicit mirror image operation, make them take left hands, by swapping the maps. */
   flip = mirror ? 2 : 0;

   for (i=0; i<c_map_ptr->size; i++) {
      int oldperson;

      oldperson = spare_setup.people[c_map_ptr->source[i]].id1;
      if ((oldperson ^ flip) & 2)
         install_rot(result, c_map_ptr->map1[i], &spare_setup, c_map_ptr->source[i], temprot);
      else
         install_rot(result, c_map_ptr->map0[i], &spare_setup, c_map_ptr->source[i], temprot);

      oldperson = spare_setup.people[c_map_ptr->source[i]+12].id1;
      if ((oldperson ^ flip) & 2)
         install_rot(result, c_map_ptr->map1[i], &spare_setup, c_map_ptr->source[i]+12, temprot);
      else
         install_rot(result, c_map_ptr->map0[i], &spare_setup, c_map_ptr->source[i]+12, temprot);
   }
}


static veryshort identity[24] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
static veryshort ftc2x4[8] = {10, 15, 3, 1, 2, 7, 11, 9};
static veryshort ftc4x4[24] = {10, 15, 3, 1, 2, 7, 11, 9, 2, 7, 11, 9, 10, 15, 3, 1, 10, 15, 3, 1, 2, 7, 11, 9};
static veryshort ftcphan[24] = {0, 2, 7, 5, 8, 10, 15, 13, 8, 10, 15, 13, 0, 2, 7, 5, 0, 2, 7, 5, 8, 10, 15, 13};
static veryshort ftl2x4[12] = {6, 11, 15, 13, 14, 3, 7, 5, 6, 11, 15, 13};
static veryshort ftcspn[8] = {-1, 5, -1, 6, -1, 11, -1, 0};
static veryshort ftlcwv[12] = {9, 10, 1, 2, 3, 4, 7, 8, 9, 10, 1, 2};
static veryshort qtlqtg[12] = {5, -1, -1, 0, 1, -1, -1, 4, 5, -1, -1, 0};
static veryshort btlqtg[12] = {5, 0, -1, -1, 1, 4, -1, -1, 5, 0, -1, -1};
static veryshort qtlbone[12] = {0, 3, -1, -1, 4, 7, -1, -1, 0, 3, -1, -1};
static veryshort qtlxwv[12] = {0, 1, -1, -1, 4, 5, -1, -1, 0, 1, -1, -1};
static veryshort ft4x4bh[16] = {9, 8, 7, -1, 6, -1, -1, -1, 3, 2, 1, -1, 0, -1, -1, -1};
static veryshort ftqtgbh[8] = {-1, -1, 10, 11, -1, -1, 4, 5};
static veryshort galtranslateh[16] = {-1,  3,  4,  2, -1, -1, -1,  5, -1,  7,  0,  6, -1, -1, -1,  1};
static veryshort galtranslatev[16] = {-1, -1, -1,  1, -1,  3,  4,  2, -1, -1, -1,  5, -1,  7,  0,  6};
#ifdef BREAKS_CAST_BACK
static veryshort phan4x4xlatea[16] = {-1, -1,  8,  6, -1, -1, 12, 10, -1, -1,  0, 14, -1, -1,  4,  2};
static veryshort phan4x4xlateb[16] = {-1,  5, -1,  7, -1,  9, -1, 11, -1, 13, -1, 15, -1,  1, -1,  3};
#endif
static veryshort s1x6translateh[12] = {0, 1, 2, 0, 0, 0, 3, 4, 5, 0, 0, 0};
static veryshort s1x6translatev[12] = {0, 0, 0, 0, 1, 2, 0, 0, 0, 3, 4, 5};
static veryshort sxwvtranslateh[12] = {0, 1, 0, 0, 2, 3, 4, 5, 0, 0, 6, 7};
static veryshort sxwvtranslatev[12] = {0, 6, 7, 0, 1, 0, 0, 2, 3, 4, 5, 0};
static veryshort sdmdtranslateh[8] = {0, 0, 0, 1, 2, 0, 0, 3};
static veryshort sdmdtranslatev[8] = {0, 3, 0, 0, 0, 1, 2, 0};
static veryshort s3dmftranslateh[12] = {0, 1, 2, 3, 0, 0, 4, 5, 6, 7, 0, 0};
static veryshort s3dmntranslateh[12] = {0, 1, 2, 0, 3, 0, 4, 5, 6, 0, 7, 0};
static veryshort s3dmftranslatev[12] = {7, 0, 0, 0, 1, 2, 3, 0, 0, 4, 5, 6};
static veryshort s3dmntranslatev[12] = {0, 7, 0, 0, 1, 2, 0, 3, 0, 4, 5, 6};
static veryshort s_vacate_star[12]   = {0, 1, 2, 0, 0, 3, 4, 5, 6, 0, 0, 7};



static veryshort octtranslateh[64] = {
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,
   0,  0,  0,  7,  0,  0,  0,  6,  0,  0,  0,  5,  0,  0,  0,  4,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  9, 10, 11,
   0,  0,  0, 15,  0,  0,  0, 14,  0,  0,  0, 13,  0,  0,  0, 12};

static veryshort octtranslatev[64] = {
   0,  0,  0, 15,  0,  0,  0, 14,  0,  0,  0, 13,  0,  0,  0, 12,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,
   0,  0,  0,  7,  0,  0,  0,  6,  0,  0,  0,  5,  0,  0,  0,  4,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  9, 10, 11};

static veryshort octt4x6lateh[64] = {
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  0, 11, 10,  9,
   0,  0,  0,  0,  0,  0,  5,  6,  0,  0,  4,  7,  0,  0,  3,  8,
   0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 13, 14,  0, 23, 22, 21,
   0,  0,  0,  0,  0,  0, 17, 18,  0,  0, 16, 19,  0,  0, 15, 20};

static veryshort octt4x6latev[64] = {
   0,  0,  0,  0,  0,  0, 17, 18,  0,  0, 16, 19,  0,  0, 15, 20,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  0, 11, 10,  9,
   0,  0,  0,  0,  0,  0,  5,  6,  0,  0,  4,  7,  0,  0,  3,  8,
   0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 13, 14,  0, 23, 22, 21};

static veryshort hextranslateh[32] = {
   0,  1,  2,  3,  4,  5,  6,  7,  0,  0,  0,  0,  0,  0,  0,  0,
   8,  9, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0};

static veryshort hextranslatev[32] = {
   0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,
   0,  0,  0,  0,  0,  0,  0,  0,  8,  9, 10, 11, 12, 13, 14, 15};

static veryshort qdmtranslateh[32] = {
   12, 13, 14, 15,  0,  1,  0,  0,  0,   0,  0,  0,  3,  0,  2,  0,
   4,   5,  6,  7,  0,  9,  0,  8,  0,   0,  0,  0, 11,  0, 10,  0};

static veryshort qdmtranslatev[32] = {
   0,  0,  0,  0,  11, 0,  10, 0,  12, 13, 14, 15, 0,  1,  0,  0,
   0,  0,  0,  0,  3,  0,  2,  0,  4,  5,  6,  7,  0,  9,  0,  8};

static veryshort j23translateh[32] = {
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  2,  0,  0,
   0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  4,  0,  5,  0,  0};

static veryshort j23translatev[32] = {
   0,  0,  0,  4,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  1,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  3,  0};



static veryshort dmdhyperh[12] = {0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 3, 0};
static veryshort linehyperh[12] = {0, 1, 0, 0, 0, 0, 2, 3, 0, 0, 0, 0};
static veryshort galhyperh[12] = {6, 0, 0, 0, 3, 1, 2, 0, 4, 0, 7, 5};
static veryshort dmdhyperv[12] = {0, 3, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0};
static veryshort linehyperv[12] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 3, 0};
static veryshort galhyperv[12] = {0, 7, 5, 6, 0, 0, 0, 3, 1, 2, 0, 4};
static veryshort starhyperh[12] =  {0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 3, 0};
static veryshort fstarhyperh[12] = {0, 0, 0, 1, 0, 0, 2, 0, 0, 3, 0, 0};
static veryshort lilstar1[8] = {0, 2, 0, 0, 3, 0, 0, 1};
static veryshort lilstar2[8] = {3, 0, 0, 1, 0, 2, 0, 0};
static veryshort lilstar3[8] = {0, 1, 0, 0, 2, 3, 0, 0};
static veryshort lilstar4[8] = {0, 0, 2, 3, 0, 0, 0, 1};


static veryshort qtbd1[12] = {5, 9, 6, 7, 9, 0, 1, 9, 2, 3, 9, 4};
static veryshort qtbd2[12] = {9, 5, 6, 7, 0, 9, 9, 1, 2, 3, 4, 9};
static veryshort qtbd3[12] = {9, 5, 6, 7, 9, 0, 9, 1, 2, 3, 9, 4};
static veryshort qtbd4[12] = {5, 9, 6, 7, 0, 9, 1, 9, 2, 3, 4, 9};
static veryshort q3x4xx1[12] = {9, 5, 0, 9, 9, 1, 9, 2, 3, 9, 9, 4};
static veryshort q3x4xx2[12] = {9, 9, 9, 9, 2, 3, 9, 9, 9, 9, 0, 1};
static veryshort q3x4xx3[12] = {9, 9, 2, 2, 9, 9, 3, 3, 9, 9, 0, 1};
static veryshort q3x4xx4[12] = {3, 3, 9, 9, 0, 1, 9, 9, 2, 2, 9, 9};



extern void do_stability(uint32 *personp, stability stab, int turning)
{
   int t, at, st, atr;

   t = turning & 3;

   switch (stab) {
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
      case stb_ac:
         /* Turn one quadrant anticlockwise. */
         do_stability(personp, stb_a, 3);
         t += 1;   /* And the rest clockwise. */
         break;
      case stb_aac:
         /* Turn two quadrants anticlockwise. */
         do_stability(personp, stb_a, 2);
         if (t == 3) t = -1;  /* And the rest clockwise. */
         t += 2;
         break;
      case stb_aaac:
         /* Turn three quadrants anticlockwise. */
         do_stability(personp, stb_a, 1);
         if (t >= 2) t -= 4;  /* And the rest clockwise. */
         t += 3;
         break;
      case stb_aaaac:
         /* Turn four quadrants anticlockwise. */
         do_stability(personp, stb_a, 0);
         if (t == 0) t = 4;   /* And the rest clockwise. */
         break;
      case stb_ca:
         /* Turn one quadrant clockwise. */
         do_stability(personp, stb_c, 1);
         if (t == 0) t = 4;   /* And the rest anticlockwise. */
         t -= 5;
         break;
      case stb_cca:
         /* Turn two quadrants clockwise. */
         do_stability(personp, stb_c, 2);
         if (t <= 1) t += 4;  /* And the rest anticlockwise. */
         t -= 6;
         break;
      case stb_ccca:
         /* Turn three quadrants clockwise. */
         do_stability(personp, stb_c, 3);
         if (t == 3) t = -1;  /* And the rest anticlockwise. */
         t -= 3;
         break;
      case stb_cccca:
         /* Turn four quadrants clockwise. */
         do_stability(personp, stb_c, 0);
         t -= 4;              /* And the rest anticlockwise. */
         break;
      case stb_aa:
         /* Turn four quadrants anticlockwise. */
         do_stability(personp, stb_a, 0);
         break;      /* And keep turning. */
      case stb_cc:
         /* Turn four quadrants clockwise. */
         do_stability(personp, stb_c, 0);
         break;      /* And keep turning. */
      default:
         *personp &= ~STABLE_MASK;
         return;
   }

   /* Now t has the number of quadrants the person turned, clockwise or anticlockwise. */

   st = (t < 0) ? -1 : 1;
   at = t * st;
   atr = at - ((*personp & (STABLE_RBIT*7)) / STABLE_RBIT);

   if (atr <= 0)
      *personp -= at*STABLE_RBIT;
   else
      *personp =
         (*personp & ~(STABLE_RBIT*7|STABLE_VBIT*3)) |
         ((*personp + (STABLE_VBIT * atr * st)) & (STABLE_VBIT*3));
}





extern long_boolean check_restriction(
   setup *ss,
   assumption_thing restr,
   long_boolean instantiate_phantoms,
   uint32 flags)
{
   uint32 q0, q1, q2, q3, q4, q5, q6, q7;
   uint32 i, k, z, t;
   int idx;
   Const veryshort *mp;
   restriction_thing *restr_thing_ptr;
   long_boolean retval = TRUE;   /* TRUE means we were not able to instantiate phantoms.
                                    It is only meaningful if instantiation was requested. */

   if (restr.assumption == cr_alwaysfail) goto restr_failed;

   if (restr.assumption == cr_nice_diamonds) {
      restr.assumption = cr_jright;
      restr.assump_col = 4;
   }
   else if (restr.assumption == cr_leads_only) {
      restr.assump_both = 2;
   }
   else if (restr.assumption == cr_trailers_only) {
      restr.assump_both = 1;
   }

   /* First, check for nice things. */

   restr_thing_ptr = get_restriction_thing(ss->kind, restr);

   if (restr_thing_ptr) {
      /* If this finds the restriction satisfied AND successfully instantiates
         phantoms, it will clear retval. */

      if (verify_restriction(ss, restr_thing_ptr, restr, instantiate_phantoms, &retval)) goto getout;
      goto restr_failed;
   }

   /* Now check all the other stuff. */

   /* This next restriction is independent of whether we are line-like or column-like. */

   if (restr.assumption == cr_siamese_in_quad) {
      for (idx=0 ; idx<8 ; idx+=2) {
         q0 = 0; q1 = 0; q2 = 0; q3 = 0;
         if ((t = ss->people[idx+0].id1) != 0) { q0 |= (t^1); q1 |= (t^3); q2 |= (t^2); q3 |= (t); }
         if ((t = ss->people[idx+1].id1) != 0) { q0 |= (t^1); q1 |= (t^3); q2 |= (t^2); q3 |= (t); }
         if ((q0&3) && (q1&3) && (q2&3) && (q3&3)) goto restr_failed;
      }
   }

   if (restr.assump_col & 1) {
      /* Restriction is "column-like". */

      switch (ss->kind) {
         case s2x4:
            switch (restr.assumption) {
               /* These ignore any people in line-like orientation. */
               case cr_quarterbox_or_col:
                  k = 0;         /* check for a reasonable "quick step" or "triple cross" setup */
                  i = 2;
                  if (ss->people[0].id1 & 1) { k |=  ss->people[0].id1;                          }
                  if (ss->people[1].id1 & 1) { k |=  ss->people[1].id1; i &=  ss->people[1].id1; }
                  if (ss->people[2].id1 & 1) { k |=  ss->people[2].id1; i &=  ss->people[2].id1; }
                  if (ss->people[3].id1 & 1) {                          i &=  ss->people[3].id1; }
                  if (ss->people[4].id1 & 1) { k |= ~ss->people[4].id1;                          }
                  if (ss->people[5].id1 & 1) { k |= ~ss->people[5].id1; i &= ~ss->people[5].id1; }
                  if (ss->people[6].id1 & 1) { k |= ~ss->people[6].id1; i &= ~ss->people[6].id1; }
                  if (ss->people[7].id1 & 1) {                          i &= ~ss->people[7].id1; }
                  if (k & ~i & 2)
                     goto restr_failed;
                  break;
               case cr_quarterbox_or_magic_col:
                  k = 0;         /* check for a reasonable "magic quick step" or "make magic" setup */
                  i = 2;
                  if (ss->people[0].id1 & 1) {                          i &= ~ss->people[0].id1; }
                  if (ss->people[1].id1 & 1) { k |=  ss->people[1].id1; i &=  ss->people[1].id1; }
                  if (ss->people[2].id1 & 1) { k |=  ss->people[2].id1; i &=  ss->people[2].id1; }
                  if (ss->people[3].id1 & 1) { k |= ~ss->people[3].id1;                          }
                  if (ss->people[4].id1 & 1) {                          i &=  ss->people[4].id1; }
                  if (ss->people[5].id1 & 1) { k |= ~ss->people[5].id1; i &= ~ss->people[5].id1; }
                  if (ss->people[6].id1 & 1) { k |= ~ss->people[6].id1; i &= ~ss->people[6].id1; }
                  if (ss->people[7].id1 & 1) { k |=  ss->people[7].id1;                          }
                  if (k & ~i & 2)
                     goto restr_failed;
                  break;
            }
            break;
         case s2x3:
            switch (restr.assumption) {
               case cr_quarterbox_or_col:
                  k = 0;         /* check for a reasonable columns-of-3 "quick step" setup */
                  i = 2;
                  if (ss->people[0].id1) { k |=  ss->people[0].id1;                          }
                  if (ss->people[1].id1) { k |=  ss->people[1].id1; i &=  ss->people[1].id1; }
                  if (ss->people[2].id1) {                          i &=  ss->people[2].id1; }
                  if (ss->people[3].id1) { k |= ~ss->people[3].id1;                          }
                  if (ss->people[4].id1) { k |= ~ss->people[4].id1; i &= ~ss->people[4].id1; }
                  if (ss->people[5].id1) {                          i &= ~ss->people[5].id1; }
                  if (k & ~i & 2)
                     goto restr_failed;
                  break;
               case cr_quarterbox_or_magic_col:
                  k = 0;         /* check for a reasonable columns-of-3 "magic quick step" setup */
                  i = 2;
                  if (ss->people[0].id1) {                          i &= ~ss->people[0].id1; }
                  if (ss->people[1].id1) { k |=  ss->people[1].id1; i &=  ss->people[1].id1; }
                  if (ss->people[2].id1) { k |= ~ss->people[2].id1;                          }
                  if (ss->people[3].id1) {                          i &=  ss->people[3].id1; }
                  if (ss->people[4].id1) { k |= ~ss->people[4].id1; i &= ~ss->people[4].id1; }
                  if (ss->people[5].id1) { k |=  ss->people[5].id1;                          }
                  if (k & ~i & 2)
                     goto restr_failed;
                  break;
            }
            break;
         case s1x2:
            switch (restr.assumption) {
               case cr_opposite_sex:
                  i = 0;
                  k = 0;
                  if (ss->people[0].id1) {
                     i = ss->people[0].id1;
                     if (!(i & (ID1_PERM_BOY | ID1_PERM_GIRL)))
                        goto restr_failed;
                  }

                  if (ss->people[1].id1) {
                     k = ss->people[1].id1;
                     if (!(k & (ID1_PERM_BOY | ID1_PERM_GIRL)))
                        goto restr_failed;
                  }

                  if ((i&k) & (ID1_PERM_BOY | ID1_PERM_GIRL))
                     goto restr_failed;
                  break;
            }
            break;
      }
   }
   else {
      /* Restriction is "line-like" or special. */

      static Const veryshort mapwkg8[3] = {2, 2, 6};
      static Const veryshort mapwkg6[3] = {2, 2, 5};
      static Const veryshort mapwkg4[3] = {2, 1, 3};
      static Const veryshort mapwkg2[3] = {2, 0, 1};
      static Const veryshort mapwk24[5] = {4, 1, 2, 6, 5};

      switch (restr.assumption) {
         case cr_awkward_centers:       /* check for centers not having left hands */
            switch (ss->kind) {
               case s2x4: mp = mapwk24; goto check_wk;
               case s1x8: mp = mapwkg8; goto check_wk;
               case s1x6: mp = mapwkg6; goto check_wk;
               case s1x4: mp = mapwkg4; goto check_wk;
               case s1x2: mp = mapwkg2; goto check_wk;
            }
            break;
         case cr_ends_are_peelable:
            /* check for ends in a "peelable" (everyone in genuine tandem somehow) box */
            if (ss->kind == s2x4) {
               q1 = 0; q0 = 0; q5 = 0; q4 = 0;
               if ((t = ss->people[0].id1) != 0) { q1 |= t; q0 |= (t^2); }
               if ((t = ss->people[3].id1) != 0) { q5 |= t; q4 |= (t^2); }
               if ((t = ss->people[4].id1) != 0) { q5 |= t; q4 |= (t^2); }
               if ((t = ss->people[7].id1) != 0) { q1 |= t; q0 |= (t^2); }
               if (((q1&3) && (q0&3)) || ((q5&3) && (q4&3)))
                  goto restr_failed;
            }
            break;
         case cr_explodable:
            if (ss->kind == s1x4) {
               t = ss->people[1].id1;
               z = ss->people[3].id1;
               if ((z & t) && ((z ^ t) & 2)) goto restr_failed;
            }
            break;
         case cr_rev_explodable:
            if (ss->kind == s1x4) {
               t = ss->people[0].id1;
               z = ss->people[2].id1;
               if ((z & t) && ((z ^ t) & 2)) goto restr_failed;
            }
            break;
      }

      if (ss->kind == s2x2) {
         switch (restr.assumption) {
            case cr_not_tboned:
               /* check for a box that is not T-boned */
               if (((ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1) & 011) == 011)
                  goto restr_failed;
               break;
            case cr_couples_only:
               /* check for everyone as a couple */
               q0 = 0; q1 = 0; q2 = 3; q3 = 3;
               q4 = 0; q5 = 0; q7 = 3; q6 = 3;
               if ((t = ss->people[0].id1) != 0) { q0 |= t; q2 &= t; q1 |= (t^2); q3 &= (t^2); }
               if ((t = ss->people[1].id1) != 0) { q0 |= t; q7 &= t; q1 |= (t^2); q6 &= (t^2); }
               if ((t = ss->people[2].id1) != 0) { q5 |= t; q7 &= t; q4 |= (t^2); q6 &= (t^2); }
               if ((t = ss->people[3].id1) != 0) { q5 |= t; q2 &= t; q4 |= (t^2); q3 &= (t^2); }
               if (((q0&3) && ((~q2)&3) && (q1&3) && ((~q3)&3)) ||
                   ((q5&3) && ((~q7)&3) && (q4&3) && ((~q6)&3)))
                  goto restr_failed;
               break;
            case cr_miniwaves:
               /* check for everyone in miniwaves */
               q0 = 0; q1 = 0; q2 = 3; q3 = 3;
               q4 = 0; q5 = 0; q7 = 3; q6 = 3;
               if ((t = ss->people[0].id1) != 0) { q0 |= t;     q2 &= t;     q1 |= (t^2); q3 &= (t^2); }
               if ((t = ss->people[1].id1) != 0) { q0 |= (t^2); q7 &= t;     q1 |= t;     q6 &= (t^2); }
               if ((t = ss->people[2].id1) != 0) { q5 |= t;     q7 &= (t^2); q4 |= (t^2); q6 &= t; }
               if ((t = ss->people[3].id1) != 0) { q5 |= (t^2); q2 &= (t^2); q4 |= t;     q3 &= t; }
               if (((q0&3) && ((~q2)&3) && (q1&3) && ((~q3)&3)) ||
                   ((q5&3) && ((~q7)&3) && (q4&3) && ((~q6)&3)))
                  goto restr_failed;
               break;
            case cr_peelable_box:
               /* check for a "peelable" (everyone in genuine tandem somehow) box */
               q0 = 0; q1 = 0; q2 = 3; q3 = 3;
               q4 = 0; q5 = 0; q7 = 3; q6 = 3;
               if ((t = ss->people[0].id1) != 0) { q0 |= t; q2 &= t; q1 |= (t^2); q3 &= (t^2); }
               if ((t = ss->people[1].id1) != 0) { q5 |= t; q2 &= t; q4 |= (t^2); q3 &= (t^2); }
               if ((t = ss->people[2].id1) != 0) { q5 |= t; q7 &= t; q4 |= (t^2); q6 &= (t^2); }
               if ((t = ss->people[3].id1) != 0) { q0 |= t; q7 &= t; q1 |= (t^2); q6 &= (t^2); }
               if (((q0&3) && ((~q2)&3) && (q1&3) && ((~q3)&3)) ||
                   ((q5&3) && ((~q7)&3) && (q4&3) && ((~q6)&3)))
                  goto restr_failed;
               break;
         }
      }
   }

   goto getout;

   check_wk:   /* check the "awkward_centers" restriction. */

   for (idx=0 ; idx < mp[0] ; idx+=2) {
      t = ss->people[mp[idx+1]].id1;
      z = ss->people[mp[idx+2]].id1;
      if ((z&t) && !((z | (~t)) & 2)) warn(warn__awkward_centers);
   }

   goto getout;

   restr_failed:

   switch (flags) {
      case CAF__RESTR_RESOLVE_OK:
         warn(warn__dyp_resolve_ok);
         break;
      case CAF__RESTR_UNUSUAL:
         warn(warn__unusual);
         break;
      case CAF__RESTR_CONTROVERSIAL:
         warn(warn_controversial);
         break;
      case CAF__RESTR_BOGUS:
         warn(warn_serious_violation);
         break;
      case CAF__RESTR_FORBID:
         fail("This call is not legal from this formation.");
      case 99:
         if (restr.assumption == cr_gen_1_4_tag)
            fail("People are not facing as in 1/4 tags.");
         else if (restr.assumption == cr_gen_3_4_tag)
            fail("People are not facing as in 3/4 tags.");
         else if (restr.assumption == cr_wave_only && restr.assump_col == 0)
            fail("People are not in waves.");
         else if (restr.assumption == cr_all_ns)
            fail("People are not in lines.");
         else if (restr.assumption == cr_all_ew)
            fail("People are not in columns.");
         else
            fail("The people do not satisfy the assumption.");
      default:
         warn(warn__do_your_part);
         break;
   }

   getout:

   /* One final check.  If there is an assumption in place that is inconsistent with the restriction
      being enforced, then it is an error, even though no live people violate the restriction. */

   switch (ss->cmd.cmd_assume.assumption) {
      case cr_gen_1_4_tag: case cr_gen_3_4_tag: case cr_qtag_like:
         switch (restr.assumption) {
            case cr_diamond_like:
               fail("An assumed facing direction for phantoms is illegal for this call.");
               break;
         }
         break;
      case cr_diamond_like:
         switch (restr.assumption) {
            case cr_gen_1_4_tag: case cr_gen_3_4_tag: case cr_qtag_like:
               fail("An assumed facing direction for phantoms is illegal for this call.");
               break;
         }
         break;
      case cr_couples_only: case cr_3x3couples_only: case cr_4x4couples_only:
         switch (restr.assumption) {
            case cr_wave_only: case cr_miniwaves:
               fail("An assumed facing direction for phantoms is illegal for this call.");
               break;
         }
         break;
      case cr_wave_only: case cr_miniwaves:
         switch (restr.assumption) {
            case cr_couples_only: case cr_3x3couples_only: case cr_4x4couples_only:
               fail("An assumed facing direction for phantoms is illegal for this call.");
               break;
         }
         break;
   }

   return retval;
}




Private void special_4_way_symm(
   callarray *tdef,
   setup *scopy,
   personrec newpersonlist[],
   int newplacelist[],
   int lilresult_mask[],
   setup *result)

{
   static int table_2x4[8] = {10, 15, 3, 1, 2, 7, 11, 9};

   static int table_2x8[16] = {
      12, 13, 14, 15, 31, 27, 23, 19,
      44, 45, 46, 47, 63, 59, 55, 51};

   static int table_3x1d[8] = {
      0, 1, 2, 3, 6, 7, 8, 9};

   static int table_4x6[24] = {
       9, 10, 11, 30, 26, 22,
      23, 27, 31, 15, 14, 13,
      41, 42, 43, 62, 58, 54,
      55, 59, 63, 47, 46, 45};

   static int table_1x16[16] = {
       0,  1,  2,  3,  4,  5,  6,  7,
      16, 17, 18, 19, 20, 21, 22, 23};

   static int table_4dmd[16] = {
      7, 5, 14, 12, 16, 17, 18, 19,
      23, 21, 30, 28, 0, 1, 2, 3};

   static int table_2x3_4dmd[6] = {
      6, 11, 13, 22, 27, 29};


   static int line_table[4] = {0, 1, 6, 7};
   static int dmd_table[4] = {0, 4, 6, 10};

   int begin_size;
   int real_index;
   int k, result_size, result_quartersize;
   int *the_table = (int *) 0;

   switch (result->kind) {
      case s2x2: case s_galaxy:
      case s_c1phan: case s4x4:
      case s_hyperglass: case s_thar:
      case s_star: case s1x1:
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
         result->kind = s8x8;
         the_table = table_2x8;
         break;
      case s3x1dmd:
         result->kind = sx1x6;
         the_table = table_3x1d;
         break;
      case s4x6:
         result->kind = s8x8;
         the_table = table_4x6;
         break;
      case s1x16:
         result->kind = sx1x16;
         the_table = table_1x16;
         break;
      case s4dmd:
         result->kind = sx4dmd;
         the_table = table_4dmd;
         break;
      case s2x3:
         result->kind = sx4dmd;
         the_table = table_2x3_4dmd;
         break;
      default:
         fail("Don't recognize ending setup for this call.");
   }

   begin_size = setup_attrs[scopy->kind].setup_limits+1;
   result_size = setup_attrs[result->kind].setup_limits+1;
   result_quartersize = result_size >> 2;
   lilresult_mask[0] = 0;
   lilresult_mask[1] = 0;

   for (real_index=0; real_index<begin_size; real_index++) {
      personrec this_person = scopy->people[real_index];
      newpersonlist[real_index].id1 = 0;
      newpersonlist[real_index].id2 = 0;
      if (this_person.id1) {
         int real_direction = this_person.id1 & 3;
         int northified_index = (real_index + (((4-real_direction)*begin_size) >> 2)) % begin_size;
         uint32 z = find_calldef(tdef, scopy, real_index, real_direction, northified_index);
         k = (z >> 4) & 0x1F;
         if (the_table) k = the_table[k];
         k = (k + real_direction*result_quartersize) % result_size;
         newpersonlist[real_index].id1 = (this_person.id1 & ~(ROLL_MASK | 077)) |
               ((z+real_direction*011) & 013) |
               ((z * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);

         if (this_person.id1 & STABLE_ENAB) {
            do_stability(&newpersonlist[real_index].id1, (stability) ((z/DBSTAB_BIT) & 0xF), (z + result->rotation));
         }

         newpersonlist[real_index].id2 = this_person.id2;
         newplacelist[real_index] = k;
         lilresult_mask[k>>5] |= (1 << (k&037));
      }
   }
}



/* This function is internal. */

Private void special_3person(
   callarray *cdef,
   callarray *ldef,
   setup *scopy,
   personrec newpersonlist[],
   int newplacelist[],
   int lilresult_mask[],
   setup *result)
{
   int real_index;
   int numout = setup_attrs[result->kind].setup_limits+1;

   for (real_index=0; real_index<3; real_index++) {
      personrec this_person = scopy->people[real_index];
      newpersonlist[real_index].id1 = 0;
      newpersonlist[real_index].id2 = 0;
      newplacelist[real_index] = -1;
      if (this_person.id1) {
         int northified_index, k;
         uint32 z;
         int real_direction = this_person.id1 & 3;

         if (scopy->kind == s_trngl) {
            int d2 = ((real_direction >> 1) & 1) * 3;
            northified_index = (real_index + d2);
         }
         else {
            northified_index = (real_direction & 2) ? 2-real_index : real_index;
         }

         z = find_calldef((real_direction & 1) ? cdef : ldef, scopy, real_index, real_direction, northified_index);
         k = (z >> 4) & 0x1F;

         if (real_direction & 2) {
            if (result->kind == s_trngl) {
               k-=3;
               if (k<0) k+=6;
            }
            else
               k = numout-1-k;
         }

         newpersonlist[real_index].id1 = (this_person.id1 & ~(ROLL_MASK | 077)) |
               ((z + real_direction * 011) & 013) |
               ((z * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);

         if (this_person.id1 & STABLE_ENAB)
            do_stability(&newpersonlist[real_index].id1, (stability) ((z/DBSTAB_BIT) & 0xF), (z+result->rotation));

         newpersonlist[real_index].id2 = this_person.id2;
         newplacelist[real_index] = k;
         lilresult_mask[0] |= (1 << k);
      }
   }

   /* Check whether the call went into the other triangle.  If so, it
      must have done so completely. */

   if (result->kind == s_trngl) {
      if (newplacelist[0] >=3 || newplacelist[1] >=3 || newplacelist[2] >=3) {
         int i;

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
}


Private int divide_the_setup(
   setup *ss,
   uint32 *newtb_p,
   callarray *calldeflist,
   int *desired_elongation_p,
   setup *result)
{
   int i, j;
   uint32 livemask;
   long_boolean recompute_anyway;
   long_boolean temp_for_2x2;
   callarray *have_1x2, *have_2x1;
   uint32 division_code = ~0;
   map_thing *division_maps;
   uint32 newtb = *newtb_p;
   uint32 callflags1 = ss->cmd.callspec->callflags1;
   uint64 final_concepts = ss->cmd.cmd_final_flags;
   setup_command conc_cmd;
   uint32 must_do_concentric = ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_KMASK;
   long_boolean matrix_aware =
         (callflags1 & CFLAG1_12_16_MATRIX_MEANS_SPLIT) &&
         (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX);

   /* It will be helpful to have a mask of where the
      live people are. */

   for (i=0, j=1, livemask = 0; i<=setup_attrs[ss->kind].setup_limits; i++, j<<=1) {
      if (ss->people[i].id1) livemask |= j;
   }

   /* Take care of "snag" and "mystic".  "Central" is illegal, and was already caught.
      We first limit it to just the few setups for which it can possibly be legal, to make
      it easier to test later. */

   if (must_do_concentric) {
      switch (ss->kind) {
         case s_rigger:
         case s_qtag:
         case s_bone:
         case s1x8:
         case s2x4:
         case s_crosswave:
            break;
         default:
            fail("Can't do \"snag/mystic\" with this call.");
      }
   }

   switch (ss->kind) {
      case s_thar:
         if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
            fail("Can't split the setup.");
         division_code = MAPCODE(s1x2,4,MPKIND__4_EDGES,1);
         goto divide_us_no_recompute;
      case s2x8:
         /* The call has no applicable 2x8 or 8x2 definition. */

         /* Check whether it has 2x4/4x2/1x8/8x1 definitions, and divide the setup if so,
            or if the caller explicitly said "2x8 matrix". */

         if ((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) || (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX)) {
            if (
                  (!(newtb & 010) || assoc(b_2x4, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_4x2, ss, calldeflist))) {
               division_code = MAPCODE(s2x4,2,MPKIND__SPLIT,0);
               /* I consider it an abomination to call such a thing as "2x8 matrix
                  swing-o-late" from 2x8 lines, expecting people to do the call
                  in split phantom boxes, or "2x8 matrix grand mix", expecting
                  twin tidal lines.  However, we allow it for consistency with such
                  things as "2x6 matrix double play" from 2x6 columns, expecting
                  12 matrix divided columns.  The correct usage should involve the
                  explicit concepts "split phantom boxes", "phantom tidal lines",
                  or "12 matrix divided columns", as appropriate. */
               if (!(callflags1 & CFLAG1_SPLIT_LARGE_SETUPS)) warn(warn__split_to_2x4s);  /* If database said to split, don't give warning. */
               goto divide_us_no_recompute;
            }
            else if (
                  (!(newtb & 010) || assoc(b_1x4, ss, calldeflist) || assoc(b_1x8, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_4x1, ss, calldeflist) || assoc(b_8x1, ss, calldeflist))) {
               division_code = MAPCODE(s1x8,2,MPKIND__SPLIT,1);
               /* See comment above about abomination. */
               if (!(callflags1 & CFLAG1_SPLIT_LARGE_SETUPS)) warn(warn__split_to_1x8s);  /* If database said to split, don't give warning. */
               goto divide_us_no_recompute;
            }
         }

         /* Otherwise, the only way this can be legal is if we can identify
            smaller setups of all real people and can do the call on them.  For
            example, we will look for 1x4 setups, so we could do things like
            swing thru from a totally offset parallelogram. */

         switch (livemask) {
            case 0xF0F0:    /* a parallelogram */
               division_code = MAPCODE(s1x4,2,MPKIND__OFFS_R_FULL,1);
               warn(warn__each1x4);
               break;
            case 0x0F0F:    /* a parallelogram */
               division_code = MAPCODE(s1x4,2,MPKIND__OFFS_L_FULL,1);
               warn(warn__each1x4);
               break;
            case 0xC3C3:    /* the outer quadruple boxes */
               division_code = MAPCODE(s2x2,4,MPKIND__SPLIT,0);
               warn(warn__each2x2);
               break;
            default:
               fail("You must specify a concept.");
         }

         goto divide_us_no_recompute;
      case s2x6:
         /* The call has no applicable 2x6 or 6x2 definition. */

         /* See if this call has applicable 2x8 definitions and matrix expansion is permitted.
            If so, that is what we must do. */

         if (  !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX) &&
               (!(newtb & 010) || assoc(b_2x8, ss, calldeflist)) &&
               (!(newtb & 001) || assoc(b_8x2, ss, calldeflist))) {
            do_matrix_expansion(ss, CONCPROP__NEEDK_2X8, TRUE);
            if (ss->kind != s2x8) fail("Failed to expand to 2X8.");  /* Should never fail, but we don't want a loop. */
            return 2;        /* And try again. */
         }

         /* Next, check whether it has 1x3/3x1/2x3/3x2/1x6/6x1 definitions, and divide the setup if so,
            and if the call permits it.  This is important for permitting "Z axle" from
            a 2x6 but forbidding "circulate".  We also enable this if the caller explicitly
            said "2x6 matrix". */

         if ((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) || (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX)) {
            if (
                  (!(newtb & 010) || assoc(b_2x3, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_3x2, ss, calldeflist))) {
               division_code = MAPCODE(s2x3,2,MPKIND__SPLIT,0);
               /* See comment above about abomination. */
               if (!(callflags1 & CFLAG1_SPLIT_LARGE_SETUPS)) warn(warn__split_to_2x3s);  /* If database said to split, don't give warning. */
               goto divide_us_no_recompute;
            }
            else if (
                  (!(newtb & 010) || assoc(b_1x3, ss, calldeflist) || assoc(b_1x6, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_3x1, ss, calldeflist) || assoc(b_6x1, ss, calldeflist))) {
               division_code = MAPCODE(s1x6,2,MPKIND__SPLIT,1);
               /* See comment above about abomination. */
               if (!(callflags1 & CFLAG1_SPLIT_LARGE_SETUPS)) warn(warn__split_to_1x6s);  /* If database said to split, don't give warning. */
               goto divide_us_no_recompute;
            }
         }

         /* Otherwise, the only way this can be legal is if we can identify
            smaller setups of all real people and can do the call on them.  For
            example, we will look for 1x4 setups, so we could do things like
            swing thru from a parallelogram. */

         switch (livemask) {
            case 07474:    /* a parallelogram */
               division_code = MAPCODE(s1x4,2,MPKIND__OFFS_R_HALF,1);
               warn(warn__each1x4);
               break;
            case 01717:    /* a parallelogram */
               division_code = MAPCODE(s1x4,2,MPKIND__OFFS_L_HALF,1);
               warn(warn__each1x4);
               break;
            case 06363: case 06060: case 00303: case 04141: case 02222:    /* the outer triple boxes */
               division_code = MAPCODE(s2x2,3,MPKIND__SPLIT,0);
               warn(warn__each2x2);
               break;
            default:
               fail("You must specify a concept.");
         }

         goto divide_us_no_recompute;
      case s1p5x8:
         /* This is a phony setup, allowed only so that we can have people temporarily
            in 50% offset 1x4's that are offset the impossible way. */

         switch (livemask) {
            case 0xF0F0:
               division_code = MAPCODE(s1x4,2,MPKIND__OFFS_L_HALF,0);
               break;
            case 0x0F0F:
               division_code = MAPCODE(s1x4,2,MPKIND__OFFS_R_HALF,0);
               break;
            default:
               fail("You must specify a concept.");
         }

         goto divide_us_no_recompute;
      case s1x12:
         /* The call has no applicable 1x12 or 12x1 definition. */

         /* See if this call has applicable 1x16 definitions and matrix expansion is permitted.
            If so, that is what we must do. */

         if (  !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX) &&
               (!(newtb & 010) || assoc(b_1x16, ss, calldeflist)) &&
               (!(newtb & 001) || assoc(b_16x1, ss, calldeflist))) {
            do_matrix_expansion(ss, CONCPROP__NEEDK_1X16, TRUE);
            if (ss->kind != s1x16) fail("Failed to expand to 1X16.");  /* Should never fail, but we don't want a loop. */
            return 2;        /* And try again. */
         }

         /* Check whether it has 1x6/6x1 definitions, and divide the setup if so,
            and if the caller explicitly said "1x12 matrix". */

         if (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX) {
            if (  (!(newtb & 010) || assoc(b_1x6, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_6x1, ss, calldeflist))) {
               division_code = MAPCODE(s1x6,2,MPKIND__SPLIT,0);
               /* See comment above about abomination. */
               warn(warn__split_to_1x6s);
               goto divide_us_no_recompute;
            }
         }
         break;
      case s1x16:
         /* The call has no applicable 1x16 or 16x1 definition. */

         /* Check whether it has 1x8/8x1 definitions, and divide the setup if so,
            and if the caller explicitly said "1x16 matrix". */

         if (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX) {
            if (
                  (!(newtb & 010) || assoc(b_1x8, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_8x1, ss, calldeflist))) {
               division_code = MAPCODE(s1x8,2,MPKIND__SPLIT,0);
               /* See comment above about abomination. */
               warn(warn__split_to_1x8s);
               goto divide_us_no_recompute;
            }
         }
         break;
      case s1x10:
         /* See if this call has applicable 1x12 or 1x16 definitions and matrix expansion is permitted.
            If so, that is what we must do.
            These two cases are required to make things like 12 matrix grand swing thru work from a 1x10. */

         if (  !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX) &&
               (!(newtb & 010) || assoc(b_1x12, ss, calldeflist)) &&
               (!(newtb & 001) || assoc(b_12x1, ss, calldeflist))) {
            do_matrix_expansion(ss, CONCPROP__NEEDK_1X12, TRUE);
            if (ss->kind != s1x12) fail("Failed to expand to 1X12.");  /* Should never fail, but we don't want a loop. */
            return 2;        /* And try again. */
         }
         /* FALL THROUGH!!!!! */
      case s1x14:      /* WARNING!!  WE FELL THROUGH!! */
         /* See if this call has applicable 1x16 definitions and matrix expansion is permitted.
            If so, that is what we must do. */

         if (  !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX) &&
               (!(newtb & 010) || assoc(b_1x16, ss, calldeflist)) &&
               (!(newtb & 001) || assoc(b_16x1, ss, calldeflist))) {
            do_matrix_expansion(ss, CONCPROP__NEEDK_1X16, TRUE);
            if (ss->kind != s1x16) fail("Failed to expand to 1X16.");  /* Should never fail, but we don't want a loop. */
            return 2;        /* And try again. */
         }
         break;
      case s4x6:
         if (callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) {
            if (
                  (!(newtb & 010) || assoc(b_2x6, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_6x2, ss, calldeflist))) {
               division_code = MAPCODE(s2x6,2,MPKIND__SPLIT,1);
               goto divide_us_no_recompute;
            }
            else if (
                  (!(newtb & 010) || assoc(b_4x3, ss, calldeflist) || assoc(b_2x3, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_3x4, ss, calldeflist) || assoc(b_3x2, ss, calldeflist))) {
               division_code = MAPCODE(s3x4,2,MPKIND__SPLIT,1);
               goto divide_us_no_recompute;
            }
         }
         break;
      case s3x8:
         if (callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) {
            if (
                  (!(newtb & 010) || assoc(b_3x4, ss, calldeflist) || assoc(b_3x2, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_4x3, ss, calldeflist) || assoc(b_2x3, ss, calldeflist))) {
               division_code = MAPCODE(s3x4,2,MPKIND__SPLIT,0);
               goto divide_us_no_recompute;
            }
         }
         break;
      case s3x6:
         /* Check whether it has 2x3/3x2/1x6/6x1 definitions, and divide the setup if so,
            or if the caller explicitly said "3x6 matrix" (not that "3x6 matrix" exists at present.) */

         if ((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) || (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX)) {
            if (
                  (!(newtb & 010) || assoc(b_3x2, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_2x3, ss, calldeflist))) {
               division_code = MAPCODE(s2x3,3,MPKIND__SPLIT,1);
               if (!(callflags1 & CFLAG1_SPLIT_LARGE_SETUPS)) warn(warn__split_to_2x3s);  /* If database said to split, don't give warning. */
               goto divide_us_no_recompute;
            }
            else if (
                  (!(newtb & 010) || assoc(b_1x6, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_6x1, ss, calldeflist))) {
               division_code = MAPCODE(s1x6,3,MPKIND__SPLIT,1);
               if (!(callflags1 & CFLAG1_SPLIT_LARGE_SETUPS)) warn(warn__split_to_1x6s);  /* If database said to split, don't give warning. */
               goto divide_us_no_recompute;
               /* YOW!!  1x3's are hard!  We need a 3x3 formation. */
            }
         }

         /* Otherwise, the only way this can be legal is if we can identify
            smaller setups of all real people and can do the call on them.  For
            example, we will look for 1x2 setups, so we could trade in individual couples scattered around. */

         switch (livemask) {
            case 0505505:
               division_code = MAPCODE(s2x3,2,MPKIND__OFFS_R_HALF,0);
               warn(warn__each1x2);
               break;
            case 0550550:
               division_code = MAPCODE(s2x3,2,MPKIND__OFFS_L_HALF,0);
               warn(warn__each1x2);
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
            division_code = MAPCODE(s1x2,4,MPKIND__4_QUADRANTS,0);
         else if ((livemask & 0x5555) == 0)
            division_code = MAPCODE(s1x2,4,MPKIND__4_QUADRANTS,1);
         else if (   (livemask & 0x55AA) == 0 ||
                     (livemask & 0xAA55) == 0 ||
                     (livemask & 0x5AA5) == 0 ||
                     (livemask & 0xA55A) == 0) {
            setup scopy;
            setup the_results[2];

            /* This is an unsymmetrical thing.  Do each quadrant (a 1x2) separately by
               using both maps, and then merge the result and hope for the best. */

            ss->cmd.cmd_misc_flags &= ~CMD_MISC__MUST_SPLIT;
            scopy = *ss;    /* "Move" can write over its input. */
            new_divided_setup_move(ss, MAPCODE(s1x2,4,MPKIND__4_QUADRANTS,0), phantest_ok, FALSE, &the_results[0]);
            new_divided_setup_move(&scopy, MAPCODE(s1x2,4,MPKIND__4_QUADRANTS,1), phantest_ok, FALSE, &the_results[1]);
            *result = the_results[1];
            result->result_flags = get_multiple_parallel_resultflags(the_results, 2);
            merge_setups(&the_results[0], merge_c1_phantom, result);
            return 1;
         }
         else
            fail("You must specify a concept.");

         goto divide_us_no_recompute;
      case sbigdmd:

         /* The only way this can be legal is if people are in genuine "T" spots. */

         switch (livemask) {
            case 01717:
               division_code = MAPCODE(s_trngl4,2,MPKIND__OFFS_L_HALF,1);
               break;
            case 07474:
               division_code = MAPCODE(s_trngl4,2,MPKIND__OFFS_R_HALF,1);
               break;
            default:
               fail("You must specify a concept.");
         }

         goto divide_us_no_recompute;
      case s2x2:
         ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

         /* Any 2x2 -> 2x2 call that acts by dividing itself into 1x2's
            is presumed to want the people in each 1x2 to stay near each other.
            We signify that by reverting to the original elongation,
            overriding anything that may have been in the call definition. */

         /* Tentatively choose a map.  We may change it later to "map_2x2v". */
         division_code = MAPCODE(s1x2,2,MPKIND__SPLIT,1);

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
                  /* The call has both definitions.
                     If the "lateral_to_selectees" flag is on (that is, the call is "run"),
                        We decide what to do according to the direction of the selectees.
                        There must be at least one, they must be collectively consistent.
                     Otherwise, we look at the manner in which the setup is T-boned
                        in order to figure out how to divide the setup. */

                  if (calldeflist->callarray_flags & CAF__LATERAL_TO_SELECTEES) {
                     uint32 selmask = 0;

                     for (i=0 ; i<4 ; i++) if (selectp(ss, i)) selmask |= ss->people[i].id1;

                     if (selmask == 0 || (selmask & 011) == 011)
                        fail("People are not working with each other in a consistent way.");
                     else if (selmask & 1)
                        { division_code = ~0 ; division_maps = &map_2x2v; }
                  }
                  else {
                     if ((((ss->people[0].id1 | ss->people[3].id1) & 011) != 011) &&
                           (((ss->people[1].id1 | ss->people[2].id1) & 011) != 011))
                        { division_code = ~0 ; division_maps = &map_2x2v; }
                     else if ((((ss->people[0].id1 | ss->people[1].id1) & 011) == 011) ||
                           (((ss->people[2].id1 | ss->people[3].id1) & 011) == 011))
                        fail("Can't figure out who should be working with whom.");
                  }

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
               if (ss->cmd.cmd_misc_flags & CMD_MISC__PHANTOMS)
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

            /* If this is "run" and people aren't T-boned, just ignore the 2x1 definition. */

            if (!(calldeflist->callarray_flags & CAF__LATERAL_TO_SELECTEES) && assoc(b_2x1, ss, calldeflist))
               elong |= (2 -  (newtb & 1));

            if (assoc(b_1x2, ss, calldeflist))
               elong |= (1 + (newtb & 1));

            if (elong == 0) {
               /* Neither 2x1 or 1x2 definition existed.  Check for 1x1.
                  If found, any division axis will work. */
               if (assoc(b_1x1, ss, calldeflist))
                  goto divide_us_no_recompute;
            }
            else {
               unsigned long int foo = (ss->cmd.prior_elongation_bits | ~elong) & 3;

               if (foo == 0) {
                  fail("Can't figure out who should be working with whom.");
               }
               else if (foo == 3) {
                  /* We are in trouble if CMD_MISC__NO_CHK_ELONG is off.
                     But, if there was a 1x1 definition, we allow it anyway.
                     This is what makes "you all" and "the K" legal from lines.
                     The "U-turn-back" is done in a 2x2 that is elongated laterally.
                     "U-turn-back" has a 1x2 definition (so that you can roll) but
                     no 2x1 definition (because, from a 2x2, it might overtake the 1x2
                     definition, in view of the fact that there is no definite priority
                     for searching for definitions, which could make people unable to
                     roll during certain phases of the moon.)  So, when we are having the
                     ends U-turn-back while in parallel lines, their 1x2's appear to be
                     illegally separated.  Since they could have done it in 1x1's, we allow
                     it.  And, incidentally, we allow a roll afterwards. */

                  if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_CHK_ELONG) && !assoc(b_1x1, ss, calldeflist))
                     fail("People are too far apart to work with each other on this call.");

                  foo ^= elong;
               }

               if (foo == 1)
                  { division_code = ~0 ; division_maps = &map_2x2v; }

               goto divide_us_no_recompute;
            }
         }

         break;
      case s_rigger:
         if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
            fail("Can't split the setup.");

         if (assoc(b_trngl4, ss, calldeflist) || assoc(b_ptrngl4, ss, calldeflist)) {
            division_maps = &map_rig_trngl4;
            goto divide_us_no_recompute;
         }

         if (must_do_concentric)
            goto do_concentrically;

         {
            int tinytb = ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1;

            /* See if this call has applicable 1x2 or 2x1 definitions, and the people in the wings are
               facing appropriately.  Then do it concentrically. */

            if ((!(tinytb & 010) || assoc(b_1x2, ss, calldeflist)) &&
                  (!(tinytb & 1) || assoc(b_2x1, ss, calldeflist)))
               goto do_concentrically;
         }
         break;
      case s3x4:
         {
            static Const veryshort map_3x4_fudge[8] = {1, 2, 4, 5, 7, 8, 10, 11};
            long_boolean forbid_little_stuff;
            setup sss;
            long_boolean really_fudged;

            /* The call has no applicable 3x4 or 4x3 definition. */
            /* First, check whether it has 2x3/3x2 definitions, and divide the setup if so,
               and if the call permits it.  This is important for permitting "Z axle" from
               a 3x4 but forbidding "circulate" (unless we give a concept like 12 matrix
               phantom columns.)  We also enable this if the caller explicitly said
               "3x4 matrix". */

            if (((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) || (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX)) &&
                  (!(newtb & 010) || assoc(b_3x2, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_2x3, ss, calldeflist))) {
               division_code = MAPCODE(s2x3,2,MPKIND__SPLIT,1);
               goto divide_us_no_recompute;
            }

            /* Search for divisions into smaller things: 2x2, 1x2, 2x1, or 1x1, in setups whose
               occupations make it clear what is meant.  We do not allow this if the call has
               a larger definition.  The reason is that we do not actually use "divided_setup_move"
               to perform the division that we want in one step.  This could require maps with
               excessive arity.  Instead, we use existing maps to break it down a little, and depend
               on recursion.  If larger definitions existed, the recursion might not do what we
               have in mind.  For example, we could get here on "checkmate" from offset columns,
               since that call has a 2x2 definition but no 4x3 definition.  However, all we do here
               is use the map that the "offset columns" concept uses.  We would then recurse on
               a virtual 4x2 column, and pick up the 8-person version of "checkmate", which would
               be wrong.  Similarly, this could lead to an "offset wave circulate" being done
               when we didn't say "offset wave".  For the call "circulate", it might be considered
               more natural to do a 12 matrix circulate.  But if the user doesn't say "12 matrix",
               we want to refuse to do it.  The only legal circulate if no concept is given is the
               2x2 circulate in each box.  So we disallow this if any possibly conflicting definition
               could be seen during the recursion. */

            forbid_little_stuff =
                  assoc(b_2x4, ss, calldeflist) ||
                  assoc(b_4x2, ss, calldeflist) ||
                  assoc(b_2x3, ss, calldeflist) ||
                  assoc(b_3x2, ss, calldeflist) ||
/*   Taking these lines out -- they prevent pass the ocean from working in offset facing lines.
                  assoc(b_dmd, ss, calldeflist) ||
                  assoc(b_pmd, ss, calldeflist) ||
*/
                  assoc(b_qtag, ss, calldeflist) ||
                  assoc(b_pqtag, ss, calldeflist);

            switch (livemask) {
               case 0xF3C: case 0xCF3:
                  /* We are in offset lines/columns, i.e. "clumps".  See if we can do the call in 2x2
                     or smaller setups. */
                  if (  forbid_little_stuff ||
                        (  !assoc(b_2x2, ss, calldeflist) &&
                           !assoc(b_1x2, ss, calldeflist) &&
                           !assoc(b_2x1, ss, calldeflist) &&
                           !assoc(b_1x1, ss, calldeflist))) fail("Don't know how to do this call in this setup.");
                  if (!matrix_aware) warn(warn__each2x2);
                  division_code = (livemask == 0xF3C) ? MAPCODE(s2x2,2,MPKIND__OFFS_L_HALF,0) : MAPCODE(s2x2,2,MPKIND__OFFS_R_HALF,0);
                  goto divide_us_no_recompute;
               case 0xEBA: case 0xD75:
                  /* We are in "Z"'s.  See if we can do the call in 1x2, 2x1, or 1x1 setups.
                     We do not allow 2x2 definitions. */
                  if (  !forbid_little_stuff &&
                        !assoc(b_2x2, ss, calldeflist) &&
                        (     ((!(newtb & 001) || assoc(b_1x2, ss, calldeflist)) &&
                               (!(newtb & 010) || assoc(b_2x1, ss, calldeflist)))      ||
                           assoc(b_1x1, ss, calldeflist))) {
                     warn(warn__each1x2);
                     division_maps = (livemask == 0xEBA) ? &map_lz12 : &map_rz12;
                     goto divide_us_no_recompute;
                  }
            }

            /* Check for everyone in either the outer 2 triple lines or the center triple line,
               and able to do the call in 1x4 or smaller setups. */

            if (((livemask & 0x3CF) == 0 || (livemask & 0xC30) == 0) &&
                     !forbid_little_stuff &&
                     (  assoc(b_1x4, ss, calldeflist) ||
                        assoc(b_4x1, ss, calldeflist) ||
                        assoc(b_1x2, ss, calldeflist) ||
                        assoc(b_2x1, ss, calldeflist) ||
                        assoc(b_1x1, ss, calldeflist))) {
               if (!matrix_aware) warn(warn__each1x4);
               division_code = MAPCODE(s1x4,3,MPKIND__SPLIT,1);
               goto divide_us_no_recompute;
            }

            /* Setup is randomly populated.  See if we have 1x2/1x1 definitions, but no 2x2.
               If so, divide the 3x4 into 3 1x4's.  We accept 1x2/2x1 definitions if the call is
               "matrix aware" (it knows that 1x4's are what it wants) or if the facing directions
               are such that that would win anyway. */

            if (  !forbid_little_stuff &&
                  !assoc(b_2x2, ss, calldeflist) &&
                  (
                     assoc(b_1x1, ss, calldeflist)
                           ||
                     ((!(newtb & 010) || assoc(b_1x2, ss, calldeflist)) &&
                      (!(newtb & 001) || assoc(b_2x1, ss, calldeflist)))
                           ||
                     (matrix_aware &&
                        (  assoc(b_1x2, ss, calldeflist) ||
                           assoc(b_2x1, ss, calldeflist)))
                  )
            ) {
               division_code = MAPCODE(s1x4,3,MPKIND__SPLIT,1);
               goto divide_us_no_recompute;
            }

            /* Now check for something that can be fudged into a "quarter tag"
               (which includes diamonds).  Note whether we fudged,
               since some calls do not tolerate it. */

            really_fudged = FALSE;
            gather(&sss, ss, map_3x4_fudge, 7, 0);

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
            sss.cmd = ss->cmd;
            sss.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
            move(&sss, really_fudged, result);
            ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;
            result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
            return 1;
         }
      case s4x4:
         /* The call has no applicable 4x4 definition. */
         /* First, check whether it has 2x4/4x2 definitions, and divide the setup if so,
            and if the call permits it. */

         if ((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) || (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX)) {
            if (        (!(newtb & 010) || assoc(b_4x2, ss, calldeflist)) &&
                        (!(newtb & 001) || assoc(b_2x4, ss, calldeflist))) {
               division_maps = &map_vsplit_f;    /* Split to left and right halves. */
               goto divide_us_no_recompute;
            }
            else if (   (!(newtb & 001) || assoc(b_4x2, ss, calldeflist)) &&
                        (!(newtb & 010) || assoc(b_2x4, ss, calldeflist))) {
               division_code = MAPCODE(s2x4,2,MPKIND__SPLIT,1);    /* Split to bottom and top halves. */
               goto divide_us_no_recompute;
            }
         }

         /* The only other way this can be legal is if we can identify
            smaller setups of all real people and can do the call on them.  For
            example, if the outside phantom lines are fully occupied and the inside
            ones empty, we could do a swing-thru.  We also identify Z's from which
            we can do "Z axle". */

         switch (livemask) {
            case 0x6666:
               division_code = MAPCODE(s1x2,4,MPKIND__4_EDGES,0);
               goto divide_us_no_recompute;
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
               ss->cmd.cmd_misc_flags |= CMD_MISC__OFFSET_Z;
               if ((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) &&
                     (!(newtb & 010) || assoc(b_3x2, ss, calldeflist)) &&
                     (!(newtb & 001) || assoc(b_2x3, ss, calldeflist)))
                  goto divide_us_no_recompute;
               break;
            case 0xA6A6: case 0x9C9C:
               division_maps = &map_lh_s2x3_3;
               ss->cmd.cmd_misc_flags |= CMD_MISC__OFFSET_Z;
               if ((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) &&
                     (!(newtb & 010) || assoc(b_3x2, ss, calldeflist)) &&
                     (!(newtb & 001) || assoc(b_2x3, ss, calldeflist)))
                  goto divide_us_no_recompute;
               break;
            case 0xE4E4: case 0xB8B8:
               division_maps = &map_rh_s2x3_2;
               ss->cmd.cmd_misc_flags |= CMD_MISC__OFFSET_Z;
               if ((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) &&
                     (!(newtb & 010) || assoc(b_2x3, ss, calldeflist)) &&
                     (!(newtb & 001) || assoc(b_3x2, ss, calldeflist)))
                  goto divide_us_no_recompute;
               break;
            case 0x6A6A: case 0xC9C9:
               division_maps = &map_lh_s2x3_2;
               ss->cmd.cmd_misc_flags |= CMD_MISC__OFFSET_Z;
               if ((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) &&
                     (!(newtb & 010) || assoc(b_2x3, ss, calldeflist)) &&
                     (!(newtb & 001) || assoc(b_3x2, ss, calldeflist)))
                  goto divide_us_no_recompute;
               break;

            case 0x4B4B: case 0xB4B4:
               /* See comment above, for 3x4. */
               {
                  long_boolean forbid_little_stuff =
                        assoc(b_2x4, ss, calldeflist) ||
                        assoc(b_4x2, ss, calldeflist) ||
                        assoc(b_2x3, ss, calldeflist) ||
                        assoc(b_3x2, ss, calldeflist) ||
                        assoc(b_dmd, ss, calldeflist) ||
                        assoc(b_pmd, ss, calldeflist) ||
                        assoc(b_qtag, ss, calldeflist) ||
                        assoc(b_pqtag, ss, calldeflist);

                  /* We are in "clumps".  See if we can do the call in 2x2 or smaller setups. */
                  if (  forbid_little_stuff ||
                        (  !assoc(b_2x2, ss, calldeflist) &&
                           !assoc(b_1x2, ss, calldeflist) &&
                           !assoc(b_2x1, ss, calldeflist) &&
                           !assoc(b_1x1, ss, calldeflist))) fail("Don't know how to do this call in this setup.");
                  if (!matrix_aware) warn(warn__each2x2);
                  division_code = (livemask == 0x4B4B) ? MAPCODE(s2x2,2,MPKIND__OFFS_L_FULL,0) : MAPCODE(s2x2,2,MPKIND__OFFS_R_FULL,0);
                  goto divide_us_no_recompute;
               }
         }

         {
            /* Setup is randomly populated.  See if we have 1x2/1x1 definitions, but no 2x2.
               If so, divide the 3x4 into 3 1x4's. */

            long_boolean forbid_little_stuff =
                  assoc(b_2x4, ss, calldeflist) ||
                  assoc(b_4x2, ss, calldeflist) ||
                  assoc(b_2x3, ss, calldeflist) ||
                  assoc(b_3x2, ss, calldeflist) ||
                  assoc(b_dmd, ss, calldeflist) ||
                  assoc(b_pmd, ss, calldeflist) ||
                  assoc(b_qtag, ss, calldeflist) ||
                  assoc(b_pqtag, ss, calldeflist);

            if (  !forbid_little_stuff &&
                  (ss->cmd.callspec->callflags1 & CFLAG1_12_16_MATRIX_MEANS_SPLIT) &&
                  (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX) &&
                  !assoc(b_2x2, ss, calldeflist) &&
                  (  assoc(b_1x2, ss, calldeflist) ||
                     assoc(b_2x1, ss, calldeflist) ||
                     assoc(b_1x1, ss, calldeflist))) {
               /* Without a lot of examination of facing directions, and whether the call has 1x2 vs. 2x1
                  definitions, and all that, we don't know which axis to use when dividing it.  But any
                  division into 2 2x4's is safe, and code elsewhere will make the tricky decisions later. */
               division_code = MAPCODE(s2x4,2,MPKIND__SPLIT,1);
               goto divide_us_no_recompute;
            }
         }

         fail("You must specify a concept.");
      case s_qtag:
         if (assoc(b_dmd, ss, calldeflist) || assoc(b_pmd, ss, calldeflist) || assoc(b_1x1, ss, calldeflist) || assoc(b_1x4, ss, calldeflist)) {
            division_code = MAPCODE(sdmd,2,MPKIND__SPLIT,1);
            goto divide_us_no_recompute;
         }

         /* Check whether it has 2x3/3x2 definitions, and divide the setup if so,
            and if the call permits it.  This is important for permitting "Z axle" from
            a 3x4 but forbidding "circulate" (unless we give a concept like 12 matrix
            phantom columns.)  We also enable this if the caller explicitly said
            "3x4 matrix". */

         if (((callflags1 & CFLAG1_SPLIT_LARGE_SETUPS) || (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX)) &&
               (!(newtb & 010) || assoc(b_3x2, ss, calldeflist)) &&
               (!(newtb & 001) || assoc(b_2x3, ss, calldeflist))) {
            division_maps = &map_qtag_2x3;
            goto divide_us_no_recompute;
         }

         if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
            fail("Can't split the setup.");

         if (must_do_concentric)
            goto do_concentrically;

         if ((!(newtb & 010) || assoc(b_1x2, ss, calldeflist)) &&
                  (!(newtb & 1) || assoc(b_2x1, ss, calldeflist))) {
            goto do_concentrically;
         }
         else if ((livemask & 0x55) == 0) {    /* Check for stuff like "heads pass the ocean; side corners */
            division_maps = &map_qtag_f1;      /* only slide thru". */
            goto divide_us_no_recompute;
         }
         else if ((livemask & 0x66) == 0) {
            division_maps = &map_qtag_f2;
            goto divide_us_no_recompute;
         }

/* ******** Regression test OK to here.  The next thing doesn't work, because the
   "do your part" junk doesn't work right!  This has been a known problem for some
   time, of course.  It's about time it got fixed.  It appears to have been broken as far back as 27.8. */

         else if ((livemask & 0x77) == 0) {    /* Check for stuff like "center two slide thru". */
            division_maps = &map_qtag_f0;
            goto divide_us_no_recompute;
         }
         break;
      case s_bone:
         if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
            fail("Can't split the setup.");

         if (assoc(b_trngl4, ss, calldeflist) || assoc(b_ptrngl4, ss, calldeflist)) {
            division_maps = &map_bone_trngl4;
            goto divide_us_no_recompute;
         }

         if (must_do_concentric)
            goto do_concentrically;

         {
            int tbi = ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1;
            int tbo = ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1;

            if ((!((tbi & 010) | (tbo & 001)) || assoc(b_1x2, ss, calldeflist)) &&
                     (!((tbi & 001) | (tbo & 010)) || assoc(b_2x1, ss, calldeflist)))
               goto do_concentrically;
         }

         if (livemask == 0xCC &&      /* Turn a bone with only the center line occupied into a 1x8. */
               (!(newtb & 010) || assoc(b_1x8, ss, calldeflist)) &&
               (!(newtb & 1) || assoc(b_8x1, ss, calldeflist))) {
            ss->kind = s1x8;
            swap_people(ss, 2, 7);
            swap_people(ss, 3, 6);
            return 2;                        /* And try again. */
         }
         break;
      case s_ptpd:
         if (assoc(b_dmd, ss, calldeflist) || assoc(b_pmd, ss, calldeflist) || assoc(b_1x1, ss, calldeflist)) {
            division_code = MAPCODE(sdmd,2,MPKIND__SPLIT,0);
            goto divide_us_no_recompute;
         }
         if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
            fail("Can't split the setup.");
         break;
      case s2x3:
         /* See if this call has applicable 1x2 or 2x1 definitions, in which case split it 3 ways. */
         if (     ((!(newtb & 010) || assoc(b_2x1, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_1x2, ss, calldeflist))) ||
                        assoc(b_1x1, ss, calldeflist)) {
            division_code = MAPCODE(s1x2,3,MPKIND__SPLIT,1);
            goto divide_us_no_recompute;
         }

         /* See if this call has applicable 1x3 or 3x1 definitions, in which case split it 2 ways. */
         if (     (!(newtb & 010) || assoc(b_1x3, ss, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_3x1, ss, calldeflist))) {
            division_code = MAPCODE(s1x3,2,MPKIND__SPLIT,1);
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
            if (ss->cmd.cmd_misc_flags & CMD_MISC__SAID_TRIANGLE) {
               if (final_concepts.final & FINAL__TRIANGLE)
                  fail("'Triangle' concept is redundant.");
            }
            else {
               if (!(final_concepts.final & FINAL__TRIANGLE))
                  fail("You must give the 'triangle' concept.");
            }

            if ((ss->people[0].id1 & d_mask) == d_east)
               division_maps = &map_trngl_box1;
            else if ((ss->people[0].id1 & d_mask) == d_west)
               division_maps = &map_trngl_box2;
            else
               fail("Can't figure out which way triangle point is facing.");

            final_concepts.final &= ~FINAL__TRIANGLE;
            ss->cmd.cmd_final_flags = final_concepts;
            divided_setup_move(ss, division_maps, phantest_ok, FALSE, result);

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
                  result->kind = s1x3;
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

            result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
            return 1;
         }
         break;
      case s1x6:
         /* See if this call has a 1x2, 2x1, or 1x1 definition, in which case split it 3 ways. */
         if (assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist) || assoc(b_1x1, ss, calldeflist)) {
            division_code = MAPCODE(s1x2,3,MPKIND__SPLIT,0);
            goto divide_us_no_recompute;
         }
         /* If it has 1x3 or 3x1 definitions, split it 2 ways. */
         if (assoc(b_1x3, ss, calldeflist) || assoc(b_3x1, ss, calldeflist)) {
            division_code = MAPCODE(s1x3,2,MPKIND__SPLIT,0);
            /* We want to be sure that the operator knows what we are doing, and why, if we divide
               a 1x6 into 1x3's.  We allow "swing thru" in a wave of 3 or 4 people.  If the operator
               wants to do a swing thru with all 6 people, use "grand swing thru". */
            warn(warn__split_1x6);
            goto divide_us_no_recompute;
         }
         break;
      case s1x2:
         /* See if the call has a 1x1 definition, in which case split it and do each part. */
         if (assoc(b_1x1, ss, calldeflist)) {
            division_code = MAPCODE(s1x1,2,MPKIND__SPLIT,0);
            goto divide_us_no_recompute;
         }
         break;
      case sdmd:
         /* See if the call has a 1x1 definition, in which case split it and do each part. */
         if (assoc(b_1x1, ss, calldeflist)) {
            division_maps = &map_dmd_1x1;
            goto divide_us_no_recompute;
         }
         break;
      case s_star:
         /* See if the call has a 1x1 definition, in which case split it and do each part. */
         if (assoc(b_1x1, ss, calldeflist)) {
            division_maps = &map_star_1x1;
            goto divide_us_no_recompute;
         }
         break;
      case s1x8:
         division_code = MAPCODE(s1x4,2,MPKIND__SPLIT,0);

         /* See if the call has a 1x4, 4x1, 1x2, 2x1, or 1x1 definition, in which case split it and do each part. */
         if (     assoc(b_1x4, ss, calldeflist) || assoc(b_4x1, ss, calldeflist)) {
            goto divide_us_no_recompute;
         }

         if (must_do_concentric)
            goto do_concentrically;

         if (     assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist) ||
                  assoc(b_1x1, ss, calldeflist)) {
            goto divide_us_no_recompute;
         }

         if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
            fail("Can't split the setup.");

         /* See if this call has applicable 1x12 or 1x16 definitions and matrix expansion is permitted.
            If so, that is what we must do. */

         if (  !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX) &&
               (!(newtb & 010) || assoc(b_1x12, ss, calldeflist) || assoc(b_1x16, ss, calldeflist)) &&
               (!(newtb & 1) || assoc(b_12x1, ss, calldeflist) || assoc(b_16x1, ss, calldeflist))) {
            do_matrix_expansion(ss, CONCPROP__NEEDK_1X12, TRUE);
            if (ss->kind != s1x12) fail("Failed to expand to 1X12.");  /* Should never fail, but we don't want a loop. */
            return 2;                        /* And try again. */
         }

         /* We might be doing some kind of "own the so-and-so" operation in which people who are ends of
            each wave in a 1x8 want to think they are points of diamonds instead.  This could happen,
            for example, with point-to-point diamonds if we say "own the <points>, flip the diamond by
            flip the diamond".  Yes, it's stupid.  Now normalize_setup turned the centerless diamonds
            into a 1x8 (it needs to do that in order for "own the <points>, trade by flip the diamond"
            to work.  We must turn that 1x8 back into diamonds.  The "own the so-and-so" concept turns
            on CMD_MISC__PHANTOMS.  If this flag weren't on, we would have no business saying "I see
            phantoms in the center 2 spots of my wave, I'm allowed to think of this as a diamond."
            The same thing is done below for 2x4's and 1x4's. */

         if ((ss->cmd.cmd_misc_flags & CMD_MISC__PHANTOMS) &&
                  (ss->people[1].id1 | ss->people[3].id1 | ss->people[5].id1 | ss->people[7].id1) == 0) {
            setup sstest = *ss;

            sstest.kind = s_ptpd;   /* It makes assoc happier if we do this now. */

            if (
                  (!(newtb & 010) || assoc(b_ptpd, &sstest, calldeflist) || assoc(b_dmd, &sstest, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_pptpd, &sstest, calldeflist) || assoc(b_pmd, &sstest, calldeflist))) {
               *ss = sstest;
               return 2;
            }
         }

         break;
      case s_crosswave:
         if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
            fail("Can't split the setup.");

         /* If we do not have a 1x4 or 4x1 definition, but we have 1x2, 2x1, or 1x1 definitions,
            do the call concentrically.  This will have the effect of having each miniwave do it.
            If we did this when a 1x4 or 4x1 definition existed, it would have the effect of having
            the people in the outer, disconnected, 1x4 work with each other across the set, which
            we do not want. */

         if (must_do_concentric)
            goto do_concentrically;

         if (!assoc(b_4x1, ss, calldeflist) && !assoc(b_1x4, ss, calldeflist) &&
               (assoc(b_2x1, ss, calldeflist) || assoc(b_1x2, ss, calldeflist) || assoc(b_1x1, ss, calldeflist)))
            goto do_concentrically;

         break;
      case s2x4:
         division_code = MAPCODE(s2x2,2,MPKIND__SPLIT,0);    /* The map we will probably use. */

         /* See if this call is being done "split" as in "split square thru" or "split dixie style",
            in which case split into boxes. */

         if (final_concepts.final & (FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED))
            goto divide_us_no_recompute;

         /* If this is "run", always split it into boxes.  If they are T-boned, they will figure it out, we hope. */

         if (calldeflist->callarray_flags & CAF__LATERAL_TO_SELECTEES)
            goto divide_us_no_recompute;

         /* See if this call has applicable 2x6 or 2x8 definitions and matrix expansion is permitted.
            If so, that is what we must do.  But if it has a 4x4 definition also, it is ambiguous,
            so we can't do it. */

         if (  !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX) &&
               !assoc(b_4x4, ss, calldeflist) &&
               (!(newtb & 010) || assoc(b_2x6, ss, calldeflist) || assoc(b_2x8, ss, calldeflist)) &&
               (!(newtb & 1) || assoc(b_6x2, ss, calldeflist) || assoc(b_8x2, ss, calldeflist))) {

            if (must_do_concentric)
               fail("Can't do \"snag/mystic\" with this call.");

            do_matrix_expansion(ss, CONCPROP__NEEDK_2X6, TRUE);
            if (ss->kind != s2x6) fail("Failed to expand to 2X6.");  /* Should never fail, but we don't want a loop. */
            return 2;                        /* And try again. */
         }

         /* If we are splitting for "central", "crazy", or "splitseq", give preference to 2x2 splitting. */

         temp_for_2x2 = TRUE;
         if ((ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)) {
            temp_for_2x2 = FALSE;    /* So we don't waste time computing it again. */
            if (assoc(b_2x2, ss, calldeflist))
               goto divide_us_no_recompute;
         }

         /* See if this call has applicable 1x4 or 4x1 definitions, in which case split it that way. */

         if ((!(newtb & 010) || assoc(b_1x4, ss, calldeflist)) &&
               (!(newtb & 1) || assoc(b_4x1, ss, calldeflist))) {
            division_code = MAPCODE(s1x4,2,MPKIND__SPLIT,1);
            goto divide_us_no_recompute;
         }

         /* See if this call has applicable 2x2 definition, in which case split into boxes. */

         if (temp_for_2x2 && assoc(b_2x2, ss, calldeflist)) goto divide_us_no_recompute;

         if (must_do_concentric)
            goto do_concentrically;

         /* See long comment above for s1x8.  The test cases for this are "own the <points>, trade
            by flip the diamond", and "own the <points>, flip the diamond by flip the diamond". */

         if ((ss->cmd.cmd_misc_flags & CMD_MISC__PHANTOMS) &&
                  (ss->people[1].id1 | ss->people[2].id1 | ss->people[5].id1 | ss->people[6].id1) == 0) {
            setup sstest = *ss;
            uint32 tbtest;

            sstest.kind = s_qtag;   /* It makes assoc happier if we do this now. */
            (void) copy_rot(&sstest, 1, &sstest, 0, 011);
            (void) copy_rot(&sstest, 0, &sstest, 7, 011);
            (void) copy_rot(&sstest, 5, &sstest, 4, 011);
            (void) copy_rot(&sstest, 4, &sstest, 3, 011);
            clear_person(&sstest, 3);
            clear_person(&sstest, 7);
            sstest.rotation--;
            tbtest = sstest.people[0].id1 | sstest.people[1].id1 | sstest.people[4].id1 | sstest.people[5].id1;

            if (
                  (!(tbtest & 010) || assoc(b_qtag, &sstest, calldeflist) || assoc(b_pmd, &sstest, calldeflist)) &&
                  (!(tbtest & 001) || assoc(b_pqtag, &sstest, calldeflist) || assoc(b_dmd, &sstest, calldeflist))) {
               *ss = sstest;
               *newtb_p = tbtest;
               return 2;                        /* And try again. */
            }
         }

         /* Look very carefully at how we split this, so we get the RESULTFLAG__SPLIT_AXIS_MASK stuff right. */

         have_1x2 = assoc(b_1x2, ss, calldeflist);
         have_2x1 = assoc(b_2x1, ss, calldeflist);

         /* See if this call has applicable 1x2 or 2x1 definitions, (but not 2x2), in a non-T-boned setup.
            If so, split into boxes.  Furthermore, if the split could have been along either axis, we set
            both RESULTFLAG__SPLIT_AXIS_MASK bits. */

         if (((newtb & 1) == 0 && have_1x2 != 0) || ((newtb & 010) == 0 && have_2x1 != 0))
            goto divide_us_no_recompute;

         /* If the splitting is into 4 side-by-side 1x2 setups, just split into 2x2's --
            that will get the correct RESULTFLAG__SPLIT_AXIS_MASK bits. */
         if (((newtb & 1) == 0 && have_2x1 != 0) || ((newtb & 010) == 0 && have_1x2 != 0))
            goto divide_us_no_recompute;

         /* Also, if some phantom concept has been used and there are 1x2 or 2x1
            definitions, we also split it into boxes even if people are T-boned.  This is what makes
            everyone do their part if we say "heads into the middle and heads are standard in split phantom
            lines, partner trade".  But we don't turn on both splitting bits in this case.  Note that,
            since the previous test failed, the setup must be T-boned if this test passes. */

         if ((ss->cmd.cmd_misc_flags & CMD_MISC__PHANTOMS) && (have_1x2 != 0 || have_2x1 != 0))
            goto divide_us_no_recompute;

         /* If we are T-boned and have 1x2 or 2x1 definitions, we need to be careful. */

         if ((newtb & 011) == 011) {
            int tbi = ss->people[1].id1 | ss->people[2].id1 | ss->people[5].id1 | ss->people[6].id1;
            int tbo = ss->people[0].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[7].id1;

            /* If the centers and ends are separately consistent, we can do the call concentrically *IF*
               the appropriate type of definition exists for the ends to work with the near person rather
               than the far one.  This is what makes "heads into the middle and everbody partner trade" work,
               and forbids "heads into the middle and everbody star thru". */

            if (((tbi & 011) != 011) && ((tbo & 011) != 011)) {
               if ((!(tbo & 010) || have_2x1 != 0) && (!(tbo & 1) || have_1x2 != 0)) {
                  if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
                     fail("Can't split the setup.");

                  goto do_concentrically;
               }

               /* Or if we have a 1x1 definition, we can divide it.  Otherwise, we lose. */
               else if (assoc(b_1x1, ss, calldeflist))
                  goto divide_us_no_recompute;
            }

            /* If the centers and ends are not separately consistent, we should just split it into 2x2's.
               Perhaps the call has both 1x2 and 2x1 definitions, and will be done sort of siamese in each quadrant.
               Another possibility is that the call has just (say) 1x2 definitions, but everyone can do their
               part and miraculously not hit each other. */

            else goto divide_us_no_recompute;
         }

         /* We are not T-boned, and there is no 1x2 or 2x1 definition.  The only possibility is that there
            is a 1x1 definition, in which case splitting into boxes will work. */

         else if (assoc(b_1x1, ss, calldeflist))
            goto divide_us_no_recompute;

         break;
      case s1x4:
         /* See if the call has a 1x2, 2x1, or 1x1 definition, in which case split it and do each part. */
         if ((assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist) || assoc(b_1x1, ss, calldeflist))) {

            /* The following makes "ends hinge" work from a grand wave.  Any 1x4 -> 2x2 call
               that acts by dividing itself into 1x2's is presumed to want the people in each 1x2
               to stay near each other.  We signify that by flipping the elongation, which we
               had previously set perpendicular to the 1x4 axis, overriding anything that may
               have been in the call definition. */

            *desired_elongation_p ^= 3;
            /* If the flags were zero and we complemented them so that both are set, that's not good. */
            if (*desired_elongation_p == 3)
               *desired_elongation_p = 0;

            division_code = MAPCODE(s1x2,2,MPKIND__SPLIT,0);
            goto divide_us_no_recompute;
         }

         /* See long comment above for s1x8.  The test cases for this are "tandem own the <points>, trade
            by flip the diamond", and "tandem own the <points>, flip the diamond by flip the diamond",
            both from a tandem diamond (the point being that there will be only one of them.) */

         if ((ss->cmd.cmd_misc_flags & CMD_MISC__PHANTOMS) &&
                  (ss->people[1].id1 | ss->people[3].id1) == 0) {
            setup sstest = *ss;

            sstest.kind = sdmd;   /* It makes assoc happier if we do this now. */

            if (
                  (!(newtb & 010) || assoc(b_dmd, &sstest, calldeflist)) &&
                  (!(newtb & 001) || assoc(b_pmd, &sstest, calldeflist))) {
               *ss = sstest;
               return 2;                        /* And try again. */
            }
         }

         break;
      case s_trngl4:
         /* See if the call has a 1x2, 2x1, or 1x1 definition, in which case split it and do each part. */
         if ((assoc(b_1x2, ss, calldeflist) || assoc(b_2x1, ss, calldeflist) || assoc(b_1x1, ss, calldeflist))) {
            division_maps = &map_tgl4_1;
            goto divide_us_no_recompute;
         }
   }

   if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT)
      fail("Can't split the setup.");

   result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
   return 0;    /* We did nothing.  An error will presumably result. */

   divide_us_no_recompute:

   if (must_do_concentric)
      fail("Can't do \"snag/mystic\" with this call.");

   recompute_anyway = (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT) != 0;
   ss->cmd.cmd_misc_flags &= ~CMD_MISC__MUST_SPLIT;
   ss->cmd.prior_elongation_bits = 0;

   if (division_code == ~0)
      divided_setup_move(ss, division_maps, phantest_ok, recompute_anyway, result);
   else
      new_divided_setup_move(ss, division_code, phantest_ok, recompute_anyway, result);

   /* If expansion to a 2x3 occurred (because the call was, for example, a "pair the line"),
      and the two 2x3's are end-to-end in a 2x6, see if we can squash phantoms.  We squash both
      internal (the center triple box) and external ones.  The external ones would probably have
      been squashed anyway due to the top level normalization, but we want this to occur
      immediately, not just at the top level, though we can't think of a concrete example
      in which it makes a difference. */
   if ((result->result_flags & RESULTFLAG__EXPAND_TO_2X3) && result->kind == s2x6) {
      if (!(result->people[2].id1 | result->people[3].id1 | result->people[8].id1 | result->people[9].id1)) {
         /* Inner spots are empty. */
         setup temp = *result;
         static Const veryshort inner_2x6[8] = {0, 1, 4, 5, 6, 7, 10, 11};

         gather(result, &temp, inner_2x6, 7, 0);
         result->kind = s2x4;
      }
      else if (!(result->people[0].id1 | result->people[5].id1 | result->people[6].id1 | result->people[11].id1)) {
         /* Outer spots are empty. */
         setup temp = *result;
         static Const veryshort outer_2x6[8] = {1, 2, 3, 4, 7, 8, 9, 10};

         gather(result, &temp, outer_2x6, 7, 0);
         result->kind = s2x4;
      }
   }
   else if ((result->result_flags & RESULTFLAG__EXPAND_TO_2X3) && result->kind == s4x6) {
      /* We do the same for two concatenated 3x4's.  This could happen if the people folding were not the ends. */
      if (!(      result->people[2].id1 | result->people[3].id1 | result->people[8].id1 | result->people[9].id1 |
                  result->people[20].id1 | result->people[21].id1 | result->people[14].id1 | result->people[15].id1)) {
         /* Inner spots are empty. */
         setup temp = *result;
         static Const veryshort outer_4x6[16] = {
            5, 6, 23, 7, 12, 13, 16, 22, 17, 18, 11, 19, 0, 1, 4, 10};

         gather(result, &temp, outer_4x6, 15, 0);
         result->kind = s4x4;
         canonicalize_rotation(result);
      }
      else if (!( result->people[0].id1 | result->people[5].id1 | result->people[6].id1 | result->people[11].id1 |
                  result->people[18].id1 | result->people[23].id1 | result->people[12].id1 | result->people[17].id1)) {
         /* Outer spots are empty. */
         setup temp = *result;
         static Const veryshort inner_4x6[16] = {4, 7, 22, 8, 13, 14, 15, 21, 16, 19, 10, 20, 1, 2, 3, 9};

         gather(result, &temp, inner_4x6, 15, 0);
         result->kind = s4x4;
         canonicalize_rotation(result);
      }
   }

   return 1;

   do_concentrically:

   conc_cmd = ss->cmd;
   concentric_move(ss, &conc_cmd, &conc_cmd, schema_concentric, 0, 0, FALSE, result);
   return 1;
}



/* For this routine, we know that callspec is a real call, with an array definition schema.
   Also, result->people have been cleared. */

extern void basic_move(
   setup *ss,
   int tbonetest,
   long_boolean fudged,
   long_boolean mirror,
   setup *result)
{
   int i, j, k;
   uint32 newtb;
   callarray *calldeflist;
   long_boolean funny;
   uint32 division_code;
   map_thing *division_maps;
   callarray *linedefinition;
   callarray *coldefinition;
   uint32 matrix_check_flag = 0;
   callarray *goodies;
   uint32 search_concepts;
   uint32 search_temp;
   int real_index, northified_index;
   int num, halfnum, final_numout;
   uint32 collision_mask;
   int collision_index;
   long_boolean collision_appears_illegal;
   uint32 result_mask;
   personrec newpersonlist[24];
   int newplacelist[24];
   int orig_elongation = 0;
   int inconsistent_rotation, inconsistent_setup;
   long_boolean four_way_startsetup;
   uint32 resultflags = 0;
   int desired_elongation = 0;
   long_boolean funny_ok1 = FALSE;
   long_boolean funny_ok2 = FALSE;
   long_boolean other_elongate = FALSE;
   calldef_block *qq;
   callspec_block *callspec = ss->cmd.callspec;
   uint64 final_concepts = ss->cmd.cmd_final_flags;
   long_boolean check_peeloff_migration = FALSE;

   /* We don't allow "central" or "invert" with array-defined calls.  We might allow "snag" or "mystic". */
   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK) {
      switch (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_KMASK) {
         case 0: case CMD_MISC2__CENTRAL_PLAIN:
            fail("Can't do \"central\" or \"invert\" with this call.");
      }
   }

   result->result_flags = 0;   /* Do this now, in case we bail out.  Note also that
      this means the RESULTFLAG__SPLIT_AXIS_MASK stuff will be clear for the normal case.
      It will only have good stuff if splitting actually occurs. */

   if (     (ss->kind == s_dead_concentric && ss->inner.skind == s1x4) ||
            (ss->kind == s_normal_concentric && ss->inner.skind == s1x4 && ss->outer.skind == nothing)) {
      tbonetest = 0;
      for (j=0; j<4; j++) tbonetest |= ss->people[j].id1;
      ss->kind = s1x8;
      ss->rotation = ss->inner.srotation;
      clear_person(ss, 4);
      clear_person(ss, 5);
      ss->people[6] = ss->people[3];
      ss->people[7] = ss->people[2];
      ss->people[2] = ss->people[1];
      ss->people[3] = ss->people[0];
      clear_person(ss, 0);
      clear_person(ss, 1);
   }
   else if ((ss->kind == s_dead_concentric && ss->inner.skind == s2x2) ||
            (ss->kind == s_normal_concentric && ss->inner.skind == s2x2 && ss->outer.skind == nothing)) {
      if (ss->concsetup_outer_elongation) {
         /* Do nothing, which will cause an error. */
      }
      else {
         tbonetest = 0;
         for (j=0; j<4; j++) tbonetest |= ss->people[j].id1;
         ss->kind = s2x4;
         ss->rotation = ss->inner.srotation;
         clear_person(ss, 4);
         clear_person(ss, 7);
         ss->people[6] = ss->people[3];
         ss->people[5] = ss->people[2];
         ss->people[2] = ss->people[1];
         ss->people[1] = ss->people[0];
         clear_person(ss, 0);
         clear_person(ss, 3);
      }
   }

   newtb = tbonetest;
   if (setup_attrs[ss->kind].setup_limits < 0) fail("Setup is extremely bizarre.");

   /* We demand that the final concepts that remain be only the following ones. */

   if (final_concepts.final &
         ~(FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED | FINAL__TRIANGLE))
      fail("This concept not allowed here.");

   /* Set up the result elongation that we will need if the result is
      a 2x2 or short6.  This comes from the original 2x2 elongation, or is set
      perpendicular to the original 1x4/diamond elongation.  If the call has the
      "parallel_conc_end" flag set, invert that elongation.

      We will override all this if we divide a 1x4 or 2x2 into 1x2's.

      What this means is that the default for a 2x2->2x2 or short6->short6 call
      is to work to "same absolute elongation".  For a 2x2 that is of course the same
      as working to footprints.  For a short6 it works to footprints if the call
      doesn't rotate, and does something a little bit weird if it does.  In either
      case the "parallel_conc_end" flag inverts that.

      The reason for putting up with this rather weird behavior is to make counter
      rotate work for 2x2 and short6 setups.  The call has the "parallel_conc_end" flag
      set, so that "ends counter rotate" will do the correct (anti-footprint) thing.
      The "parallel_conc_end" flag applies only to an entire call, not to a specific
      by-array definition, so it is set for counter rotate from a short6 also.
      Counter rotate for a short6 rotates the setup, of course, and we want it to do
      the correct shape-preserving thing, which requires that the absolute elongation
      change when "parallel_conc_end" is set.  So the default is that, in the absence
      of "parallel_conc_end", the absolute elongation stays the same, even though that
      changes the shape for a rotating short6 call. */

   switch (ss->kind) {
      case s2x2: case s_short6:
         orig_elongation = ss->cmd.prior_elongation_bits;
         break;
      case s1x2: case s1x4: case sdmd:
         orig_elongation = 2 - (ss->rotation & 1);
         break;
   }

   desired_elongation = orig_elongation & 3;

   /* Attend to a few details, but only if there are real people present. */

   if (newtb) {
      switch (ss->kind) {
         case s2x2:
            /* Now we do a special check for split-square-thru or split-dixie-style types of things. */

            if (final_concepts.final & (FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED)) {
               uint32 i1, i2, p1, p2;

               ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

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

               /* Now do the required transformation, if it is a "split square thru" type.
                  For "split dixie style" stuff, do nothing -- the database knows what to do. */

               if (!(final_concepts.final & FINAL__SPLIT_DIXIE_APPROVED)) {
                  swap_people(ss, i1, i2);
                  copy_rot(ss, i1, ss, i1, 033);
                  copy_rot(ss, i2, ss, i2, 011);

                  /* Repair the damage. */
                  newtb = ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1;
               }
            }
            break;
         case s_qtag:
            if (fudged && !(callspec->callflags1 & CFLAG1_FUDGE_TO_Q_TAG))
               fail("Can't do this call from arbitrary 3x4 setup.");
            break;
      }
   }

   /* Many of the remaining final concepts (all of the heritable ones
      except "funny" and "left", but "left" has been taken care of)
      determine what call definition we will get. */

   search_concepts = final_concepts.herit & ~INHERITFLAG_FUNNY;
   search_temp = search_concepts;

foobar:

   for (qq = callspec->stuff.arr.def_list; qq; qq = qq->next) {
      if (qq->modifier_seth == search_temp) {
         if (qq->modifier_level > calling_level && !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_CHECK_MOD_LEVEL)) {
            if (allowing_all_concepts)
               warn(warn__bad_modifier_level);
            else
               fail("Use of this modifier on this call is not allowed at this level.");
         }
         calldeflist = qq->callarray_list;
         goto use_this_calldeflist;
      }
   }

   /* We didn't find anything. */

   if (matrix_check_flag != 0) goto need_to_divide;

   calldeflist = 0;

   use_this_calldeflist:

   /* Calldeflist ought to have something now.  If not, perhaps the concept
      "magic" or "interlocked" was given, and the call has no special definition for same,
      but wants us to divide the setup magically or interlockedly.  Or a similar thing with 12 matrix. */

   if (!calldeflist) {
      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
         fail("Can't do \"snag/mystic\" with this call.");

      /* First, check for "magic" and "interlocked" stuff, and do those divisions if so. */
      if (divide_for_magic(
               ss,
               final_concepts.herit,
               search_concepts & ~(INHERITFLAG_DIAMOND | INHERITFLAG_SINGLE | INHERITFLAG_SINGLEFILE | INHERITFLAG_CROSS | INHERITFLAG_GRAND),
               result))
         goto un_mirror;

      /* Now check for 12 matrix or 16 matrix things.  If the call has the "12_16_matrix_means_split" flag, and that
         (plus possible 3x3/4x4 stuff) was what we were looking for, remove those flags and split the setup. */

      if (callspec->callflags1 & CFLAG1_12_16_MATRIX_MEANS_SPLIT) {
         uint32 z = search_concepts & ~(INHERITFLAG_3X3 | INHERITFLAG_4X4);

         switch (ss->kind) {
            case s3x4:
               if (z == INHERITFLAG_12_MATRIX || (z == 0 && (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX))) {
                  /* "12 matrix" was specified.  Split it into 1x4's in the appropriate way. */
                  division_code = MAPCODE(s1x4,3,MPKIND__SPLIT,1);
                  goto divide_us;
               }
               break;
            case s2x6:
               if (z == INHERITFLAG_12_MATRIX || (z == 0 && (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX))) {
                  /* "12 matrix" was specified.  Split it into 2x2's in the appropriate way. */
                  division_code = MAPCODE(s2x2,3,MPKIND__SPLIT,0);
                  goto divide_us;
               }
               break;
            case s2x8:
               if (z == INHERITFLAG_16_MATRIX || (z == 0 && (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX))) {
                  /* "16 matrix" was specified.  Split it into 2x2's in the appropriate way. */
                  division_code = MAPCODE(s2x2,4,MPKIND__SPLIT,0);
                  goto divide_us;
               }
               break;
            case s4x4:
               if (z == INHERITFLAG_16_MATRIX || (z == 0 && (ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX))) {
                  /* "16 matrix" was specified.  Split it into 1x4's in the appropriate way. */
                  /* But which way is appropriate?  A 4x4 is ambiguous.  Being too lazy to look at
                     the call definition (the "assoc" stuff), we assume the call wants lines, since
                     it seems that that is true for all calls that have the "12_16_matrix_means_split" property. */
   
                  if (newtb & 1) {    /* If the setup is empty and newtb is zero, it doesn't matter what we do. */
                     division_code = ~0;
                     division_maps = &map_4x4v;
                  }
                  else
                     division_code = MAPCODE(s1x4,4,MPKIND__SPLIT,1);
   
                  goto divide_us;
               }
               break;
         }
      }

      goto try_to_find_deflist;

      divide_us:

      final_concepts.herit &= ~search_concepts;
      ss->cmd.cmd_final_flags = final_concepts;
      if (division_code == ~0)
         divided_setup_move(ss, division_maps, phantest_ok, TRUE, result);
      else
         new_divided_setup_move(ss, division_code, phantest_ok, TRUE, result);
      goto un_mirror;
   }

   /* We now have an association list (setups ==> definition arrays) in calldeflist.
      Search it for an entry matching the setup we have, or else divide the setup
         until we find something.
      If we can't handle the starting setup, perhaps we need to look for "12 matrix" or
         "16 matrix" call definitions. */

   search_for_call_def:

   linedefinition = (callarray *) 0;
   coldefinition = (callarray *) 0;

   /* If we have to split the setup for "crazy", "central", or "splitseq", we
      specifically refrain from finding a call definition.  The same is true if
      we have "snag" or "mystic". */

   if (     calldeflist &&
            !(ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT) &&
            !(ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)) {
      begin_kind key1 = setup_attrs[ss->kind].keytab[0];
      begin_kind key2 = setup_attrs[ss->kind].keytab[1];

      four_way_startsetup = FALSE;

      if (key1 != b_nothing && key2 != b_nothing) {
         if (key1 == key2) {     /* This is for things like 2x2 or 1x1. */
            linedefinition = assoc(key1, ss, calldeflist);
            coldefinition = linedefinition;
            four_way_startsetup = TRUE;
         }
         else {
            /* If the setup is empty, get whatever definitions we can get, so that
               we can find the "CFLAG1_PARALLEL_CONC_END" bit,
               also known as the "other_elongate" bit. */
            if (!newtb || (newtb & 010)) linedefinition = assoc(key1, ss, calldeflist);
            if (!newtb || (newtb & 1)) coldefinition = assoc(key2, ss, calldeflist);
         }
      }
   }

   switch (ss->kind) {
      case s_short6:
      case s_bone6:
      case s_trngl:
         break;
      default:
         if (final_concepts.final & FINAL__TRIANGLE)
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

      if (ss->setupflags & CMD_MISC__SAID_SPLIT) {
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

      if (final_concepts.final & FINAL__TRIANGLE)
         fail("Triangle concept not allowed here.");

      /* We got here if either linedefinition or coldefinition had something.
         If only one of them had something, but both needed to (because the setup
         was T-boned) the normal intention is that we proceed anyway, which will
         raise an error.  However, we check here for the case of a 1x2 setup
         that has one definition but not the other, and has a 1x1 definition as well.
         In that case, we split the setup.  This allows T-boned "quarter <direction>".
         The problem is that "quarter <direction>" has a 1x2 definition (for
         "quarter in") and a 1x1 definition, but no 2x1 definition.  (If it had
         a 2x1 definition, then the splitting from a 2x2 would be ambiguous.)
         So we have to fix that. */

      if (ss->kind == s1x2 && (newtb & 011) == 011 && (!linedefinition || !coldefinition))
         goto need_to_divide;

      goto do_the_call;
   }

   try_to_find_deflist:

   /* We may have something in "calldeflist" corresponding to the modifiers on this call,
      but there was nothing listed under that definition matching the starting setup. */

   /* First, see if adding a "12 matrix" or "16 matrix" modifier to the call will help.
      We need to be very careful about this.  The code below will divide, for example,
      a 2x6 into 2x3's if CMD_MISC__EXPLICIT_MATRIX is on (that is, if the caller
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
      what people want.

      What actually goes on here is even more complicated now.  We want to allow the
      concept "12 matrix" to cause division of the set.  This means that we not only
      allow an explicit matrix ("2x6 matrix") to be treated as a "12 matrix", the way
      we always have, but we allow it to go the other way.  That is, under certain
      conditions we will turn a "12 matrix" to get turned into an explicit matrix
      concept.  We do this special action only when all else has failed.

      We do this only once.  Because of all the goto's that we execute as we
      try various things, there is danger of looping.  To make sure that this happens
      only once, we set "matrix_check_flag" nonzero when we do it, and only do it if
      that flag is zero.

      We don't do this if "snag" or "mystic" was given.  In those cases, we know exactly
      why there is no definition, and we need to call "divide_the_setup" to fix it. */

   if (matrix_check_flag == 0 && !(ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)) {
      if (ss->kind == s2x6 || ss->kind == s3x4 || ss->kind == s1x12) matrix_check_flag = INHERITFLAG_12_MATRIX;
      else if (ss->kind == s2x8 || ss->kind == s4x4 || ss->kind == s1x16) matrix_check_flag = INHERITFLAG_16_MATRIX;

      /* But we might not have set "matrix_check_flag" nonzero!  How are we going to prevent looping?
         The answer is that we won't execute the goto unless we did set set it nonzero. */

      if (!(ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX) && calldeflist == 0 && (matrix_check_flag & ~search_concepts) == 0) {

         /* Here is where we do the very special stuff.  We turn a "12 matrix" concept into an explicit matrix.
            Note that we only do it if we would have lost anyway about 25 lines below (note that "calldeflist"
            is zero, and the search again stuff won't be executed unless CMD_MISC__EXPLICIT_MATRIX is on,
            which it isn't.)  So we turn on the CMD_MISC__EXPLICIT_MATRIX bit, and we turn off the
            INHERITFLAG_12_MATRIX or INHERITFLAG_16_MATRIX bit. */

         if (matrix_check_flag == 0) {
            /* We couldn't figure out from the setup what the matrix is, so we have to expand it. */

            if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX)) {
               if (search_concepts & INHERITFLAG_12_MATRIX) {
                  do_matrix_expansion(
                     ss,
                     (ss->kind == s2x4) ? CONCPROP__NEEDK_2X6 : CONCPROP__NEEDK_TRIPLE_1X4,
                     TRUE);

                  if (ss->kind != s2x6 && ss->kind != s3x4 && ss->kind != s1x12) fail("Can't expand to a 12 matrix.");
                  matrix_check_flag = INHERITFLAG_12_MATRIX;
               }
               else if (search_concepts & INHERITFLAG_16_MATRIX) {
                  if (ss->kind == s2x6) do_matrix_expansion(ss, CONCPROP__NEEDK_2X8, TRUE);
                  else if (ss->kind != s2x4) do_matrix_expansion(ss, CONCPROP__NEEDK_1X16, TRUE);
                  /* Take no action (and hence cause an error) if the setup was a 2x4.  If someone wants to say
                     "16 matrix 4x4 peel off" from normal columns, that person needs more help than we can give. */

                  if (ss->kind != s2x8 && ss->kind != s4x4 && ss->kind != s1x16) fail("Can't expand to a 16 matrix."); 
                  matrix_check_flag = INHERITFLAG_16_MATRIX;
               }
            }
         }

         search_concepts &= ~matrix_check_flag;
         ss->cmd.cmd_final_flags.herit &= ~matrix_check_flag;
         search_temp = search_concepts;
         ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
      }
      else
         search_temp = matrix_check_flag | search_concepts;

      if ((ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX) && matrix_check_flag != 0)
         goto foobar;
   }

   /* We need to divide the setup. */

   need_to_divide:

   if (!newtb) {
      result->kind = nothing;
      goto un_mirror;
   }

   if (!calldeflist)
      fail("Can't do this call with this modifier.");

   j = divide_the_setup(ss, &newtb, calldeflist, &desired_elongation, result);
   if (j == 1) goto un_mirror;                     /* It divided and did the call.  We are done. */
   else if (j == 2) goto search_for_call_def;      /* It did some necessary expansion or transformation,
                                                      but did not do the call.  Try again. */

   /* If we get here, linedefinition and coldefinition are both zero, and we will fail. */

   /* We are about to do the call by array! */

   do_the_call:

   /* This shouldn't happen, but we are being very careful here. */
   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
      fail("Can't do \"snag/mystic\" with this call.");

   ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;
   funny = final_concepts.herit & INHERITFLAG_FUNNY;
   inconsistent_rotation = 0;
   inconsistent_setup = 0;

   if (funny && (!(calldeflist->callarray_flags & CAF__FACING_FUNNY)))
      fail("Can't do this call 'funny'.");

   if (callspec->callflags1 & CFLAG1_PARALLEL_CONC_END) other_elongate = TRUE;
   if (coldefinition && (coldefinition->callarray_flags & CAF__OTHER_ELONGATE)) other_elongate = TRUE;
   if (linedefinition && (linedefinition->callarray_flags & CAF__OTHER_ELONGATE)) other_elongate = TRUE;

   if (other_elongate) desired_elongation ^= 3;
   /* If the flags were zero and we complemented them so that both are set, that's not good. */
   if (desired_elongation == 3) desired_elongation ^= 3;

   if (!newtb) {
      result->kind = nothing;   /* Note that we get the benefit of the "CFLAG1_PARALLEL_CONC_END" stuff here.  */
      goto un_mirror;           /*   This means that a counter rotate in an empty 1x2 will still change shape. */
   }

   /* Check that "linedefinition" has been set up if we will need it. */

   goodies = (callarray *) 0;

   if ((newtb & 010) || four_way_startsetup) {
      assumption_thing t;

      if (!linedefinition) {
         switch (ss->kind) {
            case sdmd:
               fail("Can't handle people in diamond or PU quarter-tag for this call.");
            case s_trngl:
               fail("Can't handle people in triangle for this call.");
            case s_qtag:
               fail("Can't handle people in quarter tag for this call.");
            case s_ptpd:
               fail("Can't handle people in point-to-point diamonds for this call.");
            case s3x4:
               fail("Can't handle people in triple lines for this call.");
            case s2x3: case s2x4:
               fail("Can't handle people in lines for this call.");
            case s_short6:
               fail("Can't handle people in tall/short 6 for this call.");
            case s2x2:
               fail("Can't handle people in box of 4 for this call.");
            case s1x2:
               fail("Can't handle people in line of 2 for this call.");
            case s1x3:
               fail("Can't handle people in line of 3 for this call.");
            case s1x4:
               fail("Can't handle people in line of 4 for this call.");
            case s1x6:
               fail("Can't handle people in line of 6 for this call.");
            case s1x8:
               fail("Can't handle people in line of 8 for this call.");
            case s_galaxy:
               fail("Can't handle people in galaxy for this call.");
            default:
               fail("Can't do this call.");
         }
      }

      t.assumption = linedefinition->restriction;
      t.assump_col = 0;
      t.assump_both = 0;
      t.assump_cast = 0;

      if (t.assumption == cr_nice_diamonds && ss->kind == s_qtag) {
         t.assumption = cr_jright;
         t.assump_col = 4;
      }

      if (t.assumption != cr_none) (void) check_restriction(ss, t, FALSE, linedefinition->callarray_flags & CAF__RESTR_MASK);
      goodies = linedefinition;
   }

   /* Check that "coldefinition" has been set up if we will need it. */

   if ((newtb & 1) && (!four_way_startsetup)) {
      assumption_thing t;

      if (!coldefinition) {
         switch (ss->kind) {
            case sdmd:
               fail("Can't handle people in diamond or normal quarter-tag for this call.");
            case s_trngl:
               fail("Can't handle people in triangle for this call.");
            case s_qtag: case s3x4:
               fail("Can't handle people in triple columns for this call.");
            case s2x3: case s2x4:
               fail("Can't handle people in columns for this call.");
            case s_short6:
               fail("Can't handle people in tall/short 6 for this call.");
            case s1x2:
               fail("Can't handle people in column of 2 for this call.");
            case s1x4:
               fail("Can't handle people in column of 4 for this call.");
            case s1x6:
               fail("Can't handle people in column of 6 for this call.");
            case s1x8:
               fail("Can't handle people in column of 8 for this call.");
            case s_galaxy:
               fail("Can't handle people in galaxy for this call.");
            default:
               fail("Can't do this call.");
         }
      }

      t.assumption = coldefinition->restriction;
      t.assump_col = 1;
      t.assump_both = 0;
      t.assump_cast = 0;

      if (t.assumption != cr_none) (void) check_restriction(ss, t, FALSE, coldefinition->callarray_flags & CAF__RESTR_MASK);

      /* If we have linedefinition also, check for consistency. */

      if (goodies) {
         /* ***** should also check the other stupid fields! */
         inconsistent_rotation = (goodies->callarray_flags ^ coldefinition->callarray_flags) & CAF__ROT;
         if ((setup_kind) goodies->end_setup != (setup_kind) coldefinition->end_setup) inconsistent_setup = 1;
      }

      goodies = coldefinition;
   }

   result->kind = (setup_kind) goodies->end_setup;

   if (result->kind == s_normal_concentric) {         /* ***** this requires an 8-person call definition */
      setup outer_inners[2];
      int outer_elongation;
      setup p1;

      if (inconsistent_rotation | inconsistent_setup) fail("This call is an inconsistent shape-changer.");
      if (funny) fail("Sorry, can't do this call 'funny'");

      clear_people(&p1);

      for (real_index=0; real_index<8; real_index++) {
         personrec this_person = ss->people[real_index];
         if (this_person.id1) {
            uint32 z;
            int real_direction = this_person.id1 & 3;
            int d2 = (this_person.id1 << 1) & 4;
            northified_index = (real_index ^ d2);
            z = find_calldef((real_direction & 1) ? coldefinition : linedefinition, ss, real_index, real_direction, northified_index);
            k = ((z >> 4) & 0x1F) ^ (d2 >> 1);
            install_person(&p1, k, ss, real_index);
            p1.people[k].id1 = (p1.people[k].id1 & ~(ROLL_MASK | 077)) | ((z + real_direction * 011) & 013) | ((z * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);
            p1.people[k].id1 &= ~STABLE_MASK;   /* For now, can't do fractional stable on this kind of call. */
         }
      }

      for (k=0; k<4; k++) {
         (void) copy_person(&outer_inners[1], k, &p1, k);
         (void) copy_person(&outer_inners[0], k, &p1, k+4);
      }

      outer_inners[0].result_flags = 0;
      outer_inners[1].result_flags = 0;
      outer_inners[0].kind = (setup_kind) goodies->end_setup_out;
      outer_inners[1].kind = (setup_kind) goodies->end_setup_in;
      outer_inners[0].rotation = (goodies->callarray_flags & CAF__ROT_OUT) ? 1 : 0;
      outer_inners[1].rotation = goodies->callarray_flags & CAF__ROT;

      /* For calls defined by array with concentric end setup, the "other_elongate" flag,
         which comes from "CFLAG1_PARALLEL_CONC_END" or "CAF__OTHER_ELONGATE",
         turns on the outer elongation. */
      outer_elongation = outer_inners[0].rotation;
      if (other_elongate) outer_elongation ^= 1;

      normalize_concentric(schema_concentric, 1, outer_inners, outer_elongation, result);

      goto fixup;
   }
   else {
      int lilresult_mask[2];
      setup_kind tempkind;
      uint32 vacate = 0;

      result->rotation = goodies->callarray_flags & CAF__ROT;
      num = setup_attrs[ss->kind].setup_limits+1;
      halfnum = num >> 1;
      tempkind = result->kind;
      lilresult_mask[0] = 0;
      lilresult_mask[1] = 0;

      if (funny) {
         if ((ss->kind != result->kind) || result->rotation || inconsistent_rotation || inconsistent_setup)
            fail("Can't do 'funny' shape-changer.");
      }

      /* Check for people cutting through or working around an elongated 2x2 setup. */
      if (     ss->kind == s2x2 &&
               (orig_elongation & 0x3F) != 0 &&
               !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_CHK_ELONG)) {
         if (callspec->callflags1 & CFLAG1_NO_ELONGATION_ALLOWED) {
            fail("Call can't be done around the outside of the set.");
         }
         else if (goodies->callarray_flags & CAF__NO_CUTTING_THROUGH) {
            if (result->kind == s1x4)
               check_peeloff_migration = TRUE;
            else {
               for (i=0; i<4; i++) {
                  uint32 p = ss->people[i].id1;
                  if (p != 0 && ((p-i-1) & 2) == 0 && ((p ^ orig_elongation) & 1) == 0)
                     fail("Call has outsides cutting through the middle of the set.");
               }
            }
         }
      }

      if (four_way_startsetup) {
         special_4_way_symm(linedefinition, ss, newpersonlist, newplacelist, lilresult_mask, result);
      }
      else if (ss->kind == s_trngl || ss->kind == s1x3) {
         if (inconsistent_rotation | inconsistent_setup) fail("This call is an inconsistent shape-changer.");
         special_3person(coldefinition, linedefinition, ss, newpersonlist, newplacelist, lilresult_mask, result);
      }
      else {
         int halfnumoutl, halfnumoutc, numoutl, numoutc;
         veryshort *final_translatec = identity;
         veryshort *final_translatel = identity;
         int rotfudge_line = 0;
         int rotfudge_col = 0;

         numoutl = setup_attrs[result->kind].setup_limits+1;
         numoutc = setup_attrs[result->kind].setup_limits+1;

         /* Check for a 1x4 call around the outside that sends people far away without permission. */
         if (     ss->kind == s1x4 &&
                  ss->cmd.prior_elongation_bits & 0x40 &&
                  !(ss->cmd.cmd_misc_flags & CMD_MISC__NO_CHK_ELONG)) {
            if (goodies->callarray_flags & CAF__NO_CUTTING_THROUGH) {
               fail("Call has outsides going too far around the set.");
            }
         }

         if (inconsistent_setup) {
            if (inconsistent_rotation) {
               if (result->kind == s_spindle && (setup_kind) linedefinition->end_setup == s_crosswave) {
                  result->kind = sx1x6;
                  tempkind = sx1x6;
                  final_translatec = ftcspn;
                  vacate = linedefinition->callarray_flags & CAF__VACATE_CENTER;

                  if (goodies->callarray_flags & CAF__ROT) {
                     final_translatel = &ftlcwv[0];
                     rotfudge_line = 3;
                  }
                  else {
                     final_translatel = &ftlcwv[4];
                     rotfudge_line = 1;
                  }
               }
               else if (result->kind == s2x4 && (setup_kind) linedefinition->end_setup == s_qtag) {
                  /* In this case, line people are right, column people are wrong. */
                  result->rotation = linedefinition->callarray_flags & CAF__ROT;
                  result->kind = s_qtag;
                  tempkind = s_qtag;

                  if (goodies->callarray_flags & CAF__ROT) {
                     final_translatec = &qtlqtg[4];
                     rotfudge_col = 1;
                  }
                  else {
                     final_translatec = &qtlqtg[0];
                     rotfudge_col = 3;
                  }
               }
               else if (result->kind == s_bone && (setup_kind) linedefinition->end_setup == s_qtag) {
                  result->rotation = linedefinition->callarray_flags & CAF__ROT;
                  result->kind = s_qtag;
                  tempkind = s_qtag;

                  if (goodies->callarray_flags & CAF__ROT) {
                     final_translatec = &btlqtg[4];
                     rotfudge_col = 1;
                  }
                  else {
                     final_translatec = &btlqtg[0];
                     rotfudge_col = 3;
                  }
               }
               else
                  fail("T-bone call went to a weird setup.");
            }
            else {
               if (result->kind == s4x4 && (setup_kind) linedefinition->end_setup == s2x4) {
                  final_translatel = &ftc4x4[0];
                  numoutl = setup_attrs[(setup_kind) linedefinition->end_setup].setup_limits+1;
               }
               if (result->kind == s4x4 && (setup_kind) linedefinition->end_setup == s_qtag) {
                  numoutl = setup_attrs[(setup_kind) linedefinition->end_setup].setup_limits+1;
                  result->kind = sbigh;
                  tempkind = sbigh;
                  final_translatec = ft4x4bh;
                  final_translatel = ftqtgbh;
               }
               else if (result->kind == s_c1phan && (setup_kind) linedefinition->end_setup == s2x4) {
                  numoutl = setup_attrs[(setup_kind) linedefinition->end_setup].setup_limits+1;
                  final_translatel = &ftcphan[0];
               }
               else if (result->kind == s2x4 && (setup_kind) linedefinition->end_setup == s_bone) {
                  final_translatel = &qtlbone[0];
               }
               else if (result->kind == s1x8 && (setup_kind) linedefinition->end_setup == s_crosswave) {
                  final_translatel = &qtlxwv[0];
               }
               else
                  fail("T-bone call went to a weird setup.");
            }
         }
         else if (inconsistent_rotation) {
            if (result->kind == s2x4) {
               result->kind = s4x4;
               tempkind = s4x4;
               final_translatec = ftc2x4;

               if (goodies->callarray_flags & CAF__ROT) {
                  final_translatel = &ftl2x4[0];
                  rotfudge_line = 3;
               }
               else {
                  final_translatel = &ftl2x4[4];
                  rotfudge_line = 1;
               }
            }
            else
               fail("This call is an inconsistent shape-changer.");
         }

         halfnumoutl = numoutl >> 1;
         halfnumoutc = numoutc >> 1;

         for (real_index=0; real_index<num; real_index++) {
            veryshort *final_translate;
            int kt;
            callarray *the_definition;
            uint32 z;
            personrec this_person = ss->people[real_index];
            newpersonlist[real_index].id1 = 0;
            newpersonlist[real_index].id2 = 0;
            if (this_person.id1) {
               int final_direction, d2out, thisnumout;
               int real_direction = this_person.id1 & 3;
               int d2 = ((this_person.id1 >> 1) & 1) * halfnum;
               northified_index = (real_index + d2) % num;

               if (real_direction & 1) {
                  d2out = ((this_person.id1 >> 1) & 1) * halfnumoutc;
                  thisnumout = numoutc;
                  final_translate = final_translatec;
                  final_direction = rotfudge_col;
                  the_definition = coldefinition;
               }
               else {
                  d2out = ((this_person.id1 >> 1) & 1) * halfnumoutl;
                  thisnumout = numoutl;
                  final_translate = final_translatel;
                  final_direction = rotfudge_line;
                  the_definition = linedefinition;
               }

               final_direction = (final_direction+real_direction) & 3;

               z = find_calldef(the_definition, ss, real_index, real_direction, northified_index);
               k = (((z >> 4) & 0x1F) + d2out) % thisnumout;
               newpersonlist[real_index].id1 = (this_person.id1 & ~(ROLL_MASK | 077)) |
                     ((z + final_direction * 011) & 013) |
                     ((z * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);

               if (this_person.id1 & STABLE_ENAB) {
                  do_stability(&newpersonlist[real_index].id1, (stability) ((z/DBSTAB_BIT) & 0xF), (z + final_direction - real_direction + result->rotation));
               }

               newpersonlist[real_index].id2 = this_person.id2;
               kt = final_translate[k];
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
      When we do a 16-matrix circulate in a 4x4 start setup, we do NOT want to compress
         the 4x4 to a 2x4!!!!!  This is why we compare the beginning and ending setup sizes. */

      if (setup_attrs[ss->kind].setup_limits < setup_attrs[result->kind].setup_limits) {
         veryshort *permuter = (veryshort *) 0;
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
#ifdef BREAKS_CAST_BACK
               /* See if people landed classical C1 phantom spots. */
               if ((lilresult_mask[0] & 0x3333) == 0) {
                  result->kind = s_c1phan;
                  permuter = phan4x4xlatea;
               }
               else if ((lilresult_mask[0] & 0x5555) == 0) {
                  result->kind = s_c1phan;
                  permuter = phan4x4xlateb;
               }
#endif
            }
         }
         else if (result->kind == sx1x6) {
            if ((lilresult_mask[0] & 07070) == 0) {         /* Check horiz 1x6 spots. */
               result->kind = s1x6;
               permuter = s1x6translateh;
            }
            else if ((lilresult_mask[0] & 00707) == 0) {    /* Check vert 1x6 spots. */
               result->kind = s1x6;
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
            else if ((lilresult_mask[0] & 06060) == 0) {    /* Check horiz 3x1dmd spots w/points out far. */
               result->kind = s3x1dmd;
               permuter = s3dmftranslateh;
            }
            else if ((lilresult_mask[0] & 00606) == 0) {    /* Check vert 3x1dmd spots w/points out far. */
               result->kind = s3x1dmd;
               permuter = s3dmftranslatev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 05050) == 0) {    /* Check horiz 3x1dmd spots w/points in close. */
               result->kind = s3x1dmd;
               permuter = s3dmntranslateh;
            }
            else if ((lilresult_mask[0] & 00505) == 0) {    /* Check vert 3x1dmd spots w/points in close. */
               result->kind = s3x1dmd;
               permuter = s3dmntranslatev;
               rotator = 1;
            }
            else if (vacate && (lilresult_mask[0] & 03030) == 0 && (lilresult_mask[0] & 00404) == 00404) {
               /* Check for star in the middle that can be disambiguated by having someone vacate it. */
               /* We have to mark this as controversial -- the center star is actually isotropic, and
                  we are fudging it so that T-boned coordinate will work. */
               warn(warn_controversial);
               result->kind = s1x3dmd;
               permuter = s_vacate_star;
            }
            else
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == s_thar) {
            if ((lilresult_mask[0] & 0x66) == 0) {    /* Check horiz dmd spots. */
               result->kind = sdmd;
               permuter = sdmdtranslateh;
            }
            else if ((lilresult_mask[0] & 0x99) == 0) {    /* Check vert dmd spots. */
               result->kind = sdmd;
               permuter = sdmdtranslatev;
               rotator = 1;
            }
            else
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == s8x8) {
            /* See if people landed on 2x8 or 4x6 spots. */
            result->kind = s2x8;
            permuter = octtranslateh;

            if ((lilresult_mask[0] & 0x333F11FFUL) == 0 && (lilresult_mask[1] & 0x333F11FFUL) == 0) {
               result->kind = s4x6;
               permuter = octt4x6lateh;
            }
            else if ((lilresult_mask[0] & 0x11FF333FUL) == 0 && (lilresult_mask[1] & 0x11FF333FUL) == 0) {
               result->kind = s4x6;
               permuter = octt4x6latev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 0x0FFF7777UL) == 0 && (lilresult_mask[1] & 0x0FFF7777UL) == 0) {
               permuter = octtranslatev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 0x77770FFFUL) != 0 || (lilresult_mask[1] & 0x77770FFFUL) != 0)
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == sx1x16) {
            /* See if people landed on 1x16 spots. */
            result->kind = s1x16;
            permuter = hextranslateh;

            if ((lilresult_mask[0] & 0x00FF00FFUL) == 0) {
               permuter = hextranslatev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 0xFF00FF00UL) != 0)
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == sx4dmd) {
            /* See if people landed on quad diamond spots. */

            if ((lilresult_mask[0] & 0xBFD7BFD7UL) == 0) {
               result->kind = s2x3;
               permuter = j23translatev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 0xD7BFD7BFUL) == 0) {
               result->kind = s2x3;
               permuter = j23translateh;
            }
            else if ((lilresult_mask[0] & 0x50AF50AFUL) == 0) {
               result->kind = s4dmd;
               permuter = qdmtranslatev;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 0xAF50AF50UL) == 0) {
               result->kind = s4dmd;
               permuter = qdmtranslateh;
            }
            else
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == s_hyperglass) {

            /* Investigate possible diamonds and 1x4's.  If enough centers
               and ends (points) are present to determine unambiguously what
               result setup we must create, we of course do so.  Otherwise, if
               the centers are missing but points are present, we give preference
               to making a 1x4, no matter what the call definition said the ending
               setup was.  But if centers are present and the points are missing,
               we go do diamonds if the original call definition wanted diamonds. */

            if ((lilresult_mask[0] & 05757) == 0 && tempkind == sdmd) {
               result->kind = sdmd;     /* Only centers present, call wanted diamonds. */
               permuter = dmdhyperh;
            }
            else if ((lilresult_mask[0] & 07575) == 0 && tempkind == sdmd) {
               result->kind = sdmd;     /* Only centers present, call wanted diamonds. */
               permuter = dmdhyperv;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 07474) == 0) {
               result->kind = s1x4;     /* Setup is consistent with lines, though maybe centers are absent. */
               permuter = linehyperh;
            }
            else if ((lilresult_mask[0] & 04747) == 0) {
               result->kind = s1x4;     /* Setup is consistent with lines, though maybe centers are absent. */
               permuter = linehyperv;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 05656) == 0) {
               result->kind = sdmd;     /* Setup is consistent with diamonds, though maybe centers are absent. */
               permuter = dmdhyperh;
            }
            else if ((lilresult_mask[0] & 06565) == 0) {
               result->kind = sdmd;     /* Setup is consistent with diamonds, though maybe centers are absent. */
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
         else if (result->kind == slittlestars) {
            if ((lilresult_mask[0] & 0xCC) == 0) {
               result->kind = s2x2;
               permuter = lilstar3;
            }
            else if ((lilresult_mask[0] & 0x33) == 0) {
               result->kind = s1x4;
               permuter = lilstar4;
            }
            else if ((lilresult_mask[0] & 0x2D) == 0) {
               result->kind = s_trngl4;
               permuter = lilstar1;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 0xD2) == 0) {
               result->kind = s_trngl4;
               permuter = lilstar2;
               rotator = 3;
            }
            else
               fail("Call went to improperly-formed setup.");
         }
         else if (result->kind == sbigdmd) {
            if ((lilresult_mask[0] & 02222) == 0) {        /* All outside */
               result->kind = s_qtag;
               permuter = qtbd1;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 04141) == 0) {   /* All inside */
               result->kind = s_qtag;
               permuter = qtbd2;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 02121) == 0) {   /* Some inside, some outside */
               result->kind = s_qtag;
               permuter = qtbd3;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 04242) == 0) {   /* Some inside, some outside */
               result->kind = s_qtag;
               permuter = qtbd4;
               rotator = 1;
            }
         }
         else if (result->kind == s2x3) {
            /* This call turned a smaller setup (like a 1x4) into a 2x3.
               It is presumably a call like "pair the line" or "step and slide".
               Flag the result setup so that the appropriate phantom-squashing
               will take place if two of these results are placed end-to-end. */
            resultflags |= RESULTFLAG__EXPAND_TO_2X3;
         }
         else if (result->kind == s3x4 && ss->kind == s1x4) {
            /* This could be the even more glorious form of the above, for "fold".
               If we can strip it down to a 2x3 (because the ends were the ones doing the fold),
               do so now.  In any case, set the flag so that the 3-to-2 squashing can take place later. */
            if ((lilresult_mask[0] & 03131) == 0) {
               result->kind = s2x3;
               permuter = q3x4xx1;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 01717) == 0) {
               result->kind = s1x4;
               permuter = q3x4xx2;
               rotator = 0;
            }
            else if ((lilresult_mask[0] & 01567) == 0 || (lilresult_mask[0] & 01673) == 0) {
               result->kind = s_trngl4;
               permuter = q3x4xx3;
               rotator = 1;
            }
            else if ((lilresult_mask[0] & 06715) == 0 || (lilresult_mask[0] & 07316) == 0) {
               result->kind = s_trngl4;
               permuter = q3x4xx4;
               rotator = 3;
            }

            resultflags |= RESULTFLAG__EXPAND_TO_2X3;
         }

         if (permuter) {
            uint32 r = 011*((-rotator) & 3);

            for (real_index=0; real_index<num; real_index++) {
               if (ss->people[real_index].id1) {
                  newplacelist[real_index] = permuter[newplacelist[real_index]];
                  if (rotator)
                     newpersonlist[real_index].id1 = rotperson(newpersonlist[real_index].id1, r);
               }
            }

            result->rotation += rotator;
         }
      }
   }

   /* Install all the people. */

   collision_mask = 0;
   collision_appears_illegal = FALSE;
   result_mask = 0;
   final_numout = setup_attrs[result->kind].setup_limits+1;

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
            else {
               /* We have a collision.  This may result in an error being raised. */

               collision_person1 = result->people[k].id1;   /* Prepare the error message. */
               collision_person2 = newperson.id1;
               error_message1[0] = '\0';
               error_message2[0] = '\0';

               if (final_numout > 12 || result->people[k+12].id1 != 0)
                  longjmp(longjmp_ptr->the_buf, 3);    /* We can't handle it at all -- just raise the error. */

               /* Store the person in the overflow area
                  (12 higher than the main area, which is why we only permit
                  this if the result setup size is <= 12) and record the fact
                  in the collision_mask so we can figure out what to do. */

               result->people[k+12] = newperson;
               collision_mask |= (1 << k);
               collision_index = k;        /* In case we need to report a mundane collision. */

               if (  (callspec->callflags1 & CFLAG1_TAKE_RIGHT_HANDS) ||
                     (  (callspec->callflags1 & CFLAG1_ENDS_TAKE_RIGHT_HANDS) && result->kind == s1x4 && !(k&1)  )) {
               /* Collisions are legal. */
               }
               else {
                  collision_appears_illegal = TRUE;
               }
            }
         }
      }
   }

   if (funny) {
      if (!funny_ok1 || !funny_ok2) warn(warn__not_funny);
   }
   else if (collision_mask)  {
      fix_collision(ss->cmd.cmd_misc_flags, collision_mask, collision_index, result_mask, collision_appears_illegal, mirror, result);
   }

   fixup:

   reinstate_rotation(ss, result);

   un_mirror:

   canonicalize_rotation(result);

   if (check_peeloff_migration) {
      /* Check that the resultant 1x4 is parallel to the original 2x2 elongation,
         and that everyone stayed on their own side of the split. */
      if (((orig_elongation ^ result->rotation) & 1) == 0)
         fail("People are too far apart to work with each other on this call.");

      for (i=0; i<4; i++) {
         int z = (i-result->rotation+1) & 2;
         uint32 p = ss->people[i].id1;
         if (     (((p ^ result->people[z].id1) & PID_MASK) != 0) &&
                  (((p ^ result->people[z+1].id1) & PID_MASK) != 0))
            fail("Call can't be done around the outside of the set.");
      }
   }

   /* We take out any elongation info that divided_setup_move may have put in
      and override it with the correct info. */

   result->result_flags = (result->result_flags & (~3)) | resultflags | (desired_elongation & 3);
}
