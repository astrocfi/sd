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

    This is for version 30. */

/* This defines the following functions:
   canonicalize_rotation
   reinstate_rotation
   move
*/

#include "sd.h"


extern void canonicalize_rotation(setup *result) {

   if (result->kind == s_1x1) {
      (void) copy_rot(result, 0, result, 0, (result->rotation & 3) * 011);
      result->rotation = 0;
   }
   else if ((result->kind == s4x4) ||
            (result->kind == s2x2) ||
            (result->kind == s_star) ||
            (result->kind == s_thar) ||
            (result->kind == s_bigblob) ||
            (result->kind == s_c1phan) ||
            (result->kind == s_hyperglass) ||
            (result->kind == s_galaxy)) {
      /* The setup has 4-way symmetry.  We can canonicalize it so the
         result rotation is zero. */
      int i, rot, rot11, delta, bigd, i0, i1, i2, i3, j0, j1, j2, j3;
      personrec x0, x1, x2, x3;

      rot = result->rotation & 3;
      if (rot == 0) return;
      rot11 = rot * 011;
      bigd = setup_limits[result->kind] + 1;
      delta = bigd >> 2;

      i0 = 1;
      i1 = i0 + delta;
      i2 = i1 + delta;
      i3 = i2 + delta;
      j0 = (rot-4)*delta+1;
      j1 = j0 + delta;
      j2 = j1 + delta;
      j3 = j2 + delta;
      for (i=0; i<delta; i++) {
         if ((--i0) < 0) i0 += bigd;
         if ((--i1) < 0) i1 += bigd;
         if ((--i2) < 0) i2 += bigd;
         if ((--i3) < 0) i3 += bigd;
         if ((--j0) < 0) j0 += bigd;
         if ((--j1) < 0) j1 += bigd;
         if ((--j2) < 0) j2 += bigd;
         if ((--j3) < 0) j3 += bigd;
         x0 = result->people[i0];
         x1 = result->people[i1];
         x2 = result->people[i2];
         x3 = result->people[i3];
         result->people[j0].id1 = rotperson(x0.id1, rot11);
         result->people[j0].id2 = x0.id2;
         result->people[j1].id1 = rotperson(x1.id1, rot11);
         result->people[j1].id2 = x1.id2;
         result->people[j2].id1 = rotperson(x2.id1, rot11);
         result->people[j2].id2 = x2.id2;
         result->people[j3].id1 = rotperson(x3.id1, rot11);
         result->people[j3].id2 = x3.id2;
      }

      result->rotation = 0;
   }
   else if (result->kind == s_1x3) {
      if (result->rotation & 2) {

         /* Must turn this setup upside-down. */
   
         swap_people(result, 0, 2);
         (void) copy_rot(result, 0, result, 0, 022);
         (void) copy_rot(result, 1, result, 1, 022);
         (void) copy_rot(result, 2, result, 2, 022);
      }
      result->rotation &= 1;
   }
   else if (((setup_limits[result->kind] & ~07776) == 1)) {
      /* We have a setup of an even number of people.  We know how to canonicalize
         this.  The resulting rotation should be 0 or 1. */

      if (result->rotation & 2) {

         /* Must turn this setup upside-down. */
   
         int i, offs;
   
         offs = (setup_limits[result->kind]+1) >> 1;     /* Half the setup size. */
   
         for (i=0; i<offs; i++) {
            swap_people(result, i, i+offs);
            (void) copy_rot(result, i, result, i, 022);
            (void) copy_rot(result, i+offs, result, i+offs, 022);
         }
      }
      result->rotation &= 1;
   }
   else
      result->rotation &= 3;
}


extern void reinstate_rotation(setup *ss, setup *result) {

   int globalrotation;

   switch (ss->kind) {
      case s_normal_concentric:
         globalrotation = 0;
         break;
      default:
         globalrotation = ss->rotation;
   }
   
   switch (result->kind) {
      case s_normal_concentric:
         result->inner.srotation += globalrotation;
         result->outer.srotation += globalrotation;
         break;
      case nothing:
         break;
      default:
         result->rotation += globalrotation;
         break;
   }

   canonicalize_rotation(result);
}



typedef struct gloop {
    int x;                  /* This person's coordinates, calibrated so that a matrix */
    int y;                  /*   position cooresponds to an increase by 4. */
    long_boolean sel;       /* True if this person is selected.  (False if selectors not in use.) */
    long_boolean done;      /* Used for loop control on each pass */
    long_boolean realdone;  /* Used for loop control on each pass */
    int boybit;             /* 1 if boy, 0 if not (might be neither). */
    int girlbit;            /* 1 if girl, 0 if not (might be neither). */ 
    int dir;                /* This person's initial facing direction, 0 to 3. */
    int deltax;             /* How this person will move, relative to his own facing */
    int deltay;             /*   direction, when call is finally executed. */
    int deltarot;           /* How this person will turn. */
    int rollinfo;           /* How this person's roll info will be set. */
    struct gloop *nextse;   /* Points to next person south (dir even) or east (dir odd.) */
    struct gloop *nextnw;   /* Points to next person north (dir even) or west (dir odd.) */
    long_boolean tbstopse;  /* True if nextse/nextnw is zero because the next spot */
    long_boolean tbstopnw;  /*   is occupied by a T-boned person (as opposed to being empty.) */
    } matrix_rec;




/* This function is internal. */

Private void start_matrix_call(
   setup *ss,
   int *nump,
   matrix_rec matrix_info[],
   long_boolean use_selector,
   setup *people)

{
   int i;
   coordrec *thingyptr;

   clear_people(people);
   
   thingyptr = setup_coords[ss->kind];
   if (!thingyptr) fail("Can't do this in this setup.");
   
   if (setup_limits[ss->kind] < 0) fail("Can't do this in this setup.");        /* this is actually superfluous */
   
   *nump = 0;
   for (i=0; i<=setup_limits[ss->kind]; i++) {
      if (ss->people[i].id1) {
         if (*nump == 8) fail("?????too many people????");
         (void) copy_person(people, *nump, ss, i);
         matrix_info[*nump].x = thingyptr->xca[i];
         matrix_info[*nump].y = thingyptr->yca[i];

         matrix_info[*nump].done = FALSE;
         matrix_info[*nump].realdone = FALSE;

         if (use_selector)
            matrix_info[*nump].sel = selectp(people, *nump);
         else
            matrix_info[*nump].sel = FALSE;

         matrix_info[*nump].dir = people->people[*nump].id1 & 3;
   
         matrix_info[*nump].girlbit = (people->people[*nump].id2 & ID2_GIRL) ? 1 : 0;
         matrix_info[*nump].boybit = (people->people[*nump].id2 & ID2_BOY) ? 1 : 0;
         matrix_info[*nump].nextse = 0;
         matrix_info[*nump].nextnw = 0;
         matrix_info[*nump].deltax = 0;
         matrix_info[*nump].deltay = 0;
         matrix_info[*nump].deltarot = 0;
         matrix_info[*nump].rollinfo = ROLLBITM;
         matrix_info[*nump].tbstopse = FALSE;
         matrix_info[*nump].tbstopnw = FALSE;

         (*nump)++;
      }
   }
}


/* This function is internal. */

Private void finish_matrix_call(
   matrix_rec matrix_info[],
   int nump,
   setup *people,
   setup *result)

