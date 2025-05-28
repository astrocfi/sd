/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

/* SD -- square dance caller's helper.

    Copyright (C) 1990-1998  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */

/* This defines the following functions:
   initialize_menus
*/

#include <string.h>
#include "sd.h"

/* Global to this file. */

static callspec_block *empty_menu[] = {(callspec_block *) 0};

/* This gets temporarily allocated.  It persists through the entire initialization. */
Private callspec_block **global_temp_call_list;
Private int global_callcount;     /* Index into the above. */

#define WEST (d_west|ROLLBITL)
#define EAST (d_east|ROLLBITL)
#define NORT (d_north|ROLLBITL)
#define SOUT (d_south|ROLLBITL)

/* The following 8 definitions are taken verbatim from sdtables.c . */
#define B1A (0000 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_HCOR|ID1_PERM_HEAD|ID1_PERM_BOY)
#define G1A (0100 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_HEAD|ID1_PERM_GIRL)
#define B2A (0200 | ID1_PERM_NSG|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_SIDE|ID1_PERM_BOY)
#define G2A (0300 | ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_HCOR|ID1_PERM_SIDE|ID1_PERM_GIRL)
#define B3A (0400 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_HCOR|ID1_PERM_HEAD|ID1_PERM_BOY)
#define G3A (0500 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_HEAD|ID1_PERM_GIRL)
#define B4A (0600 | ID1_PERM_NSG|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_SIDE|ID1_PERM_BOY)
#define G4A (0700 | ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_HCOR|ID1_PERM_SIDE|ID1_PERM_GIRL)

/* In all of these setups in which people are facing, they are normal couples.  This makes initialization of things like star thru,
   ladies chain, and curlique work.  The setup for starting DPT has the appropriate sex for triple star thru. */
Private setup test_setup_1x8  = {s1x8, 0, {0}, {{NORT|B4A, 0}, {SOUT|G3A, 0}, {SOUT|B3A, 0}, {NORT|G4A, 0}, {SOUT|B2A, 0}, {NORT|G1A, 0}, {NORT|B1A, 0}, {SOUT|G2A, 0}}, 0};
Private setup test_setup_l1x8 = {s1x8, 0, {0}, {{SOUT|B4A, 0}, {NORT|G3A, 0}, {NORT|B3A, 0}, {SOUT|G4A, 0}, {NORT|B2A, 0}, {SOUT|G1A, 0}, {SOUT|B1A, 0}, {NORT|G2A, 0}}, 0};
Private setup test_setup_dpt  = {s2x4, 0, {0}, {{EAST|G2A, 0}, {EAST|B3A, 0}, {WEST|G3A, 0}, {WEST|B2A, 0}, {WEST|G4A, 0}, {WEST|B1A, 0}, {EAST|G1A, 0}, {EAST|B4A, 0}}, 0};
Private setup test_setup_cdpt = {s2x4, 0, {0}, {{WEST|G4A, 0}, {WEST|G3A, 0}, {EAST|B3A, 0}, {EAST|B4A, 0}, {EAST|G2A, 0}, {EAST|G1A, 0}, {WEST|B1A, 0}, {WEST|B2A, 0}}, 0};
Private setup test_setup_rcol = {s2x4, 0, {0}, {{EAST|B4A, 0}, {EAST|G3A, 0}, {EAST|B3A, 0}, {EAST|G4A, 0}, {WEST|B2A, 0}, {WEST|G1A, 0}, {WEST|B1A, 0}, {WEST|G2A, 0}}, 0};
Private setup test_setup_lcol = {s2x4, 0, {0}, {{WEST|G2A, 0}, {WEST|B1A, 0}, {WEST|G1A, 0}, {WEST|B2A, 0}, {EAST|G4A, 0}, {EAST|B3A, 0}, {EAST|G3A, 0}, {EAST|B4A, 0}}, 0};
Private setup test_setup_8ch  = {s2x4, 0, {0}, {{EAST|B4A, 0}, {WEST|G3A, 0}, {EAST|B3A, 0}, {WEST|G4A, 0}, {WEST|B2A, 0}, {EAST|G1A, 0}, {WEST|B1A, 0}, {EAST|G2A, 0}}, 0};
Private setup test_setup_tby  = {s2x4, 0, {0}, {{WEST|G3A, 0}, {EAST|B4A, 0}, {WEST|G4A, 0}, {EAST|B3A, 0}, {EAST|G1A, 0}, {WEST|B2A, 0}, {EAST|G2A, 0}, {WEST|B1A, 0}}, 0};
Private setup test_setup_lin  = {s2x4, 0, {0}, {{SOUT|G3A, 0}, {SOUT|B4A, 0}, {SOUT|G4A, 0}, {SOUT|B3A, 0}, {NORT|G1A, 0}, {NORT|B2A, 0}, {NORT|G2A, 0}, {NORT|B1A, 0}}, 0};
Private setup test_setup_lout = {s2x4, 0, {0}, {{NORT|B4A, 0}, {NORT|G3A, 0}, {NORT|B3A, 0}, {NORT|G4A, 0}, {SOUT|B2A, 0}, {SOUT|G1A, 0}, {SOUT|B1A, 0}, {SOUT|G2A, 0}}, 0};
Private setup test_setup_rwv  = {s2x4, 0, {0}, {{NORT|B4A, 0}, {SOUT|G3A, 0}, {NORT|B3A, 0}, {SOUT|G4A, 0}, {SOUT|B2A, 0}, {NORT|G1A, 0}, {SOUT|B1A, 0}, {NORT|G2A, 0}}, 0};
Private setup test_setup_lwv  = {s2x4, 0, {0}, {{SOUT|B4A, 0}, {NORT|G3A, 0}, {SOUT|B3A, 0}, {NORT|G4A, 0}, {NORT|B2A, 0}, {SOUT|G1A, 0}, {NORT|B1A, 0}, {SOUT|G2A, 0}}, 0};
Private setup test_setup_r2fl = {s2x4, 0, {0}, {{NORT|B4A, 0}, {NORT|G3A, 0}, {SOUT|G4A, 0}, {SOUT|B3A, 0}, {SOUT|B2A, 0}, {SOUT|G1A, 0}, {NORT|G2A, 0}, {NORT|B1A, 0}}, 0};
Private setup test_setup_l2fl = {s2x4, 0, {0}, {{SOUT|G3A, 0}, {SOUT|B4A, 0}, {NORT|B3A, 0}, {NORT|G4A, 0}, {NORT|G1A, 0}, {NORT|B2A, 0}, {SOUT|B1A, 0}, {SOUT|G2A, 0}}, 0};





