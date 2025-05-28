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

    This is for version 25. */

/* This defines the following functions:
   initialize_menus
*/

#include <string.h>
#include "sd.h"

/* Global to this file. */

/* These two get temporarily allocated.  They persist through the entire initialization. */
static callspec_block **global_temp_call_list;
static char **global_temp_call_name_list;
static int global_callcount;     /* Index into the above. */

/* This contains the menu-presentable (e.g. "@" escapes turned into "<ANYONE>", etc.)
   text corresponding to "main_call_lists[call_list_any]", that is, the alphabetized
   universal menu.  It persists through the entire initialization. */
static char **global_main_call_name_list;

#define SB (ID2_SIDE|ID2_BOY)
#define HB (ID2_HEAD|ID2_BOY)
#define SG (ID2_SIDE|ID2_GIRL)
#define HG (ID2_HEAD|ID2_GIRL)
#define WEST(x) (d_west|(x)<<6)
#define EAST(x) (d_east|(x)<<6)
#define NORT(x) (d_north|(x)<<6)
#define SOUT(x) (d_south|(x)<<6)

static setup test_setup_1x8  = {s1x8, 0, {{NORT(6), SB}, {SOUT(5), HG}, {SOUT(4), HB}, {NORT(7), SG}, {SOUT(2), SB}, {NORT(1), HG}, {NORT(0), HB}, {SOUT(3), SG}}, 0};
static setup test_setup_l1x8 = {s1x8, 0, {{SOUT(6), SB}, {NORT(5), HG}, {NORT(4), HB}, {SOUT(7), SG}, {NORT(2), SB}, {SOUT(1), HG}, {SOUT(0), HB}, {NORT(3), SG}}, 0};
static setup test_setup_dpt  = {s2x4, 0, {{EAST(6), SB}, {EAST(5), HG}, {WEST(4), HB}, {WEST(7), SG}, {WEST(2), SB}, {WEST(1), HG}, {EAST(0), HB}, {EAST(3), SG}}, 0};
static setup test_setup_cdpt = {s2x4, 0, {{WEST(6), SB}, {WEST(5), HG}, {EAST(4), HB}, {EAST(7), SG}, {EAST(2), SB}, {EAST(1), HG}, {WEST(0), HB}, {WEST(3), SG}}, 0};
static setup test_setup_rcol = {s2x4, 0, {{EAST(6), SB}, {EAST(5), HG}, {EAST(4), HB}, {EAST(7), SG}, {WEST(2), SB}, {WEST(1), HG}, {WEST(0), HB}, {WEST(3), SG}}, 0};
static setup test_setup_lcol = {s2x4, 0, {{WEST(6), SB}, {WEST(5), HG}, {WEST(4), HB}, {WEST(7), SG}, {EAST(2), SB}, {EAST(1), HG}, {EAST(0), HB}, {EAST(3), SG}}, 0};
static setup test_setup_8ch  = {s2x4, 0, {{EAST(6), SB}, {WEST(5), HG}, {EAST(4), HB}, {WEST(7), SG}, {WEST(2), SB}, {EAST(1), HG}, {WEST(0), HB}, {EAST(3), SG}}, 0};
static setup test_setup_tby  = {s2x4, 0, {{WEST(6), SB}, {EAST(5), HG}, {WEST(4), HB}, {EAST(7), SG}, {EAST(2), SB}, {WEST(1), HG}, {EAST(0), HB}, {WEST(3), SG}}, 0};
static setup test_setup_lin  = {s2x4, 0, {{SOUT(6), SB}, {SOUT(5), HG}, {SOUT(4), HB}, {SOUT(7), SG}, {NORT(2), SB}, {NORT(1), HG}, {NORT(0), HB}, {NORT(3), SG}}, 0};
static setup test_setup_lout = {s2x4, 0, {{NORT(6), SB}, {NORT(5), HG}, {NORT(4), HB}, {NORT(7), SG}, {SOUT(2), SB}, {SOUT(1), HG}, {SOUT(0), HB}, {SOUT(3), SG}}, 0};
static setup test_setup_rwv  = {s2x4, 0, {{NORT(6), SB}, {SOUT(5), HG}, {NORT(4), HB}, {SOUT(7), SG}, {SOUT(2), SB}, {NORT(1), HG}, {SOUT(0), HB}, {NORT(3), SG}}, 0};
static setup test_setup_lwv  = {s2x4, 0, {{SOUT(6), SB}, {NORT(5), HG}, {SOUT(4), HB}, {NORT(7), SG}, {NORT(2), SB}, {SOUT(1), HG}, {NORT(0), HB}, {SOUT(3), SG}}, 0};
static setup test_setup_r2fl = {s2x4, 0, {{NORT(6), SB}, {NORT(5), HG}, {SOUT(4), HB}, {SOUT(7), SG}, {SOUT(2), SB}, {SOUT(1), HG}, {NORT(0), HB}, {NORT(3), SG}}, 0};
static setup test_setup_l2fl = {s2x4, 0, {{SOUT(6), SB}, {SOUT(5), HG}, {NORT(4), HB}, {NORT(7), SG}, {NORT(2), SB}, {NORT(1), HG}, {SOUT(0), HB}, {SOUT(3), SG}}, 0};





