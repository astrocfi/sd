/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992  William B. Ackerman.

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

    This is for version 27. */

/* This file contains stuff for tandem and as-couples moves. */

/* This defines the following functions:
   tandem_couples_move
   initialize_tandem_tables
*/

#include "sd.h"

typedef struct {
   personrec real_front_people[MAX_PEOPLE];
   personrec real_back_people[MAX_PEOPLE];
   setup virtual_setup;
   setup virtual_result;
   int vertical_people[MAX_PEOPLE];    /* 1 if original people were near/far; 0 if lateral */
   int twosomep[MAX_PEOPLE];           /* 0: solid / 1: twosome / 2: solid-to-twosome / 3: twosome-to-solid */
   unsigned int single_mask;
   } tandrec;

typedef struct {
   int map1[8];
   int map2[8];
   unsigned int sidewaysmask;       /* lateral pairs in inside numbering -- only alternate bits used! */
   unsigned int outsidemask;
   int limit;
   int rot;
   unsigned int insinglemask;       /* relative to insetup numbering, those people that are NOT paired */
   unsigned int outsinglemask;      /* relative to outsetup numbering, those people that are NOT paired */
   unsigned int outunusedmask;
   setup_kind insetup;
   setup_kind outsetup;
   } tm_thing;


static tm_thing maps_isearch[] = {

/*         map1                              map2                  sidemask outsidemask limit rot            insetup outsetup            old name */
   {{7, 6, 4, 5},                   {0, 1, 3, 2},                     0,     0000,         4, 0,  0,  0, 0,  s1x4,  s2x4},            /* "2x4_4" - see below */
   {{0, 2, 5, 7},                   {1, 3, 4, 6},                  0x55,     0377,         4, 0,  0,  0, 0,  s2x2,  s2x4},
   {{2, 5, 7, 0},                   {3, 4, 6, 1},                     0,     0377,         4, 1,  0,  0, 0,  s2x2,  s2x4},
   {{3, 2},                         {0, 1},                           0,     0000,         2, 0,  0,  0, 0,  s_1x2, s2x2},
   {{0, 3},                         {1, 2},                           0,     0017,         2, 1,  0,  0, 0,  s_1x2, s2x2},
   {{0, 3},                         {1, 2},                         0x5,     0017,         2, 0,  0,  0, 0,  s_1x2, s1x4},
   {{0},                            {1},                            0x1,     0003,         1, 0,  0,  0, 0,  s_1x1, s_1x2},           /* 1x2 */  /* this one is known to be good */
   {{0},                            {1},                              0,     0003,         1, 1,  0,  0, 0,  s_1x1, s_1x2},           /* ????? */
   {{0, 3, 5, 6},                   {1, 2, 4, 7},                  0x55,     0377,         4, 0,  0,  0, 0,  s1x4,  s1x8},
   {{15, 14, 12, 13, 8, 9, 11, 10}, {0, 1, 3, 2, 7, 6, 4, 5},         0,     0000,         8, 0,  0,  0, 0,  s1x8,  s2x8},
   {{11, 10, 9, 6, 7, 8},           {0, 1, 2, 5, 4, 3},               0,     0000,         6, 0,  0,  0, 0,  s_1x6, s2x6},
   {{10, 15, 3, 1, 4, 5, 6, 8},     {12, 13, 14, 0, 2, 7, 11, 9},     0,     0000,         8, 0,  0,  0, 0,  s2x4,  s4x4},
   {{14, 3, 7, 5, 8, 9, 10, 12},    {0, 1, 2, 4, 6, 11, 15, 13},      0,   0xFFFF,         8, 1,  0,  0, 0,  s2x4,  s4x4},
   {{2, 5, 7, 9, 10, 0},            {3, 4, 6, 8, 11, 1},              0,   0x0FFF,         6, 1,  0,  0, 0,  s_2x3, s3x4},
   {{0, 2, 4, 6, 9, 11, 13, 15},    {1, 3, 5, 7, 8, 10, 12, 14}, 0x5555,   0xFFFF,         8, 0,  0,  0, 0,  s2x4,  s2x8},

   /* Next one is for centers in tandem in lines, making a virtual bone6. */
   {{0, 3, 5, 4, 7, 6},             {-1, -1, 2, -1, -1, 1},           0,     0000,         6, 0,  0,  0, 0,  s_bone6, s2x4},
   /* Next two are for certain ends in tandem in an H, making a virtual bone6. */
   {{10, 3, 5, 6, 9, 11},           {0, -1, -1, 4, -1, -1},           0,     0000,         6, 0,  0,  0, 0,  s_bone6, s3x4},
   {{0, 4, 5, 6, 9, 11},            {-1, 3, -1, -1, 10, -1},          0,     0000,         6, 0,  0,  0, 0,  s_bone6, s3x4},
   /* Next one is for ends in tandem in lines, making a virtual short6. */
   {{2, 4, 5, 6, 7, 1},             {-1, 3, -1, -1, 0, -1},       0x104,     0000,         6, 1,  0,  0, 0,  s_short6, s2x4},
   /* Next two are for certain center column people in tandem in a 1/4 tag, making a virtual short6. */
   {{3, 2, 4, 5, 6, 0},             {1, -1, -1, 7, -1, -1},       0x041,     0000,         6, 1,  0,  0, 0,  s_short6, s_qtag},
   {{1, 2, 4, 5, 6, 7},             {-1, -1, 3, -1, -1, 0},       0x410,     0000,         6, 1,  0,  0, 0,  s_short6, s_qtag},
   /* Next three are for various people in tandem in columns of 8, making virtual columns of 6. */
   {{0, 2, 3, 5, 6, 7},             {1, -1, -1, 4, -1, -1},       0x041,     0063,         6, 0,  0,  0, 0,  s_2x3, s2x4},
   {{0, 1, 3, 4, 6, 7},             {-1, 2, -1, -1, 5, -1},       0x104,     0146,         6, 0,  0,  0, 0,  s_2x3, s2x4},
   {{0, 1, 2, 4, 5, 7},             {-1, -1, 3, -1, -1, 6},       0x410,     0314,         6, 0,  0,  0, 0,  s_2x3, s2x4},
   /* Next three are for various people as couples in a C1 phantom or 1/4 tag, making virtual columns of 6. */
   {{3, 7, 5, 9, 15, 13},           {1, -1, -1, 11, -1, -1},          0,     0000,         6, 0,  0,  0, 0,  s_2x3, s_c1phan},
   {{1, 3, 4, 5, 6, 0},             {-1, 2, -1, -1, 7, -1},           0,     0314,         6, 1,  0,  0, 0,  s_2x3, s_qtag},
   {{0, 2, 6, 8, 10, 12},           {-1, -1, 4, -1, -1, 14},          0,     0000,         6, 0,  0,  0, 0,  s_2x3, s_c1phan},
   /* Next three are for various people in tandem in a wing or PTP diamonds, making a virtual line of 6. */
   {{6, 7, 5, 2, 3, 4},             {-1, -1, 0, -1, -1, 1},           0,     0000,         6, 0,  0,  0, 0,  s_1x6, s_rigger},
   {{0, 3, 2, 4, 5, 6},             {-1, 1, -1, -1, 7, -1},           0,     0000,         6, 0,  0,  0, 0,  s_1x6, s_ptpd},
   {{5, 6, 7, 4, 2, 3},             {0, -1, -1, 1, -1, -1},           0,     0000,         6, 0,  0,  0, 0,  s_1x6, s_bone},

   {{0, 3, 5, 6},                   {1, 2, 4, 7},                     0,     0377,         4, 1,  0,  0, 0,  sdmd,  s_qtag},
   {{3, 7, 9, 13},                  {1, 5, 11, 15},                0x44,   0xA0A0,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},        /* xyz      */  
   {{0, 6, 10, 12},                 {2, 4, 8, 14},                 0x11,   0x0505,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},        /* pqr      */  
   {{15, 3, 5, 9},                  {13, 1, 7, 11},                0x44,   0x0A0A,         4, 0,  0,  0, 0,  s2x2,  s4x4},      /* These do C1-phantom-like stuff from fudgy 4x4 */
   {{10, 3, 7, 6},                  {15, 14, 2, 11},               0x11,   0x8484,         4, 0,  0,  0, 0,  s2x2,  s4x4},      /* They must follow the pair just above. */
   {{6, 0, 3, 5},                   {7, 1, 2, 4},                  0x55,     0377,         4, 0,  0,  0, 0,  sdmd,  s_rigger},
   {{6, 5, 3, 4},                   {7, 0, 2, 1},                  0x11,     0314,         4, 0,  0,  0, 0,  s1x4,  s_rigger},       /* must be after "2x4_4" */
   {{5, 6, 4, 3},                   {0, 7, 1, 2},                  0x44,     0314,         4, 0,  0,  0, 0,  s1x4,  s_bone},
   {{0, 3, 5, 6},                   {1, 2, 4, 7},                  0x11,     0063,         4, 0,  0,  0, 0,  sdmd,  s_crosswave},     /* crosswave*/  
   {{7, 1, 4, 6},                   {0, 2, 3, 5},                  0x44,     0146,         4, 0,  0,  0, 0,  sdmd,  s2x4},         /* must be after "2x4_4" */
   /* Next one is for so-and-so in tandem in a bone6, making a virtual line of 4. */
   {{4, 5, 3, 2},                   {0, -1, 1, -1},                   0,     0000,         4, 0,  0,  0, 0,  s1x4,  s_bone6},
   /* Next one is for so-and-so in tandem in a short6, making a virtual line of 4. */
   {{1, 0, 4, 5},                   {-1, 2, -1, 3},                   0,     0055,         4, 1,  0,  0, 0,  s1x4,  s_short6},
   /* Next three are for so-and-so as couples in a line of 8, making a virtual line of 6. */
   {{0, 1, 3, 4, 5, 6},             {-1, -1, 2, -1, -1, 7},       0x410,     0314,         6, 0,  0,  0, 0,  s_1x6, s1x8},
   {{0, 1, 2, 4, 7, 6},             {-1, 3, -1, -1, 5, -1},       0x104,     0252,         6, 0,  0,  0, 0,  s_1x6, s1x8},
   {{0, 3, 2, 5, 7, 6},             {1, -1, -1, 4, -1, -1},       0x041,     0063,         6, 0,  0,  0, 0,  s_1x6, s1x8},
   /* Next two are for so-and-so as couples in a line of 6, making a virtual line of 4. */
   {{0, 1, 3, 5},                   {-1, 2, -1, 4},                0x44,     0066,         4, 0,  0,  0, 0,  s1x4,  s_1x6},
   {{0, 2, 4, 5},                   {1, -1, 3, -1},                0x11,     0033,         4, 0,  0,  0, 0,  s1x4,  s_1x6},
   /* Next 4 are for so-and-so in tandem from a column of 6, making a virtual column of 4.  The first two are the real maps,
      and the other two take care of the reorientation that sometimes happens when coming out of a 2x2. */
   {{0, 1, 3, 5},                   {-1, 2, -1, 4},                0x44,     0066,         4, 0,  0,  0, 0,  s2x2,  s_2x3},
   {{0, 2, 4, 5},                   {1, -1, 3, -1},                0x11,     0033,         4, 0,  0,  0, 0,  s2x2,  s_2x3},
   {{1, 3, 5, 0},                   {2, -1, 4, -1},                   0,     0066,         4, 1,  0,  0, 0,  s2x2,  s_2x3},
   {{2, 4, 5, 0},                   {-1, 3, -1, 1},                   0,     0033,         4, 1,  0,  0, 0,  s2x2,  s_2x3},
   {{0, 2, 4, 7, 9, 11},            {1, 3, 5, 6, 8, 10},          0x555,   0x0FFF,         6, 0,  0,  0, 0,  s_2x3, s2x6},

   {{0},                            {0},                              0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};


extern void initialize_tandem_tables(void)
{
   tm_thing *map_search;

   for (map_search = maps_isearch; map_search->outsetup != nothing; map_search++) {
      int i;
      unsigned int m;
      unsigned int imask = 0;
      unsigned int omask = 0;
      unsigned int osidemask = 0;

      /* All 1's for people in outer setup. */
      unsigned int alloutmask = (1 << (setup_limits[map_search->outsetup]+1))-1;

      for (i=0, m=1; i<map_search->limit; i++, m<<=1) {
         alloutmask &= ~(1 << map_search->map1[i]);
         if (map_search->map2[i] < 0) {
            imask |= m;
            omask |= 1 << map_search->map1[i];
         }
         else {
            alloutmask &= ~(1 << map_search->map2[i]);
            if (((map_search->sidewaysmask >> (i*2)) ^ map_search->rot) & 1) {
               osidemask |= 1 << map_search->map1[i];
               osidemask |= 1 << map_search->map2[i];
            }
         }
      }

      map_search->insinglemask = imask;
      map_search->outsinglemask = omask;
      map_search->outunusedmask = alloutmask;

      /* We can't encode the virtual person number in the required 3-bit field if this is > 8. */
      if (map_search->limit != setup_limits[map_search->insetup]+1) fail("Tandem table initialization failed: limit wrong.");
      if (map_search->limit > 8) fail("Tandem table initialization failed: limit too big.");
      if (map_search->outsidemask != osidemask) fail("Tandem table initialization failed: Smask.");
   }
}


static void unpack_us(
   tm_thing *map_ptr,
   unsigned int orbitmask,
   tandrec *tandstuff,
   setup *result)
{
   int i;
   unsigned int m, o;

   result->kind = map_ptr->outsetup;
   result->rotation = tandstuff->virtual_result.rotation - map_ptr->rot;
   result->setupflags = tandstuff->virtual_result.setupflags;

   for (i=0, m=map_ptr->insinglemask, o=orbitmask; i<map_ptr->limit; i++, m>>=1, o>>=2) {
      int z = tandstuff->virtual_result.people[i].id1;

      if (map_ptr->rot) z = rotcw(z);

      if (m & 1) {
         /* Unpack single person. */
         if (z == 0) clear_person(result, map_ptr->map1[i]);
         else {
            personrec f;
            int ii = (z & 0700) >> 6;
            f = tandstuff->real_front_people[ii];
            if (f.id1) f.id1 = (f.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            result->people[map_ptr->map1[i]] = f;
         }
      }
      else {
         /* Unpack tandem/couples person. */
         personrec f, b;

         if (z == 0) {
            clear_person(result, map_ptr->map1[i]);
            clear_person(result, map_ptr->map2[i]);
         }
         else {
            int ii = (z >> 6) & 7;
            f = tandstuff->real_front_people[ii];
            b = tandstuff->real_back_people[ii];
            if (f.id1) f.id1 = (f.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            if (b.id1) b.id1 = (b.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));

            if ((o + map_ptr->rot + 1) & 2) {
               result->people[map_ptr->map1[i]] = b;
               result->people[map_ptr->map2[i]] = f;
            }
            else {
               result->people[map_ptr->map1[i]] = f;
               result->people[map_ptr->map2[i]] = b;
            }
         }
      }
   }

   canonicalize_rotation(result);
}




/* Lat = 0 means the people we collapsed, relative to the incoming geometry, were one
   behind the other.  Lat = 1 means they were lateral.  "Incoming geometry" does
   not include the incoming rotation field, since we ignore it.  We are not responsible
   for the rotation field of the incoming setup.

  The canonical storage of the real people, while we are doing the virtual call, is as follows:

   Real_front_people gets person on left (lat=1) near person (lat=0).
   Real_back_people gets person on right (lat=1) or far person (lat=0). */

static void pack_us(
   personrec *s,
   tm_thing *map_ptr,
   int fraction,
   long_boolean twosome,
   tandrec *tandstuff)
{
   int i;
   int lat;
   unsigned int m, sgl;

   tandstuff->virtual_setup.rotation = map_ptr->rot;
   tandstuff->virtual_setup.kind = map_ptr->insetup;

   for (i=0, m=map_ptr->sidewaysmask, sgl=map_ptr->insinglemask; i<map_ptr->limit; i++, m>>=2, sgl>>=1) {
      personrec f, b;
      personrec *ptr = &tandstuff->virtual_setup.people[i];

      f = s[map_ptr->map1[i]];

      lat = (m ^ map_ptr->rot) & 1;

      if (sgl&1) {
         ptr->id1 = (f.id1 & ~0700) | (i << 6) | BIT_VIRTUAL;
         ptr->id2 = f.id2;
         b.id1 = 0xFFFFFFFF;
      }
      else {
         b = s[map_ptr->map2[i]];

         if (!(tandstuff->virtual_setup.setupflags & SETUPFLAG__PHANTOMS) && ((b.id1 ^ f.id1) & BIT_PERSON))
            fail("Use phantom tandem/phantom couples concept instead.");
         
         if (b.id1 | f.id1) {
            unsigned int vp1;
         
            if (twosome >= 2) {
               if ((b.id1 | f.id1) & STABLE_MASK)
                  fail("Sorry, can't nest fractional stable/twosome.");
            }

            /* Create the virtual person.  When both people are present, anding
               the real peoples' id2 bits gets the right bits.  For example,
               the virtual person will be a boy and able to do a tandem star thru
               if both real people were boys.  Remove the identity field (700 bits)
               from id1 and replace with a virtual person indicator.  Check that
               direction, roll, and stability parts of id1 are consistent. */
         
            if (b.id1 & f.id1 & BIT_PERSON) {
               /* If both people are real, check that they face the same way. */
               if ((b.id1 ^ f.id1) & 077)
                  fail("People not facing same way for tandem or as couples.");
               vp1 = (b.id1 & f.id1 & ~0700) | (i << 6) | BIT_VIRTUAL;
               /* If they have different fractional stability states, just clear them -- they can't do it. */
               if ((b.id1 ^ f.id1) & STABLE_MASK) vp1 &= ~STABLE_MASK;
               /* If they have different roll states, just clear them -- they can't roll. */
               if ((b.id1 ^ f.id1) & ROLL_MASK) vp1 &= ~ROLL_MASK;
               ptr->id1 = vp1;
               ptr->id2 = b.id2 & f.id2;
            }
            else {
               ptr->id1 = ((b.id1 | f.id1) & ~0700) | (i << 6) | BIT_VIRTUAL;
               ptr->id2 = b.id2 | f.id2;
            }

            if (twosome >= 2)
               ptr->id1 |= STABLE_ENAB | (STABLE_RBIT * fraction);
         }
         else {
            ptr->id1 = 0;
            ptr->id2 = 0;
         }
      
         tandstuff->vertical_people[i] = lat ^ 1;   /* 1 if original people were near/far; 0 if lateral */
         tandstuff->twosomep[i] = twosome;
      }

      if (map_ptr->rot)   /* Compensate for above rotation. */
         (void) copy_rot(&tandstuff->virtual_setup, i, &tandstuff->virtual_setup, i, 033);

      tandstuff->real_front_people[i] = f;
      tandstuff->real_back_people[i] = b;
   }
}




extern void tandem_couples_move(
   setup *ss,
   parse_block *parseptr,
   callspec_block *callspec,
   final_set final_concepts,
   selector_kind selector,
   int twosome,               /* solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3 */
   int fraction,              /* number, if doing fractional twosome/solid */
   int phantom,               /* normal=0 / phantom=1 / gruesome=2 */
   int tnd_cpl_siam,          /* tandem=0 / couples=1 / siamese=2 / skew=3 */
   setup *result)
{
   selector_kind saved_selector;
   parse_block *conceptptrcopy;
   tandrec tandstuff;
   tm_thing *map;
   tm_thing *map_search;
   unsigned int nsmask, ewmask, allmask;
   int i;
   unsigned int j, jbit;
   unsigned int hmask;
   unsigned int orbitmask;
   unsigned int sglmask;
   unsigned int livemask;

   long_boolean fractional = FALSE;

   conceptptrcopy = parseptr;
   tandstuff.single_mask = 0;
   nsmask = 0;
   ewmask = 0;
   allmask = 0;

   if (setup_limits[ss->kind] < 0) fail("Can't do tandem/couples concept from this position.");

   /* We use the phantom indicator to forbid an already-distorted setup.
      The act of forgiving phantom pairing is based on the setting of the
      SETUPFLAG__PHANTOMS bit in the incoming setup, not on the phantom indicator. */

   if ((ss->setupflags & SETUPFLAG__DISTORTED) && (phantom != 0))
      fail("Can't specify phantom tandem/couples in virtual or distorted setup.");

   /* Find out who is selected, if this is a "so-and-so are tandem". */
   saved_selector = current_selector;
   if (selector != selector_uninitialized)
      current_selector = selector;

   for (i=0, jbit=1; i<=setup_limits[ss->kind]; i++, jbit<<=1) {
      unsigned int p = ss->people[i].id1;
      if (p) {
         allmask |= jbit;
         if ((selector != selector_uninitialized) && !selectp(ss, i))
            tandstuff.single_mask |= jbit;
         else {
            if (p & 1)
               ewmask |= jbit;
            else
               nsmask |= jbit;
         }
      }
   }
   
   current_selector = saved_selector;

   if (!allmask) {
      result->kind = nothing;
      return;
   }

   if (twosome >= 2) fractional = TRUE;

   if (fractional && fraction > 4)
      fail("Can't do fractional twosome more than 4/4.");


   if (tnd_cpl_siam > 2)
      fail("Sorry, can't do skew/skewsome.");
   else if (tnd_cpl_siam > 1) {
      /* Siamese. */
      switch (ss->kind) {
         case s_c1phan:
            switch ((ewmask << 16) | nsmask) {
               case 0x0000AAAA:
                  j = 0xA0A0; goto foox;
               case 0x00005555:
                  j = 0x0505; goto foox;
               case 0xAAAA0000:
                  j = 0x0A0A; goto foox;
               case 0x55550000:
                  j = 0x5050; goto foox;
            }
            break;
         case s4x4:
            switch ((ewmask << 16) | nsmask) {
               case 0x0000AAAA:
                  j = 0x0A0A; goto foox;
               case 0x0000CCCC:
                  j = 0x8484; goto foox;
               case 0xAAAA0000:
                  j = 0xA0A0; goto foox;
               case 0xCCCC0000:
                  j = 0x4848; goto foox;
            }
            break;
         case s_qtag:
            switch ((ewmask << 8) | nsmask) {
               case 0x33CC:
                  warn(warn__ctrscpls_endstand);
                  j = 0xCC; goto foox;
               case 0xCC33:
                  warn(warn__ctrstand_endscpls);
                  j = 0x33; goto foox;
            }
            break;
         case s_rigger:
            switch ((ewmask << 8) | nsmask) {
               case 0xFF00: case 0xCC33:
                  warn(warn__ctrscpls_endstand);
                  j = 0x33; goto foox;
               case 0x00FF: case 0x33CC:
                  warn(warn__ctrstand_endscpls);
                  j = 0xCC; goto foox;
            }
            break;
         case s_bone:
            switch ((ewmask << 8) | nsmask) {
               case 0xFF00:
                  warn(warn__ctrstand_endscpls);
                  j = 0x33; goto foox;
               case 0x00FF:
                  warn(warn__ctrscpls_endstand);
                  j = 0xCC; goto foox;
            }
            break;
         case s_crosswave:
            switch ((ewmask << 8) | nsmask) {
               case 0xFF00:
                  warn(warn__ctrscpls_endstand);
                  j = 0xCC; goto foox;
               case 0x00FF:
                  warn(warn__ctrstand_endscpls);
                  j = 0x33; goto foox;
            }
            break;
         case s2x4:
            switch ((ewmask << 8) | nsmask) {
               case 0xFF00: case 0x9966:
                  warn(warn__ctrstand_endscpls);
                  j = 0x99; goto foox;
               case 0x00FF: case 6699:
                  warn(warn__ctrscpls_endstand);
                  j = 0x66; goto foox;
               case 0x33CC:
                  j = 0xCC; goto foox;
               case 0xCC33:
                  j = 0x33; goto foox;
            }
            break;
      }

      fail("Can't do Siamese in this setup.");

      foox:
      ewmask ^= j;
      nsmask ^= j;
   }
   else if (tnd_cpl_siam & 1) {
      /* Couples -- swap masks.  Tandem -- do nothing. */
      j = ewmask;
      ewmask = nsmask;
      nsmask = j;
   }

   /* Now ewmask and nsmask have the info about who is paired with whom. */
   ewmask &= ~tandstuff.single_mask;         /* Clear out unpaired people. */
   nsmask &= ~tandstuff.single_mask;

   map_search = maps_isearch;
   while (map_search->outsetup != nothing) {
      if ((map_search->outsetup == ss->kind) &&
            (map_search->outsinglemask == tandstuff.single_mask) &&
            (!(allmask & map_search->outunusedmask)) &&
            (!(ewmask & (~map_search->outsidemask))) &&
            (!(nsmask & map_search->outsidemask))) {
         map = map_search;
         goto fooy;
      }
      map_search++;
   }
   fail("Can't do this tandem or couples call in this setup or with these people selected.");

   fooy:

   /* We also use the subtle aspects of the phantom indicator to tell what kind
      of setup we allow, and whether pairings must be parallel to the long axis. */

   if (phantom == 1) {
      if (ss->kind != s2x8 && ss->kind != s4x4 && ss->kind != s3x4 && ss->kind != s2x6)
         fail("Must have a 4x4, 2x8, 3x4, or 2x6 setup to do this concept.");
   }
   else if (phantom == 2) {
      if (ss->kind != s2x8 || map->insetup != s2x4)
         fail("Can't do gruesome concept in this setup.");
   }

   tandstuff.virtual_setup.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   pack_us(ss->people, map, fraction, twosome, &tandstuff);
   update_id_bits(&tandstuff.virtual_setup);
   move(&tandstuff.virtual_setup, conceptptrcopy, callspec, final_concepts, FALSE, &tandstuff.virtual_result);

   if (setup_limits[tandstuff.virtual_result.kind] < 0)
      fail("Don't recognize ending position from this tandem or as couples call.");

   sglmask = 0;
   livemask = 0;    /* Only alternate bits used! */
   orbitmask = 0;   /* Bits appear here in pairs! */

   /* Compute orbitmask, livemask, and sglmask. */

   for (i=0; i<=setup_limits[tandstuff.virtual_result.kind]; i++) {
      int p = tandstuff.virtual_result.people[i].id1;
      if (p) {
         int vpi;

         if (fractional) {
            if (!(p & STABLE_ENAB))
               fail("fractional twosome not supported for this call.");
         }

         vpi = (p >> 6) & 7;
         livemask |= 1 << (2*i);
         if (tandstuff.real_back_people[vpi].id1 == 0xFFFFFFFF) {
            sglmask |= 1 << i;
         }
         else {
            unsigned int orbit;

            orbit = p - tandstuff.virtual_setup.rotation +
                  tandstuff.virtual_result.rotation - tandstuff.virtual_setup.people[vpi].id1;

            if (twosome == 2) {
               orbit -= ((p & (STABLE_VBIT*3)) / STABLE_VBIT);
            }
            else if (twosome == 3) {
               orbit = ((p & (STABLE_VBIT*3)) / STABLE_VBIT);
            }
            else if (twosome == 1) {
               orbit = 0;
            }

            orbitmask |= ((orbit - tandstuff.virtual_result.rotation - tandstuff.vertical_people[vpi]) & 3) << (2*i);
         }

         if (fractional)
            tandstuff.virtual_result.people[i].id1 &= ~STABLE_MASK;
      }
   }

   hmask = (~orbitmask) & livemask & 0x55555555;    /* Pick out only low bits for map search. */

   map_search = maps_isearch;
   while (map_search->outsetup != nothing) {
      if ((map_search->insetup == tandstuff.virtual_result.kind) &&
            (map_search->insinglemask == sglmask) &&
            ((map_search->sidewaysmask & livemask) == hmask)) {
         unpack_us(map_search, orbitmask, &tandstuff, result);
         reinstate_rotation(ss, result);
         return;
      }
      map_search++;
   }

   fail("Don't recognize ending position from this tandem or as couples call.");
}
