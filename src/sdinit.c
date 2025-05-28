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

    This is for version 29. */

/* This defines the following functions:
   initialize_menus
*/

#include <string.h>
#include "sd.h"

/* Global to this file. */

/* These two get temporarily allocated.  They persist through the entire initialization. */
Private callspec_block **global_temp_call_list;
Private char **global_temp_call_name_list;
Private int global_callcount;     /* Index into the above. */

/* This contains the menu-presentable (e.g. "@" escapes turned into "<ANYONE>", etc.)
   text corresponding to "main_call_lists[call_list_any]", that is, the alphabetized
   universal menu.  It persists through the entire initialization. */
Private char **global_main_call_name_list;

#define SB (ID2_SIDE|ID2_BOY)
#define HB (ID2_HEAD|ID2_BOY)
#define SG (ID2_SIDE|ID2_GIRL)
#define HG (ID2_HEAD|ID2_GIRL)
#define WEST(x) (d_west|(x)<<6)
#define EAST(x) (d_east|(x)<<6)
#define NORT(x) (d_north|(x)<<6)
#define SOUT(x) (d_south|(x)<<6)

/* In all of these setups in which people are facing, they are normal couples.  This makes initialization of things like star thru,
   ladies chain, and curlique work. */
Private setup test_setup_1x8  = {s1x8, 0, {{NORT(6), SB}, {SOUT(5), HG}, {SOUT(4), HB}, {NORT(7), SG}, {SOUT(2), SB}, {NORT(1), HG}, {NORT(0), HB}, {SOUT(3), SG}}, 0};
Private setup test_setup_l1x8 = {s1x8, 0, {{SOUT(6), SB}, {NORT(5), HG}, {NORT(4), HB}, {SOUT(7), SG}, {NORT(2), SB}, {SOUT(1), HG}, {SOUT(0), HB}, {NORT(3), SG}}, 0};
Private setup test_setup_dpt  = {s2x4, 0, {{EAST(6), SB}, {EAST(4), HB}, {WEST(5), HG}, {WEST(7), SG}, {WEST(2), SB}, {WEST(0), HB}, {EAST(1), HG}, {EAST(3), SG}}, 0};
Private setup test_setup_cdpt = {s2x4, 0, {{WEST(7), SG}, {WEST(5), HG}, {EAST(4), HB}, {EAST(6), SB}, {EAST(3), SG}, {EAST(1), HG}, {WEST(0), HB}, {WEST(2), SB}}, 0};
Private setup test_setup_rcol = {s2x4, 0, {{EAST(6), SB}, {EAST(5), HG}, {EAST(4), HB}, {EAST(7), SG}, {WEST(2), SB}, {WEST(1), HG}, {WEST(0), HB}, {WEST(3), SG}}, 0};
Private setup test_setup_lcol = {s2x4, 0, {{WEST(6), SB}, {WEST(5), HG}, {WEST(4), HB}, {WEST(7), SG}, {EAST(2), SB}, {EAST(1), HG}, {EAST(0), HB}, {EAST(3), SG}}, 0};
Private setup test_setup_8ch  = {s2x4, 0, {{EAST(6), SB}, {WEST(5), HG}, {EAST(4), HB}, {WEST(7), SG}, {WEST(2), SB}, {EAST(1), HG}, {WEST(0), HB}, {EAST(3), SG}}, 0};
Private setup test_setup_tby  = {s2x4, 0, {{WEST(5), HG}, {EAST(6), SB}, {WEST(7), SG}, {EAST(4), HB}, {EAST(1), HG}, {WEST(2), SB}, {EAST(3), SG}, {WEST(0), HB}}, 0};
Private setup test_setup_lin  = {s2x4, 0, {{SOUT(5), HG}, {SOUT(6), SB}, {SOUT(7), SG}, {SOUT(4), HB}, {NORT(1), HG}, {NORT(2), SB}, {NORT(3), SG}, {NORT(0), HB}}, 0};
Private setup test_setup_lout = {s2x4, 0, {{NORT(6), SB}, {NORT(5), HG}, {NORT(4), HB}, {NORT(7), SG}, {SOUT(2), SB}, {SOUT(1), HG}, {SOUT(0), HB}, {SOUT(3), SG}}, 0};
Private setup test_setup_rwv  = {s2x4, 0, {{NORT(6), SB}, {SOUT(5), HG}, {NORT(4), HB}, {SOUT(7), SG}, {SOUT(2), SB}, {NORT(1), HG}, {SOUT(0), HB}, {NORT(3), SG}}, 0};
Private setup test_setup_lwv  = {s2x4, 0, {{SOUT(6), SB}, {NORT(5), HG}, {SOUT(4), HB}, {NORT(7), SG}, {NORT(2), SB}, {SOUT(1), HG}, {NORT(0), HB}, {SOUT(3), SG}}, 0};
Private setup test_setup_r2fl = {s2x4, 0, {{NORT(6), SB}, {NORT(5), HG}, {SOUT(4), HB}, {SOUT(7), SG}, {SOUT(2), SB}, {SOUT(1), HG}, {NORT(0), HB}, {NORT(3), SG}}, 0};
Private setup test_setup_l2fl = {s2x4, 0, {{SOUT(6), SB}, {SOUT(5), HG}, {NORT(4), HB}, {NORT(7), SG}, {NORT(2), SB}, {NORT(1), HG}, {SOUT(0), HB}, {SOUT(3), SG}}, 0};