/* This cleans up the text of the alphabetized main call list in
   "main_call_lists[call_list_any]", putting the menu-presentable form
   into global_main_call_name_list, from which various subsets will be picked out
   for the menus.  It simply re-uses the stored string where it can, and allocates
   fresh memory if a substitution took place. */

static void create_call_name_list(void)
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
               if (c == '0') {
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
               else if (c == '7' || c == 'j') {
                  /* Skip over @7...@8 and @j...@l stuff. */
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


static void create_menu(call_list_kind cl, char *call_name_list[])
{
   int i;

   for (i=0; i<number_of_calls[cl]; i++) {
      uims_add_call_to_menu(cl, i, call_name_list[i]);
   }
   uims_finish_call_menu(cl, menu_names[cl]);
}


/* These variables are actually local to test_starting_setup, but they are
   expected to be preserved across the longjmp, so they must be static. */
static parse_block *parse_mark;
static int call_index;


static void test_starting_setup(call_list_kind cl, setup test_setup)
{
   callspec_block *test_call;
   real_jmp_buf my_longjmp_buffer;
   int i;

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
      "no one advance to a column" are illegal.  If this doesn't work, we will
      try a few others before giving up. */
   selector_for_initialize = selector_beaux;

   try_another_selector:

   /* Do the call.  An error will signal and go to try_again. */

   selector_used = FALSE;

   history_ptr = 1;
   initialize_parse();

   /* If the call has the "rolldefine" schema, we accept it, since the test setups
      are all in the "roll unsupported" state. */

   if (test_call->schema == schema_roll) goto accept;

   (void) deposit_call(test_call);
   history[history_ptr].state = test_setup;
   history[history_ptr+1].warnings.bits[0] = 0;
   history[history_ptr+1].warnings.bits[1] = 0;
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
   main_call_lists[cl] = (callspec_block **) get_mem(global_callcount*sizeof(int));
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

static callspec_block **the_array;


static long_boolean callcompare(callspec_block *x, callspec_block *y)
{
   char *m, *n;

   m = x->name;
   n = y->name;
   for (;;) {
      if (*m == '@') {
         /* Skip over @7...@8 and @j...@l stuff. */
         if (m[1] == '7' || m[1] == 'j') {
            while (*++m != '@');
         }
         m += 2;   
      }
      else if (*n == '@') {
         if (n[1] == '7' || n[1] == 'j') {
            while (*++n != '@');
         }
         n += 2;
      }
      /* We need to ignore blanks too, otherwise "@6 run" will come out at the beginning of the list instead of under "r". */
      else if (*m == ' ') m++;
      else if (*n == ' ') n++;
      else if (!*m) return(TRUE);
      else if (!*n) return(FALSE);
      else if (*m < *n) return(TRUE);
      else if (*m > *n) return(FALSE);
      else
         { m++; n++; }
   }
}

static void heapify(int lo, int hi)
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



static void heapsort(int n)
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


static void create_misc_call_lists(void)
{
   int j;
   long_boolean accept_it;
   int i, callcount;

   /* GCOL */

   callcount = 0;

   for (j=0; j<number_of_calls[call_list_any]; j++) {
      accept_it = FALSE;
      if (main_call_lists[call_list_any][j]->schema != schema_by_array) accept_it = TRUE;
      else if (main_call_lists[call_list_any][j]->callflags & cflag__step_to_wave) {
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
   main_call_lists[call_list_gcol] = (callspec_block **) get_mem(callcount*sizeof(int));
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
      else if (main_call_lists[call_list_any][j]->callflags & cflag__rear_back_from_qtag) {
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
   main_call_lists[call_list_qtag] = (callspec_block **) get_mem(callcount*sizeof(int));
   for (i=0; i < callcount; i++) {
      main_call_lists[call_list_qtag][i] = global_temp_call_list[i];
   }

   /* Create the menu for it. */

   create_menu(call_list_qtag, global_temp_call_name_list);
}


/* These are used by the database reading stuff. */

static int last_datum, last_12;
static callspec_block *call_root;
static predptr_pair *this_predlist, *temp_predlist, *temp_predblock;
static callarray *tp;
/* This shows the highest index we have seen so far.  It must never exceed max_base_calls-1. */
static int highest_base_call;


static void read_halfword(void)
{
   last_datum = read_from_database();
   last_12 = last_datum & 0xFFF;
}


static void read_fullword(void)
{
   int t = read_from_database();
   last_datum = t << 16 | read_from_database();
}


static void create_fresh_call_block(void)
{
   callarray *tt;

   tt = (callarray*) get_mem(sizeof(callarray));
   tt->next = 0;
   tt->start_setup = b_1x2;
   tt->qualifier = sq_none;
   tt->end_setup = s_1x2;
   tt->callarray_flags = 0;
   tt->restriction = cr_any;
   this_predlist = (predptr_pair *) 0;
   tp = tt;
}


/* Found an error while reading CALL out of the database.
   Print an error message and quit.
   Should take the call as an argument, but since this entire file uses global variables,
   we will, too. */

static void database_error(char *message)
{
    print_line(message);
    if (call_root && call_root->name && *(call_root->name)) {
	print_line("  While reading this call from the database:");
	print_line(call_root->name);
    }
    exit_program(1);
}


static void read_level_3_groups(void)

{
   int j, char_count;
   callarray *current_call_block;

   read_halfword();

   if ((last_datum & 0xE000) != 0x6000) {
      database_error("database phase error 3");
   }

   current_call_block = 0;

   while ((last_datum & 0xE000) == 0x6000) {
      if (!current_call_block)
         current_call_block = tp;
      else {
         create_fresh_call_block();
         current_call_block->next = tp;
         current_call_block = tp;
      }

      tp->callarray_flags = last_12;

      read_halfword();       /* Get qualifier and start setup. */
      tp->qualifier = (search_qualifier) ((last_datum & 0xFF00) >> 8);
      tp->start_setup = (begin_kind) (last_datum & 0xFF);

      read_halfword();       /* Get restriction and end setup. */

      tp->restriction = (call_restriction) ((last_datum & 0xFF00) >> 8);

      if (tp->callarray_flags & CAF__CONCEND) {      /* See if "concendsetup" was used. */
         tp->end_setup = s_normal_concentric;
         tp->end_setup_in = (setup_kind) (last_datum & 0xFF);
         read_halfword();       /* Get outer setup and outer rotation. */
         tp->end_setup_out = (setup_kind) (last_datum & 0xFF);
      }
      else {
         tp->end_setup = (setup_kind) (last_datum & 0xFF);
      }

      if (tp->callarray_flags & CAF__PREDS) {
         /* Read error message text. */
         read_halfword();
         char_count = last_datum & 0xFF;

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
            temp_predlist = (predptr_pair*) get_mem(sizeof(predptr_pair));
            temp_predlist->pred = pred_table[last_datum];
            /* If this call uses a predicate that takes a selector, flag the call so that
               we will query the user for that selector. */
            if (last_datum < SELECTOR_PREDS)
               call_root->callflags |= cflag__requires_selector;
            for (j=0; j < begin_sizes[tp->start_setup]; j++) {
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
            temp_predblock = this_predlist;
            this_predlist = this_predlist->next;
            temp_predblock->next = temp_predlist;
            temp_predlist = temp_predblock;
         }

         tp->stuff.prd.predlist = temp_predlist;

         this_predlist = 0;           /* clear it for next time (might be in same call) */
      }
      else {
         for (j=0; j < begin_sizes[tp->start_setup]; j++) {
            read_halfword();
            tp->stuff.def[j] = last_datum;
         }
         read_halfword();
      }
   }
}


static void check_tag(int tag)
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

static void build_database(call_list_mode_t call_list_mode)
{
   int i, j, char_count;
   calldef_schema call_schema;
   int local_callcount;
   char *np, c;
   int savetag, saveflags;
   level savelevel;
   calldef_block *zz, *yy;
   callspec_block **local_call_list;

   open_database();    /* This sets up the values of abs_max_calls and max_base_calls. */

   /* This list will be permanent. */
   base_calls = (callspec_block **) get_mem(max_base_calls * sizeof(callspec_block *));

   /* These three will be temporary.  The first two last through the entire initialization
      process.  The last one only in this procedure. */
   global_temp_call_list = (callspec_block **) get_mem(abs_max_calls * sizeof(callspec_block *));
   global_temp_call_name_list = (char **) get_mem(abs_max_calls * sizeof(char *));
   local_call_list = (callspec_block **) get_mem(abs_max_calls * sizeof(callspec_block *));

   /* Clear the tag list.  Calls will fill this in as they announce themselves. */
   for (i=0; i < max_base_calls; i++) base_calls[i] = 0;

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
      savelevel = (level) last_datum;

      read_fullword();       /* Get boolean flags. */
      saveflags = last_datum;

      read_halfword();       /* Get char count and schema. */
      call_schema = (calldef_schema) (last_datum & 0xFF);
      char_count = (last_datum >> 8) & 0xFF;

      /* **** We should only allocate the call root if we are going to use this call,
         either because it is tagged or because it is legal at this level!!!! */

      /* Now that we know how long the name is, create the block and fill in the saved stuff. */

      call_root = (callspec_block*) get_mem(sizeof(callspec_block) + char_count + 1);
      np = (char *) (((int) call_root) + sizeof(callspec_block));
      call_root->name = np;

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
      call_root->callflags = saveflags;
      /* If we are operating at the "all" level, make fractions visible everywhere, to aid in debugging. */
      if (calling_level == l_dontshow) call_root->callflags |= cflag__visible_fractions | cflag__first_part_visible;

      /* Now read in the name itself. */

      for (j=2; j <= char_count+1; j += 2) {
         read_halfword();

         *np++ = (last_datum >> 8) & 0xFF;
         if (j != char_count+1)
            *np++ = last_datum & 0xFF;
      }

      *np = '\0';

      /* Scan the name for "@6" or "@e", fill in "needselector" or
         "left_changes_name" flag if found. */

      np = call_root->name;

      while (c = *np++) {
         if (c == '@') {
            if ((c = *np++) == '6' || c == 'k') call_root->callflags |= cflag__requires_selector;
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

               call_root->callflags |= cflag__requires_selector;
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

      create_fresh_call_block();

      zz = (calldef_block*) get_mem(sizeof(calldef_block));
      zz->next = 0;
      zz->modifier_set = 0;
      zz->callarray_list = tp;
      call_root->stuff.arr.def_list = zz;

      switch (call_schema) {
         case schema_by_array:
            /* Demand a level 2 group. */
            if ((last_datum) != 0x4000) {
               database_error("database phase error 2");
            }

            read_level_3_groups();

            /* Check for more level 2 groups. */

            while ((last_datum & 0xE000) == 0x4000) {
               create_fresh_call_block();
               yy = (calldef_block*) get_mem(sizeof(calldef_block));
               zz->next = yy;
               zz = yy;
               zz->modifier_level = (level) (last_datum & 0xFF);
               zz->next = 0;
               zz->callarray_list = tp;
               /* We make use of the fact that HERITABLE_FLAG_MASK, which has all the
                  bits we could read in, lies only in the left half.  This was tested
                  by some compile-time code near the start of this file. */
               read_halfword();
               zz->modifier_set = last_datum << 16;

               read_level_3_groups();
            }
            break;

         case schema_sequential:
         case schema_split_sequential:
            {
               int next_definition_index = 0;

               /* Demand a level 2 group. */
               if ((last_datum & 0xE000) != 0x4000) {
                  database_error("database phase error 6");
               }

               while ((last_datum & 0xE000) == 0x4000) {
                  if (next_definition_index >= SEQDEF_MAX) {
                     database_error("database error: too many sequential parts");
                  }
                  check_tag(last_12);
                  call_root->stuff.def.defarray[next_definition_index].call_id = last_12;
                  read_fullword();
                  call_root->stuff.def.defarray[next_definition_index++].modifiers = (defmodset) last_datum;
                  call_root->stuff.def.defarray[next_definition_index].call_id = 0;
                  read_halfword();
               }
               break;
            }

         case schema_nothing:
         case schema_matrix:
         case schema_partner_matrix:
         case schema_roll:
            break;

         default:          /* These are all the variations of concentric. */
            /* Demand a level 2 group. */
            if ((last_datum & 0xE000) != 0x4000) {
               database_error("database phase error 7");
            }

            check_tag(last_12);
            call_root->stuff.conc.innerdef.call_id = last_12;
            read_fullword();
            call_root->stuff.conc.innerdef.modifiers = (defmodset) last_datum;
            read_halfword();
            check_tag(last_12);
            call_root->stuff.conc.outerdef.call_id = last_12;
            read_fullword();
            call_root->stuff.conc.outerdef.modifiers = (defmodset) last_datum;
            read_halfword();
            break;
      }
   }

   number_of_calls[call_list_any] = local_callcount;
   main_call_lists[call_list_any] = (callspec_block **) get_mem(local_callcount*sizeof(int));
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
   /* Test that the necessary constants are in step with each other.
      This "if" should never get executed.  We expect compilers to optimize
      it away, and perhaps print a warning about it. */
   
   if (cflag__diamond_is_inherited     != dfm_inherit_diamond ||
      cflag__reverse_means_mirror      != dfm_inherit_reverse ||
      cflag__left_means_mirror         != dfm_inherit_left ||
      cflag__funny_is_inherited        != dfm_inherit_funny ||
      cflag__intlk_is_inherited        != dfm_inherit_intlk ||
      cflag__magic_is_inherited        != dfm_inherit_magic ||
      cflag__grand_is_inherited        != dfm_inherit_grand ||
      cflag__twelvematrix_is_inherited != dfm_inherit_12_matrix ||
      cflag__cross_is_inherited        != dfm_inherit_cross ||
      cflag__single_is_inherited       != dfm_inherit_single ||
      cflag__diamond_is_inherited      != FINAL__DIAMOND ||
      cflag__reverse_means_mirror      != FINAL__REVERSE ||
      cflag__left_means_mirror         != FINAL__LEFT ||
      cflag__funny_is_inherited        != FINAL__FUNNY ||
      cflag__intlk_is_inherited        != FINAL__INTERLOCKED ||
      cflag__magic_is_inherited        != FINAL__MAGIC ||
      cflag__grand_is_inherited        != FINAL__GRAND ||
      cflag__twelvematrix_is_inherited != FINAL__12_MATRIX ||
      cflag__cross_is_inherited        != FINAL__CROSS ||
      cflag__single_is_inherited       != FINAL__SINGLE ||
   
   /* Also test that HERITABLE_FLAG_MASK lies only in the left half.
      This will be necessary in the code below which reads in the
      "modifier_set" field from the database. */
   
      (HERITABLE_FLAG_MASK & 0x0000FFFF)) {
      init_error("constants not consistent");
      final_exit(1);
   }

   /* Read in the calls database, then start the user interface package,
      whatever that might be, and finally create all the call menus.
      If we are writing a call list, we do all this differently.
      The user interface is never actually started, and we exit
      from the program after writing the list. */

   build_database(call_list_mode);

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
         return;   /* That's all! */
   }

   /* Now the array "main_call_lists[call_list_any]" and "global_main_call_name_list"
         have the stuff for the calls that we will actually use.
      The remaining tasks are to make the subcall lists for other setups (e.g.
         those calls legal from columns), and initialize the menus with the
         subsetted text. */

   uims_preinitialize();

   /* This is the universal menu. */
   create_menu(call_list_any, global_main_call_name_list);

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
}