{
   int i, place;
   int xmax, xpar, ymax, ypar, x, y, k;
   unsigned int signature;
   coordrec *checkptr;

   xmax = xpar = ymax = ypar = signature = 0;

   for (i=0; i<nump; i++) {
      people->people[i].id1 = (rotperson(people->people[i].id1, matrix_info[i].deltarot*011) & (~ROLL_MASK)) | matrix_info[i].rollinfo;

      /* If this person's position has low bit on, that means we consider his coordinates
         not sufficiently well-defined that we will allow him to do any pressing or
         trucking.  He is only allowed to turn.  That is, we will require deltax and
         deltay to be zero.  An example of this situation is the points of a galaxy. */

      /* ****** This seems to be too restrictive.  There may have been good reason for doing this
         at one time, but now it makes all press and truck calls illegal in C1 phantoms.  The
         table for C1 phantoms has been carefully chosen to make things legal only within one's
         own miniwave, but it requires odd numbers.  Perhaps we need to double the resolution
         of things in matrix_info[i].x or y, but that should wait until after version 28 is released. */

      if (((matrix_info[i].x | matrix_info[i].y) & 1) && (matrix_info[i].deltax | matrix_info[i].deltay))
         fail("Someone's ending position is not well defined.");

      switch (matrix_info[i].dir) {
         case 0:
            matrix_info[i].x += matrix_info[i].deltax;
            matrix_info[i].y += matrix_info[i].deltay;
            break;
         case 1:
            matrix_info[i].x += matrix_info[i].deltay;
            matrix_info[i].y -= matrix_info[i].deltax;
            break;
         case 2:
            matrix_info[i].x -= matrix_info[i].deltax;
            matrix_info[i].y -= matrix_info[i].deltay;
            break;
         case 3:
            matrix_info[i].x -= matrix_info[i].deltay;
            matrix_info[i].y += matrix_info[i].deltax;
            break;
      }

      x = matrix_info[i].x;
      y = matrix_info[i].y;

      /* Compute new max, parity, and signature info. */

      if ((x < 0) || ((x == 0) && (y < 0))) { x = -x; y = -y; }
      signature |= 1 << ((31000 + 12*x - 11*y) % 31);
      if (y < 0) y = -y;
      /* Now x and y have both had absolute values taken. */
      if (x > xmax) xmax = x;
      if (y > ymax) ymax = y;
      k = x | 4;
      xpar |= (k & (~(k-1)));
      k = y | 4;
      ypar |= (k & (~(k-1)));
   }

   ypar |= (xmax << 20) | (xpar << 16) | (ymax << 4);

   result->rotation = 0;

   if ((ypar == 0x00A20026) && ((signature & (~0x08008404)) == 0)) {
      checkptr = setup_coords[s_rigger];
      goto doit;
   }
   else if ((ypar == 0x00770077) && ((signature & (~0x00418004)) == 0)) {
      checkptr = setup_coords[s_galaxy];
      goto doit;
   }
   else if ((ypar == 0x00950095) && ((signature & (~0x22008080)) == 0)) {
      checkptr = setup_coords[s_thar];
      goto doit;
   }
   else if ((ypar == 0x00950075) && ((signature & (~0x20018000)) == 0)) {
      checkptr = setup_coords[s_crosswave];
      goto doit;
   }
   else if ((ypar == 0x00A20026) && ((signature & (~0x01040420)) == 0)) {
      checkptr = setup_coords[s_bone];
      goto doit;
   }
   else if ((ypar == 0x00840026) && ((signature & (~0x04000308)) == 0)) {
      checkptr = setup_coords[s_spindle];
      goto doit;
   }
   else if ((ypar == 0x00A200A2) && ((signature & (~0x101CC4E6)) == 0)) {
      checkptr = setup_coords[s_bigblob];
      goto doit;
   }
   else if ((ypar == 0x00670055) && ((signature & (~0x01000420)) == 0)) {
      checkptr = setup_coords[s_qtag];
      goto doit;
   }
   else if ((ypar == 0x00550067) && ((signature & (~0x08410200)) == 0)) {
      checkptr = setup_coords[s_qtag];
      goto doitrot;
   }
   else if ((ypar == 0x00A60055) && ((signature & (~0x09000480)) == 0)) {
      checkptr = setup_coords[s_3x1dmd];
      goto doit;
   }
   /* Depending on how the setup is actually occupied, xmax and ymax may vary.
      ***** We need to work this out carefully.  Maybe parity should be high
      and max low so that range checking will suffice. */
   else if (((ypar == 0x00A300A3) || (ypar == 0x00B300B3)) && ((signature & (~0x38003B87)) == 0)) {
      checkptr = setup_coords[s_c1phan];
      goto doit;
   }
   /* If certain far out people are missing, xmax will be different, but we will
       still need to go to a 3dmd. */
   else if (((ypar == 0x00A70055) || (ypar == 0x00770055) || (ypar == 0x00730055)) && ((signature & (~0x29008480)) == 0)) {
      checkptr = setup_coords[s_3dmd];
      goto doit;
   }
   /* If certain far out people are missing, xmax will be different, but we will
       still need to go to a 4dmd. */
   else if (((ypar == 0x00E30055) || (ypar == 0x00B30055) || (ypar == 0x00A30055)) && ((signature & (~0x0940A422)) == 0)) {
      checkptr = setup_coords[s_4dmd];
      goto doit;
   }
   else if ((ypar == 0x00550057) && ((signature & (~0x20000620)) == 0)) {
      checkptr = setup_coords[s_hrglass];
      goto doit;
   }
   else if ((ypar == 0x00930035) && ((signature & (~0x05200100)) == 0)) {
      checkptr = setup_coords[s_ptpd];
      goto doit;
   }
   else if ((ypar == 0x00620044) && ((signature & (~0x11800C40)) == 0)) {
      checkptr = setup_coords[s3x4];
      goto doit;
   }
   else if ((ypar == 0x00440062) && ((signature & (~0x0C202300)) == 0)) {
      checkptr = setup_coords[s3x4];
      goto doitrot;
   }
   else if ((ypar == 0x00E20004) && ((signature & (~0x09002400)) == 0)) {
      checkptr = setup_coords[s1x8];
      goto doit;
   }
   else if ((ypar == 0x000400E2) && ((signature & (~0x08004202)) == 0)) {
      checkptr = setup_coords[s1x8];
      goto doitrot;
   }
   else if ((ypar == 0x01220004) && ((signature & (~0x49002400)) == 0)) {
      checkptr = setup_coords[s1x10];
      goto doit;
   }
   else if ((ypar == 0x01620004) && ((signature & (~0x49012400)) == 0)) {
      checkptr = setup_coords[s1x12];
      goto doit;
   }
   else if ((ypar == 0x01A20004) && ((signature & (~0x49012404)) == 0)) {
      checkptr = setup_coords[s1x14];
      goto doit;
   }
   else if ((ypar == 0x01E20004) && ((signature & (~0x49092404)) == 0)) {
      checkptr = setup_coords[s1x16];
      goto doit;
   }
   else if ((ypar == 0x00620022) && ((signature & (~0x00088006)) == 0)) {
      checkptr = setup_coords[s2x4];
      goto doit;
   }
   else if ((ypar == 0x00220062) && ((signature & (~0x10108004)) == 0)) {
      checkptr = setup_coords[s2x4];
      goto doitrot;
   }
   else if ((ypar == 0x00A20022) && ((signature & (~0x000C8026)) == 0)) {
      checkptr = setup_coords[s2x6];
      goto doit;
   }
   else if ((ypar == 0x002200A2) && ((signature & (~0x10108484)) == 0)) {
      checkptr = setup_coords[s2x6];
      goto doitrot;
   }
   else if ((ypar == 0x00E20022) && ((signature & (~0x004C8036)) == 0)) {
      checkptr = setup_coords[s2x8];
      goto doit;
   }
   else if ((ypar == 0x002200E2) && ((signature & (~0x12908484)) == 0)) {
      checkptr = setup_coords[s2x8];
      goto doitrot;
   }
   else if ((ypar == 0x00A20062) && ((signature & (~0x109CC067)) == 0)) {
      checkptr = setup_coords[s4x6];
      goto doit;
   }
   else if ((ypar == 0x006200A2) && ((signature & (~0x1918C4C6)) == 0)) {
      checkptr = setup_coords[s4x6];
      goto doitrot;
   }
   else if ((ypar == 0x00620062) && ((signature & (~0x1018C046)) == 0)) {
      checkptr = setup_coords[s4x4];
      goto doit;
   }
   /* **** These last ones are sort of a crock.  They are designed to make
      matrix calls work in distorted or virtual setups in some circumstances
      (i.e. if no one changes coordinates.)  However, they won't work in the
      presence of unsymmetrical phantoms.  What we really should do is, if
      the setup is virtual/distorted (or maybe the test should be if no one
      moved) just force people back to the same setup they started in. */
   else if ((ypar == 0x00220022) && ((signature & (~0x00008004)) == 0)) {
      checkptr = setup_coords[s2x2];
      goto doit;
   }
   else if ((ypar == 0x00620004) && ((signature & (~0x01000400)) == 0)) {
      checkptr = setup_coords[s1x4];
      goto doit;
   }
   else if ((ypar == 0x00220004) && ((signature & (~0x01000000)) == 0)) {
      checkptr = setup_coords[s_1x2];
      goto doit;
   }

   fail("Can't handle this result matrix.");

   

doit:
      result->kind = checkptr->result_kind;
      for (i=0; i<nump; i++) {
         place = checkptr->diagram[28 - ((matrix_info[i].y >> 2) << checkptr->xfactor) + (matrix_info[i].x >> 2)];
         if (place < 0) fail("Person has moved into a grossly ill-defined location.");
         if ((checkptr->xca[place] != matrix_info[i].x) || (checkptr->yca[place] != matrix_info[i].y))
            fail("Person has moved into a slightly ill-defined location.");
         install_person(result, place, people, i);
         result->people[place].id1 &= ~STABLE_MASK;   /* For now, can't do fractional stable on this kind of call. */
      }
      
      return;

doitrot:
      result->kind = checkptr->result_kind;
      result->rotation = 1;   
      for (i=0; i<nump; i++) {
         place = checkptr->diagram[27 - ((matrix_info[i].x >> 2) << checkptr->xfactor) - (matrix_info[i].y >> 2)];
         if (place < 0) fail("Person has moved into a grossly ill-defined location.");
         if ((checkptr->xca[place] != -matrix_info[i].y) || (checkptr->yca[place] != matrix_info[i].x))
            fail("Person has moved into a slightly ill-defined location.");
         install_rot(result, place, people, i, 033);
         result->people[place].id1 &= ~STABLE_MASK;   /* For now, can't do fractional stable on this kind of call. */
      }
      
      return;
}



/* This function is internal. */

Private void matrixmove(
   setup *ss,
   callspec_block *callspec,
   setup *result)

{
   int datum;
   setup people;
   matrix_rec matrix_info[9];
   int i, nump, alldelta;

   alldelta = 0;

   start_matrix_call(ss, &nump, matrix_info, TRUE, &people);

   for (i=0; i<nump; i++) {
      if (matrix_info[i].sel) {
         /* This is legal if girlbit or boybit is on (in which case we use the appropriate datum)
            or if the two data are identical so the sex doesn't matter. */
         if ((matrix_info[i].girlbit | matrix_info[i].boybit) == 0 &&
                  (callspec->stuff.matrix.stuff[0] != callspec->stuff.matrix.stuff[1]))
            fail("Can't determine sex of this person.");

         datum = callspec->stuff.matrix.stuff[matrix_info[i].girlbit];
         alldelta |= (  matrix_info[i].deltax = ( ((datum >> 7) & 0x1F) - 16) << 1  );
         alldelta |= (  matrix_info[i].deltay = ( ((datum >> 2) & 0x1F) - 16) << 1  );
         matrix_info[i].deltarot = datum & 03;
         matrix_info[i].rollinfo = (datum >> 12) * ROLLBITR;
      }
   }

   if ((alldelta != 0) && (ss->setupflags & SETUPFLAG__DISTORTED))
      fail("This call not allowed in distorted or virtual setup.");
   
   finish_matrix_call(matrix_info, nump, &people, result);
}