/* This cleans up the text of a call or concept name, returning the
   menu-presentable form to be put into the "menu_name" field.  It
   simply re-uses the stored string where it can, and allocates fresh memory
   if a substitution took place. */

Private Const char *translate_menu_name(Const char *orig_name, uint32 *escape_bits_p)
{
   int j;
   char c;
   int namelength;
   long_boolean atsign = FALSE;

   /* See if the name has an 'at' sign, in which case we have to modify it to
      get the actual menu text.  Also, find out what escape flags we need to set. */

   namelength = 0;
   for (;;) {
      c = orig_name[namelength++];
      if (!c) break;
      else if (c == '@') {
         atsign = TRUE;
         c = orig_name[namelength++];
         if (c == 'e') *escape_bits_p |= ESCAPE_WORD__LEFT;
         else if (c == 'j' || c == 'C') *escape_bits_p |= ESCAPE_WORD__CROSS;
         else if (c == 'J' || c == 'M') *escape_bits_p |= ESCAPE_WORD__MAGIC;
         else if (c == 'E' || c == 'I') *escape_bits_p |= ESCAPE_WORD__INTLK;
      }
   }

   if (atsign) {
      char tempname[200];
      char *temp_ptr;
      char *new_ptr;
      int templength;

      temp_ptr = tempname;
      templength = 0;

      for (j = 0; j < namelength; j++) {
         c = orig_name[j];

         if (c == '@') {
            Const char *q = get_escape_string(orig_name[++j]);
            if (q && *q) {
               while (*q) temp_ptr[templength++] = *q++;
               continue;
            }
            else if (q) {
               /* Skip over @7...@8, @n .. @o, @j .. @l, and @J...@L stuff. */
               while (orig_name[j] != '@') j++;
               j++;
            }

            /* Be sure we don't leave two consecutive blanks in the text. */
            if (orig_name[j+1] == ' ' && templength != 0 && temp_ptr[templength-1] == ' ') j++;
         }
         else
            temp_ptr[templength++] = c;
      }

      tempname[templength] = '\0';
      /* Must copy the text into some fresh memory, being careful about overflow. */
      new_ptr = get_mem(templength+1);
      for (j=0; j<=templength; j++) new_ptr[j] = tempname[j];
      return new_ptr;
   }
   else
      return orig_name;
}



/* These variables are actually local to test_starting_setup, but they are
   expected to be preserved across the longjmp, so they must be static. */
Private parse_block *parse_mark;
Private int call_index;
Private callspec_block *test_call;
Private long_boolean crossiness;
Private long_boolean magicness;
Private long_boolean intlkness;


Private void test_starting_setup(call_list_kind cl, Const setup *test_setup)
{
   real_jmp_buf my_longjmp_buffer;
   int i;

   uims_database_tick(10);

   call_index = -1;
   global_callcount = 0;
   /* Mark the parse block allocation, so that we throw away the garbage
      created by failing attempts. */
   parse_mark = mark_parse_blocks();

   /* Create a special error handler. */

   longjmp_ptr = &my_longjmp_buffer;          /* point the global pointer at it. */
   if (setjmp(my_longjmp_buffer.the_buf)) {

      /* A call failed.  If the call had some mandatory substitution, pass it anyway. */

      if (mandatory_call_used) goto accept;

      /* Or a bad choice of selector or number may be the cause.
         Try different selectors first. */

      if (selector_used) {
         /* This call used a selector and didn't like it.  Try again with
            a different selector, until we run out of ideas. */
         switch (selector_for_initialize) {
            case selector_beaus:
               selector_for_initialize = selector_ends;
               goto try_another_selector;
            case selector_ends:
               selector_for_initialize = selector_everyone;
               goto try_another_selector;
            case selector_everyone:
               selector_for_initialize = selector_none;
               goto try_another_selector;
            case selector_none:
               /* When testing columns, we use an additional selector.  The way
                  the test setups are arranged, these effectively select #1 and #2 in the
                  column.  They make "<anyone> mark time" work. */
               /* Also, for tidal waves, we select boys.  That makes "relay the
                  shadow but <anyone> criss cross it" work. */
               if (test_setup == &test_setup_rcol || test_setup == &test_setup_lcol) {
                  selector_for_initialize = selector_headcorners;
                  goto try_another_selector;
               }
               else if (test_setup == &test_setup_1x8 || test_setup == &test_setup_l1x8) {
                  selector_for_initialize = selector_boys;
                  goto try_another_selector;
               }
         }
      }

      /* Now try a different number. */

      if (number_used) {
         /* This call used a number and didn't like it.  Try again with
            a different number, until we run out of ideas. */

         if (number_for_initialize < 4) {
            /* We try all numbers from 1 to 4.  We need to do this to get
               "exchange the boxes N/4" on the waves menu". */
            number_for_initialize++;
            goto try_another_number;
         }
      }

      /* Now try giving the "cross" modifier. */

      if (test_call->callflagsf & ESCAPE_WORD__CROSS) {
         if (!crossiness) {
            crossiness = TRUE;
            goto try_another_cross;
         }
      }

      /* Now try giving the "magic" modifier. */

      if (test_call->callflagsf & ESCAPE_WORD__MAGIC) {
         if (!magicness) {
            magicness = TRUE;
            goto try_another_magic;
         }
      }

      /* Now try giving the "interlocked" modifier. */

      if (test_call->callflagsf & ESCAPE_WORD__INTLK) {
         if (!intlkness) {
            intlkness = TRUE;
            goto try_another_intlk;
         }
      }

      /* Otherwise fall through and go on to the next call. */
   }

  try_again:
   /* Throw away garbage. */
   release_parse_blocks_to_mark(parse_mark);
   call_index++;
   if (call_index >= number_of_calls[call_list_any]) goto finished;
   test_call = main_call_lists[call_list_any][call_index];

   /* Set the selector (for "so-and-so advance to a column", etc) to "beaus".
      This seems to make most calls work -- note that "everyone run" and
      "no one advance to a column" are illegal.  If "beaus" doesn't work, we will
      try "ends" (for the call "fold"), "all", and finally "none" (for the call
      "run"), before giving up. */

   intlkness = FALSE;

   try_another_intlk:

   magicness = FALSE;

   try_another_magic:

   crossiness = FALSE;

   try_another_cross:

   number_for_initialize = 1;

   try_another_number:

   selector_for_initialize = selector_beaus;

   try_another_selector:

   /* Do the call.  An error will signal and go to try_again. */

   selector_used = FALSE;
   number_used = FALSE;
   mandatory_call_used = FALSE;

   history_ptr = 1;

   history[history_ptr].centersp = 0;
   history[history_ptr].state = *test_setup;
   initialize_parse();

   /* If the call has the "rolldefine" schema, we accept it, since the test setups
      are all in the "roll unsupported" state. */

   if (test_call->schema == schema_roll) goto accept;

   /* If the call has the "matrix" schema, and it is sex-dependent, we accept it,
      since the test setups that we use might have people placed in such a way
      that something like "1/2 truck" is illegal. */

   if (test_call->schema == schema_matrix &&
       test_call->stuff.matrix.stuff[0] != test_call->stuff.matrix.stuff[1])
      goto accept;

   /* We also accept "<ATC> your neighbor" and "<ANYTHING> motivate" calls,
      since we don't know what the tagging call will be. */
   if (test_call->callflagsf & (CFLAGH__TAG_CALL_RQ_MASK | CFLAGH__CIRC_CALL_RQ_BIT)) goto accept;

   if (crossiness)
      (void) deposit_concept(&concept_descriptor_table[cross_concept_index]);

   if (magicness)
      (void) deposit_concept(&concept_descriptor_table[magic_concept_index]);

   if (intlkness)
      (void) deposit_concept(&concept_descriptor_table[intlk_concept_index]);

   if (deposit_call(test_call, &null_options)) goto try_again;
   toplevelmove();

   /* It seems to have worked, save it.  We don't care about warnings here. */

  accept:
   global_temp_call_list[global_callcount] = test_call;
   global_callcount++;
   goto try_again;

  finished:

   /* Restore the global error handler. */

   longjmp_ptr = &longjmp_buffer;

   /* Create the call list itself. */

   number_of_calls[cl] = global_callcount;
   main_call_lists[cl] = (callspec_block **) get_mem(global_callcount * sizeof(callspec_block *));
   for (i=0; i < global_callcount; i++) {
      main_call_lists[cl][i] = global_temp_call_list[i];
   }

   /* Create the menu for it. */

   uims_create_menu(cl);
}