/* This cleans up the text of the alphabetized main call list in
   "main_call_lists[call_list_any]", putting the menu-presentable form
   into global_main_call_name_list, from which various subsets will be picked out
   for the menus.  It simply re-uses the stored string where it can, and allocates
   fresh memory if a substitution took place. */

Private void create_call_name_list(void)
{
   int i, j;
   char *name_ptr;
   long_boolean atsign;
   char c;
   int namelength;

   for (i=0; i<number_of_calls[call_list_any]; i++) {
      name_ptr = main_call_lists[call_list_any][i]->name;
      atsign = FALSE;

      /* See if the name has an 'at' sign, in which case we have to modify it to
         get the actual menu text. */

      namelength = 0;
      for (;;) {
         c = name_ptr[namelength];
         if (!c) break;
         else if (c == '@') atsign = TRUE;
         namelength++;
      }

      if (atsign) {
         char tempname[200];
         char *temp_ptr;
         int templength;

         temp_ptr = tempname;
         templength = 0;

         for (j = 0; j < namelength; j++) {
            c = name_ptr[j];

            if (c == '@') {
               j++;
               c = name_ptr[j];
               if (c == '0' || c == 'm') {
                  char *p;

                  p = temp_ptr+templength;
                  string_copy(&p, "<ANYTHING>");
                  templength = p - temp_ptr;
               }
               else if (c == '6' || c == 'k') {
                  char *p;

                  p = temp_ptr+templength;
                  string_copy(&p, "<ANYONE>");
                  templength = p - temp_ptr;
               }
               else if (c == 'h') {
                  char *p;

                  p = temp_ptr+templength;
                  string_copy(&p, "<DIRECTION>");
                  templength = p - temp_ptr;
               }
               else if (c == '9') {
                  char *p;

                  p = temp_ptr+templength;
                  string_copy(&p, "<N>");
                  templength = p - temp_ptr;
               }
               else if (c == 'a' || c == 'b') {
                  char *p;

                  p = temp_ptr+templength;
                  string_copy(&p, "<N/4>");
                  templength = p - temp_ptr;
               }
               else if (c == '7' || c == 'n' || c == 'j') {
                  /* Skip over @7...@8, @n .. @o, and @j...@l stuff. */
                  while (name_ptr[j] != '@') j++;
                  j++;
               }
            }
            else if (c != ' ' || templength == 0 || temp_ptr[templength-1] != ' ')
               temp_ptr[templength++] = c;
         }

         tempname[templength] = '\0';
         /* Must copy the text into some fresh memory, being careful about overflow. */
         name_ptr = get_mem(templength+1);
         for (j=0; j<=templength; j++) name_ptr[j] = tempname[j];
      }

      global_main_call_name_list[i] = name_ptr;
   }
}


Private void create_menu(call_list_kind cl, char *call_name_list[])
{
   int i;

   for (i=0; i<number_of_calls[cl]; i++) {
      uims_add_call_to_menu(cl, i, call_name_list[i]);
   }
   uims_finish_call_menu(cl, menu_names[cl]);
}