/* This function is internal. */

Private void do_pair(
   matrix_rec *ppp,        /* Selected person */
   matrix_rec *qqq,        /* Unselected person */
   callspec_block *callspec,
   int flip,
   int filter)             /* 1 to do N/S facers, 0 for E/W facers. */
{
   int base;
   int datum;
   int flags;

   flags = callspec->stuff.matrix.flags;

   if ((filter ^ ppp->dir) & 1) {
      base = (ppp->dir & 2) ? 6 : 4;
      if (!(flags & MTX_USE_SELECTOR)) base &= 3;
      base ^= flip;

      /* This is legal if girlbit or boybit is on (in which case we use the appropriate datum)
         or if the two data are identical so the sex doesn't matter. */
      if ((ppp->girlbit | ppp->boybit) == 0 &&
               (callspec->stuff.matrix.stuff[base] != callspec->stuff.matrix.stuff[base+1]))
         fail("Can't determine sex of this person.");

      datum = callspec->stuff.matrix.stuff[base+ppp->girlbit];
      if (datum == 0) fail("Can't do this call.");
   
      ppp->deltax = (((datum >> 7) & 0x1F) - 16) << 1;
      ppp->deltay = (((datum >> 2) & 0x1F) - 16) << 1;
      ppp->deltarot = datum & 3;
      ppp->rollinfo = (datum >> 12) * ROLLBITR;
      ppp->realdone = TRUE;
   }
   ppp->done = TRUE;

   if ((filter ^ qqq->dir) & 1) {
      base = (qqq->dir & 2) ? 0 : 2;
      if (flags & MTX_IGNORE_NONSELECTEES) base |= 4;
      base ^= flip;

      /* This is legal if girlbit or boybit is on (in which case we use the appropriate datum)
         or if the two data are identical so the sex doesn't matter. */
      if ((qqq->girlbit | qqq->boybit) == 0 &&
               (callspec->stuff.matrix.stuff[base] != callspec->stuff.matrix.stuff[base+1]))
         fail("Can't determine sex of this person.");

      datum = callspec->stuff.matrix.stuff[base+qqq->girlbit];
      if (datum == 0) fail("Can't do this call.");

      qqq->deltax = (((datum >> 7) & 0x1F) - 16) << 1;
      qqq->deltay = (((datum >> 2) & 0x1F) - 16) << 1;
      qqq->deltarot = datum & 3;
      qqq->rollinfo = (datum >> 12) * ROLLBITR;
      qqq->realdone = TRUE;
   }
   qqq->done = TRUE;
}


/* This function is internal. */

Private void do_matrix_chains(
   matrix_rec matrix_info[],
   int nump,
   callspec_block *callspec,
   int filter)                        /* 1 for E/W chains, 0 for N/S chains. */

{
   long_boolean another_round;
   int i, j, flags;

   flags = callspec->stuff.matrix.flags;

   /* Find adjacency relationships, and fill in the "se"/"nw" pointers. */

   for (i=0; i<nump; i++) {
      if ((flags & MTX_IGNORE_NONSELECTEES) && (!matrix_info[i].sel)) continue;
      for (j=0; j<nump; j++) {
         if ((flags & MTX_IGNORE_NONSELECTEES) && (!matrix_info[j].sel)) continue;
         /* Find out if these people are adjacent in the right way. */

         if (    ( filter && matrix_info[j].x == matrix_info[i].x + 4 && matrix_info[j].y == matrix_info[i].y)
            ||   (!filter && matrix_info[j].y == matrix_info[i].y - 4 && matrix_info[j].x == matrix_info[i].x)   ) {

            /* Now, if filter = 1, person j is just east of person i.
               If filter = 0, person j is just south of person i. */

            if (flags & MTX_TBONE_IS_OK) {
               matrix_info[i].nextse = &matrix_info[j];     /* Make the chain independently of facing direction. */
               matrix_info[j].nextnw = &matrix_info[i];
            }
            else {
               if ((matrix_info[i].dir ^ filter) & 1) {
                  if ((matrix_info[i].dir ^ matrix_info[j].dir) & 1) {
                     if (!(flags & MTX_STOP_AND_WARN_ON_TBONE)) fail("People are T-boned.");
                     matrix_info[i].tbstopse = TRUE;
                     matrix_info[j].tbstopnw = TRUE;
                  }
                  else {
                     if ((flags & MTX_MUST_FACE_SAME_WAY) && (matrix_info[i].dir ^ matrix_info[j].dir))
                        fail("Paired people must face the same way.");
                     matrix_info[i].nextse = &matrix_info[j];
                     matrix_info[j].nextnw = &matrix_info[i];
                  }
               }
            }
            break;
         }
      }
   }

   /* Pick out pairs of people and move them. */

   another_round = TRUE;

   while (another_round) {
      another_round = FALSE;

      for (i=0; i<nump; i++) {
         if (!matrix_info[i].done) {

            /* This person might be ready to be paired up with someone. */

            if (matrix_info[i].nextse) {
               if (matrix_info[i].nextnw)
                  /* This person has neighbors on both sides.  Can't do anything yet. */
                  ;
               else {
                  /* This person has a neighbor on south/east side only. */

                  if (matrix_info[i].tbstopnw) warn(warn__not_tbone_person);

                  if (   (!(flags & MTX_USE_SELECTOR))  ||  matrix_info[i].sel   ) {
                     if ((!(flags & MTX_IGNORE_NONSELECTEES)) && matrix_info[i].nextse->sel) {
                        fail("Two adjacent selected people.");
                     }
                     else {

                        /* Do this pair.  First, chop the pair off from anyone else. */

                        if (matrix_info[i].nextse->nextse) matrix_info[i].nextse->nextse->nextnw = 0;
                        matrix_info[i].nextse->nextse = 0;
                        another_round = TRUE;
                        do_pair(&matrix_info[i], matrix_info[i].nextse, callspec, 0, filter);
                     }
                  }
               }
            }
            else {
               if (matrix_info[i].nextnw) {
                  /* This person has a neighbor on north/west side only. */

                  if (matrix_info[i].tbstopse) warn(warn__not_tbone_person);

                  if (   (!(flags & MTX_USE_SELECTOR))  ||  matrix_info[i].sel   ) {
                     if ((!(flags & MTX_IGNORE_NONSELECTEES)) && matrix_info[i].nextnw->sel) {
                        fail("Two adjacent selected people.");
                     }
                     else {
                        /* Do this pair.  First, chop the pair off from anyone else. */

                        if (matrix_info[i].nextnw->nextnw) matrix_info[i].nextnw->nextnw->nextse = 0;
                        matrix_info[i].nextnw->nextnw = 0;
                        another_round = TRUE;
                        do_pair(&matrix_info[i], matrix_info[i].nextnw, callspec, 2, filter);
                     }
                  }
               }
               else {
                  /* Person is alone.  If this is his lateral axis, mark him done and don't move him. */

                  if ((matrix_info[i].dir ^ filter) & 1) {
                     if (matrix_info[i].tbstopse || matrix_info[i].tbstopnw) warn(warn__not_tbone_person);
                     if (   (!(flags & MTX_USE_SELECTOR))  ||  matrix_info[i].sel   )
                        fail("Person has no one to work with.");
                     matrix_info[i].done = TRUE;
                  }
               }
            }
         }
      }
   }
}






/* This function is internal. */

Private void partner_matrixmove(
   setup *ss,
   callspec_block *callspec,
   setup *result)

{
   int flags;
   setup people;
   matrix_rec matrix_info[9];
   int i, nump;

   if (ss->setupflags & SETUPFLAG__DISTORTED)
      fail("This call not allowed in distorted or virtual setup.");

   flags = callspec->stuff.matrix.flags;

   start_matrix_call(ss, &nump, matrix_info, flags & MTX_USE_SELECTOR, &people);

   /* Make the lateral chains first. */

   do_matrix_chains(matrix_info, nump, callspec, 1);

   /* Now clean off the pointers in preparation for the second pass. */

   for (i=0; i<nump; i++) {
      matrix_info[i].done = FALSE;
      matrix_info[i].nextse = 0;
      matrix_info[i].nextnw = 0;
      matrix_info[i].tbstopse = FALSE;
      matrix_info[i].tbstopnw = FALSE;
   }

   /* Vertical chains next. */

   do_matrix_chains(matrix_info, nump, callspec, 0);

   /* Scan for people who ought to have done something but didn't. */

   for (i=0; i<nump; i++) {
      if (!matrix_info[i].realdone) {
         if (   (!(flags & MTX_USE_SELECTOR))  ||  matrix_info[i].sel   ) {
            fail("Person could not identify other person to work with.");
         }
      }
   }

   finish_matrix_call(matrix_info, nump, &people, result);
}


/* This function is internal. */

Private void rollmove(
   setup *ss,
   callspec_block *callspec,
   setup *result)

{
   int i;
   unsigned int rot, st;

   if (setup_limits[ss->kind] < 0) fail("Can't roll in this setup.");
   
   result->kind = ss->kind;
   result->rotation = ss->rotation;
   
   for (i=0; i<=setup_limits[ss->kind]; i++) {
      if (ss->people[i].id1) {
         rot = 0;
         st = ((unsigned int) stb_z)*DBSTAB_BIT; 
         if (!(callspec->callflags1 & CFLAG1_REQUIRES_SELECTOR) || selectp(ss, i)) {
            switch (ss->people[i].id1 & ROLL_MASK) {
               case ROLLBITL: rot = 033, st = ((unsigned int) stb_a)*DBSTAB_BIT; break;
               case ROLLBITM: break;
               case ROLLBITR: rot = 011; st = ((unsigned int) stb_c)*DBSTAB_BIT; break;
               default: fail("Roll not supported after previous call.");
            }
         }
         install_rot(result, i, ss, i, rot);

         if (result->people[i].id1 & STABLE_ENAB) {
            do_stability(&result->people[i].id1, st, rot);
         }
      }
      else
         clear_person(result, i);
   }
}