/* Definition: a region of the array "a" is said to have the "heap" property
   (or "be a heap") if:
      Whenever a[i] and a[2i+1] are both in the region, a[i] >= a[2i+1], and
      Whenever a[i] and a[2i+2] are both in the region, a[i] >= a[2i+2].

   Two special properties are noteworthy:
      1: If hi <= 2*lo, the region from lo to hi inclusive is automatically a heap,
         since there are no such pairs lying in the region.
      2: If the region that is a heap is [0..hi], a[0] is greater than or equal to
         every other element of the region.  Why?  Because a[0] >= a[1..2],
         and a[1] >= a[3..4] while a[2] >= a[5..6], so a[1] and a[2]
         are collectively >= a[3..6], which are >= a[7..14], etc. */

         /* Heapify causes the region between lo-1 and hi-1, inclusive, to be a heap, given that the
            region from lo to hi-1 was already a heap.  It works as follows:  when we declare
            "lo-1" to be part of the region, the only way it can fail to be a heap is if a[lo-1]
            is too small -- it might be less than a[2*lo-1] or a[2*lo].  If this is the case,
            we swap it with the larger of a[2*lo-1] and a[2*lo-1].  Now whatever we swapped it
            with got smaller, so it might fail to meet the heap property with respect to the
            elements farther down the line, so we repeat the process until we are off the end
            of the region. */


/* This pointer to a call array is used by the heapsort routine. */

Private callspec_block **the_array;


Private long_boolean callcompare(callspec_block *x, callspec_block *y)
{
   char *m = x->name;
   char *n = y->name;

   for (;;) {
      char mc = *m;
      char nc = *n;

      /* First, skip over everything that we need to, in both m and n.  This includes blanks, hyphens, and
         insignificant escape sequences.
         Not sure about the continuing accuracy or helpfulness of the comments just below. */

      /* We need to ignore blanks; otherwise "@6 run" will come out at the beginning of the list instead of under "r". */
      /* And hyphens too, so that "1-3-2 quarter the alter" will be listed under "q" (or whatever) rather than "-". */

      /* The current order is:
         <ATC>        (-8)
         <DIRECTION>  (-7)
         <ANYONE>     (-6)
         <ANYCIRC>    (-5)
         <ANYTHING>   (-4)
         <N>          (-3)
         <N/4>        (-2)
         <Nth>        (-1) */

      /* First, skip blanks and hyphens, in both m and n. */

      if (mc == ' ' || mc == '-') { m++; continue; }
      else if (nc == ' ' || nc == '-') { n++; continue; }

      /* Next, skip elided stuff in the "m" stream. */

      if (mc == '@') {
         mc = *++m;

         switch (mc) {
            case 'v': case 'w': case 'x': case 'y':
               mc = -8; break;
            case 'h':
               mc = -7; break;
            case '6': case 'k':
               mc = -6; break;
            case 'N':
               mc = -5; break;
            case '0': case 'm':
               mc = -4; break;
            case '9':
               mc = -3; break;
            case 'a': case 'b': case 'B': case 'D':
               mc = -2; break;
            case 'u':
               mc = -1; break;
            case '7': case 'n': case 'j': case 'J': case 'E':
               /* Skip over @7...@8, @n .. @o, and @j...@l stuff. */
               while (*m++ != '@');
               /* FALL THROUGH!!!!! */
            default:
               m++;
               continue;
         }
      }

      /* And in the "n" stream. */

      if (nc == '@') {
         nc = *++n;

         switch (nc) {
            case 'v': case 'w': case 'x': case 'y':
               nc = -8; break;
            case 'h':
               nc = -7; break;
            case '6': case 'k':
               nc = -6; break;
            case 'N':
               nc = -5; break;
            case '0': case 'm':
               nc = -4; break;
            case '9':
               nc = -3; break;
            case 'a': case 'b': case 'B': case 'D':
               nc = -2; break;
            case 'u':
               nc = -1; break;
            case '7': case 'n': case 'j': case 'J': case 'E':
               /* Skip over @7...@8, @n .. @o, and @j...@l stuff. */
               while (*n++ != '@');
               /* FALL THROUGH!!!!! */
            default:
               n++;
               continue;
         }
      }

      if (!mc) return TRUE;
      else if (!nc) return FALSE;
      else if (mc != nc) return (mc < nc);
      else { m++; n++; }
   }
}