/* These variables are actually local to test_starting_setup, but they are
   expected to be preserved across the longjmp, so they must be static. */
Private parse_block *parse_mark;
Private int call_index;


Private void test_starting_setup(call_list_kind cl, setup test_setup)
{
   callspec_block *test_call;
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

      /* A call failed.  A bad choice of selector may be the cause. */

      if (selector_used) {
         /* This call used a selector and didn't like it.  Try again with
            a different selector, until we run out of ideas. */
         switch (selector_for_initialize) {
            case selector_beaux:
               selector_for_initialize = selector_ends;
               goto try_another_selector;
            case selector_ends:
               selector_for_initialize = selector_all;
               goto try_another_selector;
            case selector_all:
               selector_for_initialize = selector_none;
               goto try_another_selector;
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

   /* Set the selector (for "so-and-so advance to a column", etc) to "beaux".
      This seems to make most calls work -- note that "everyone run" and
      "no one advance to a column" are illegal.  If "beaux" doesn't work, we will
      try "ends" (for the call "fold") and then "all" before giving up. */
   selector_for_initialize = selector_beaux;

   try_another_selector:

   /* Do the call.  An error will signal and go to try_again. */

   selector_used = FALSE;

   history_ptr = 1;

   history[history_ptr].centersp = 0;
   history[history_ptr].state = test_setup;
   initialize_parse();

   /* If the call has the "rolldefine" schema, we accept it, since the test setups
      are all in the "roll unsupported" state. */

   if (test_call->schema == schema_roll) goto accept;

   (void) deposit_call(test_call);
   toplevelmove();

   /* It seems to have worked, save it.  We don't care about warnings here. */

  accept:
   global_temp_call_list[global_callcount] = test_call;
   global_temp_call_name_list[global_callcount] = global_main_call_name_list[call_index];
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

   create_menu(cl, global_temp_call_name_list);
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
   char *m, *n;

   m = x->name;
   n = y->name;
   for (;;) {
      if (*m == '@') {
         /* Skip over @7...@8, @n .. @o, and @j...@l stuff. */
         if (m[1] == '7' || m[1] == 'n' || m[1] == 'j') {
            while (*++m != '@');
         }
         m += 2;   
      }
      else if (*n == '@') {
         if (n[1] == '7' || n[1] == 'n' || n[1] == 'j') {
            while (*++n != '@');
         }
         n += 2;
      }
      /* We need to ignore blanks too, otherwise "@6 run" will come out at the beginning of the list instead of under "r". */
      else if (*m == ' ') m++;
      else if (*n == ' ') n++;
      /* And hyphens too, so that "1-3-2 quarter the alter" will be listed under "q" rather than "-". */
      else if (*m == '-') m++;
      else if (*n == '-') n++;
      else if (!*m) return(TRUE);
      else if (!*n) return(FALSE);
      else if (*m < *n) return(TRUE);
      else if (*m > *n) return(FALSE);
      else
         { m++; n++; }
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


Private void create_misc_call_lists(void)
{
   int j;
   long_boolean accept_it;
   int i, callcount;

   /* GCOL */

   callcount = 0;

   for (j=0; j<number_of_calls[call_list_any]; j++) {
      accept_it = FALSE;
      if (main_call_lists[call_list_any][j]->schema != schema_by_array) accept_it = TRUE;
      else if (main_call_lists[call_list_any][j]->callflags1 & CFLAG1_STEP_TO_WAVE) {
         if (  assoc(b_4x2, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_4x1, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_2x2, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_2x1, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list))
            accept_it = TRUE;
      }
      else {
         if (  assoc(b_8x1, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_4x1, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_2x1, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list))
            accept_it = TRUE;
      }

      if (accept_it) {
         global_temp_call_list[callcount] = main_call_lists[call_list_any][j];
         global_temp_call_name_list[callcount] = global_main_call_name_list[j];
         callcount++;
      }
   }

   /* Create the call list itself. */

   number_of_calls[call_list_gcol] = callcount;
   main_call_lists[call_list_gcol] = (callspec_block **) get_mem(callcount * sizeof(callspec_block *));
   for (i=0; i < callcount; i++) {
      main_call_lists[call_list_gcol][i] = global_temp_call_list[i];
   }

   /* Create the menu for it. */

   create_menu(call_list_gcol, global_temp_call_name_list);

   /* QTAG */

   callcount = 0;

   for (j=0; j<number_of_calls[call_list_any]; j++) {
      accept_it = FALSE;
      if (main_call_lists[call_list_any][j]->schema != schema_by_array) accept_it = TRUE;
      else if (main_call_lists[call_list_any][j]->callflags1 & CFLAG1_REAR_BACK_FROM_QTAG) {
         if (  assoc(b_4x2, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_4x1, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list))
            accept_it = TRUE;
      }
      else {
         if (assoc(b_qtag, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_pqtag, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_dmd, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_pmd, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_1x2, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list) ||
               assoc(b_2x1, (setup *) 0, main_call_lists[call_list_any][j]->stuff.arr.def_list->callarray_list))
            accept_it = TRUE;
      }

      if (accept_it) {
         global_temp_call_list[callcount] = main_call_lists[call_list_any][j];
         global_temp_call_name_list[callcount] = global_main_call_name_list[j];
         callcount++;
      }
   }

   /* Create the call list itself. */

   number_of_calls[call_list_qtag] = callcount;
   main_call_lists[call_list_qtag] = (callspec_block **) get_mem(callcount * sizeof(callspec_block *));
   for (i=0; i < callcount; i++) {
      main_call_lists[call_list_qtag][i] = global_temp_call_list[i];
   }

   /* Create the menu for it. */

   create_menu(call_list_qtag, global_temp_call_name_list);
}


/* These are used by the database reading stuff. */

Private unsigned int last_datum, last_12;
Private callspec_block *call_root;
Private callarray *tp;
/* This shows the highest index we have seen so far.  It must never exceed max_base_calls-1. */
Private int highest_base_call;


Private void read_halfword(void)
{
   last_datum = read_16_from_database();
   last_12 = last_datum & 0xFFF;
}


Private void read_fullword(void)
{
   int t = read_16_from_database();
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

   read_halfword();

   if ((last_datum & 0xE000) != 0x6000) {
      database_error("database phase error 3");
   }

   current_call_block = 0;

   while ((last_datum & 0xE000) == 0x6000) {
      begin_kind this_start_setup;
      search_qualifier this_qualifier;
      call_restriction this_restriction;
      setup_kind end_setup;
      setup_kind end_setup_out;
      int this_start_size;
      unsigned int these_flags;
      int extra;

      these_flags = last_12;

      read_halfword();       /* Get qualifier and start setup. */
      this_qualifier = (search_qualifier) ((last_datum & 0xFF00) >> 8);
      this_start_setup = (begin_kind) (last_datum & 0xFF);
      this_start_size = begin_sizes[this_start_setup];

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
      tp->qualifier = this_qualifier;
      tp->start_setup = this_start_setup;
      tp->restriction = this_restriction;

      if (these_flags & CAF__CONCEND) {      /* See if "concendsetup" was used. */
         tp->end_setup = s_normal_concentric;
         tp->end_setup_in = end_setup;
         tp->end_setup_out = end_setup_out;
      }
      else {
         tp->end_setup = end_setup;
      }

      if (these_flags & CAF__PREDS) {
         predptr_pair *temp_predlist;
         predptr_pair *this_predlist = (predptr_pair *) 0;

         /* Read error message text. */

         for (j=1; j <= ((char_count+1) >> 1); j++) {
            read_halfword();
            tp->stuff.prd.errmsg[(j << 1)-2] = (last_datum >> 8) & 0xFF;
            if ((j << 1) != char_count+1) tp->stuff.prd.errmsg[(j << 1)-1] = last_datum & 0xFF;
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
            temp_predlist->pred = pred_table[last_datum];
            /* If this call uses a predicate that takes a selector, flag the call so that
               we will query the user for that selector. */
            if (last_datum < SELECTOR_PREDS)
               call_root->callflags1 |= CFLAG1_REQUIRES_SELECTOR;
            for (j=0; j < this_start_size; j++) {
               read_halfword();
               temp_predlist->arr[j] = last_datum;
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
            tp->stuff.def[j] = last_datum;
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


/* This fills the permanent array "main_call_lists[call_list_any]" with the stuff
      read in from the database, including name pointer fields containing the original text
      with "@" escapes.  It also sets "number_of_calls[call_list_any]" to the size thereof.
         It does this only for calls that are legal at this level.  If we are just
         writing a call list, a call is legal only if its level is exactly the level
         specified when the program was invoked, rather than the usual case of including
         any call whose level is at or below the specified level.  If we are doing a
         "write full" we do the usual action.
   It also fills in the "base_calls" array with tagged calls, independent of level. */

Private void build_database(call_list_mode_t call_list_mode)
{
   int i, j, char_count;
   calldef_schema call_schema;
   int local_callcount;
   char *np, c;
   int savetag, saveflags1, saveflags;
   dance_level savelevel;
   callspec_block **local_call_list;

   /* This list will be permanent. */
   base_calls = (callspec_block **) get_mem(max_base_calls * sizeof(callspec_block *));

   /* These three will be temporary.  The first two last through the entire initialization
      process.  The last one only in this procedure. */
   global_temp_call_list = (callspec_block **) get_mem(abs_max_calls * sizeof(callspec_block *));
   global_temp_call_name_list = (char **) get_mem(abs_max_calls * sizeof(char *));
   local_call_list = (callspec_block **) get_mem(abs_max_calls * sizeof(callspec_block *));

   /* Clear the tag list.  Calls will fill this in as they announce themselves. */
   for (i=0; i < max_base_calls; i++) base_calls[i] = (callspec_block *) 0;

   highest_base_call = 0;

   read_halfword();

   local_callcount = 0;

   for (;;) {
      if ((last_datum & 0xE000) == 0) break;

      if ((last_datum & 0xE000) != 0x2000) {
         database_error("database phase error 1");
      }

      savetag = last_12;     /* Get tag, if any. */

      read_halfword();       /* Get level. */
      savelevel = (dance_level) last_datum;

      read_fullword();       /* Get top level flags, first word. */
      saveflags1 = last_datum;

      read_fullword();       /* Get top level flags, second word. */
      saveflags = last_datum;

      read_halfword();       /* Get char count and schema. */
      call_schema = (calldef_schema) (last_datum & 0xFF);
      char_count = (last_datum >> 8) & 0xFF;

      /* **** We should only allocate the call root if we are going to use this call,
         either because it is tagged or because it is legal at this level!!!! */

      /* Now that we know how long the name is, create the block and fill in the saved stuff. */
      /* We subtract 3 because 4 chars are already present, but we need one extra for the pad. */

      call_root = (callspec_block *) get_mem(sizeof(callspec_block) + char_count - 3);
      np = call_root->name;

      if (savetag) {
         check_tag(savetag);
         base_calls[savetag] = call_root;
      }

      if (savelevel == calling_level || (call_list_mode != call_list_mode_writing && savelevel < calling_level)) {
         if (local_callcount >= abs_max_calls)
            database_error("Too many base calls -- mkcalls made an error.");
         local_call_list[local_callcount++] = call_root;
      }

      call_root->age = 0;
      call_root->callflags1 = saveflags1;
      call_root->callflagsh = saveflags;
      /* If we are operating at the "all" level, make fractions visible everywhere, to aid in debugging. */
      if (calling_level == l_dontshow) call_root->callflags1 |= CFLAG1_VISIBLE_FRACTIONS | CFLAG1_FIRST_PART_VISIBLE;

      /* Now read in the name itself. */

      for (j=0; j<char_count; j++)
         *np++ = read_8_from_database();

      *np = '\0';

      /* Scan the name for "@6" or "@e", fill in "needselector" or
         "left_changes_name" flag if found. */

      np = call_root->name;

      while (c = *np++) {
         if (c == '@') {
            if ((c = *np++) == '6' || c == 'k')
               call_root->callflags1 |= CFLAG1_REQUIRES_SELECTOR;
            else if (c == 'h')
               call_root->callflags1 |= CFLAG1_REQUIRES_DIRECTION;
         }
      }

      read_halfword();       /* Get next thing, whatever that is. */

      call_root->schema = call_schema;

      switch (call_schema) {
         case schema_nothing:
            continue;
         case schema_matrix:
            {
               int left_half = last_datum;
               read_halfword();
               call_root->stuff.matrix.flags = ((left_half & 0xFFFF) << 16) | (last_datum & 0xFFFF);
               read_halfword();

               for (j=0; j < 2; j++) {
                  call_root->stuff.matrix.stuff[j] = last_datum & 0xFFFF;
                  read_halfword();
               }

               call_root->callflags1 |= CFLAG1_REQUIRES_SELECTOR;
               continue;
            }
         case schema_partner_matrix:
            {
               int left_half = last_datum;
               read_halfword();
               call_root->stuff.matrix.flags = ((left_half & 0xFFFF) << 16) | (last_datum & 0xFFFF);
               read_halfword();

               for (j=0; j < 8; j++) {
                  call_root->stuff.matrix.stuff[j] = last_datum & 0xFFFF;
                  read_halfword();
               }

               continue;
            }
         case schema_roll:
            continue;
      }

      switch (call_schema) {
         case schema_by_array:
            {
               calldef_block *zz, *yy;
   
               /* Demand a level 2 group. */
               if ((last_datum) != 0x4000) {
                  database_error("database phase error 2");
               }
   
               zz = (calldef_block *) get_mem(sizeof(calldef_block));
               zz->next = 0;
               zz->modifier_seth = 0;
               zz->modifier_level = l_mainstream;
               call_root->stuff.arr.def_list = zz;
   
               read_level_3_groups(zz);
   
               /* Check for more level 2 groups. */
   
               while ((last_datum & 0xE000) == 0x4000) {
                  yy = (calldef_block *) get_mem(sizeof(calldef_block));
                  zz->next = yy;
                  zz = yy;
                  zz->modifier_level = (dance_level) (last_datum & 0xFF);
                  zz->next = 0;
                  read_fullword();
                  zz->modifier_seth = last_datum;
   
                  read_level_3_groups(zz);
               }
            }
            break;
         case schema_sequential:
         case schema_split_sequential:
            {
               by_def_item templist[100];
               int next_definition_index = 0;

               /* Demand a level 2 group. */
               if ((last_datum & 0xE000) != 0x4000) {
                  database_error("database phase error 6");
               }

               while ((last_datum & 0xE000) == 0x4000) {
                  check_tag(last_12);
                  templist[next_definition_index].call_id = last_12;
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
            if ((last_datum & 0xE000) != 0x4000) {
               database_error("database phase error 7");
            }

            check_tag(last_12);
            call_root->stuff.conc.innerdef.call_id = last_12;
            read_fullword();
            call_root->stuff.conc.innerdef.modifiers1 = (defmodset) last_datum;
            read_fullword();
            call_root->stuff.conc.innerdef.modifiersh = (defmodset) last_datum;
            read_halfword();
            check_tag(last_12);
            call_root->stuff.conc.outerdef.call_id = last_12;
            read_fullword();
            call_root->stuff.conc.outerdef.modifiers1 = (defmodset) last_datum;
            read_fullword();
            call_root->stuff.conc.outerdef.modifiersh = (defmodset) last_datum;
            read_halfword();
            break;
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
}



extern void initialize_menus(call_list_mode_t call_list_mode)
{
   int arithtest = 2081607680;

   /* This "if" should never get executed.  We expect compilers to optimize
      it away, and perhaps print a warning about it. */

   /* Test that the constants ROLL_BIT and DBROLL_BIT are in the right
      relationship, with ROLL_BIT >= DBROLL_BIT, that is, the roll bits
      in a person record are to the left of the roll bits in the binary database.
      This is because of expressions "ROLL_BIT/DBROLL_BIT" in sdbasic.c to
      align stuff from the binary database into the person record. */

   if (ROLL_BIT < DBROLL_BIT) {
      init_error("constants not consistent -- program has been compiled incorrectly.");
      final_exit(1);
   }
   else if ((508205 << 12) != arithtest) {
      init_error("arithmetic is less than 32 bits -- program has been compiled incorrectly.");
      final_exit(1);
   }

   /* Read in the calls database, then start the user interface package,
      whatever that might be, and finally create all the call menus.
      If we are writing a call list, we do all this differently.
      The user interface is never actually started, and we exit
      from the program after writing the list. */

   /* Opening the database sets up the values of
      abs_max_calls and max_base_calls.
      Must do before telling the uims so any open failure messages
      come out first. */
   open_database();
   uims_database_tick_max(10*16);
   build_database(call_list_mode);
   uims_database_tick(10);

   the_array = main_call_lists[call_list_any];
   heapsort(number_of_calls[call_list_any]);

   /* Now the permanent array "main_call_lists[call_list_any]" has all the legal calls,
         including name pointer fields containing the original text with "@" escapes,
         sorted alphabetically.
      The remaining tasks are to make the subcall lists for other setups (e.g.
         those calls legal from columns), and clean up the names that we will actually
         display in the menus (no "@" signs) and initialize the menus with the
         cleaned-up and subsetted text. */

   global_main_call_name_list = (char **) get_mem(abs_max_calls * sizeof(char *));
   create_call_name_list();
   uims_database_tick(5);

   /* Now temporary array "global_main_call_name_list" has the menu-presentable
      form of the text corresponding to "main_call_lists[call_list_any]". */

   /* Do special stuff if we are reading or writing a call list file. */

   if (call_list_mode != call_list_mode_none) {
      if (call_list_mode == call_list_mode_abridging) {
         char abridge_call[100];
         int i, j;
   
         while (read_from_call_list_file(abridge_call, 99)) {
            /* Remove the newline character. */
            abridge_call[strlen(abridge_call)-1] = '\0';
            /* Search through the call name list for this call.
               Why don't we use a more efficient search, based on the fact
               that the call list has been alphabetized?  Because it was
               alphabetized before the '@' escapes were expanded.  It
               is no longer in alphabetical order. */
            for (i=0; i<number_of_calls[call_list_any]; i++) {
               if (!strcmp(abridge_call, global_main_call_name_list[i])) {
                  /* Delete this call and move all subsequent calls down one position. */
                  for (j=i+1; j<number_of_calls[call_list_any]; j++) {
                     global_main_call_name_list[j-1] = global_main_call_name_list[j];
                     main_call_lists[call_list_any][j-1] = main_call_lists[call_list_any][j];
                  }
                  number_of_calls[call_list_any]--;
                  break;
               }
            }
         }
      }
      else {      /* Writing a list of some kind. */
         int i;
         for (i=0; i<number_of_calls[call_list_any]; i++) {
            write_to_call_list_file(global_main_call_name_list[i]);
         }
      }

      /* Close the file. */
      if (close_call_list_file())
         exit_program(1);

      if (call_list_mode != call_list_mode_abridging)
         goto getout;   /* That's all! */
   }

   /* Now the array "main_call_lists[call_list_any]" and "global_main_call_name_list"
         have the stuff for the calls that we will actually use.
      The remaining tasks are to make the subcall lists for other setups (e.g.
         those calls legal from columns), and initialize the menus with the
         subsetted text. */

   uims_preinitialize();

   /* This is the universal menu. */
   create_menu(call_list_any, global_main_call_name_list);
   uims_database_tick(5);

   /* Create the special call menus for restricted setups. */

   test_starting_setup(call_list_1x8, test_setup_1x8);           /* RH grand wave */
   test_starting_setup(call_list_l1x8, test_setup_l1x8);         /* LH grand wave */
   test_starting_setup(call_list_dpt, test_setup_dpt);           /* DPT */
   test_starting_setup(call_list_cdpt, test_setup_cdpt);         /* completed DPT */
   test_starting_setup(call_list_rcol, test_setup_rcol);         /* RCOL */
   test_starting_setup(call_list_lcol, test_setup_lcol);         /* LCOL */
   test_starting_setup(call_list_8ch, test_setup_8ch);           /* 8CH */
   test_starting_setup(call_list_tby, test_setup_tby);           /* TBY */
   test_starting_setup(call_list_lin, test_setup_lin);           /* LIN */
   test_starting_setup(call_list_lout, test_setup_lout);         /* LOUT */
   test_starting_setup(call_list_rwv, test_setup_rwv);           /* RWV */
   test_starting_setup(call_list_lwv, test_setup_lwv);           /* LWV */
   test_starting_setup(call_list_r2fl, test_setup_r2fl);         /* R2FL */
   test_starting_setup(call_list_l2fl, test_setup_l2fl);         /* L2FL */

   create_misc_call_lists();

   /* This was just temporary, for copying into menus. */
   free_mem(global_main_call_name_list);

   /* These were global to the initialization, but they go away also. */
   free_mem(global_temp_call_list);
   free_mem(global_temp_call_name_list);

   getout:

   uims_database_tick_end();
   return;
}