/* Strip out those concepts that do not have the "dfm__xxx" flag set saying that they are to be
   inherited to this part of the call.  BUT: the "INHERITFLAG_LEFT" flag controls
   both "INHERITFLAG_REVERSE" and "INHERITFLAG_LEFT", turning the former into the latter.  This makes reverse
   circle by, touch by, and clean sweep work. */

Private final_set get_mods_for_subcall(final_set new_final_concepts, defmodset this_modh, int callflagsh)
{
   final_set retval;

   retval = new_final_concepts;

   /* If this subcall has "inherit_reverse" or "inherit_left" given, but the top-level call
      doesn't permit the corresponding flag to be given, we should turn any "reverse" or
      "left" modifier that was given into the other one, and cause that to be inherited.
      This is what turns, for example, part 3 of "*REVERSE* clean sweep" into a "*LEFT* 1/2 tag". */


   if (this_modh & ~callflagsh & (INHERITFLAG_REVERSE | INHERITFLAG_LEFT)) {
      if (new_final_concepts & (INHERITFLAG_REVERSE | INHERITFLAG_LEFT))
         retval |= (INHERITFLAG_REVERSE | INHERITFLAG_LEFT);
   }

   retval &= ~(new_final_concepts & HERITABLE_FLAG_MASK & ~this_modh);
   
   return (retval);
}


Private void move_with_real_call(
   setup *ss,
   parse_block *parseptr,
   callspec_block *callspec,
   final_set final_concepts,
   long_boolean qtfudged,
   setup *result)