Private void heapify(int lo, int hi)
{
   int j = lo-1;

   for (;;) {
      callspec_block *temp;
      int k = j*2+1;

      if (k+1 > hi) return;
      if (k+1 < hi) {
         if (callcompare(the_array[k], the_array[k+1])) k++;
      }
      if (callcompare(the_array[k], the_array[j])) return;
      temp = the_array[j];
      the_array[j] = the_array[k];
      the_array[k] = temp;
      j = k;
   }
}



Private void heapsort(int n)
{
   int i;

   /* First, turn the whole array into a heap, building downward from the top, since adding
      one more item at the bottom is what heapify is good at.  We don't start calling heapify
      until the low limit is n/2, since heapify wouldn't have anything to do until then. */

   for (i=n/2; i>0; i--) heapify(i, n);

   /* Now we use the property that a[0] has the largest element.  We pull that out and move it
      to the end.  We declare that item to no longer be part of the region we are interested in.
      Since we have changed a[0], we call heapify to repair the damage, on the smaller region.
      We repeat this, pulling out the largest element of the remaining heap (which is always
      element 0), and letting the heap shrink down to nothing. */

   for (i=n; i>1; i--) {
      callspec_block *temp;

      temp = the_array[0];
      the_array[0] = the_array[i-1];
      the_array[i-1] = temp;
      heapify(1, i-1);
   }
}


Private void create_misc_call_lists(call_list_kind cl)
{
   int j;
   long_boolean accept_it;
   int i, callcount;

   callcount = 0;

   for (j=0; j<number_of_calls[call_list_any]; j++) {
      callspec_block *callp = main_call_lists[call_list_any][j];
      callspec_block *callq = callp;

      accept_it = FALSE;

      if (cl == call_list_gcol) {     /* GCOL */
         if (callp->schema != schema_by_array) accept_it = TRUE;
         else if (callp->callflags1 & CFLAG1_STEP_TO_WAVE) {
            if (  assoc(b_4x2, (setup *) 0, callp->stuff.arr.def_list->callarray_list) ||
                  assoc(b_4x1, (setup *) 0, callp->stuff.arr.def_list->callarray_list) ||
                  assoc(b_2x2, (setup *) 0, callp->stuff.arr.def_list->callarray_list) ||
                  assoc(b_2x1, (setup *) 0, callp->stuff.arr.def_list->callarray_list))
               accept_it = TRUE;
         }
         else {
            if (  assoc(b_8x1, (setup *) 0, callp->stuff.arr.def_list->callarray_list) ||
                  assoc(b_4x1, (setup *) 0, callp->stuff.arr.def_list->callarray_list) ||
                  assoc(b_2x1, (setup *) 0, callp->stuff.arr.def_list->callarray_list) ||
                  assoc(b_1x1, (setup *) 0, callp->stuff.arr.def_list->callarray_list))
               accept_it = TRUE;
         }
      }
      else {      /* QTAG */

         /* Special stuff: We try to make "mix" not legal, while "swing and circle <N/4>" is legal. */
/* Unfortunately, this makes lots of regression tests fail, because things that used to be parsible
but not executable are now not parsable.  So we take it out.
         if (callp->schema == schema_sequential && !(callp->callflags1 & CFLAG1_REAR_BACK_FROM_QTAG) && callp->stuff.def.howmanyparts >= 1) {
            callq = base_calls[callp->stuff.def.defarray[0].call_id];
         }
*/
         if (callq->schema != schema_by_array) accept_it = TRUE;
         else if ((callp->callflags1 & CFLAG1_STEP_REAR_MASK) == CFLAG1_REAR_BACK_FROM_QTAG) {
            if (  assoc(b_4x2, (setup *) 0, callq->stuff.arr.def_list->callarray_list) ||
                  assoc(b_4x1, (setup *) 0, callq->stuff.arr.def_list->callarray_list))
               accept_it = TRUE;
         }
         else {
            if (assoc(b_qtag, (setup *) 0, callq->stuff.arr.def_list->callarray_list) ||
                  assoc(b_pqtag, (setup *) 0, callq->stuff.arr.def_list->callarray_list) ||
                  assoc(b_dmd, (setup *) 0, callq->stuff.arr.def_list->callarray_list) ||
                  assoc(b_pmd, (setup *) 0, callq->stuff.arr.def_list->callarray_list) ||
                  assoc(b_1x2, (setup *) 0, callq->stuff.arr.def_list->callarray_list) ||
                  assoc(b_2x1, (setup *) 0, callq->stuff.arr.def_list->callarray_list))
               accept_it = TRUE;
         }
      }

      if (accept_it) {
         global_temp_call_list[callcount] = callp;
         callcount++;
      }
   }

   /* Create the call list itself. */

   number_of_calls[cl] = callcount;
   main_call_lists[cl] = (callspec_block **) get_mem(callcount * sizeof(callspec_block *));

   for (i=0; i < callcount; i++) {
      main_call_lists[cl][i] = global_temp_call_list[i];
   }

   /* Create the menu for it. */
   uims_create_menu(cl);
}


/* These are used by the database reading stuff. */

static uint32 last_datum, last_12;
static callspec_block *call_root;
static callarray *tp;
/* This shows the highest index we have seen so far.  It must never exceed max_base_calls-1. */
static int highest_base_call;


Private void read_halfword(void)
{
   last_datum = read_16_from_database();
   last_12 = last_datum & 0xFFF;
}


Private void read_fullword(void)
{
   uint32 t = read_16_from_database();
   last_datum = t << 16 | read_16_from_database();
}



/* Found an error while reading CALL out of the database.
   Print an error message and quit.
   Should take the call as an argument, but since this entire file uses global variables,
   we will, too. */

Private void database_error(char *message)
{
   uims_database_error(message, call_root ? call_root->name : 0);
   exit_program(1);
}


Private void read_level_3_groups(calldef_block *where_to_put)
{
   int j, char_count;
   callarray *current_call_block;

   if ((last_datum & 0xE000) != 0x6000)
      database_error("database phase error 3");

   current_call_block = 0;

   while ((last_datum & 0xE000) == 0x6000) {
      begin_kind this_start_setup;
      unsigned short this_qualifierstuff;
      call_restriction this_restriction;
      setup_kind end_setup;
      setup_kind end_setup_out;
      int this_start_size;
      uint32 these_flags;
      int extra;

      these_flags = (last_datum & 0x1FFF) << 8;    /* We allow 21 callarray_flags. */
      read_halfword();       /* Get callaray continuation and start setup. */
      these_flags |= ((last_datum & 0xFF00) >> 8);
      this_start_setup = (begin_kind) (last_datum & 0xFF);
      this_start_size = begin_sizes[this_start_setup];

      read_halfword();       /* Get qualifier stuff. */
      this_qualifierstuff = (uint16) last_datum;
      read_halfword();       /* Get restriction and end setup. */
      this_restriction = (call_restriction) ((last_datum & 0xFF00) >> 8);
      end_setup = (setup_kind) (last_datum & 0xFF);

      if (these_flags & CAF__CONCEND) {      /* See if "concendsetup" was used. */
         read_halfword();       /* Get outer setup and outer rotation. */
         end_setup_out = (setup_kind) (last_datum & 0xFF);
      }

      if (these_flags & CAF__PREDS) {
         read_halfword();    /* Get error message count. */
         char_count = last_datum & 0xFF;
         /* We will naturally get 4 items in the "stuff.prd.errmsg" field; we are responsible all for the others. */
         /* We subtract 3 because 4 chars are already present, but we need one extra for the pad. */
         extra = (char_count-3) * sizeof(char);
      }
      else {
         /* We will naturally get 4 items in the "stuff.def" field; we are responsible all for the others. */
         extra = (this_start_size-4) * sizeof(unsigned short int);
      }

      tp = (callarray *) get_mem(sizeof(callarray) + extra);
      tp->next = 0;

      if (!current_call_block)
         where_to_put->callarray_list = tp;   /* First time. */
      else {
         current_call_block->next = tp;       /* Subsequent times. */
      }

      current_call_block = tp;
      tp->callarray_flags = these_flags;
      tp->qualifierstuff = this_qualifierstuff;
      tp->start_setup = (uint8) this_start_setup;
      tp->restriction = this_restriction;

      if (these_flags & CAF__CONCEND) {      /* See if "concendsetup" was used. */
         tp->end_setup = (uint8) s_normal_concentric;
         tp->end_setup_in = (uint8) end_setup;
         tp->end_setup_out = (uint8) end_setup_out;
      }
      else {
         tp->end_setup = (uint8) end_setup;
      }

      if (these_flags & CAF__PREDS) {
         predptr_pair *temp_predlist;
         predptr_pair *this_predlist = (predptr_pair *) 0;

         /* Read error message text. */

         for (j=1; j <= ((char_count+1) >> 1); j++) {
            read_halfword();
            tp->stuff.prd.errmsg[(j << 1)-2] = (char) ((last_datum >> 8) & 0xFF);
            if ((j << 1) != char_count+1) tp->stuff.prd.errmsg[(j << 1)-1] = (char) (last_datum & 0xFF);
         }

         tp->stuff.prd.errmsg[char_count] = '\0';

         read_halfword();

         /* Demand level 4 group. */
         if (last_datum != 0x8000) {
            database_error("database phase error 4");
         }

         while ((last_datum & 0xE000) == 0x8000) {
            read_halfword();       /* Read predicate indicator. */
            /* "predptr_pair" will get us 4 items in the "arr" field; we are responsible all for the others. */
            temp_predlist = (predptr_pair *) get_mem(sizeof(predptr_pair) +
                    (this_start_size-4) * sizeof(unsigned short));
            temp_predlist->pred = &pred_table[last_datum];
            /* If this call uses a predicate that takes a selector, flag the call so that
               we will query the user for that selector. */

            if ((int) last_datum < selector_preds)
               call_root->callflagsf |= CFLAGH__REQUIRES_SELECTOR;

            for (j=0; j < this_start_size; j++) {
               read_halfword();
               temp_predlist->arr[j] = (uint16) last_datum;
            }

            temp_predlist->next = this_predlist;
            this_predlist = temp_predlist;
            read_halfword();    /* Get next level 4 header, or whatever. */
         }

         /* need to reverse stuff in "this_predlist" */
         temp_predlist = 0;
         while (this_predlist) {
            predptr_pair *revptr = this_predlist;
            this_predlist = this_predlist->next;
            revptr->next = temp_predlist;
            temp_predlist = revptr;
         }

         tp->stuff.prd.predlist = temp_predlist;
      }
      else {
         for (j=0; j < this_start_size; j++) {
            read_halfword();
            tp->stuff.def[j] = (uint16) last_datum;
         }
         read_halfword();
      }
   }
}


Private void check_tag(int tag)
{
   if (tag >= max_base_calls)
      database_error("Too many tagged calls -- mkcalls made an error.");
   if (tag > highest_base_call) highest_base_call = tag;
}