{
   int subcall_index;
   final_set temp_concepts, conc1, conc2;
   long_boolean qtf;
   parse_block *cp1;
   parse_block *cp2;
   warning_info saved_warnings;
   int tbonetest;
   setup tempsetup;
   final_set new_final_concepts;
   callspec_block *call1, *call2;
   calldef_schema the_schema;
   long_boolean mirror;

   /* We have a genuine call.  Presumably all serious concepts have been disposed of
      (that is, nothing interesting will be found in parseptr -- it might be
      useful to check that someday) and we just have the callspec and the final
      concepts.
      The first thing we must do is check for a call whose schema is single (cross)
      concentric.  If so, be sure the setup is divided into 1x4's or diamonds. */

   the_schema = callspec->schema;
   if (the_schema == schema_maybe_single_concentric)
      the_schema = (final_concepts & INHERITFLAG_SINGLE) ? schema_single_concentric : schema_concentric;
   else if (the_schema == schema_maybe_single_cross_concentric)
      the_schema = (final_concepts & INHERITFLAG_SINGLE) ? schema_single_cross_concentric : schema_cross_concentric;
   else if (the_schema == schema_maybe_matrix_conc_star)
      if (final_concepts & INHERITFLAG_12_MATRIX)
         the_schema = schema_conc_star12;
      else if (final_concepts & INHERITFLAG_16_MATRIX)
         the_schema = schema_conc_star16;
      else
         the_schema = schema_conc_star;

   /* Do some quick error checking for visible fractions.  For now, either flag is acceptable.  Later, we will
      distinguish between the "visible_fractions" and "first_part_visible" flags. */
   if ((ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK) &&
            (((the_schema != schema_sequential) && (the_schema != schema_split_sequential)) || (!(callspec->callflags1 & (CFLAG1_VISIBLE_FRACTIONS | CFLAG1_FIRST_PART_VISIBLE)))))
      fail("This call can't be fractionalized.");

   switch (the_schema) {
      case schema_single_concentric:
      case schema_single_cross_concentric:
         switch (ss->kind) {
            case s2x4:
               divided_setup_move(ss, parseptr, callspec, final_concepts, (*map_lists[s1x4][1])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
               return;
            case s1x8:
               divided_setup_move(ss, parseptr, callspec, final_concepts, (*map_lists[s1x4][1])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
               return;
            case s_qtag:
               divided_setup_move(ss, parseptr, callspec, final_concepts, (*map_lists[sdmd][1])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
               return;
            case s_ptpd:
               divided_setup_move(ss, parseptr, callspec, final_concepts, (*map_lists[sdmd][1])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
               return;
         }
         break;
      case schema_single_concentric_together:
         switch (ss->kind) {
            case s1x8:
               divided_setup_move(ss, parseptr, callspec, final_concepts, (*map_lists[s1x4][1])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
               return;
            case s_ptpd:
               divided_setup_move(ss, parseptr, callspec, final_concepts, (*map_lists[sdmd][1])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
               return;
         }
         break;
   }

   /* If the "diamond" concept has been given and the call doesn't want it, we do
      the "diamond single wheel" variety. */

   if (INHERITFLAG_DIAMOND & final_concepts & (~callspec->callflagsh))  {
      /* If the call is sequentially or concentrically defined, the top level flag is required
         before the diamond concept can be inherited.  Since that flag is off, it is an error. */
      if (the_schema != schema_by_array)
         fail("Can't do this call with the 'diamond' concept.");

      ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

      switch (ss->kind) {
         case sdmd:
            divided_setup_move(ss, parseptr, callspec, final_concepts & ~INHERITFLAG_DIAMOND,
                  (*map_lists[s_1x2][1])[MPKIND__DMD_STUFF][0], phantest_ok, TRUE, result);
            return;
         case s_qtag:
            /* If in a qtag, perhaps we ought to divide into single diamonds and try again.
               BUT: if "magic" or "interlocked" is also present, we don't.  We let basic_move deal with
               it.  It will come back here after it has done what it needs to. */
   
            if ((final_concepts & (INHERITFLAG_MAGIC | INHERITFLAG_INTLK)) == 0) {
               /* Divide into diamonds and try again.  Note that we do not clear the concept. */
               divided_setup_move(ss, parseptr, callspec, final_concepts,
                     (*map_lists[sdmd][1])[MPKIND__SPLIT][1], phantest_ok, FALSE, result);
               return;
            }
            break;
         case s_ptpd:
            /* If in point-to-point diamonds, perhaps we ought to divide into single diamonds and try again.
               BUT: if "magic" or "interlocked" is also present, we don't.  We let basic_move deal with
               it.  It will come back here after it has done what it needs to. */
   
            if ((final_concepts & (INHERITFLAG_MAGIC | INHERITFLAG_INTLK)) == 0) {
               /* Divide into diamonds and try again.  Note that we do not clear the concept. */
               divided_setup_move(ss, parseptr, callspec, final_concepts,
                     (*map_lists[sdmd][1])[MPKIND__SPLIT][0], phantest_ok, FALSE, result);
               return;
            }
            break;
         default: fail("Must have diamonds for this concept.");
      }
   }

   mirror = FALSE;

   /* It may be appropriate to step to a wave or rear back from one.
      This is only legal if the flag forbidding same is off.
      Furthermore, if certain modifiers have been given, we don't allow it. */

   if (final_concepts & (INHERITFLAG_MAGIC | INHERITFLAG_INTLK | INHERITFLAG_12_MATRIX | INHERITFLAG_16_MATRIX | INHERITFLAG_FUNNY))
      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;

   /* But, alas, if fractionalization is on, we can't do it yet, because we don't
      know whether we are starting at the beginning.  In the case of fractionalization,
      we will do it later.  In that case, we already know that the call is sequentially
      defined, and so we will get to it later. */

   if ((!(ss->setupflags & (SETUPFLAG__NO_STEP_TO_WAVE | SETUPFLAG__FRACTIONALIZE_MASK))) &&
         (callspec->callflags1 & (CFLAG1_REAR_BACK_FROM_R_WAVE | CFLAG1_REAR_BACK_FROM_QTAG | CFLAG1_STEP_TO_WAVE))) {

      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;  /* Can only do it once. */

      if (final_concepts & INHERITFLAG_LEFT) {
         mirror_this(ss);
         mirror = TRUE;
      }

      touch_or_rear_back(ss, mirror, callspec->callflags1);

      /* But, if the "left_means_touch_or_check" flag is set, we only wanted the "left" flag for the
         purpose of what "touch_or_rear_back" just did.  So, in that case, we turn off the "left"
         flag and set things back to normal. */

      if (callspec->callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) {
         final_concepts &= ~INHERITFLAG_LEFT;
         if (mirror) mirror_this(ss);
         mirror = FALSE;
      }
   }

   /* At this point, we may have mirrored the setup and, of course, left the switch "mirror"
      on.  We did it only as needed for the [touch / rear back / check] stuff.  What we
      did doesn't actually count.  In particular, if the call is defined concentrically
      or sequentially, mirroring the setup in response to "left" is *NOT* the right thing
      to do.  The right thing is to pass the "left" flag to all subparts that have the
      "inherit_left" invocation flag, and letting events take their course.  So we allow
      the "INHERITFLAG_LEFT" bit to remain in "final_concepts", because it is still important
      to know whether we have been invoked with the "left" modifier. */

   if (ss->setupflags & SETUPFLAG__DOING_ENDS) {
      /* Check for special case of ends doing a call like "detour" which specifically
         allows just the ends part to be done. */
      ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;
      if ((the_schema == schema_concentric || the_schema == schema_rev_checkpoint) &&
            (DFM1_ENDSCANDO & callspec->stuff.conc.outerdef.modifiers1)) {

         /* Copy the concentricity flags from the call definition into the setup.  All the fuss
            in database.h about concentricity flags co-existing with setupflags refers
            to this moment. */
         ss->setupflags |= (callspec->stuff.conc.outerdef.modifiers1 & DFM1_CONCENTRICITY_FLAG_MASK);

         callspec = base_calls[callspec->stuff.conc.outerdef.call_id];
         the_schema = callspec->schema;
         if (the_schema == schema_maybe_single_concentric)
            the_schema = (final_concepts & INHERITFLAG_SINGLE) ? schema_single_concentric : schema_concentric;
         else if (the_schema == schema_maybe_single_cross_concentric)
            the_schema = (final_concepts & INHERITFLAG_SINGLE) ? schema_single_cross_concentric : schema_cross_concentric;
         else if (the_schema == schema_maybe_matrix_conc_star)
            if (final_concepts & INHERITFLAG_12_MATRIX)
               the_schema = schema_conc_star12;
            else if (final_concepts & INHERITFLAG_16_MATRIX)
               the_schema = schema_conc_star16;
            else
               the_schema = schema_conc_star;
      }
   }

   /* Enforce the restriction that only tagging or scooting calls are allowed in certain contexts. */

   if ((final_concepts & FINAL__MUST_BE_TAG) && (!(callspec->callflags1 & CFLAG1_IS_TAG_CALL)))
      fail("Only a tagging call is allowed here.");
   else if ((final_concepts & FINAL__MUST_BE_SCOOT) && (!(callspec->callflags1 & CFLAG1_IS_SCOOT_CALL)))
      fail("Only a scoot/tag (chain thru) (and scatter) call is allowed here.");

   final_concepts &= ~(FINAL__MUST_BE_TAG | FINAL__MUST_BE_SCOOT);

   /* If the "split" concept has been given and this call uses that concept for a special
      meaning (split square thru, split dixie style), set the special flag to determine that
      action, and remove the split concept.  Why remove it?  So that "heads split catch grand
      mix 3" will work.  If we are doing a "split catch", we don't really want to split the
      setup into 2x2's that are isolated from each other. */

   if (final_concepts & FINAL__SPLIT) {
      ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;
      if (callspec->callflags1 & CFLAG1_SPLIT_LIKE_SQUARE_THRU)
         final_concepts = (final_concepts | FINAL__SPLIT_SQUARE_APPROVED) & (~FINAL__SPLIT);
      else if (callspec->callflags1 & CFLAG1_SPLIT_LIKE_DIXIE_STYLE)
         final_concepts = (final_concepts | FINAL__SPLIT_DIXIE_APPROVED) & (~FINAL__SPLIT);
   }

   /* NOTE: We may have mirror-reflected the setup.  "Mirror" is true if so.  We may need to undo this. */

   /* If this is the "split sequential" schema and we have not already done so,
      cause splitting to take place. */

   if (the_schema == schema_split_sequential && !(final_concepts & FINAL__SPLIT_SEQ_DONE))
      final_concepts |= FINAL__SPLIT | FINAL__SPLIT_SEQ_DONE;

   /* If the split concept is still present, do it. */

   if (final_concepts & FINAL__SPLIT) {
      map_thing *split_map;

      final_concepts &= ~FINAL__SPLIT;
      ss->setupflags |= (SETUPFLAG__SAID_SPLIT | SETUPFLAG__NO_EXPAND_MATRIX);

      /* We can't handle the mirroring, so undo it. */
      if (mirror) { mirror_this(ss); mirror = FALSE; }

      if (ss->kind == s2x4) split_map = (*map_lists[s2x2][1])[MPKIND__SPLIT][0];
      else if (ss->kind == s1x8) split_map = (*map_lists[s1x4][1])[MPKIND__SPLIT][0];
      else if (ss->kind == s_ptpd) split_map = (*map_lists[sdmd][1])[MPKIND__SPLIT][0];
      else if (ss->kind == s_qtag) split_map = (*map_lists[sdmd][1])[MPKIND__SPLIT][1];
      else if (ss->kind == s2x2) {
         /* "Split" was given while we are already in a 2x2?  The only way that
            can be legal is if the word "split" was meant as a modifier for "split square thru"
            etc., rather than as a virtual-setup concept, or if the "split sequential" schema
            is in use.  In those cases, some "split approved" flag will still be on. */

         if (!(final_concepts & (FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED | FINAL__SPLIT_SEQ_DONE)))
            fail("Split concept is meaningless in a 2x2.");

         move(ss, parseptr, callspec, final_concepts, qtfudged, result);
         return;
      }
      else
         fail("Can't do split concept in this setup.");

      divided_setup_move(ss, parseptr, callspec, final_concepts, split_map, phantest_ok, TRUE, result);
      return;
   }

   tbonetest = 0;
   if (setup_limits[ss->kind] >= 0) {
      int j;

      for (j=0; j<=setup_limits[ss->kind]; j++) tbonetest |= ss->people[j].id1;
      if (!(tbonetest & 011)) {
         if (ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK)
            fail("Can't fractionalize a call if no one is doing it.");
         result->kind = nothing;
         return;
      }
   }

   /* We can't handle the mirroring unless the schema is by_array, so undo it. */

   if (the_schema != schema_by_array) {
      if (mirror) { mirror_this(ss); mirror = FALSE; }
   }

   switch (the_schema) {
      case schema_nothing:
         if (final_concepts) fail("Illegal concept for this call.");
         *result = *ss;
         result->setupflags = ((ss->setupflags & SETUPFLAG__ELONGATE_MASK) / SETUPFLAG__ELONGATE_BIT) * RESULTFLAG__ELONGATE_BIT;
         break;
      case schema_matrix:
         if (final_concepts) fail("Illegal concept for this call.");
         matrixmove(ss, callspec, result);
         reinstate_rotation(ss, result);
         break;
      case schema_partner_matrix:
         if (final_concepts) fail("Illegal concept for this call.");
         partner_matrixmove(ss, callspec, result);
         reinstate_rotation(ss, result);
         break;
      case schema_roll:
         if (final_concepts) fail("Illegal concept for this call.");
         rollmove(ss, callspec, result);
         result->setupflags = ((ss->setupflags & SETUPFLAG__ELONGATE_MASK) / SETUPFLAG__ELONGATE_BIT) * RESULTFLAG__ELONGATE_BIT;
         break;
      case schema_by_array:
         /* Dispose of the "left" concept first -- it can only mean mirror.  If it is on,
            mirroring may already have taken place. */

         if (final_concepts & INHERITFLAG_LEFT) {
/* ***** why isn't this particular error test taken care of more generally elsewhere? */
            if (!(callspec->callflagsh & INHERITFLAG_LEFT)) fail("Can't do this call 'left'.");
            if (!mirror) mirror_this(ss);
            mirror = TRUE;
            final_concepts &= ~INHERITFLAG_LEFT;
         }

         /* The "reverse" concept might mean mirror, or it might be genuine. */

         if ((final_concepts & INHERITFLAG_REVERSE) && (callspec->callflagsh & INHERITFLAG_REVERSE)) {
            /* This "reverse" just means mirror. */
            if (mirror) fail("Can't do this call 'left' and 'reverse'.");
            if (!mirror) mirror_this(ss);
            mirror = TRUE;
            final_concepts &= ~INHERITFLAG_REVERSE;
         }

         /* If the "reverse" flag is still set in final_concepts, it means a genuine
            reverse as in reverse cut/flip the diamond or reverse change-O. */

         basic_move(ss, parseptr, callspec, final_concepts, tbonetest, qtfudged, mirror, result);
         break;
      default:
   
         /* Must be sequential or some form of concentric. */
   
         new_final_concepts = final_concepts;
   
         /* We demand that the final concepts that remain be only those in the following list,
            which includes all of the "heritable" concepts. */

         if (new_final_concepts &
               ~(FINAL__SPLIT | HERITABLE_FLAG_MASK | FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED | FINAL__SPLIT_SEQ_DONE))
            fail("This concept not allowed here.");

         /* Now we demand that, if the concept was given, the call had the appropriate flag set saying
            that the concept is legal and will be inherited to the children. */

         if (HERITABLE_FLAG_MASK & new_final_concepts & (~callspec->callflagsh)) fail("Can't do this call with this concept.");

         if (the_schema == schema_sequential || the_schema == schema_split_sequential) {
            int current_elongation;
            int finalsetupflags = 0;
            int highlimit;
            long_boolean instant_stop = FALSE;
            long_boolean first_call = TRUE;
            int total = callspec->stuff.def.howmanyparts;
            long_boolean reverse_order = FALSE;
   
            int numer = ((ss->setupflags & (SETUPFLAG__FRACTIONALIZE_BIT*07))   / SETUPFLAG__FRACTIONALIZE_BIT);
            int denom = ((ss->setupflags & (SETUPFLAG__FRACTIONALIZE_BIT*070))  / (SETUPFLAG__FRACTIONALIZE_BIT*8));
            int key   = ((ss->setupflags & (SETUPFLAG__FRACTIONALIZE_BIT*0700)) / (SETUPFLAG__FRACTIONALIZE_BIT*64));

            qtf = qtfudged;

            if (new_final_concepts & FINAL__SPLIT) {
               if (callspec->callflags1 & CFLAG1_SPLIT_LIKE_SQUARE_THRU)
                  new_final_concepts |= FINAL__SPLIT_SQUARE_APPROVED;
               else if (callspec->callflags1 & CFLAG1_SPLIT_LIKE_DIXIE_STYLE)
                  new_final_concepts |= FINAL__SPLIT_DIXIE_APPROVED;
            }

            /* Iterate over the parts of the call. */

            highlimit = 1000000;
            subcall_index = 0;          /* Where we start, in the absence of special stuff. */

            /* If SETUPFLAG__FRACTIONALIZE_MASK stuff is nonzero, we are being asked to do something special.
               Read the three indicators.  Their meaning is as follows:
                  high 3 bits   middle 3 bits   low 3 bits
                     "key"       "denom"          "numer"

                       0            0             nonzero       Do first <numer> parts of the call.

                       1/3          0             nonzero       Do stuff strictly after first <numer> parts of the call.
                                                                If key=3, demand that "finish_means_skip_first_part" is on.

                       0         nonzero          nonzero       Do first <numer>/<denom> of the call.

                       1         nonzero          nonzero       Do stuff strictly after first <numer>/<denom> of the call.

                       2            0             nonzero       Do just the part <numer>, set
                                                                    RESULTFLAG__DID_LAST_PART if it was last part.

                       2            1             nonzero       Do just the part size-<numer>, that is, N=1 means do
                                                                    last part, N=2 means do next-to-last, etc. set
                                                                    RESULTFLAG__DID_LAST_PART if it was first part. */

            if (ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK) {
               if (key == 2) {
                  /* Just do the "numer" part of the call (or that part counting from end), and tell if it was last. */
                  if (numer > total) fail("The indicated part number doesn't exist.");
                  reverse_order = (denom != 0);
                  if (reverse_order) first_call = FALSE;
                  subcall_index = reverse_order ? total-numer : numer-1;
                  instant_stop = TRUE;
                  /* If only first part is visible, this is illegal unless we are doing first part. */
                  if (!(callspec->callflags1 & CFLAG1_VISIBLE_FRACTIONS) && (subcall_index != 0))
                     fail("This call can't be fractionalized.");
               }
               else {
                  /* Do parts up to (key = 0) or after (key = 1 or 3) the indicated part.
                     The indicated part may be an absolute part number (denom=0) or a fraction. */

                  int indicated_part;

                  if (key == 3 && !(callspec->callflags1 & CFLAG1_FINISH_MEANS_SKIP_FIRST))
                     fail("This call can't be 'finished'.");

                  if (denom) {
                     /* Amount to do was given as a fraction. */
                     if (numer >= denom) fail("Fraction must be proper.");
                     indicated_part = total * numer;
                     if ((indicated_part % denom) != 0) fail("This call can't be fractionalized with this fraction.");
                     indicated_part = indicated_part / denom;
                     /* If only first part is visible, this is illegal. */
                     if (!(callspec->callflags1 & CFLAG1_VISIBLE_FRACTIONS))
                        fail("This call can't be fractionalized.");
                  }
                  else {
                     indicated_part = numer;
                     /* If only first part is visible, this is illegal unless we are breaking just after the first part. */
                     if (!(callspec->callflags1 & CFLAG1_VISIBLE_FRACTIONS) && (indicated_part != 1))
                        fail("This call can't be fractionalized.");
                  }

                  if (key != 0) {
                     /* Do the last section of the call, starting just after indicated_part. */
                     subcall_index = indicated_part;
                     if (subcall_index > total) fail("The indicated part number doesn't exist.");
                  }
                  else {
                     /* Do the first section of the call, up to the indicated_part. */
                     highlimit = indicated_part;
                     if (highlimit > total) fail("The indicated part number doesn't exist.");
                  }
               }
            }

            current_elongation = ss->setupflags & SETUPFLAG__ELONGATE_MASK;
            if (ss->kind != s2x2 && ss->kind != s_short6) current_elongation = 0;

            /* Did we neglect to do the touch/rear back stuff because fractionalization was enabled?
               If so, now is the time to correct that.  We only do it for the first part, and only if
               doing parts in forward order. */

            /* Test for all this is "random left, swing thru".
               The test cases for this stuff are such things as "left swing thru". */

            if ((!(ss->setupflags & SETUPFLAG__NO_STEP_TO_WAVE)) && (subcall_index == 0) && !reverse_order &&
                  (callspec->callflags1 & (CFLAG1_REAR_BACK_FROM_R_WAVE | CFLAG1_REAR_BACK_FROM_QTAG | CFLAG1_STEP_TO_WAVE))) {

               ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;  /* Can only do it once. */

               if (new_final_concepts & INHERITFLAG_LEFT) {
                  if (!mirror) mirror_this(ss);
                  mirror = TRUE;
               }

               touch_or_rear_back(ss, mirror, callspec->callflags1);
            }

            /* See comments above relating to "left_means_touch_or_check".  We are about to
               un-mirror this in any case. */

            if (callspec->callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)
               final_concepts &= ~INHERITFLAG_LEFT;

            /* See comment earlier about mirroring.  For sequentially or concentrically defined
               calls, the "left" flag does not mean mirror; it is simplt passed to subcalls.
               So we must put things into their normal state.  If we did any mirroring, it was
               onlt to facilitate the action of "touch_or_rear_back". */

            if (mirror) mirror_this(ss);
            mirror = FALSE;

            *result = *ss;

            if (highlimit > total) highlimit = total;

            for (;;) {
               int j;
               setup tttt;
               by_def_item *this_item;
               defmodset this_mod1, this_modh;
               int saved_number_fields = current_number_fields;
               int count_to_use;

               if (subcall_index >= highlimit) break;

               this_item = &callspec->stuff.def.defarray[subcall_index];

               this_mod1 = this_item->modifiers1;
               this_modh = this_item->modifiersh;

               temp_concepts = get_mods_for_subcall(new_final_concepts, this_modh, callspec->callflagsh);
               get_real_subcall(parseptr, this_item, temp_concepts, &cp1, &call1, &conc1);

               /* If this context requires a tagging or scoot call, pass that fact on. */
               if (DFM1_MUST_BE_TAG_CALL & this_mod1) conc1 |= FINAL__MUST_BE_TAG;
               else if (DFM1_MUST_BE_SCOOT_CALL & this_mod1) conc1 |= FINAL__MUST_BE_SCOOT;

               current_number_fields >>= ((DFM1_NUM_SHIFT_MASK & this_mod1) / DFM1_NUM_SHIFT_BIT) * 4;
               count_to_use = current_number_fields & 0xF;

               if (DFM1_REPEAT_N & this_mod1) {
                  tempsetup = *result;
                  for (j = 1; j <= count_to_use; j++) {
                     tttt = tempsetup;
                     tttt.setupflags = (ss->setupflags & ~(SETUPFLAG__FRACTIONALIZE_MASK | SETUPFLAG__ELONGATE_MASK)) | current_elongation;
                     if (!first_call) tttt.setupflags |= SETUPFLAG__NO_CHK_ELONG;

                     move(&tttt, cp1, call1, conc1, qtf, &tempsetup);
                     finalsetupflags |= tempsetup.setupflags;
                  }
               }
               else if (DFM1_REPEAT_NM1 & this_mod1) {
                  tempsetup = *result;
                  for (j = 1; j <= count_to_use - 1; j++) {
                     tttt = tempsetup;
                     tttt.setupflags = (ss->setupflags & ~(SETUPFLAG__FRACTIONALIZE_MASK | SETUPFLAG__ELONGATE_MASK)) | current_elongation;
                     if (!first_call) tttt.setupflags |= SETUPFLAG__NO_CHK_ELONG;

                     move(&tttt, cp1, call1, conc1, qtf, &tempsetup);
                     finalsetupflags |= tempsetup.setupflags;
                  }
               }
               else if (DFM1_REPEAT_N_ALTERNATE & this_mod1) {
                  tempsetup = *result;

                  /* Read the call after this one -- we will alternate between the two. */
                  get_real_subcall(parseptr, &callspec->stuff.def.defarray[subcall_index+1], temp_concepts, &cp2, &call2, &conc2);

                  for (j = 1; j <= count_to_use; j++) {
                     tttt = tempsetup;
                     tttt.setupflags = (ss->setupflags & ~(SETUPFLAG__FRACTIONALIZE_MASK | SETUPFLAG__ELONGATE_MASK)) | current_elongation;
                     if (!first_call) tttt.setupflags |= SETUPFLAG__NO_CHK_ELONG;

                     if (j&1)
                        move(&tttt, cp1, call1, conc1, qtf, &tempsetup);
                     else
                        move(&tttt, cp2, call2, conc2, qtf, &tempsetup);
                     finalsetupflags |= tempsetup.setupflags;
                  }
                  subcall_index++;     /* Skip over the second call. */
               }
               else {
                  tttt = *result;
                  tttt.setupflags = (ss->setupflags & ~(SETUPFLAG__FRACTIONALIZE_MASK | SETUPFLAG__ELONGATE_MASK)) | current_elongation;
                  if (!first_call) tttt.setupflags |= SETUPFLAG__NO_CHK_ELONG;

                  if ((DFM1_CPLS_UNLESS_SINGLE & this_mod1) && !(new_final_concepts & INHERITFLAG_SINGLE)) {
                     tandem_couples_move(&tttt, cp1, call1, conc1,
                           selector_uninitialized, 0, 0, 0, 1, &tempsetup);
                  }
                  else
                     move(&tttt, cp1, call1, conc1, qtf, &tempsetup);

                  finalsetupflags |= tempsetup.setupflags;
               }

               current_number_fields = saved_number_fields;

               /* If this call is "roll transparent", restore roll info from before the call for those people
                  that are marked as roll-neutral. */

               if (DFM1_ROLL_TRANSPARENT & this_mod1) {
                  /* Can only do this if we understand the setups. */
                  if ((setup_limits[tempsetup.kind] >= 0) && (setup_limits[result->kind] >= 0)) {
                     int u, v;

                     for (u=0; u<=setup_limits[tempsetup.kind]; u++) {
                        if (tempsetup.people[u].id1 & ROLLBITM) {
                           /* This person is roll-neutral.  Reinstate his original roll info, by
                              searching for him in the starting setup. */
                           tempsetup.people[u].id1 &= ~ROLL_MASK;
                           for (v=0; v<=setup_limits[result->kind]; v++) {
                              if (((tempsetup.people[u].id1 ^ result->people[v].id1) & 0700) == 0)
                                 tempsetup.people[u].id1 |= (result->people[v].id1 & ROLL_MASK);
                           }
                        }
                     }
                  }
               }

               /* Whenever the ending setup is 2x2, we are responsible for computing the
                  "current_elongation" field of the result. */

               /* The following comments used to be in this code, back when it was much
                  more complicated than it is now. */

               /* If the setup has undergone a transition from a 1x4/diamond to a 2x2, make a note
                  of what the elongation should be according to the call's "parallel_conc_end" flag.
                  Remember that "result" contatins the setup at the start of this step, and "tempsetup"
                  has the setup at the end of this step.  The xor's with the rotations are done to
                  compensate for the fact that we may have done, say, a lockit, in an earlier step.
                  The final value that we have to provide for result->parallel_conc_end must state
                  the elongation relative to the original 1x4 orientation.  Whew!
               I believe the test cases for this stuff are locker's choice and split cast, the
                  latter being done from a grand line.  In that case, the ends have to hinge and
                  then trade.  The hinge has them stay close together, so they are as if in columns.
                  The trade then must preserve that.
               This used to xor the current setup rotation and the original setup rotation into the
                  equation, believing that, if a lockit happened earlier, we want the
                  "parallel_conc_end" property on the 1x4->2x2 subcall to apply to the entire
                  call.  I no longer believe that is correct.  What happens now is that each subcall is
                  evaluated independently.  If a 1x4->2x2 subcall occurs, we find out, from looking
                  at the call descriptor, where the dancers would like to finish that subcall,
                  and proceed from there. */

               /* Next part: we have to preserve the meaning of the "parallel_conc_end" flag
                  for 2x2->2x2 calls also.  Its meaning in this case is "go to antispots".
                  This is necessary to make "ENDS counter rotate" work from lines.  (Note
                  that counter rotate has this flag set.)  For a compound call, we obviously
                  want to xor all the flags of the constituent parts.  That is why we do an
                  xor here.  (Note that we initialized it to zero.)  But the stuff just above
                  overwrote it.  That's OK.  Any 1x4->2x2 call ought to destroy the "antispots"
                  info from previous 2x2->2x2 calls.  But after the 1x4->2x2 call has filled
                  in the elongation info, subsequent 2x2->2x2 calls requesting antispots should
                  flip the elongation.
               The test case for this is N/4 chain and circulate in.  The ends do a double
                  circulate, which is defined sequentially as 2 circulates.  Before this code was
                  put in, basic_move honored the parallel_conc_end/antispots flag for atomic
                  2x2->2x2 calls, but it wasn't honored here for concatenations of such.  The
                  final state of result->parallel_conc_end was random, so it would sometimes
                  think the ends should go to column spots. */

               if (tempsetup.kind == s2x2) {
                  switch (result->kind) {
                     case s1x4: case sdmd: case s2x2:
                        current_elongation = (((tempsetup.setupflags & RESULTFLAG__ELONGATE_MASK) / RESULTFLAG__ELONGATE_BIT) * SETUPFLAG__ELONGATE_BIT);
                        break;
   
                     /* Otherwise (perhaps the setup was a star) we have no idea how to elongate the setup. */
   
                     default:
                        current_elongation = 0;
                        break;
                  }
               }
               else
                  current_elongation = 0;

               *result = tempsetup;

               /* We allow expansion on the first part, and then shut it off for later parts.
                  This is required for things like 12 matrix grand swing thru from a 1x8 or 1x10. */
               ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

               /* Remove outboard phantoms. 
                  It used to be that normalize_setup was not called
                  here.  It was found that we couldn't do things like, from a suitable offset wave,
                  [triple line 1/2 flip] back to a wave, that is, start offset and finish normally.
                  So this has been added.  However, there may have been a reason for not normalizing.
                  If any problems are found, it may be that a flag needs to be added to seqdef calls
                  saying whether to remove outboard phantoms after each part. */

               /* But we *DON'T* remove the phantoms if "explicit matrix" is on, as of version 29.45.
                  This is to make "1x12 matrix grand swing thru" work from a simple grand wave.
                  The intention is to add outboard phantoms and then do the call in a gigantic way.
                  We added two outboard phantoms on each end, then did the swing half, resulting
                  in another 1x12 with only the center 8 spots occupied.  If outboard phantoms are
                  removed, the effect of the explicit "1x12 matrix" modifier is lost.  The "grand
                  1/2 by the left" is done on the 1x8.  So we leave the phantoms in.  Actually,
                  making this call operate properly required a database change as well.  We had to
                  add 1x12 and 1x16 starting setups for "1/2 by the left or right".  Otherwise,
                  on "1x12 matrix grand swing thru", it would just do it in each 1x6.  Also, we
                  don't want the warning "Do the call in each 1x6" to be given, though it is
                  admittedly better to have the program warn us when it's about to do something
                  wrong than to do the wrong thing silently. */

               if (!tttt.setupflags & SETUPFLAG__EXPLICIT_MATRIX)
                  normalize_setup(result, simple_normalize);

               qtf = FALSE;

               new_final_concepts &= ~(FINAL__SPLIT | FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED);

               subcall_index++;
               first_call = FALSE;

               /* If we are being asked to do just one part of a call (from SETUPFLAG__FRACTIONALIZE_MASK),
                  exit now.  Also, see if we just did the last part. */

               if ((ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK) && instant_stop) {
                  /* Check whether we honored the last possible request.  That is,
                     whether we did the last part of the call in forward order, or
                     the first part in reverse order. */
                  if (numer >= total) finalsetupflags |= RESULTFLAG__DID_LAST_PART;
                  break;
               }
            }

            result->setupflags = (finalsetupflags & ~RESULTFLAG__ELONGATE_MASK) | ((current_elongation / SETUPFLAG__ELONGATE_BIT) * RESULTFLAG__ELONGATE_BIT);
         }
         else {

            /* Must be some form of concentric. */

            ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;     /* We think this is the right thing to do. */
            saved_warnings = history[history_ptr+1].warnings;

            temp_concepts = get_mods_for_subcall(new_final_concepts, callspec->stuff.conc.innerdef.modifiersh, callspec->callflagsh);
            get_real_subcall(parseptr, &callspec->stuff.conc.innerdef, temp_concepts, &cp1, &call1, &conc1);

            /* Do it again. */

            temp_concepts = get_mods_for_subcall(new_final_concepts, callspec->stuff.conc.outerdef.modifiersh, callspec->callflagsh);
            get_real_subcall(parseptr, &callspec->stuff.conc.outerdef, temp_concepts, &cp2, &call2, &conc2);

            /* Fudge a 3x4 into a 1/4-tag if appropriate. */

            if (ss->kind == s3x4 && (callspec->callflags1 & CFLAG1_FUDGE_TO_Q_TAG) &&
                  (the_schema == schema_concentric || the_schema == schema_cross_concentric)) {

               if (ss->people[0].id1) {
                  if (ss->people[1].id1) fail("Can't do this call from arbitrary 3x4 setup.");
               }
               else (void) copy_person(ss, 0, ss, 1);

               if (ss->people[3].id1) {
                  if (ss->people[2].id1) fail("Can't do this call from arbitrary 3x4 setup.");
                  else (void) copy_person(ss, 1, ss, 3);
               }
               else (void) copy_person(ss, 1, ss, 2);

               (void) copy_person(ss, 2, ss, 4);
               (void) copy_person(ss, 3, ss, 5);

               if (ss->people[6].id1) {
                  if (ss->people[7].id1) fail("Can't do this call from arbitrary 3x4 setup.");
                  else (void) copy_person(ss, 4, ss, 6);
               }
               else (void) copy_person(ss, 4, ss, 7);

               if (ss->people[9].id1) {
                  if (ss->people[8].id1) fail("Can't do this call from arbitrary 3x4 setup.");
                  else (void) copy_person(ss, 5, ss, 9);
               }
               else (void) copy_person(ss, 5, ss, 8);

               (void) copy_person(ss, 6, ss, 10);
               (void) copy_person(ss, 7, ss, 11);

               ss->kind = s_qtag;
               ss->setupflags |= SETUPFLAG__DISTORTED;
            }

            concentric_move(
               ss,
               cp1, cp2, call1, call2, conc1, conc2,
               the_schema,
               callspec->stuff.conc.innerdef.modifiers1,
               callspec->stuff.conc.outerdef.modifiers1,
               result);

            if (DFM1_SUPPRESS_ELONGATION_WARNINGS & callspec->stuff.conc.outerdef.modifiers1) {
               history[history_ptr+1].warnings.bits[0] &= ~(Warnings_About_Conc_elongation);
            }
            history[history_ptr+1].warnings.bits[0] |= saved_warnings.bits[0];
            history[history_ptr+1].warnings.bits[1] |= saved_warnings.bits[1];
         }

         break;
   }

   /* Reflect back if necessary. */
   if (mirror) mirror_this(result);
   canonicalize_rotation(result);
}



/* The current interpretation of the elongation flags, on input and output, is now
   as follows:
   
   Note first that, on input and output, the elongation bits are only meaningful in
      a 2x2 or short6 setup.
   On input, nonzero bits in the SETUPFLAG__ELONGATE_MASK with a 2x2 setup mean that
      the setup _was_ _actually_ _elongated_, and that the elongation is actually felt
      by the dancers.  In this case, the "move" routine is entitled to raise an error
      if the 2x2 call is awkward.  For example, a star thru from facing couples is
      illegal if the elongate bit is on that makes the people far away from the one
      they are facing.  It follows from this that, if we call concentric star thru,
      these bits will be cleared before calling "move", since the concentric concept
      forgives awkward elongation.  Of course, the "concentric_move" routine will
      remember the actual elongation in order to move people to the correct ending
      formation.
   On output, nonzero bits in the RESULTFLAG__ELONGATE_MASK with a 2x2 setup mean that,
      _if_ _result_ _elongation_ _is_ _required_, this is what it should be.  It does
      _not_ mean that such elongation actually exists.  Whoever called "move" must
      make that judgement.  These bits are set when, for example, a 1x4 -> 2x2 call
      is executed, to indicate how to elongate the result, if it turns out that those
      people were working around the outside.  This determination is made not just on
      the "checkpoint rule" that says they go perpendicular to their original axis,
      but also on the basis of the "parallel_conc_end" flag in the call.  That is, these
      bits tell which way to go if the call had been "ends do <whatever>".  Of course,
      if the concentric concept was in use, this information is not used.  Instead,
      "concentric_move" overrides the bits we return with an absolute "checkpoint rule"
      application.

   It may well be that the goal described above is not actually implemented correctly.
*/


extern void move(
   setup *ss,
   parse_block *parseptr,
   callspec_block *callspec,
   final_set final_concepts,
   long_boolean qtfudged,
   setup *result)
{
   parse_block *saved_magic_diamond;
   final_set new_final_concepts;
   final_set check_concepts;
   parse_block *parseptrcopy;

   clear_people(result);
   result->setupflags = 0;

   if (callspec) {
      move_with_real_call(ss, parseptr, callspec, final_concepts, qtfudged, result);
      return;
   }

   /* Scan the "final" concepts, remembering them and their end point. */
   last_magic_diamond = 0;
   parseptrcopy = process_final_concepts(parseptr, TRUE, &new_final_concepts);

   saved_magic_diamond = last_magic_diamond;

   /* See if there were any "non-final" ones present also. */

   new_final_concepts |= final_concepts;         /* Include any old ones we had. */

   /* These are the concepts that we are interested in. */

   check_concepts = new_final_concepts & ~(FINAL__MUST_BE_TAG | FINAL__MUST_BE_SCOOT);

   if (parseptrcopy->concept->kind <= marker_end_of_list) {
      int saved_number_fields = current_number_fields;
      selector_kind saved_selector = current_selector;
      direction_kind saved_direction = current_direction;

      /* There are no "non-final" concepts.  The only concepts are the final ones that
         have been encoded into new_final_concepts. */

      /* We must read the selector out of the concept list and use it for this call to "move".
         We are effectively using it as an argument to "move", with all the care that must go
         into invocations of recursive procedures.  However, at its point of actual use, it
         must be in a global variable.  Therefore, we explicitly save and restore that
         global variable (in a dynamic variable local to this instance) rather than passing
         it as an explicit argument.  By saving it and restoring it in this way, we make
         things like "checkpoint bounce the beaux by bounce the belles" work. */
      
      current_selector = parseptrcopy->selector;
      current_direction = parseptrcopy->direction;
      current_number_fields = parseptrcopy->number;
      move_with_real_call(ss, parseptrcopy, parseptrcopy->call, new_final_concepts, qtfudged, result);
      current_selector = saved_selector;
      current_direction = saved_direction;
      current_number_fields = saved_number_fields;
   }
   else {
      /* We now know that there are "non-final" (virtual setup) concepts present. */


      if (check_concepts == 0) {
         /* Look for virtual setup concept that can be done by dispatch from table, with no
            intervening final concepts. */
   
         if (do_big_concept(ss, parseptrcopy, result)) {
            canonicalize_rotation(result);
            return;
         }
      }

      /* Some final concept (e.g. "magic") is present in front of our virtual setup concept.
         We have to dispose of it.  This means that expanding the matrix (e.g. 2x4->2x6)
         and stepping to a wave or rearing back from one are no longer legel. */

      ss->setupflags |= (SETUPFLAG__NO_EXPAND_MATRIX | SETUPFLAG__NO_STEP_TO_WAVE);

      /* There are a few "final" concepts that
         will not be treated as such if there are non-final concepts occurring
         after them.  Instead, they will be treated as virtual setup concepts.
         This is what makes "magic once removed trade" work, for
         example.  On the other hand, if there are no non-final concepts following, treat these as final.
         This is what makes "magic transfer" or "split square thru" work. */

      if (check_concepts == FINAL__SPLIT) {
         map_thing *split_map;
   
         ss->setupflags |= SETUPFLAG__SAID_SPLIT;

         if (ss->kind == s2x4) split_map = (*map_lists[s2x2][1])[MPKIND__SPLIT][0];
         else if (ss->kind == s1x8) split_map = (*map_lists[s1x4][1])[MPKIND__SPLIT][0];
         else if (ss->kind == s_ptpd) split_map = (*map_lists[sdmd][1])[MPKIND__SPLIT][0];
         else if (ss->kind == s_qtag) split_map = (*map_lists[sdmd][1])[MPKIND__SPLIT][1];
         else fail("Can't do split concept in this setup.");
   
         divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~FINAL__SPLIT,
            split_map, phantest_ok, TRUE, result);
      }
      else if ((check_concepts & ~INHERITFLAG_DIAMOND) == INHERITFLAG_MAGIC) {
         if (ss->kind == s2x4) {
            divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~INHERITFLAG_MAGIC,
               &map_2x4_magic, phantest_ok, TRUE, result);
         }
         else if (ss->kind == s_qtag) {
            divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~INHERITFLAG_MAGIC,
               &map_qtg_magic, phantest_ok, TRUE, result);

            /* Since more concepts follow the magic and/or interlocked stuff, we can't
               allow the concept to be just "magic" etc.  We have to change it to
               "magic diamond, ..."  Otherwise, things could come out sounding like
               "magic diamond as couples quarter right" when we should really be saying
               "magic diamond, diamond as couples quarter right".  Therefore, we are going
               to do something seriously hokey: we are going to change the concept descriptor
               to one whose name has the extra "diamond" word.  We do this by marking the
               setupflags word in the result.  The actual hokey stuff will be done presently. */

            result->setupflags |= RESULTFLAG__NEED_DIAMOND;
         }
         else if (ss->kind == s_ptpd) {
            divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~INHERITFLAG_MAGIC,
               &map_ptp_magic, phantest_ok, TRUE, result);
            result->setupflags |= RESULTFLAG__NEED_DIAMOND;      /* See above. */
         }
         else
            fail("Can't do magic concept in this context.");
      }
      else if ((check_concepts & ~INHERITFLAG_DIAMOND) == INHERITFLAG_INTLK) {
         if (ss->kind == s_qtag) {
            divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~INHERITFLAG_INTLK,
               &map_qtg_intlk, phantest_ok, TRUE, result);
            result->setupflags |= RESULTFLAG__NEED_DIAMOND;      /* See above. */
         }
         if (ss->kind == s_ptpd) {
            divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~INHERITFLAG_INTLK,
               &map_ptp_intlk, phantest_ok, TRUE, result);
            result->setupflags |= RESULTFLAG__NEED_DIAMOND;      /* See above. */
         }
         else
            fail("Can't do interlocked concept in this context.");
      }
      else if ((check_concepts & ~INHERITFLAG_DIAMOND) == (INHERITFLAG_INTLK | INHERITFLAG_MAGIC)) {
         if (ss->kind == s_qtag) {
            divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~(INHERITFLAG_INTLK | INHERITFLAG_MAGIC),
                  &map_qtg_magic_intlk, phantest_ok, TRUE, result);
            result->setupflags |= RESULTFLAG__NEED_DIAMOND;      /* See above. */
         }
         if (ss->kind == s_ptpd) {
            divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~(INHERITFLAG_INTLK | INHERITFLAG_MAGIC),
                  &map_ptp_magic_intlk, phantest_ok, TRUE, result);
            result->setupflags |= RESULTFLAG__NEED_DIAMOND;      /* See above. */
         }
         else
            fail("Can't do magic interlocked concept in this context.");
      }
      else if (check_concepts == INHERITFLAG_DIAMOND) {
         if (ss->kind == sdmd) {
            divided_setup_move(ss, parseptrcopy, NULLCALLSPEC, new_final_concepts & ~INHERITFLAG_DIAMOND,
                  (*map_lists[s_1x2][1])[MPKIND__DMD_STUFF][0], phantest_ok, TRUE, result);
         }
         else if (ss->kind == s_qtag) {
            /* Divide into diamonds and try again.  (Note that we back up the concept pointer.) */
            divided_setup_move(ss, parseptr, NULLCALLSPEC, 0,
                  (*map_lists[sdmd][1])[MPKIND__SPLIT][1], phantest_ok, FALSE, result);
         }
         else
            fail("Must have diamonds for this concept.");
      }
      else
         fail2("Can't do this concept with other concepts preceding it:", parseptrcopy->concept->name);
   }

   /* If execution of the call raised a request that we change a concept name from "magic" to
      "magic diamond,", for example, do so. */

   if (result->setupflags & RESULTFLAG__NEED_DIAMOND) {
      if (saved_magic_diamond && saved_magic_diamond->concept->value.arg1 == 0) {
         if (saved_magic_diamond->concept->kind == concept_magic) saved_magic_diamond->concept = &special_magic;
         else if (saved_magic_diamond->concept->kind == concept_interlocked) saved_magic_diamond->concept = &special_interlocked;
      }
   }
}