Private void read_in_call_definition(void)
{
   int j;
   int lim = 8;
   uint32 left_half;

   read_halfword();

   switch (call_root->schema) {
      case schema_nothing:
         break;
      case schema_roll:
         break;
      case schema_matrix:
         lim = 2;
         /* !!!! FALL THROUGH !!!! */
      case schema_partner_matrix:
         /* !!!! FELL THROUGH !!!! */
         left_half = last_datum;
         read_halfword();
         call_root->stuff.matrix.flags = ((left_half & 0xFFFF) << 16) | (last_datum & 0xFFFF);

         if (call_root->stuff.matrix.flags & MTX_USE_SELECTOR)
            call_root->callflagsh |= CFLAGH__REQUIRES_SELECTOR;
         if (call_root->stuff.matrix.flags & MTX_USE_NUMBER)
            call_root->callflags1 |= CFLAG1_NUMBER_BIT;

         call_root->stuff.matrix.stuff = (uint32 *) get_mem(sizeof(uint32)*8);

         for (j=0; j<lim; j++) {
            uint32 firstpart;

            read_halfword();

            firstpart = last_datum & 0xFFFF;

            if (firstpart) {
               read_halfword();
               call_root->stuff.matrix.stuff[j] = firstpart | ((last_datum & 0xFFFF) << 16);
            }
            else {
               call_root->stuff.matrix.stuff[j] = 0;
            }               
         }

         read_halfword();
         break;
      case schema_by_array:
         {
            calldef_block *zz, *yy;

            zz = (calldef_block *) get_mem(sizeof(calldef_block));
            zz->next = 0;
            zz->modifier_seth = 0;
            zz->modifier_level = l_mainstream;
            call_root->stuff.arr.def_list = zz;

            read_level_3_groups(zz);    /* The first group. */

            while ((last_datum & 0xE000) == 0x4000) {
               yy = (calldef_block *) get_mem(sizeof(calldef_block));
               zz->next = yy;
               zz = yy;
               zz->modifier_level = (dance_level) (last_datum & 0xFF);
               zz->next = 0;
               read_fullword();
               zz->modifier_seth = last_datum;
               read_halfword();
               read_level_3_groups(zz);
            }
         }
         break;
      case schema_sequential:
      case schema_split_sequential:
      case schema_sequential_with_fraction:
      case schema_sequential_with_split_1x8_id:
         {
            by_def_item templist[100];
            int next_definition_index = 0;

            /* Demand a level 2 group. */
            if ((last_datum & 0xE000) != 0x4000)
               database_error("database phase error 6");

            while ((last_datum & 0xE000) == 0x4000) {
               check_tag(last_12);
               templist[next_definition_index].call_id = (uint16) last_12;
               read_fullword();
               templist[next_definition_index].modifiers1 = (defmodset) last_datum;
               read_fullword();
               templist[next_definition_index++].modifiersh = (defmodset) last_datum;
               read_halfword();
            }

            call_root->stuff.def.howmanyparts = next_definition_index;
            call_root->stuff.def.defarray = (by_def_item *) get_mem((next_definition_index) * sizeof(by_def_item));

            while (--next_definition_index >= 0)
               call_root->stuff.def.defarray[next_definition_index] = templist[next_definition_index];
         }
         break;
      default:          /* These are all the variations of concentric. */
         /* Demand a level 2 group. */
         if ((last_datum & 0xE000) != 0x4000)
            database_error("database phase error 7");

         check_tag(last_12);
         call_root->stuff.conc.innerdef.call_id = (uint16) last_12;
         read_fullword();
         call_root->stuff.conc.innerdef.modifiers1 = (defmodset) last_datum;
         read_fullword();
         call_root->stuff.conc.innerdef.modifiersh = (defmodset) last_datum;
         read_halfword();
         check_tag(last_12);
         call_root->stuff.conc.outerdef.call_id = (uint16) last_12;
         read_fullword();
         call_root->stuff.conc.outerdef.modifiers1 = (defmodset) last_datum;
         read_fullword();
         call_root->stuff.conc.outerdef.modifiersh = (defmodset) last_datum;
         read_halfword();
         break;
   }
}



/* This fills the permanent array "main_call_lists[call_list_any]" with the stuff
      read in from the database, including name pointer fields containing the original text
      with "@" escapes.  It also sets "number_of_calls[call_list_any]" to the size thereof.
         It does this only for calls that are legal at this level.  If we are just
         writing a call list, a call is legal only if its level is exactly the level
         specified when the program was invoked, rather than the usual case of including
         any call whose level is at or below the specified level.  If we are doing a
         "write full" we do the usual action.
   It also fills in the "base_calls" array with tagged calls, independent of level. */

extern void build_database(call_list_mode_t call_list_mode)
{
   int i, j, char_count;
   calldef_schema call_schema;
   int local_callcount;
   char *np, c;
   dance_level this_level;
   callspec_block **local_call_list;
   dance_level acceptable_level = calling_level;

   uims_database_tick_max(10*16);

   if (call_list_mode == call_list_mode_none)
      acceptable_level = higher_acceptable_level[calling_level];

   for (i=0 ; i<NUM_TAGGER_CLASSES ; i++) {
      number_of_taggers[i] = 0;
      tagger_calls[i] = (callspec_block **) 0;
   }

   number_of_circcers = 0;
   circcer_calls = (callspec_block **) 0;

   /* This list will be permanent. */
   base_calls = (callspec_block **) get_mem(max_base_calls * sizeof(callspec_block *));

   /* These two will be temporary.  The first lasts through the entire initialization
      process.  The second one only in this procedure. */
   global_temp_call_list = (callspec_block **) get_mem(abs_max_calls * sizeof(callspec_block *));
   local_call_list = (callspec_block **) get_mem(abs_max_calls * sizeof(callspec_block *));

   /* Clear the tag list.  Calls will fill this in as they announce themselves. */
   for (i=0; i < max_base_calls; i++) base_calls[i] = (callspec_block *) 0;

   highest_base_call = 0;

   read_halfword();

   local_callcount = 0;

   for (;;) {
      int savetag;
      uint32 saveflags1, saveflags2, saveflagsh;

      if ((last_datum & 0xE000) == 0) break;

      if ((last_datum & 0xE000) != 0x2000) {
         database_error("database phase error 1");
      }

      savetag = last_12;     /* Get tag, if any. */

      read_halfword();       /* Get level and 8 bits of "callflags2" stuff. */
      this_level = (dance_level) (last_datum & 0xFF);
      saveflags2 = last_datum >> 8;

      read_fullword();       /* Get top level flags, first word.
                                This is the "callflags1" stuff. */
      saveflags1 = last_datum;

      read_fullword();       /* Get top level flags, second word.
                                This is the "heritflags" stuff. */
      saveflagsh = last_datum;

      read_halfword();       /* Get char count and schema. */
      call_schema = (calldef_schema) (last_datum & 0xFF);
      char_count = (last_datum >> 8) & 0xFF;

      /* **** We should only allocate the call root if we are going to use this call,
         either because it is tagged or because it is legal at this level!!!! */

      /* Now that we know how long the name is, create the block and fill in the saved stuff. */
      /* We subtract 3 because 4 chars are already present, but we need one extra for the pad. */

      call_root = (callspec_block *) get_mem(sizeof(callspec_block) + char_count - 3);
      call_root->menu_name = (Cstring) 0;
      np = call_root->name;

      if (savetag) {
         check_tag(savetag);
         base_calls[savetag] = call_root;
      }

      call_root->age = 0;
      call_root->level = (int) this_level;
      call_root->schema = call_schema;
      call_root->callflags1 = saveflags1;
      call_root->callflagsf = saveflags2 << 24;    /* Will get "CFLAGH" and "ESCAPE_WORD"
                                                      bits later. */
      call_root->callflagsh = saveflagsh;
      /* If we are operating at the "all" level, make fractions visible everywhere, to aid in debugging. */
      if (calling_level == l_dontshow) call_root->callflags1 |= 3*CFLAG1_VISIBLE_FRACTION_BIT;

      /* Now read in the name itself. */

      for (j=0; j<char_count; j++)
         *np++ = (char) read_8_from_database();

      *np = '\0';

      /* Scan the name for "@6" or "@e", fill in "needselector" or
         "left_changes_name" flag if found. */

      np = call_root->name;

      while ((c = *np++)) {
         if (c == '@') {
            if ((c = *np++) == '6' || c == 'k')
               call_root->callflagsf |= CFLAGH__REQUIRES_SELECTOR;
            else if (c == 'h')
               call_root->callflagsf |= CFLAGH__REQUIRES_DIRECTION;
            else if (c == 'D')
               call_root->callflagsf |= CFLAGH__ODD_NUMBER_ONLY;
            else if (c == 'v')
               call_root->callflagsf |= (CFLAGH__TAG_CALL_RQ_BIT*1);
            else if (c == 'w')
               call_root->callflagsf |= (CFLAGH__TAG_CALL_RQ_BIT*2);
            else if (c == 'x')
               call_root->callflagsf |= (CFLAGH__TAG_CALL_RQ_BIT*3);
            else if (c == 'y')
               call_root->callflagsf |= (CFLAGH__TAG_CALL_RQ_BIT*4);
            else if (c == 'N')
               call_root->callflagsf |= CFLAGH__CIRC_CALL_RQ_BIT;
         }
      }

      read_in_call_definition();

      /* We accept a call if:
         (1) we are writing out just this list, and the call matches the desired level exactly, or
         (2) we are writing out this list and those below, and the call is <= the desired level, or
         (3) we are running, and the call is <= the desired level or the "higher acceptable level".
            The latter is c3x if the desired level is c3, or c4x if the desired level is c4.
            That way, c3x calls will be included.  We will print a warning if they are used. */

      if (     this_level == calling_level ||
               (call_list_mode != call_list_mode_writing && this_level <= acceptable_level)) {

         /* Process tag base calls specially. */
         if (call_root->callflags1 & CFLAG1_BASE_TAG_CALL_MASK) {
            int tagclass = ((call_root->callflags1 & CFLAG1_BASE_TAG_CALL_MASK) / CFLAG1_BASE_TAG_CALL_BIT) - 1;

            /* All classes go into list 0.  Additionally, the other classes go into their own list. */
            number_of_taggers[tagclass]++;
            tagger_calls[tagclass] = get_more_mem(tagger_calls[tagclass], number_of_taggers[tagclass]*sizeof(callspec_block *));
            tagger_calls[tagclass][number_of_taggers[tagclass]-1] = call_root;
            if (tagclass != 0) {
               number_of_taggers[0]++;
               tagger_calls[0] = get_more_mem(tagger_calls[0], number_of_taggers[0]*sizeof(callspec_block *));
               tagger_calls[0][number_of_taggers[0]-1] = call_root;
            }
            else if (call_root->callflagsf & CFLAGH__TAG_CALL_RQ_MASK) {
               /* But anything that invokes a tagging call goes into each list,
                  inheriting its own class. */
               int xxx;

               /* Iterate over all tag classes except class 0. */
               for (xxx=1 ; xxx<NUM_TAGGER_CLASSES ; xxx++) {
                  callspec_block *new_call =
                     (callspec_block *) get_mem(sizeof(callspec_block) + char_count - 3);
                  (void) memcpy(new_call, call_root, sizeof(callspec_block) + char_count - 3);
                  /* Fix it up. */
                  new_call->callflagsf =
                     (new_call->callflagsf & !CFLAGH__TAG_CALL_RQ_MASK) |
                     CFLAGH__TAG_CALL_RQ_BIT*(xxx+1);
                  number_of_taggers[xxx]++;
                  tagger_calls[xxx] =
                     get_more_mem(tagger_calls[xxx],
                                  number_of_taggers[xxx]*sizeof(callspec_block *));
                  tagger_calls[xxx][number_of_taggers[xxx]-1] = new_call;
               }
            }
         }
         else {
            /* But circ calls are treated normally, as well as being put on the special list. */
            if (call_root->callflags1 & CFLAG1_BASE_CIRC_CALL) {
               number_of_circcers++;
               circcer_calls =
                  get_more_mem(circcer_calls, number_of_circcers*sizeof(callspec_block *));
               circcer_calls[number_of_circcers-1] = call_root;
            }
            if (local_callcount >= abs_max_calls)
               database_error("Too many base calls -- mkcalls made an error.");
            local_call_list[local_callcount++] = call_root;
         }
      }
   }

   number_of_calls[call_list_any] = local_callcount;
   main_call_lists[call_list_any] = (callspec_block **) get_mem(local_callcount * sizeof(callspec_block *));
   for (i=0; i < local_callcount; i++) {
      main_call_lists[call_list_any][i] = local_call_list[i];
   }
   free_mem(local_call_list);

   for (i=1; i <= highest_base_call; i++) {
      if (!base_calls[i]) {
         print_id_error(i);
         exit_program(1);
      }
   }

   close_database();
   uims_database_tick(10);
}



extern void initialize_menus(call_list_mode_t call_list_mode)
{
   uint32 escape_bit_junk;
   uint32 uj;
   int i, j;

   /* Make the translated names for all calls and concepts.  These have the "<...>"
      phrases, suitable for external display on menus, instead of "@" escapes. */

   for (i=0; i<number_of_calls[call_list_any]; i++)
      main_call_lists[call_list_any][i]->menu_name =
         translate_menu_name(main_call_lists[call_list_any][i]->name,
                             &main_call_lists[call_list_any][i]->callflagsf);

   for (i=0 ; i<NUM_TAGGER_CLASSES ; i++) {
      for (uj=0; uj<number_of_taggers[i]; uj++)
         tagger_calls[i][uj]->menu_name =
            translate_menu_name(tagger_calls[i][uj]->name, &tagger_calls[i][uj]->callflagsf);
   }

   for (uj=0; uj<number_of_circcers; uj++)
      circcer_calls[uj]->menu_name =
         translate_menu_name(circcer_calls[uj]->name, &circcer_calls[uj]->callflagsf);

   /* Do the base calls (calls that are used in definitions of other calls).  These may have
      already been done, if they were on the level. */
   for (i=1; i <= highest_base_call; i++) {
      if (!base_calls[i]->menu_name)
         base_calls[i]->menu_name =
            translate_menu_name(base_calls[i]->name, &base_calls[i]->callflagsf);
   }

   for (i=0; concept_descriptor_table[i].kind != marker_end_of_list; i++)
      concept_descriptor_table[i].menu_name =
         translate_menu_name(concept_descriptor_table[i].name, &escape_bit_junk);

   the_array = main_call_lists[call_list_any];
   heapsort(number_of_calls[call_list_any]);

   /* Now the permanent array "main_call_lists[call_list_any]" has all the legal calls,
         including name pointer fields containing the original text with "@" escapes,
         sorted alphabetically.
      The remaining tasks are to make the subcall lists for other setups (e.g.
         those calls legal from columns), and clean up the names that we will actually
         display in the menus (no "@" signs) and initialize the menus with the
         cleaned-up and subsetted text. */

   uims_database_tick(5);

   /* Do special stuff if we are reading or writing a call list file. */

   if (call_list_mode != call_list_mode_none) {
      if (call_list_mode == call_list_mode_abridging) {
         char abridge_call[100];
   
         while (read_from_call_list_file(abridge_call, 99)) {
            /* Remove the newline character. */
            abridge_call[strlen(abridge_call)-1] = '\0';
            /* Search through the call name list for this call.
               Why don't we use a more efficient search, based on the fact
               that the call list has been alphabetized?  Because it was
               alphabetized before the '@' escapes were expanded.  It
               is no longer in alphabetical order. */
            for (i=0; i<number_of_calls[call_list_any]; i++) {
               if (!strcmp(abridge_call, main_call_lists[call_list_any][i]->name)) {
                  /* Delete this call and move all subsequent calls down one position. */
                  for (j=i+1; j<number_of_calls[call_list_any]; j++) {
                     main_call_lists[call_list_any][j-1] = main_call_lists[call_list_any][j];
                  }
                  number_of_calls[call_list_any]--;
                  break;
               }
            }
         }
      }
      else {      /* Writing a list of some kind. */
         for (i=0; i<number_of_calls[call_list_any]; i++) {
            write_to_call_list_file(main_call_lists[call_list_any][i]->menu_name);
         }
      }

      /* Close the file. */
      if (close_call_list_file())
         exit_program(1);

      if (call_list_mode != call_list_mode_abridging)
         goto getout;   /* That's all! */
   }

   /* Now the array "main_call_lists[call_list_any]"
         has the stuff for the calls that we will actually use.
      The remaining tasks are to make the subcall lists for other setups (e.g.
         those calls legal from columns), and initialize the menus with the
         subsetted text. */

   /* This is the universal menu. */
   uims_create_menu(call_list_any);
   uims_database_tick(5);

   /* Create the special call menus for restricted setups. */

   test_starting_setup(call_list_1x8,  &test_setup_1x8);          /* RH grand wave */
   test_starting_setup(call_list_l1x8, &test_setup_l1x8);         /* LH grand wave */
   test_starting_setup(call_list_dpt,  &test_setup_dpt);          /* DPT */
   test_starting_setup(call_list_cdpt, &test_setup_cdpt);         /* completed DPT */
   test_starting_setup(call_list_rcol, &test_setup_rcol);         /* RCOL */
   test_starting_setup(call_list_lcol, &test_setup_lcol);         /* LCOL */
   test_starting_setup(call_list_8ch,  &test_setup_8ch);          /* 8CH */
   test_starting_setup(call_list_tby,  &test_setup_tby);          /* TBY */
   test_starting_setup(call_list_lin,  &test_setup_lin);          /* LIN */
   test_starting_setup(call_list_lout, &test_setup_lout);         /* LOUT */
   test_starting_setup(call_list_rwv,  &test_setup_rwv);          /* RWV */
   test_starting_setup(call_list_lwv,  &test_setup_lwv);          /* LWV */
   test_starting_setup(call_list_r2fl, &test_setup_r2fl);         /* R2FL */
   test_starting_setup(call_list_l2fl, &test_setup_l2fl);         /* L2FL */
   create_misc_call_lists(call_list_gcol);                        /* GCOL */
   create_misc_call_lists(call_list_qtag);                        /* QTAG */

   /* This was global to the initialization, but it goes away also. */
   free_mem(global_temp_call_list);

   /* Initialize the special empty call menu */

   main_call_lists[call_list_empty] = empty_menu;
   number_of_calls[call_list_empty] = 0;

   getout:

   uims_database_tick_end();
   return;
}
