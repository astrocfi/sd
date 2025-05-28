// -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:3; fill-column:88 -*-

// SD -- square dance caller's helper.
//
//    Copyright (C) 1990-2007  William B. Ackerman.
//
//    This file is part of "Sd".
//
//    Sd is free software; you can redistribute it and/or modify it
//    under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Sd is distributed in the hope that it will be useful, but WITHOUT
//    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
//    License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Sd; if not, write to the Free Software Foundation, Inc.,
//    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    This is for version 37.

/* This defines the following functions:
   parse_level
   start_sel_dir_num_iterator
   iterate_over_sel_dir_num
   install_outfile_string
   get_first_session_line
   get_next_session_line
   prepare_to_read_menus
   process_session_info
   close_init_file
   general_final_exit
   conzept::translate_concept_names
   configuration::startinfolist
   configuration::initialize
   start_stats_file_from_GLOB_stats_filename
   open_session
and the following external variables:
   selector_for_initialize
   direction_for_initialize
   number_for_initialize
   color_index_list
*/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "sd.h"
#include "sort.h"
#include "mapcachefile.h"

selector_kind selector_for_initialize;
direction_kind direction_for_initialize;
int number_for_initialize;
int *color_index_list;

// Global to this file.

static call_with_name *empty_menu[] = {(call_with_name *) 0};

// This gets temporarily allocated.  It persists through the entire initialization.
static int *global_temp_call_indices;
static int global_callcount;     /* Index into the above. */



/* These variables are actually local to test_starting_setup, but they are
   expected to be preserved across the throw, so they must be static. */
static parse_block *parse_mark;
static int call_index;
static call_with_name *test_call;
static bool crossiness;
static bool magicness;
static bool intlkness;


extern bool parse_level(Cstring s, Cstring *break_ptr /*= 0*/)
{
   int len = strlen(s);
   Cstring breakpos = strpbrk(s, "-:");
   if (breakpos) len = breakpos-s;
   if (break_ptr) *break_ptr = breakpos;

   switch (s[0]) {
      case 'm': case 'M': calling_level = l_mainstream; return true;
      case 'p': case 'P': case '+': calling_level = l_plus; return true;
      case 'a': case 'A':
         if (s[1] == '1' && len == 2) calling_level = l_a1;
         else if (s[1] == '2' && len == 2) calling_level = l_a2;
         else if (s[1] == 'l' && s[2] == 'l' && len == 3) calling_level = l_dontshow;
         else return false;
         return true;
      case 'c': case 'C':
         if (s[1] == '3' && (s[2] == 'a' || s[2] == 'A') && len == 3)
            calling_level = l_c3a;
         else if (s[1] == '3' && (s[2] == 'x' || s[2] == 'X') && len == 3)
            calling_level = l_c3x;
         else if (s[1] == '4' && (s[2] == 'a' || s[2] == 'A') && len == 3)
            calling_level = l_c4a;
         else if (s[1] == '4' && (s[2] == 'x' || s[2] == 'X') && len == 3)
            calling_level = l_c4x;
         else {
            if (len == 2) {
               switch (s[1]) {
                  case '1': calling_level = l_c1; return true;
                  case '2': calling_level = l_c2; return true;
                  case '3': calling_level = l_c3; return true;
                  case '4': calling_level = l_c4; return true;
                  default: return false;
               }
            }
            else return false;
         }
         return true;
      default:
         return false;
   }
}


void start_sel_dir_num_iterator()
{
   selector_for_initialize = selector_beaus;
   direction_for_initialize = direction_right;
   number_for_initialize = 1;
}


bool iterate_over_sel_dir_num(
   bool enable_selector_iteration,
   bool enable_direction_iteration,
   bool enable_number_iteration)
{
   // Try different selectors first.

   if (selector_used && enable_selector_iteration) {
      // This call used a selector and didn't like it.  Try again with
      // a different selector, until we run out of ideas.
      switch (selector_for_initialize) {
      case selector_beaus:
         selector_for_initialize = selector_ends;
         return true;
      case selector_ends:
         selector_for_initialize = selector_leads;
         return true;
      case selector_leads:
         // This will select just one end of each wave in parallel waves or a tidal wave,
         // so "prefer the <anyone> out roll circulate" will work.
         selector_for_initialize = selector_sideboys;
         return true;
      case selector_sideboys:
         selector_for_initialize = selector_everyone;
         return true;
      case selector_everyone:
         // This will select #1 and #2 in columns,
         // so "<anyone> mark time" will work.
         selector_for_initialize = selector_headcorners;
         return true;
      case selector_headcorners:
         // This will select the ends of each wave in a tidal wave,
         // so "relay the shadow but <anyone> criss cross it" will work.
         selector_for_initialize = selector_boys;
         return true;
      case selector_boys:
         selector_for_initialize = selector_none;
         return true;
      }
   }

   // Now try a different direction.

   if (direction_used && enable_direction_iteration) {
      // This call used a direction and didn't like it.  Try again with
      // a different direction, until we run out of ideas.
      switch (direction_for_initialize) {
      case direction_right:
         // This allows "spin the windmill, outsides (no direction)" from facing lines.
         direction_for_initialize = direction_no_direction;
         selector_for_initialize = selector_beaus;
         return true;
      }
   }

   // Now try a different number.  Only do this if the call actually
   // consumes numbers, and the wildcard matching has not filled in all
   // required numbers.

   if (number_used && enable_number_iteration) {

      /* Try again with a different number, until we run out of ideas. */

      if (number_for_initialize < 4) {
         /* We try all numbers from 1 to 4.  We need to do 1-4 to get
            "exchange the boxes N/4" on the waves menu.
            Getting calls like "N-N-N-N change the web" and
            "N-N-N-N relay the top" requires that we play games
            with the list, setting the second item odd. */
         number_for_initialize++;
         selector_for_initialize = selector_beaus;
         direction_for_initialize = direction_right;
         return true;
      }
   }

   return false;
}





static void test_starting_setup(call_list_kind cl, const setup & test_setup)
{
   gg->init_step(do_tick, 2);

   call_index = -1;
   global_callcount = 0;
   /* Mark the parse block allocation, so that we throw away the garbage
      created by failing attempts. */
   parse_mark = mark_parse_blocks();

 try_again:

   // Throw away garbage.
   release_parse_blocks_to_mark(parse_mark);
   call_index++;
   if (call_index >= number_of_calls[call_list_any]) goto finished;
   test_call = main_call_lists[call_list_any][call_index];

   /* Set the selector (for "so-and-so advance to a column", etc) to "beaus".
      This seems to make most calls work -- note that "everyone run" and
      "no one advance to a column" are illegal.  If "beaus" doesn't work, we will
      try "ends" (for the call "fold"), "all", and finally "none" (for the call
      "run"), before giving up. */

   intlkness = false;
 try_another_intlk:
   magicness = false;
 try_another_magic:
   crossiness = false;
 try_another_cross:
   start_sel_dir_num_iterator();
 try_another_selector:

   selector_used = false;
   direction_used = false;
   number_used = false;
   mandatory_call_used = false;

   configuration::history_ptr = 1;

   configuration::current_config().init_centersp_specific();
   configuration::current_config().state = test_setup;
   initialize_parse();

   // If the call has the "rolldefine" schema, we accept it, since the test setups
   // are all in the "roll unsupported" state.
   if (test_call->the_defn.schema == schema_roll) goto accept;

   // If the call takes 3 or more numeric arguments, accept it.  This makes
   // "hinge by I x J x K" work from columns.
   if ((test_call->the_defn.callflags1 & ((uint32) CFLAG1_NUMBER_MASK)) >=
       3*((uint32) CFLAG1_NUMBER_BIT))
      goto accept;

   // If the call has the "matrix" schema, and it is sex-dependent, we accept it,
   // since the test setups that we use might have people placed in such a way
   // that something like "1/2 truck" is illegal.
   if (test_call->the_defn.schema == schema_matrix &&
       test_call->the_defn.stuff.matrix.matrix_def_list->items[0] !=
       test_call->the_defn.stuff.matrix.matrix_def_list->items[1])
      goto accept;

   // We also accept "<ATC> your neighbor" and "<ANYTHING> motivate" calls,
   // since we don't know what the tagging call will be.
   if (test_call->the_defn.callflagsf & (CFLAGH__TAG_CALL_RQ_MASK | CFLAGH__CIRC_CALL_RQ_BIT))
      goto accept;

   // Do the call.  An error will signal and go to try_again.

   try {
      if (crossiness)
         deposit_concept(&concept_descriptor_table[useful_concept_indices[UC_cross]]);

      if (magicness)
         deposit_concept(&concept_descriptor_table[useful_concept_indices[UC_magic]]);

      if (intlkness)
         deposit_concept(&concept_descriptor_table[useful_concept_indices[UC_intlk]]);

      if (deposit_call(test_call, &null_options)) {
         // The problem may be just that the current number is
         // inconsistent with the call's "odd number only" requirement.
         number_used = true;
         if (iterate_over_sel_dir_num(true, true, true)) goto try_another_selector;
         goto try_again;
      }
      toplevelmove();
   }
   catch(error_flag_type) {

      // A call failed.  If the call had some mandatory substitution, pass it anyway.

      if (mandatory_call_used) goto accept;

      // Or a bad choice of selector or number may be the cause.
      // Try different selectors first.

      if (iterate_over_sel_dir_num(true, true, true))
         goto try_another_selector;

      // Now try giving the "cross" modifier.

      if ((test_call->the_defn.callflagsf & ESCAPE_WORD__CROSS) && !crossiness) {
         crossiness = true;
         goto try_another_cross;
      }

      // Now try giving the "magic" modifier.

      if ((test_call->the_defn.callflagsf & ESCAPE_WORD__MAGIC) && !magicness) {
         magicness = true;
         goto try_another_magic;
      }

      // Now try giving the "interlocked" modifier.

      if ((test_call->the_defn.callflagsf & ESCAPE_WORD__INTLK) && !intlkness) {
         intlkness = true;
         goto try_another_intlk;
      }

      // Otherwise, reject this call.
      goto try_again;
   }

   // It seems to have worked, save it.  We don't care about warnings here.

 accept:
   global_temp_call_indices[global_callcount] = call_index;
   global_callcount++;
   goto try_again;

 finished:

   // Create the call list itself.  First, just fill it with the indices.
   // This is, of course, illegal, and requires a horrible cast.  But we need
   // to do this in order to read and write write the cache file in an invariant format.
   // We will turn them into pointers later.

   number_of_calls[cl] = global_callcount;
   main_call_lists[cl] = (call_with_name **) get_mem(global_callcount * sizeof(call_with_name *));

   memcpy(main_call_lists[cl],
          global_temp_call_indices,
          global_callcount*sizeof(call_with_name *));
}


static int canonicalize(char * & cp)
{
   for (;;) {
      int mc = *cp;

      // Skip blanks and hyphens.
      if (mc == ' ' || mc == '-') { cp++; continue; }

      if (mc == '@') {
         // Turn escape codes into very large numbers,
         // so they will be listed at the end.
         mc = *++cp;
         switch (mc) {
         case 'v': case 'w': case 'x': case 'y':
            return 500;   // <ATC>
         case 'h':
            return 501;   // <DIRECTION>
         case '6': case 'k': case 'K':
            return 502;   // <ANYONE>
         case 'N':
            return 503;   // <ANYCIRC>
         case '0': case 'T': case 'm':
            return 504;   // <ANYTHING>
         case '9':
            return 505;   // <N>
         case 'a': case 'b': case 'B': case 'D':
            return 506;   // <N/4>
         case 'u':
            return 507;   // <Nth>
         case '7': case 'n': case 'j': case 'J': case 'E': case 'Q':
            // Skip over @7...@8, @n .. @o, and @j...@l stuff.
            while (*cp++ != '@');
            // FALL THROUGH!!!!!
         default:
            cp++;
            continue;
         }
      }

      // Canonicalize to lower case.
      return (mc >= 'A' && mc <= 'Z') ? mc + 'a'-'A' : mc;
   }
}

class DBCOMPARE {
public:
   static bool inorder(call_with_name *x, call_with_name *y)
   {
      char *mp = x->name;
      char *np = y->name;

      for (;;) {
         // First, skip over everything that we need to, in both m and n.
         // This includes blanks, hyphens, and insignificant escape sequences.
         int mc = canonicalize(mp);
         if (!mc) return true;
         int nc = canonicalize(np);
         if (!nc) return false;
         if (mc != nc) return (mc < nc);
         mp++;
         np++;
      }
   }
};


static void create_misc_call_lists(call_list_kind cl)
{
   int j;

   int callcount = 0;

   for (j=0; j<number_of_calls[call_list_any]; j++) {
      call_with_name *callp = main_call_lists[call_list_any][j];

      if (cl == call_list_gcol) {     // tidal column
         if (callp->the_defn.schema != schema_by_array || callp->the_defn.compound_part)
            goto accept;    // We don't understand it.

         callarray *deflist = callp->the_defn.stuff.arr.def_list->callarray_list;

         if ((callp->the_defn.callflags1 & CFLAG1_STEP_REAR_MASK) == CFLAG1_STEP_TO_WAVE) {
            if (assoc(b_4x2, (setup *) 0, deflist) ||
                assoc(b_4x1, (setup *) 0, deflist) ||
                assoc(b_2x2, (setup *) 0, deflist) ||
                assoc(b_2x1, (setup *) 0, deflist))
               goto accept;
         }
         else {
            if (assoc(b_8x1, (setup *) 0, deflist) ||
                assoc(b_4x1, (setup *) 0, deflist) ||
                assoc(b_2x1, (setup *) 0, deflist) ||
                assoc(b_1x1, (setup *) 0, deflist))
               goto accept;
         }
      }
      else {      // qtag
         call_with_name *callq = callp;

         if (callq->the_defn.schema != schema_by_array || callq->the_defn.compound_part)
            goto accept;    // We don't understand it.

         callarray *deflist = callq->the_defn.stuff.arr.def_list->callarray_list;
         uint32 touch_flags = callq->the_defn.callflags1 & CFLAG1_STEP_REAR_MASK;

         switch (touch_flags) {
         case CFLAG1_REAR_BACK_FROM_QTAG:
         case CFLAG1_REAR_BACK_FROM_EITHER:
            if (assoc(b_4x2, (setup *) 0, deflist) ||
                assoc(b_4x1, (setup *) 0, deflist))
               goto accept;
            break;
         }

         switch (touch_flags) {
         case CFLAG1_STEP_TO_WAVE:
         case CFLAG1_REAR_BACK_FROM_EITHER:
            if (assoc(b_thar, (setup *) 0, deflist))
               goto accept;
         }

         if (assoc(b_qtag, (setup *) 0, deflist) ||
             assoc(b_pqtag, (setup *) 0, deflist) ||
             assoc(b_dmd, (setup *) 0, deflist) ||
             assoc(b_pmd, (setup *) 0, deflist) ||
             assoc(b_1x2, (setup *) 0, deflist) ||
             assoc(b_2x1, (setup *) 0, deflist) ||
             assoc(b_1x1, (setup *) 0, deflist))
            goto accept;
      }

      continue;

   accept:
      global_temp_call_indices[callcount] = j;
      callcount++;
   }

   // Create the call list itself.

   number_of_calls[cl] = callcount;
   main_call_lists[cl] = (call_with_name **) get_mem(callcount * sizeof(call_with_name *));

   memcpy(main_call_lists[cl],
          global_temp_call_indices,
          callcount*sizeof(call_with_name *));
}


// These are used by the database reading stuff.

static uint32 last_datum, last_12;
static call_with_name *call_root;
static callarray *tp;
// This shows the highest index we have seen so far.  It must never exceed max_base_calls-1.
static int highest_base_call;

static FILE *init_file;
static int session_linenum = 0;

// 0 for "no session" line, 1 for real ones, 2 for "new session".
static int session_line_state = 0;

static char rewrite_filename_as_star[2] = { '\0' , '\0' };  // First char could be "*" or "+".
static FILE *database_file;
static FILE *abridge_file;
static FILE *stats_file = (FILE *) 0;


static uint32 read_8_from_database()
{
   return fgetc(database_file) & 0xFF;
}


static uint32 read_16_from_database()
{
   uint32 bar;

   bar = (read_8_from_database() & 0xFF) << 8;
   bar |= read_8_from_database() & 0xFF;
   return bar;
}


static bool read_database_header(char *msg1, char *msg2)
{
   int format_version, n, j;

   if (read_16_from_database() != DATABASE_MAGIC_NUM) {
      sprintf(msg1,
              "Database file \"%s\" has improper format.", database_filename);
      return true;
   }

   format_version = read_16_from_database();
   if (format_version != DATABASE_FORMAT_VERSION) {
      sprintf(msg1,
              "Database format version (%d) is not the required one (%d)",
              format_version, DATABASE_FORMAT_VERSION);
      strncpy(msg2, "You must recompile the database.", 199);
      return true;
   }

   abs_max_calls = read_16_from_database();
   max_base_calls = read_16_from_database();

   n = read_16_from_database();

   if (n > 80) {
      strncpy(msg1, "Database version string is too long.", 199);
      return true;
   }

   for (j=0; j<n; j++)
      database_version[j] = (unsigned char) read_8_from_database();

   database_version[j] = '\0';
   return false;
}


static void read_halfword()
{
   last_datum = read_16_from_database();
   last_12 = last_datum & 0xFFF;
}


static void read_fullword()
{
   uint32 t = read_16_from_database();
   last_datum = t << 16 | read_16_from_database();
}



// Found an error while reading a call out of the database.
// Print an error message and quit.
// Should take the call as an argument, but since this entire file uses global variables,
// we will, too.

static void database_error_exit(char *message)
{
   if (call_root)
      gg->fatal_error_exit(1, message, call_root->name);
   else
      gg->fatal_error_exit(1, message);
}


static void read_level_3_groups(calldef_block *where_to_put)
{
   int j, char_count;
   callarray *current_call_block;

   if ((last_datum & 0xE000) != 0x6000)
      database_error_exit("database phase error 3");

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
         // We will naturally get 4 items in the "stuff.prd.errmsg" field;
         // we are responsible all for the others.
         // We subtract 3 because 4 chars are already present, but we need one extra for the pad.
         extra = (char_count-3) * sizeof(char);
      }
      else {
         // We will naturally get 4 items in the "stuff.def" field;
         // we are responsible all for the others.
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

         // Read error message text.

         for (j=1; j <= ((char_count+1) >> 1); j++) {
            read_halfword();
            tp->stuff.prd.errmsg[(j << 1)-2] = (char) ((last_datum >> 8) & 0xFF);
            if ((j << 1) != char_count+1)
               tp->stuff.prd.errmsg[(j << 1)-1] = (char) (last_datum & 0xFF);
         }

         tp->stuff.prd.errmsg[char_count] = '\0';

         read_halfword();

         // Demand level 4 group.
         if (last_datum != 0x8000) {
            database_error_exit("database phase error 4");
         }

         while ((last_datum & 0xE000) == 0x8000) {
            read_halfword();       /* Read predicate indicator. */
            // "predptr_pair" will get us 4 items in the "arr" field;
            // we are responsible all for the others.
            temp_predlist = (predptr_pair *) get_mem(sizeof(predptr_pair) +
                    (this_start_size-4) * sizeof(unsigned short));
            temp_predlist->pred = &pred_table[last_datum];
            // If this call uses a predicate that takes a selector, flag the call so that
            // we will query the user for that selector.

            if ((int) last_datum < selector_preds)
               call_root->the_defn.callflagsf |= CFLAGH__REQUIRES_SELECTOR;

            for (j=0; j < this_start_size; j++) {
               read_halfword();
               temp_predlist->arr[j] = (uint16) last_datum;
            }

            temp_predlist->next = this_predlist;
            this_predlist = temp_predlist;
            read_halfword();    /* Get next level 4 header, or whatever. */
         }

         // Need to reverse stuff in "this_predlist".
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


static void check_tag(int tag)
{
   if (tag >= max_base_calls)
      database_error_exit("Too many tagged calls -- mkcalls made an error");
   if (tag > highest_base_call) highest_base_call = tag;
}


static void read_in_call_definition(calldefn *root_to_use, int char_count)
{
   // If we are operating at the "all" level, make fractions visible everywhere,
   // to aid in debugging.

   if (calling_level == l_dontshow)
      root_to_use->callflags1 |= 3*CFLAG1_VISIBLE_FRACTION_BIT;

   if (char_count) {

      // Read in the name itself, unless we are recursing for a compound call.

      char *np, c;
      int j;

      // We know that call_root has what we want if char_count != 0.
      np = call_root->name;

      for (j=0; j<char_count; j++)
         *np++ = (char) read_8_from_database();

      *np = '\0';

      /* Scan the name for "@6" or "@e", fill in "needselector" or
         "left_changes_name" flag if found. */

      np = call_root->name;

      while ((c = *np++)) {
         if (c == '@') {
            switch ((c = *np++)) {
            case '6': case 'k': case 'K':
               root_to_use->callflagsf |= CFLAGH__REQUIRES_SELECTOR;
               break;
            case 'h':
               root_to_use->callflagsf |= CFLAGH__REQUIRES_DIRECTION;
               break;
            case 'D':
               root_to_use->callflagsf |= CFLAGH__ODD_NUMBER_ONLY;
               break;
            case 'v':
               root_to_use->callflagsf |= (CFLAGH__TAG_CALL_RQ_BIT*1);
               break;
            case 'w':
               root_to_use->callflagsf |= (CFLAGH__TAG_CALL_RQ_BIT*2);
               break;
            case 'x':
               root_to_use->callflagsf |= (CFLAGH__TAG_CALL_RQ_BIT*3);
               break;
            case 'y':
               root_to_use->callflagsf |= (CFLAGH__TAG_CALL_RQ_BIT*4);
               break;
            case 'N':
               root_to_use->callflagsf |= CFLAGH__CIRC_CALL_RQ_BIT;
               break;
            case '0':
               root_to_use->callflagsf |= CFLAGH__HAS_AT_ZERO;
               break;
            case 'm':
               root_to_use->callflagsf |= CFLAGH__HAS_AT_M;
               break;
            }
         }
         else if (c == '[' || c == ']')
            database_error_exit("calls may not have brackets in their name");
      }
   }

   int j;
   int lim = 16;
   uint32 left_half;

   read_halfword();

   switch (root_to_use->schema) {
   case schema_alias:
      check_tag(last_12);
      root_to_use->stuff.conc.innerdef.call_id = (uint16) last_12;
      read_halfword();
      break;
   case schema_nothing:
   case schema_nothing_noroll:
   case schema_roll:
   case schema_recenter:
      break;
   case schema_matrix:
      lim = 2;
      // !!!! FALL THROUGH !!!!
   case schema_partner_matrix:
   case schema_partner_partial_matrix:
      // !!!! FELL THROUGH !!!!
      {
         left_half = last_datum;
         read_halfword();
         root_to_use->stuff.matrix.matrix_flags =
            ((left_half & 0xFFFF) << 16) | (last_datum & 0xFFFF);

         if (root_to_use->stuff.matrix.matrix_flags & MTX_USE_SELECTOR)
            root_to_use->callflagsh |= CFLAGH__REQUIRES_SELECTOR;
         if (root_to_use->stuff.matrix.matrix_flags & MTX_USE_NUMBER)
            root_to_use->callflags1 |= CFLAG1_NUMBER_BIT;

         matrix_def_block *this_matrix_block = 
            (matrix_def_block *) get_mem(sizeof(matrix_def_block) + sizeof(uint32)*(lim-2));
         root_to_use->stuff.matrix.matrix_def_list = this_matrix_block;

         this_matrix_block->modifier_level = calling_level;
         this_matrix_block->alternate_def_flags = 0;
         this_matrix_block->next = (matrix_def_block *) 0;

      next_matrix_clause:

         for (j=0; j<lim; j++) {
            uint32 firstpart;

            read_halfword();
            firstpart = last_datum & 0xFFFF;

            if (firstpart) {
               read_halfword();
               this_matrix_block->items[j] =
                  firstpart | ((last_datum & 0xFFFF) << 16);
            }
            else {
               this_matrix_block->items[j] = 0;
            }               
         }

         read_halfword();
         // Check for compound definition.
         if ((last_datum & 0xE000) == 0x4000) {
            this_matrix_block->next = (matrix_def_block *)
               get_mem(sizeof(matrix_def_block) + sizeof(uint32)*(lim-2));

            this_matrix_block = this_matrix_block->next;
            this_matrix_block->modifier_level = (dance_level) (last_datum & 0xFFF);
            read_fullword();
            this_matrix_block->alternate_def_flags = last_datum;
            this_matrix_block->next = (matrix_def_block *) 0;
            goto next_matrix_clause;
         }
      }

      break;
   case schema_by_array:
      {
         calldef_block *zz, *yy;

         zz = (calldef_block *) get_mem(sizeof(calldef_block));
         zz->next = 0;
         zz->modifier_seth = 0;
         zz->modifier_level = l_mainstream;
         root_to_use->stuff.arr.def_list = zz;

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
   case schema_sequential_alternate:
   case schema_sequential_remainder:
      {
         by_def_item templist[100];
         int next_definition_index = 0;

         /* Demand a level 2 group. */
         if ((last_datum & 0xE000) != 0x4000)
            database_error_exit("database phase error 6");

         while ((last_datum & 0xE000) == 0x4000) {
            check_tag(last_12);
            templist[next_definition_index].call_id = (uint16) last_12;
            read_fullword();
            templist[next_definition_index].modifiers1 = (mods1_word) last_datum;
            read_fullword();
            templist[next_definition_index++].modifiersh = last_datum;
            read_halfword();
         }

         root_to_use->stuff.seq.howmanyparts = next_definition_index;
         root_to_use->stuff.seq.defarray = (by_def_item *)
            get_mem((next_definition_index) * sizeof(by_def_item));

         while (--next_definition_index >= 0)
            root_to_use->stuff.seq.defarray[next_definition_index] =
               templist[next_definition_index];
      }
      break;
   default:          /* These are all the variations of concentric. */
      /* Demand a level 2 group. */
      if ((last_datum & 0xE000) != 0x4000)
         database_error_exit("database phase error 7");

      check_tag(last_12);
      root_to_use->stuff.conc.innerdef.call_id = (uint16) last_12;
      read_fullword();
      root_to_use->stuff.conc.innerdef.modifiers1 = (mods1_word) last_datum;
      read_fullword();
      root_to_use->stuff.conc.innerdef.modifiersh = last_datum;
      read_halfword();
      check_tag(last_12);
      root_to_use->stuff.conc.outerdef.call_id = (uint16) last_12;
      read_fullword();
      root_to_use->stuff.conc.outerdef.modifiers1 = (mods1_word) last_datum;
      read_fullword();
      root_to_use->stuff.conc.outerdef.modifiersh = last_datum;
      read_halfword();
      break;
   }

   // Look for compound definition.

   root_to_use->compound_part = (calldefn *) 0;

   if (last_datum == 0x3FFF) {
      calldef_schema call_schema;

      calldefn *recursed_call_root = (calldefn *)
         get_mem(sizeof(calldefn));

      read_halfword();       // Get level (not really) and 12 bits of "callflags2" stuff.
      uint32 saveflags2 = last_datum >> 4;
      read_fullword();       // Get top level flags, first word.
                             // This is the "callflags1" stuff.
      uint32 saveflags1 = last_datum;
      read_fullword();       // Get top level flags, second word.
                             // This is the "heritflags" stuff.
      uint32 saveflagsh = last_datum;
      read_halfword();       // Get char count (ignore same) and schema.
      call_schema = (calldef_schema) (last_datum & 0xFF);
      recursed_call_root->frequency = 0;
      recursed_call_root->level = 0;
      recursed_call_root->schema = call_schema;
      recursed_call_root->callflags1 = saveflags1;
      // Will get "CFLAGH" and "ESCAPE_WORD" bits later.
      recursed_call_root->callflagsf = saveflags2 << 20;
      recursed_call_root->callflagsh = saveflagsh;
      read_in_call_definition(recursed_call_root, 0);    // Recurse.
      root_to_use->compound_part = recursed_call_root;
   }
}




// This makes sure that outfile_string is a legal filename.
// Returns FALSE if error occurs.  No action taken in that case.
// We do not allow blanks in the file name.  To do so would make
// the parsing of session lines ambiguous.
extern bool install_outfile_string(const char newstring[])
{
   char test_string[MAX_FILENAME_LENGTH];

   rewrite_filename_as_star[0] = '\0';

   // Clean off leading blanks, and stop after any internal blank.

   sscanf(newstring, "%s", test_string);
   if (!test_string[0]) return false;   // Null file name is not allowed.

   // Look for special file string of "*" or "+".
   // If so, generate a new file name.
   // If the character is "+", make the name unique.

   if ((test_string[0] == '*' || test_string[0] == '+') && !test_string[1]) {
      time_t clocktime;
      FILE *filetest;
      char junk[30], junk2[30], t1[20], t2[20], t3[20], t4[20], t5[20];
      char letter[2];
      char *p;

      letter[0] = 'a';
      letter[1] = '\0';
      time(&clocktime);
      sscanf(ctime(&clocktime), "%s %s %s %s %s", t1, t2, t3, t4, t5);

      // Now t2 = "Jan", t3 = "16", and t5 = "1996".

      strncpy(junk, t3, 3);
      strncat(junk, t2, 3);
      strncat(junk, &t5[strlen(t5)-2], 2);
      for (p=junk ; *p ; p++) *p = tolower(*p);  // Month in lower case.
      strncpy(junk2, junk, 10);           // This should be "16jan96".

      for (;;) {
         strcat(junk2, filename_strings[calling_level]);

         // If the given filename is "+", accept it immediately.
         // Otherwise, fuss with the generated name until we get a
         // nonexistent file.

         if (test_string[0] == '+' || (filetest = fopen(junk2, "r")) == 0) break;
         fclose(filetest);
         if (letter[0] == 'z'+1) letter[0] = 'A';
         else if (letter[0] == 'Z'+1) return false;
         strncpy(junk2, junk, 10);
         strncat(junk2, letter, 4);     /* Try appending a letter. */
         letter[0]++;
      }

      strncpy(outfile_string, junk2, MAX_FILENAME_LENGTH);
      last_file_position = -1;
      rewrite_filename_as_star[0] = test_string[0];
      return true;
   }

   strncpy(outfile_string, test_string, MAX_FILENAME_LENGTH);
   last_file_position = -1;
   return true;
}


static bool find_init_file_region(Cstring key, int length)
{
   char line[MAX_FILENAME_LENGTH];

   if (!init_file) return false;

   if (fseek(init_file, 0, SEEK_SET))
      return false;

   // Search for the indicator.
   // We need to use strncmp, and give an explicit length,
   // because various operating systems put various types of
   // newline garbage at the end of the line when we read it
   // from the file.

   for (;;) {
      if (!fgets(line, MAX_FILENAME_LENGTH, init_file)) return false;
      if (!strncmp(line, key, length)) return true;
   }
}

extern bool get_first_session_line()
{
   session_line_state = 0;

   // If we are writing a call list file, that's all we do.

   if (glob_abridge_mode >= abridge_mode_writing)
      return true;

   // Or if the file didn't exist, or we are in diagnostic mode.
   if (!init_file || ui_options.diagnostic_mode) return true;

   // Search for the "[Sessions]" indicator.

   if (!find_init_file_region("[Sessions]", 10))
      return true;

   return false;
}


extern bool get_next_session_line(char *dest)
{
   int j;
   char line[MAX_FILENAME_LENGTH];

   if (session_line_state == 0) {
      session_line_state = 1;
      if (dest) sprintf(dest, "  0     (no session)");
      return true;
   }
   else if (session_line_state == 2)
      return false;

   if (!fgets(line, MAX_FILENAME_LENGTH, init_file) || line[0] == '\n' || line[0] == '[') {
      session_line_state = 2;
      if (dest) sprintf(dest, "%3d     (create a new session)", session_linenum+1);
      return true;
   }

   j = strlen(line);
   if (j>0) line[j-1] = '\0';   // Strip off the <NEWLINE> -- we don't want it.
   session_linenum++;
   if (dest) sprintf(dest, "%3d  %s", session_linenum, line);
   return true;
}


extern void prepare_to_read_menus()
{
   uint32 arithtest = 2081607680;

   // This "if" should never get executed.  We expect compilers to optimize
   // it away, and perhaps print a warning about it.

   // Test that the constants ROLL_BIT and DBROLL_BIT are in the right
   // relationship, with ROLL_BIT >= DBROLL_BIT, that is, the roll bits
   // in a person record are to the left of the roll bits in the binary database.
   // This is because of expressions "ROLL_BIT/DBROLL_BIT" in sdbasic.cpp to
   // align stuff from the binary database into the person record.

   if ((int) NROLL_BIT < (int) NDBROLL_BIT)
      gg->fatal_error_exit(1, "Constants not consistent",
                           "program has been compiled incorrectly.");
   else if ((508205 << 12) != arithtest)
      gg->fatal_error_exit(1, "Arithmetic is less than 32 bits",
                           "program has been compiled incorrectly.");
   else if (l_nonexistent_concept > 15)
      gg->fatal_error_exit(1, "Too many levels",
                           "program has been compiled incorrectly.");
   else if (NUM_QUALIFIERS > 125)
      gg->fatal_error_exit(1, "Insufficient qualifier space",
                           "program has been compiled incorrectly.");
   else if (NUM_PLAINMAP_KINDS > 252)
      gg->fatal_error_exit(1, "Insufficient mapkind space",
                           "program has been compiled incorrectly.");

   // We need to take away the "zig-zag" directions if the level is below A2.

   if (calling_level < zig_zag_level) {
      last_direction_kind = direction_zigzag-1;
      direction_names[direction_zigzag].name = (Cstring) 0;
      direction_names[direction_zigzag].name_uc = (Cstring) 0;
   }

   if (glob_abridge_mode < abridge_mode_writing) {
      int i;

      // Find out how big the command menu needs to be.

      for (num_command_commands = 0 ;
           command_menu[num_command_commands].command_name ;
           num_command_commands++) ;

      command_commands = (Cstring *) get_mem(sizeof(Cstring) * num_command_commands);
      command_command_values =
         (command_kind *) get_mem(sizeof(command_kind) * num_command_commands);

      for (i = 0 ; i < num_command_commands; i++) {
         command_commands[i] = command_menu[i].command_name;
         command_command_values[i] = command_menu[i].action;
      }

      // Find out how big the startup menu needs to be.

      for (num_startup_commands = 0 ;
           startup_menu[num_startup_commands].startup_name ;
           num_startup_commands++) ;

      startup_commands = (Cstring *) get_mem(sizeof(Cstring) * num_startup_commands);
      startup_command_values =
         (start_select_kind *) get_mem(sizeof(start_select_kind) * num_startup_commands);

      for (i = 0 ; i < num_startup_commands; i++) {
         startup_commands[i] = startup_menu[i].startup_name;
         startup_command_values[i] = startup_menu[i].action;
      }

      // Find out how big the resolve menu needs to be.

      for (number_of_resolve_commands = 0 ;
           resolve_menu[number_of_resolve_commands].command_name ;
           number_of_resolve_commands++) ;

      resolve_command_strings = (Cstring *) get_mem(sizeof(Cstring) * number_of_resolve_commands);
      resolve_command_values = (resolve_command_kind *)
         get_mem(sizeof(resolve_command_kind) * number_of_resolve_commands);

      for (i = 0 ; i < number_of_resolve_commands; i++) {
         resolve_command_strings[i] = resolve_menu[i].command_name;
         resolve_command_values[i] = resolve_menu[i].action;
      }
   }
}

extern int process_session_info(Cstring *error_msg)
{
   int i, j;

   if (session_index == 0)
      return 1;

   if (session_index <= session_linenum) {
      char line[MAX_FILENAME_LENGTH];
      int ccount;
      int num_fields_parsed;
      char junk_name[MAX_FILENAME_LENGTH];
      char filename_string[MAX_FILENAME_LENGTH];
      char session_levelstring[MAX_FILENAME_LENGTH+10];

      // Find the "[Sessions]" indicator again.

      if (!find_init_file_region("[Sessions]", 10)) {
         *error_msg = "Can't find correct position in session file.";
         return 3;
      }

      // Skip over the lines before the one we want.

      for (i=0 ; i<session_index ; i++) {
         if (!fgets(line, MAX_FILENAME_LENGTH, init_file)) break;
      }

      if (i != session_index)
         return 1;

      j = strlen(line);
      if (j>0) line[j-1] = '\0';   // Strip off the <NEWLINE> -- we don't want it.

      num_fields_parsed = sscanf(line, "%s %s %d %n%s",
                                 filename_string, session_levelstring,
                                 &sequence_number, &ccount,
                                 junk_name);

      if (num_fields_parsed < 3) {
         *error_msg = "Bad format in session file.";
         return 3;
      }

      Cstring breakpos = 0;
      Cstring statspos = 0;

      if (!parse_level(session_levelstring, &breakpos)) {
         *error_msg = "Bad level given in session file.";
         return 3;
      }

      // Look for an abridge list or stats list, immediately after the level,
      // separated by a minus sign and/or colon.
      if (breakpos && *breakpos == '-') {
         int len = strlen(breakpos+1);
         statspos = strchr(breakpos+1, ':');
         if (statspos) len = statspos-breakpos-1;

         // If there is already a file name, the operator is overriding
         // the name from the session.  Use the override.  Don't take
         // the name from the session.
         if (abridge_filename[0] == 0) {
            if (len > MAX_TEXT_LINE_LENGTH-1) len = MAX_TEXT_LINE_LENGTH-1;
            strncpy(abridge_filename, breakpos+1, len);
            abridge_filename[len] = 0;
         }

         if (abridge_filename[0] != 0) {
            // The session line specifies an abridgement file.
            // Use it, unless the user specified "delete_abridgement",
            // in which case, we specifically don't use it.
            // In that case, it will be deleted from the session line
            // when the program exits and the initialization file
            // is rewritten.
            glob_abridge_mode =
               (glob_abridge_mode == abridge_mode_deleting_abridge) ?
               abridge_mode_none : abridge_mode_abridging;
         }
      }
      else
         statspos = breakpos;  // Maybe there is a stats name but not an abridge name.

      if (GLOB_stats_filename[0] == 0 && statspos)
         strncpy(GLOB_stats_filename, statspos+1, MAX_TEXT_LINE_LENGTH);

      if (num_fields_parsed == 4)
         strncpy(header_comment, line+ccount, MAX_TEXT_LINE_LENGTH);
      else
         header_comment[0] = 0;

      if (!install_outfile_string(filename_string)) {
         *error_msg = "Bad file name in session file, using default instead.";
         return 2;    // This return code will not abort the session.
      }
   }
   else {
      // We are creating a new session to be appended to the file.
      sequence_number = 1;
      creating_new_session = true;
   }

   return 0;
}


static bool get_accelerator_line(char line[])
{
   for ( ;; ) {
      if (!fgets(line, MAX_FILENAME_LENGTH, init_file) || line[0] == '\n' || line[0] == '[') return false;

      int j = strlen(line);
      if (j>0) line[j-1] = '\0';   // Strip off the <NEWLINE> -- we don't want it.

      if (line[0] != '#') return true;
   }
}


extern void close_init_file()
{
   if (init_file) fclose(init_file);
}


static int write_back_session_line(FILE *wfile)
{
   char *filename = rewrite_filename_as_star[0] ? rewrite_filename_as_star : outfile_string;
   char level_and_abridge_name[MAX_TEXT_LINE_LENGTH];
   strncpy(level_and_abridge_name, getout_strings[calling_level], MAX_TEXT_LINE_LENGTH);

   // Write the abridge file name, unless the abridgement is being deleted.
   if (glob_abridge_mode != abridge_mode_none && abridge_filename[0]) {
      strcat(level_and_abridge_name, "-");
      strcat(level_and_abridge_name, abridge_filename);
   }

   // Write the stats file name.
   if (GLOB_doing_frequency && GLOB_stats_filename[0]) {
      strcat(level_and_abridge_name, ":");
      strcat(level_and_abridge_name, GLOB_stats_filename);
   }

   if (header_comment[0])
      return
         fprintf(wfile, "%-20s %-11s %6d      %s\n",
                 filename,
                 level_and_abridge_name,
                 sequence_number,
                 header_comment);
   else
      return
         fprintf(wfile, "%-20s %-11s %6d\n",
                 filename,
                 level_and_abridge_name,
                 sequence_number);
}


static void rewrite_init_file()
{
   if (session_index != 0 || rewrite_with_new_style_filename) {
      char line[MAX_FILENAME_LENGTH];
      char errmsg[MAX_TEXT_LINE_LENGTH];
      FILE *rfile;
      FILE *wfile;
      int i;

      remove(SESSION2_FILENAME);

      if (rename(SESSION_FILENAME, SESSION2_FILENAME)) {
         strncpy(errmsg, "Failed to save file '" SESSION_FILENAME
                 "' in '" SESSION2_FILENAME "':\n",
                 MAX_TEXT_LINE_LENGTH);
         strncat(errmsg, get_errstring(), MAX_FILENAME_LENGTH);
         strncat(errmsg, ".", MAX_FILENAME_LENGTH);
         gg->serious_error_print(errmsg);
      }
      else {
         if (!(rfile = fopen(SESSION2_FILENAME, "r"))) {
            strncpy(errmsg, "Failed to open '" SESSION2_FILENAME "'.",
                    MAX_TEXT_LINE_LENGTH);
            gg->serious_error_print(errmsg);
         }
         else {
            if (!(wfile = fopen(SESSION_FILENAME, "w"))) {
               strncpy(errmsg, "Failed to open '" SESSION_FILENAME "'.",
                       MAX_TEXT_LINE_LENGTH);
               gg->serious_error_print(errmsg);
            }
            else {
               bool more_stuff = false;

               if (rewrite_with_new_style_filename) {
                  // Search for the "[Options]" indicator, copying stuff that we skip.

                  for (;;) {
                     if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) goto copy_done;
                     if (fputs(line, wfile) == EOF) goto copy_failed;
                     if (!strncmp(line, "[Options]", 9)) break;
                     else if (!strncmp(line, "[Sessions]", 10)) goto got_sessions;
                  }

                  bool got_the_command = false;

                  for (;;) {
                     if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) goto copy_done;

                     if (!strncmp(line, "new_style_filename", 18))
                        got_the_command = true;

                     if (line[0] == '\n' || !strncmp(line, "[Sessions]", 10)) {
                        // At the end of the section.
                        if (!got_the_command) {
                           if (fputs("new_style_filename\n", wfile) == EOF) goto copy_failed;
                        }

                        if (fputs("\n", wfile) == EOF) goto copy_failed;

                        if (line[0] == '\n')
                           goto search_for_sessions;
                        else
                           goto got_sessions;
                     }

                     // Don't copy this line is it is "old_style".
                     if (strncmp(line, "old_style_filename", 18)) {
                        if (fputs(line, wfile) == EOF) goto copy_failed;
                     }
                  }
               }

               // Search for the "[Sessions]" indicator, copying stuff that we skip.

            search_for_sessions:

               for (;;) {
                  if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) goto copy_done;
                  if (fputs(line, wfile) == EOF) goto copy_failed;
                  if (!strncmp(line, "[Sessions]", 10)) goto got_sessions;
               }

            got_sessions:

               for (i=0 ; ; i++) {
                  if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) break;
                  if (line[0] == '\n') { more_stuff = true; break; }

                  if (i == session_index-1) {
                     if (write_back_session_line(wfile) < 0)
                        goto copy_failed;
                  }
                  else if (i == -session_index-1) {
                  }
                  else {
                     if (fputs(line, wfile) == EOF) goto copy_failed;
                  }
               }

               if (i < session_index) {
                  // User has requested a line number larger than the file.
                  // Append a new line.
                  if (write_back_session_line(wfile) < 0)
                     goto copy_failed;
               }

               if (more_stuff) {
                  if (fputs("\n", wfile) == EOF) goto copy_failed;
                  for (;;) {
                     if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) break;
                     if (fputs(line, wfile) == EOF) goto copy_failed;
                  }
               }

               goto copy_done;

            copy_failed:

               strncpy(errmsg, "Failed to write to '" SESSION_FILENAME "'.",
                       MAX_TEXT_LINE_LENGTH);
               gg->serious_error_print(errmsg);

            copy_done:

               fclose(wfile);
            }

            fclose(rfile);

            // Write out the stats file if there is one.

            if (GLOB_doing_frequency && GLOB_stats_filename[0]) {
               stats_file = fopen(GLOB_decorated_stats_filename, "w");

               if (stats_file) {     // If it's gone, don't do anything.
                  for (i=0 ; i<number_of_calls[call_list_any] ; i++) {
                     if (main_call_lists[call_list_any][i]->the_defn.frequency > 0) {
                        fprintf(stats_file, "%-4d %s\n",
                                main_call_lists[call_list_any][i]->the_defn.frequency,
                                main_call_lists[call_list_any][i]->menu_name);
                     }
                  }

                  for (i=0 ; concept_descriptor_table[i].kind != concept_diagnose ; i++) {
                     if (concept_descriptor_table[i].frequency > 0) {
                        fprintf(stats_file, "%-4d %s\n",
                                concept_descriptor_table[i].frequency,
                                concept_descriptor_table[i].menu_name);
                     }
                  }

                  fclose(stats_file);
               }
            }
         }
      }
   }
}


extern void general_final_exit(int code)
{
   // If this is Sd, of course "ttu_initialize" won't have been called.
   // (It doesn't even exist.)
   //
   // But if this is Sdtty, "ttu_initialize" will have been called
   // unless we are just doing "command-line help".  That is, the user typed
   // "sdtty -help".  In that case, "rewrite_init_file", will do nothing,
   // so it won't matter that "ttu_initialize" wasn't called.

   if (glob_abridge_mode < abridge_mode_writing)
      rewrite_init_file();

   // If this is Sdtty, this next procedure will call "ttu_terminate".
   // If we had just been printing command-line help, "ttu_initialize"
   // will not have happened.

   gg->terminate(code);
}



/* This fills the permanent array "main_call_lists[call_list_any]" with the stuff
      read in from the database, including name pointer fields containing the original text
      with "@" escapes.  It also sets "number_of_calls[call_list_any]" to the size thereof.
         It does this only for calls that are legal at this level.  If we are just
         writing a call list with a "write_list" command, a call is legal only if
         its level is exactly the level
         specified when the program was invoked, rather than the usual case of including
         any call whose level is at or below the specified level.  If we are doing a
         "write_full_list" we do the usual action.
   It also fills in the "base_calls" array with tagged calls, independent of level. */

static void build_database(abridge_mode_t abridge_mode)
{
   int i, char_count;
   int local_callcount;

   for (i=0 ; i<NUM_TAGGER_CLASSES ; i++) {
      number_of_taggers[i] = 0;
      tagger_calls[i] = (call_with_name **) 0;
   }

   number_of_circcers = 0;
   circcer_calls = (call_with_name **) 0;

   // This list will be permanent.
   base_calls = (call_with_name **) get_mem(max_base_calls * sizeof(call_with_name *));

   // These two will be temporary.  The first lasts through the entire initialization
   // process.  The second one only in this procedure.
   global_temp_call_indices = (int *) get_mem(abs_max_calls * sizeof(call_with_name *));
   call_with_name **local_call_list =
      (call_with_name **) get_mem(abs_max_calls * sizeof(call_with_name *));

   // Clear the tag list.  Calls will fill this in as they announce themselves.
   for (i=0; i < max_base_calls; i++) base_calls[i] = (call_with_name *) 0;

   highest_base_call = 0;

   read_halfword();

   local_callcount = 0;

   for (;;) {
      int savetag;
      calldef_schema call_schema;

      if ((last_datum & 0xE000) == 0) break;

      if ((last_datum & 0xE000) != 0x2000) {
         database_error_exit("database phase error 1");
      }

      savetag = last_12;     // Get tag, if any.

      read_halfword();       // Get level and 12 bits of "callflags2" stuff.
      dance_level this_level = (dance_level) (last_datum & 0xF);
      uint32 saveflags2 = last_datum >> 4;

      read_fullword();       // Get top level flags, first word.
                             // This is the "callflags1" stuff.
      uint32 saveflags1 = last_datum;

      read_fullword();       // Get top level flags, second word.
                             // This is the "heritflags" stuff.
      uint32 saveflagsh = last_datum;

      read_halfword();       // Get char count and schema.
      call_schema = (calldef_schema) (last_datum & 0xFF);
      char_count = (last_datum >> 8) & 0xFF;

      /* **** We should only allocate the call root if we are going to use this call,
         either because it is tagged or because it is legal at this level!!!! */

      // Now that we know how long the name is, create the block and fill in the saved stuff.
      // We subtract 3 because 4 chars are already present, but we need one extra for the pad.

      call_root = (call_with_name *) get_mem(sizeof(call_with_name) + char_count - 3);
      call_root->menu_name = (Cstring) 0;

      if (savetag) {
         check_tag(savetag);
         base_calls[savetag] = call_root;
      }

      call_root->the_defn.frequency = 0;
      call_root->the_defn.level = (int) this_level;
      call_root->the_defn.schema = call_schema;
      call_root->the_defn.callflags1 = saveflags1;
      call_root->the_defn.callflagsf = saveflags2 << 20;
      // Will get "CFLAGH" and "ESCAPE_WORD" bits later.
      call_root->the_defn.callflagsh = saveflagsh;
      read_in_call_definition(&call_root->the_defn, char_count);

      // We accept a call if:
      // (1) we are writing out just this list, and the call matches the desired level exactly,
      //          or
      // (2) we are writing out this list and those below, and the call is <= the desired level,
      //          or
      // (3) we are running, and the call is <= the desired level or the
      //    "higher acceptable level".
      //    The latter is c3x if the desired level is c3, or c4x if the desired level is c4.
      //    That way, c3x calls will be included.  We will print a warning if they are used.

      if (this_level == calling_level ||
          (abridge_mode != abridge_mode_writing && this_level <= calling_level)) {

         // Process tag base calls specially.
         if (call_root->the_defn.callflags1 & CFLAG1_BASE_TAG_CALL_MASK) {
            int tagclass = ((call_root->the_defn.callflags1 & CFLAG1_BASE_TAG_CALL_MASK) /
                            CFLAG1_BASE_TAG_CALL_BIT) - 1;

            // All classes go into list 0.  Additionally, the other classes
            // go into their own list.
            number_of_taggers[tagclass]++;
            tagger_calls[tagclass] = (call_with_name **)
               get_more_mem(tagger_calls[tagclass],
                            number_of_taggers[tagclass]*sizeof(call_with_name *));
            tagger_calls[tagclass][number_of_taggers[tagclass]-1] = call_root;
            if (tagclass != 0) {
               number_of_taggers[0]++;
               tagger_calls[0] = (call_with_name **)
                  get_more_mem(tagger_calls[0],
                               number_of_taggers[0]*sizeof(call_with_name *));
               tagger_calls[0][number_of_taggers[0]-1] = call_root;
            }
            else if (call_root->the_defn.callflagsf & CFLAGH__TAG_CALL_RQ_MASK) {
               // But anything that invokes a tagging call goes into each list,
               // inheriting its own class.

               // Iterate over all tag classes except class 0.
               for (int xxx=1 ; xxx<NUM_TAGGER_CLASSES ; xxx++) {
                  call_with_name *new_call =
                     (call_with_name *) get_mem(sizeof(call_with_name) + char_count - 3);
                  memcpy(new_call, call_root, sizeof(call_with_name) + char_count - 3);
                  /* Fix it up. */
                  new_call->the_defn.callflagsf =
                     (new_call->the_defn.callflagsf & !CFLAGH__TAG_CALL_RQ_MASK) |
                     CFLAGH__TAG_CALL_RQ_BIT*(xxx+1);
                  number_of_taggers[xxx]++;
                  tagger_calls[xxx] = (call_with_name **)
                     get_more_mem(tagger_calls[xxx],
                                  number_of_taggers[xxx]*sizeof(call_with_name *));
                  tagger_calls[xxx][number_of_taggers[xxx]-1] = new_call;
               }
            }
         }
         else {
            if (local_callcount >= abs_max_calls)
               database_error_exit("Too many base calls -- mkcalls made an error");
            local_call_list[local_callcount++] = call_root;
         }
      }
   }

   // Check that all tagged calls have been filled in.

   for (i=1; i <= highest_base_call; i++) {
      if (!base_calls[i]) {
         char msg [50];
         sprintf(msg, "%d", i);
         gg->fatal_error_exit(1, "Call didn't identify self", msg);
      }
   }

   // Translate the aliases.

   for (i=0 ; i<local_callcount ; i++) {
      while (local_call_list[i]->the_defn.schema == schema_alias) {
         local_call_list[i]->the_defn =
            base_calls[local_call_list[i]->the_defn.stuff.conc.innerdef.call_id]->the_defn;
      }
   }

   // Process the circulate calls.

   for (i=0 ; i<local_callcount ; i++) {
      call_with_name *t = local_call_list[i];
      if (t->the_defn.callflags1 & CFLAG1_BASE_CIRC_CALL) {
         number_of_circcers++;
         circcer_calls = (call_with_name **)
            get_more_mem(circcer_calls, number_of_circcers*sizeof(call_with_name *));
         circcer_calls[number_of_circcers-1] = t;
      }
   }

   number_of_calls[call_list_any] = local_callcount;
   main_call_lists[call_list_any] =
      (call_with_name **) get_mem(local_callcount * sizeof(call_with_name *));

   memcpy(main_call_lists[call_list_any],
          local_call_list,
          local_callcount*sizeof(call_with_name *));

   free(local_call_list);

   fclose(database_file);
}


// We will call "do_tick" with total arguments of 32.
// 2 at the end of "build_database", after we have read the database file.
// 1 after sorting the database.
// 1 after making the universal menu.
// 2 after testing each starting setup and making its menu.  There are 14.

#define TICK_TOTAL 32



/* This cleans up the text of a call or concept name, returning the
   menu-presentable form to be put into the "menu_name" field.  It
   simply re-uses the stored string where it can, and allocates fresh memory
   if a substitution took place. */

static const char *translate_menu_name(const char *orig_name, uint32 *escape_bits_p)
{
   int j;
   char c;
   int namelength;
   bool atsign = false;

   /* See if the name has an 'at' sign, in which case we have to modify it to
      get the actual menu text.  Also, find out what escape flags we need to set. */

   namelength = 0;
   for (;;) {
      c = orig_name[namelength++];
      if (!c) break;
      else if (c == '@') {
         atsign = true;
         c = orig_name[namelength++];
         if (c == 'e') *escape_bits_p |= ESCAPE_WORD__LEFT;
         else if (c == 'j' || c == 'C') *escape_bits_p |= ESCAPE_WORD__CROSS;
         else if (c == 'J' || c == 'M') *escape_bits_p |= ESCAPE_WORD__MAGIC;
         else if (c == 'E' || c == 'I') *escape_bits_p |= ESCAPE_WORD__INTLK;
         else if (c == 'G' || c == 'Q')
            *escape_bits_p |= ESCAPE_WORD__GRAND;
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
            const char *q = get_escape_string(orig_name[++j]);
            if (q && *q) {
               while (*q) temp_ptr[templength++] = *q++;
               continue;
            }
            else if (q) {
               /* Skip over @7...@8, @n .. @o, @j .. @l, and @J...@L stuff. */
               while (orig_name[j] != '@') j++;
               j++;
            }

            // Be sure we don't leave two consecutive blanks in the text.
            if (orig_name[j+1] == ' ' && templength != 0 && temp_ptr[templength-1] == ' ') j++;
         }
         else
            temp_ptr[templength++] = c;
      }

      tempname[templength] = '\0';
      // Must copy the text into some fresh memory, being careful about overflow.
      new_ptr = (char *) get_mem(templength+1);
      for (j=0; j<=templength; j++) new_ptr[j] = tempname[j];
      return new_ptr;
   }
   else
      return orig_name;
}


void conzept::translate_concept_names()
{
   int i;
   uint32 escape_bit_junk;

   for (i=0; conzept::unsealed_concept_descriptor_table[i].kind != marker_end_of_list; i++) {
      unsealed_concept_descriptor_table[i].menu_name =
         translate_menu_name(unsealed_concept_descriptor_table[i].name, &escape_bit_junk);

      unsealed_concept_descriptor_table[i].frequency = 0;
   }

   // "Seal" the concept table.  It will be made visible outside of the
   // concept class as a constant array.

   concept_descriptor_table = conzept::unsealed_concept_descriptor_table;
}

/* BEWARE!!  This list is keyed to the definition of "start_select_kind" in sd.h. */
startinfo configuration::startinfolist[start_select_as_they_are+1];

void configuration::initialize()
{
   configuration::startinfolist[0].name = "???";
   configuration::startinfolist[0].into_the_middle = false;
   configuration::startinfolist[0].the_setup = setup(nothing, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

   configuration::startinfolist[start_select_h1p2p].name = "Heads 1P2P";
   configuration::startinfolist[start_select_h1p2p].into_the_middle = false;
   configuration::startinfolist[start_select_h1p2p].the_setup = setup(s2x4, 1,
                                                                      0300|d_south,ID3_G2, 0200|d_south,ID3_B2,
                                                                      0100|d_south,ID3_G1, 0000|d_south,ID3_B1,
                                                                      0700|d_north,ID3_G4, 0600|d_north,ID3_B4,
                                                                      0500|d_north,ID3_G3, 0400|d_north,ID3_B3);

   configuration::startinfolist[start_select_s1p2p].name = "Sides 1P2P";
   configuration::startinfolist[start_select_s1p2p].into_the_middle = false;
   configuration::startinfolist[start_select_s1p2p].the_setup = setup(s2x4, 0,
                                                                      0500|d_south,ID3_G3, 0400|d_south,ID3_B3,
                                                                      0300|d_south,ID3_G2, 0200|d_south,ID3_B2,
                                                                      0100|d_north,ID3_G1, 0000|d_north,ID3_B1,
                                                                      0700|d_north,ID3_G4, 0600|d_north,ID3_B4);

   configuration::startinfolist[start_select_heads_start].name = "HEADS";
   configuration::startinfolist[start_select_heads_start].into_the_middle = true;
   configuration::startinfolist[start_select_heads_start].the_setup = setup(s2x4, 0,
                                                                            0600|d_east,ID3_B4,  0500|d_south,ID3_G3,
                                                                            0400|d_south,ID3_B3, 0300|d_west,ID3_G2,
                                                                            0200|d_west,ID3_B2,  0100|d_north,ID3_G1,
                                                                            0000|d_north,ID3_B1, 0700|d_east,ID3_G4);

   configuration::startinfolist[start_select_sides_start].name = "SIDES";
   configuration::startinfolist[start_select_sides_start].into_the_middle = true;
   configuration::startinfolist[start_select_sides_start].the_setup = setup(s2x4, 1,
                                                                            0400|d_east,ID3_B3,  0300|d_south,ID3_G2,
                                                                            0200|d_south,ID3_B2, 0100|d_west,ID3_G1,
                                                                            0000|d_west,ID3_B1,  0700|d_north,ID3_G4,
                                                                            0600|d_north,ID3_B4, 0500|d_east,ID3_G3);

   configuration::startinfolist[start_select_as_they_are].name = "From squared set";
   configuration::startinfolist[start_select_as_they_are].into_the_middle = false;
   configuration::startinfolist[start_select_as_they_are].the_setup = setup(s4x4, 0,
                                                                            0000|d_north,ID3_B1, 0100|d_north,ID3_G1,
                                                                            0200|d_west,ID3_B2,  0300|d_west,ID3_G2,
                                                                            0400|d_south,ID3_B3, 0500|d_south,ID3_G3,
                                                                            0600|d_east,ID3_B4,  0700|d_east,ID3_G4,
                                                                            0x9ADE1256);
}


// The internal color scheme is:
//
// 0 - not used
// 1 - substitute yellow
// 2 - red
// 3 - green
// 4 - yellow
// 5 - blue
// 6 - magenta
// 7 - cyan
//
// The substitute yellow is for use when normal_video (white background)
// is selected.  It is dark yellow if available, otherwise black.

// Alternating blue and red.
static int bold_person_colors[8] = {5, 2, 5, 2, 5, 2, 5, 2};

// Red, green, blue, yellow, red for wraparound if coloring by corner.
static int couple_colors_rgby[9] = {2, 2, 3, 3, 5, 5, 4, 4, 2};

// Red, green, yellow, blue.
static int couple_colors_rgyb[8] = {2, 2, 3, 3, 4, 4, 5, 5};

// Yellow, green, red, blue.
static int couple_colors_ygrb[8] = {4, 4, 3, 3, 2, 2, 5, 5};


int useful_concept_indices[UC_extent];


extern void start_stats_file_from_GLOB_stats_filename()
{
   GLOB_doing_frequency = true;
   strncpy(GLOB_decorated_stats_filename, GLOB_stats_filename, MAX_TEXT_LINE_LENGTH);
   if (!strchr(GLOB_stats_filename, '.'))
      strncat(GLOB_decorated_stats_filename, ".txt", MAX_FILENAME_LENGTH);
}


extern bool open_session(int argc, char **argv)
{
   int i, j;
   uint32 uj;
   int argno;
   char line[MAX_FILENAME_LENGTH];
   char **args;
   int nargs = argc;

   GLOB_stats_filename[0] = 0;

   /* Copy the arguments, so that we can use "realloc" to grow the list. */

   args = (char **) get_mem(nargs * sizeof(char *));

   memcpy(args, argv, nargs * sizeof(char *));

   /* Read the initialization file, looking for options. */

   init_file = fopen(SESSION_FILENAME, "r");
   int insert_pos = 1;

   // Search for the "[Options]" indicator.

   if (find_init_file_region("[Options]", 9)) {
      for (;;) {
         char *lineptr = line;

         // Blank line or line starting with left bracket ends the section.
         if (!fgets(&line[1], MAX_FILENAME_LENGTH, init_file) || line[1] == '\n' || line[1] == '[') break;

         j = strlen(&line[1]);
         if (j>0) line[j] = '\0';   /* Strip off the <NEWLINE> -- we don't want it. */
         line[0] = '-';             /* Put a '-' in front of it. */

         for (;;) {
            char token[MAX_FILENAME_LENGTH];
            int newpos;

            /* Break the line into tokens, and insert each as a command-line argument. */

            /* We need to put a blank at the end, so that the "%s %n" spec won't get confused. */

            j = strlen(lineptr);
            if (j > 0 && lineptr[j-1] != ' ') {
               lineptr[j] = ' ';
               lineptr[j+1] = '\0';
            }
            if (sscanf(lineptr, "%s%n ", token, &newpos) != 1) break;

            j = strlen(token)+1;
            nargs++;
            args = (char **) get_more_mem(args, nargs * sizeof(char *));
            for (i=nargs-1 ; i>insert_pos ; i--) args[i] = args[i-1];
            args[insert_pos] = (char *) get_mem(j);
            memcpy(args[insert_pos], token, j);
            insert_pos++;
            lineptr += newpos;
         }
      }
   }

   // This lets the user interface intercept command line arguments that it is interested in.
   gg->process_command_line(&nargs, &args);

   glob_abridge_mode = abridge_mode_none;
   calling_level = l_nonexistent_concept;    /* Mark it uninitialized. */

   for (argno=1; argno < nargs; argno++) {
      if (args[argno][0] == '-') {
         if (strcmp(&args[argno][1], "write_list") == 0) {
            glob_abridge_mode = abridge_mode_writing;
            if (argno+1 < nargs)
               strncpy(abridge_filename, args[argno+1], MAX_TEXT_LINE_LENGTH);
         }
         else if (strcmp(&args[argno][1], "write_full_list") == 0) {
            glob_abridge_mode = abridge_mode_writing_full;
            if (argno+1 < nargs)
               strncpy(abridge_filename, args[argno+1], MAX_TEXT_LINE_LENGTH);
         }
         else if (strcmp(&args[argno][1], "abridge") == 0) {
            glob_abridge_mode = abridge_mode_abridging;
            if (argno+1 < nargs)
               strncpy(abridge_filename, args[argno+1], MAX_TEXT_LINE_LENGTH);
         }
         else if (strcmp(&args[argno][1], "sequence") == 0) {
	     if (argno+1 < nargs) new_outfile_string = args[argno+1];
         }
         else if (strcmp(&args[argno][1], "db") == 0) {
            if (argno+1 < nargs) database_filename = args[argno+1];
         }
         else if (strcmp(&args[argno][1], "sequence_num") == 0) {
            if (argno+1 < nargs) {
               if (sscanf(args[argno+1], "%d", &ui_options.sequence_num_override) != 1)
                  gg->bad_argument("Bad number", args[argno+1], 0);
            }
         }
         else if (strcmp(&args[argno][1], "session") == 0) {
            if (argno+1 < nargs) {
               if (sscanf(args[argno+1], "%d", &ui_options.force_session) != 1)
                  gg->bad_argument("Bad number", args[argno+1], 0);
            }
         }
         else if (strcmp(&args[argno][1], "resolve_test") == 0) {
            if (argno+1 < nargs) {
               if (sscanf(args[argno+1], "%d", &ui_options.resolve_test_minutes) != 1)
                  gg->bad_argument("Bad number", args[argno+1], 0);
            }
         }
         else if (strcmp(&args[argno][1], "print_length") == 0) {
            if (argno+1 < nargs) {
               if (sscanf(args[argno+1], "%d", &ui_options.max_print_length) != 1)
                  gg->bad_argument("Bad number", args[argno+1], 0);
            }
         }
         else if (strcmp(&args[argno][1], "delete_abridge") == 0)
            { glob_abridge_mode = abridge_mode_deleting_abridge; continue; }
         else if (strcmp(&args[argno][1], "no_intensify") == 0)
            { ui_options.no_intensify = true; continue; }
         else if (strcmp(&args[argno][1], "reverse_video") == 0)
            { ui_options.reverse_video = true; continue; }
         else if (strcmp(&args[argno][1], "normal_video") == 0)
            { ui_options.reverse_video = false; continue; }
         else if (strcmp(&args[argno][1], "pastel_color") == 0)
            { ui_options.pastel_color = true; continue; }
         else if (strcmp(&args[argno][1], "bold_color") == 0)
            { ui_options.pastel_color = false; continue; }
         else if (strcmp(&args[argno][1], "no_color") == 0)
            { ui_options.color_scheme = no_color; continue; }
         else if (strcmp(&args[argno][1], "use_magenta") == 0)
            { ui_options.use_magenta = true; continue; }
         else if (strcmp(&args[argno][1], "use_cyan") == 0)
            { ui_options.use_cyan = true; continue; }
         else if (strcmp(&args[argno][1], "color_by_couple") == 0)
            { ui_options.color_scheme = color_by_couple; continue; }
         else if (strcmp(&args[argno][1], "color_by_couple_rgyb") == 0)
            { ui_options.color_scheme = color_by_couple_rgyb; continue; }
         else if (strcmp(&args[argno][1], "color_by_couple_ygrb") == 0)
            { ui_options.color_scheme = color_by_couple_ygrb; continue; }
         else if (strcmp(&args[argno][1], "color_by_corner") == 0)
            { ui_options.color_scheme = color_by_corner; continue; }
         else if (strcmp(&args[argno][1], "no_sound") == 0)
            { ui_options.no_sound = true; continue; }
         else if (strcmp(&args[argno][1], "tab_changes_focus") == 0)
            { ui_options.tab_changes_focus = true; continue; }
         else if (strcmp(&args[argno][1], "keep_all_pictures") == 0)
            { ui_options.keep_all_pictures = true; continue; }
         else if (strcmp(&args[argno][1], "single_click") == 0)
            { ui_options.accept_single_click = true; continue; }
         else if (strcmp(&args[argno][1], "no_checkers") == 0)
            { if (ui_options.no_graphics != 2) ui_options.no_graphics = 1; continue; }
         else if (strcmp(&args[argno][1], "no_graphics") == 0)
            { ui_options.no_graphics = 2; continue; }
         else if (strcmp(&args[argno][1], "diagnostic") == 0)
            { ui_options.diagnostic_mode = true; continue; }
         else if (strcmp(&args[argno][1], "singlespace") == 0)
            { ui_options.singlespace_mode = true; continue; }
         else if (strcmp(&args[argno][1], "no_warnings") == 0)
            { ui_options.nowarn_mode = true; continue; }
         else if (strcmp(&args[argno][1], "concept_levels") == 0)
            { allowing_all_concepts = true; continue; }
         else if (strcmp(&args[argno][1], "minigrand_getouts") == 0)
            { allowing_minigrand = true; continue; }
         else if (strcmp(&args[argno][1], "active_phantoms") == 0)
            { using_active_phantoms = true; continue; }
         else if (strcmp(&args[argno][1], "discard_after_error") == 0)
            { retain_after_error = false; continue; }
         else if (strcmp(&args[argno][1], "retain_after_error") == 0)
            { retain_after_error = true; continue; }
         else if (strcmp(&args[argno][1], "new_style_filename") == 0)
            { filename_strings = new_filename_strings; continue; }
         else if (strcmp(&args[argno][1], "old_style_filename") == 0)
            { filename_strings = old_filename_strings; continue; }
         else
            gg->bad_argument("Unknown flag", args[argno], 0);

         argno++;
         if (argno >= nargs)
            gg->bad_argument("This flag must be followed by a number or file name",
                             args[argno-1], 0);
      }
      else if (!parse_level(args[argno])) {
         gg->bad_argument("Unknown calling level argument", args[argno],
            "Known calling levels: m, p, a1, a2, c1, c2, c3a, c3, c3x, c4a, c4, or c4x.");
      }
   }

   free(args);
   general_initialize();

   /* If we have a calling level at this point, fill in the output file name.
      If we do not have a calling level, we will either get it from the session
      file, in which case we will get the file name also, or we will have to query
      the user.  In the latter case, we will do this step again. */

   if (calling_level != l_nonexistent_concept)
      strncat(outfile_string, filename_strings[calling_level], MAX_FILENAME_LENGTH);

   /* At this point, the command-line arguments, and the preferences in the "[Options]"
      section of the initialization file, have been processed.  Some of those things
      may still interact with the start-up procedure.  They are:

         glob_abridge_mode   [default = abridge_mode_none]
         calling_level       [default = l_nonexistent_concept]
         new_outfile_string  [default = (char *) 0, this is just a pointer]
         abridge_filename    [default = 0 in position 0], this is allocated]
         database_filename   [default = "sd_calls.dat", this is just a pointer]
   */

   // Now open the session file and find out what we are doing.

   // This could return true, either with session_index<0 for deletion,
   // or because of error, to get immediate exit.
   if (gg->init_step(get_session_info, 0)) {
      close_init_file();
      return true;
   }

   // Set up the color translations based on the user's options.

   color_index_list = couple_colors_rgby;   // Default = color_by_couple.

   switch (ui_options.color_scheme) {
   case color_by_gender: case no_color:
      // It doesn't really matter if "no_color" is selected,
      // as long as we put in something.  The Windows interface
      // code simply sets the palette so that all colors are
      // monochrome, and then uses color_index_list.
      color_index_list = bold_person_colors;
      break;
   case color_by_corner:
      color_index_list = couple_colors_rgby+1;
      break;
   case color_by_couple_rgyb:
      color_index_list = couple_colors_rgyb;
      break;
   case color_by_couple_ygrb:
      color_index_list = couple_colors_ygrb;
      break;
   }

   // Make necessary changes.  This is something of a kludge, since we are
   // writing over what should have been a constant array.

   // If the background is white, bright yellow won't be visible.  Change it to
   // dark yellow.  (If reverse_video or no_intensify, the background is black
   // or grey, and bright yellow is OK.)
   if (!ui_options.reverse_video && !ui_options.no_intensify) {
      for (i=0 ; i<8 ; i++) {
         if (color_index_list[i] == 4) color_index_list[i] = 1;
      }
   }

   // If color_by_gender, pastel applies to both red and blue.
   // Otherwise, we need the flags "use_cyan" or "use_magenta".

   if (ui_options.use_cyan ||
       (ui_options.pastel_color && ui_options.color_scheme == color_by_gender)) {
      for (i=0 ; i<8 ; i++) {
         if (color_index_list[i] == 5) color_index_list[i] = 7;
      }
   }

   if (ui_options.use_magenta ||
       (ui_options.pastel_color && ui_options.color_scheme == color_by_gender)) {
      for (i=0 ; i<8 ; i++) {
         if (color_index_list[i] == 2) color_index_list[i] = 6;
      }
   }

   if (ui_options.sequence_num_override > 0)
      sequence_number = ui_options.sequence_num_override;

   if (calling_level == l_nonexistent_concept)
      gg->init_step(final_level_query, 0);

   if (new_outfile_string)
      install_outfile_string(new_outfile_string);

   // Translate the concept menu names, and then export the sealed
   // concept list for ordinary folks to see in a constant array.
   conzept::translate_concept_names();

   // Scan for "useful" concepts, that is, concepts that will help with
   // functions like "normalize".

   for (i=0 ; i<UC_extent ; i++)
      useful_concept_indices[i] = -1;

   for (i=0; concept_descriptor_table[i].kind != marker_end_of_list; i++) {
      if (concept_descriptor_table[i].useful != UC_none) {
         if (useful_concept_indices[concept_descriptor_table[i].useful] >= 0)
            gg->fatal_error_exit(1, "Concept registered twice.");

         useful_concept_indices[concept_descriptor_table[i].useful] = i;
      }
   }

   for (i = 1 ; i < UC_extent ; i++) {
     if (useful_concept_indices[i] < 0)
        gg->fatal_error_exit(1, "Concept failed to register.");
   }

   starting_sequence_number = sequence_number;

   gg->init_step(init_database1, 0);

   initialize_sdlib();
   prepare_to_read_menus();

   // Opening the database sets up the values of
   // abs_max_calls and max_base_calls.
   // Must do before telling the uims so any open failure messages
   // come out first.

   char *sourcenames[2] = {database_filename, abridge_filename};
   bool binaryfileflags[2] = {true, false};
   FILE *database_input_files[2];

   if (glob_abridge_mode >= abridge_mode_writing) {
      database_input_files[1] = fopen(abridge_filename, "w");

      if (!database_input_files[1])
         gg->fatal_error_exit(1, "Can't open abridgement file", abridge_filename);
   }

   {
      char cachename[MAX_TEXT_LINE_LENGTH];
      strncpy(cachename, getout_strings[calling_level], MAX_TEXT_LINE_LENGTH);
      strcat(cachename, "cache");

      MAPPED_CACHE_FILE cache_stuff((glob_abridge_mode == abridge_mode_abridging) ? 2 : 1,
                                    sourcenames, database_input_files,
                                    cachename, 7, binaryfileflags);

      int *mapped_cache = cache_stuff.map_address();

      database_file = database_input_files[0];
      abridge_file = database_input_files[1];

      if (!database_file)
         gg->fatal_error_exit(1, "Can't open database file.");

      if (glob_abridge_mode == abridge_mode_abridging && !abridge_file)
         gg->fatal_error_exit(1, "Can't open abridgement file", abridge_filename);

      char session_error_msg1[200], session_error_msg2[200];
      session_error_msg1[0] = 0;
      session_error_msg2[0] = 0;

      if (read_database_header(session_error_msg1, session_error_msg2))
         gg->fatal_error_exit(1, session_error_msg1, session_error_msg2);

      // This actually reads the calls database file and creates the
      // "any" menu.  It calls init_step(init_calibrate_tick), which calibrates
      // the progress bar.
      build_database(glob_abridge_mode);

      gg->init_step(init_database2, 0);
      gg->init_step(calibrate_tick, TICK_TOTAL);
      gg->init_step(do_tick, 2);

      // Make the cardinal/ordinal tables.

      for (i=0 ; i<NUM_CARDINALS ; i++) {
         int tens_digit = i/10;
         int units_digit = i - tens_digit*10;
         char *cardptr = new char [3]; 
         char *ordptr = new char [5]; 
         cardinals[i] = cardptr;
         ordinals[i] = ordptr;
         if (tens_digit > 0) {
            *cardptr++ = (char) ('0'+tens_digit);
            *ordptr++ = (char) ('0'+tens_digit);
         }
         *cardptr++ = (char) ('0'+units_digit);
         *ordptr++ = (char) ('0'+units_digit);

         if (units_digit == 1 && tens_digit != 1) {
            *ordptr++ = 's';
            *ordptr++ = 't';
         }
         else if (units_digit == 2 && tens_digit != 1) {
            *ordptr++ = 'n';
            *ordptr++ = 'd';
         }
         else if (units_digit == 3 && tens_digit != 1) {
            *ordptr++ = 'r';
            *ordptr++ = 'd';
         }
         else {
            *ordptr++ = 't';
            *ordptr++ = 'h';
         }

         *cardptr++ = (char) 0;
         *ordptr++ = (char) 0;
      }

      cardinals[NUM_CARDINALS] = (Cstring) 0;
      ordinals[NUM_CARDINALS] = (Cstring) 0;

      // Make the translated names for all calls and concepts.  These have the "<...>"
      // phrases, suitable for external display on menus, instead of "@" escapes.

      for (i=0; i<number_of_calls[call_list_any]; i++)
         main_call_lists[call_list_any][i]->menu_name =
            translate_menu_name(main_call_lists[call_list_any][i]->name,
                                &main_call_lists[call_list_any][i]->the_defn.callflagsf);

      for (i=0 ; i<NUM_TAGGER_CLASSES ; i++) {
         for (uj=0; uj<number_of_taggers[i]; uj++)
            tagger_calls[i][uj]->menu_name =
               translate_menu_name(tagger_calls[i][uj]->name,
                                   &tagger_calls[i][uj]->the_defn.callflagsf);
      }

      for (uj=0; uj<number_of_circcers; uj++)
         circcer_calls[uj]->menu_name =
            translate_menu_name(circcer_calls[uj]->name, &circcer_calls[uj]->the_defn.callflagsf);

      // Do the base calls (calls that are used in definitions of other calls).
      // These may have already been done, if they were on the level.
      for (i=1; i <= highest_base_call; i++) {
         if (!base_calls[i]->menu_name)
            base_calls[i]->menu_name =
               translate_menu_name(base_calls[i]->name, &base_calls[i]->the_defn.callflagsf);
      }

      SORT<call_with_name *, DBCOMPARE>::heapsort(main_call_lists[call_list_any],
                                                  number_of_calls[call_list_any]);

      // Now the permanent array "main_call_lists[call_list_any]" has all the legal calls,
      //    including name pointer fields containing the original text with "@" escapes,
      //    sorted alphabetically.
      // The remaining tasks are to make the subcall lists for other setups (e.g.
      //    those calls legal from columns), and clean up the names that we will actually
      //    display in the menus (no "@" signs) and initialize the menus with the
      //    cleaned-up and subsetted text.

      gg->init_step(do_tick, 1);

      // Do special stuff if we are reading or writing an abridgement file.

      if (glob_abridge_mode >= abridge_mode_abridging) {
         if (glob_abridge_mode == abridge_mode_abridging) {
            char abridge_call[MAX_TEXT_LINE_LENGTH];

            while (fgets(abridge_call, 99, abridge_file)) {
               // Remove the newline character.
               int ll;
               while ((ll = strlen(abridge_call)-1) >= 0 && abridge_call[ll] == '\n')
                  abridge_call[ll] = '\0';

               // Search through the call name list for this call.
               // Why don't we use a more efficient search, based on the fact
               // that the call list has been alphabetized?  Because it was
               // alphabetized before the '@' escapes were expanded.  It
               // is no longer in alphabetical order.
               for (i=0; i<number_of_calls[call_list_any]; i++) {
                  if (!strcmp(abridge_call, main_call_lists[call_list_any][i]->menu_name)) {
                     // Delete this call and move all subsequent calls down one position.
                     for (j=i+1; j<number_of_calls[call_list_any]; j++) {
                        main_call_lists[call_list_any][j-1] = main_call_lists[call_list_any][j];
                     }
                     number_of_calls[call_list_any]--;
                     break;
                  }
               }
            }

            if (fclose(abridge_file))
               gg->fatal_error_exit(1, "Can't close abridgement file");
         }
         else {      // Writing a list of some kind.
            for (i=0; i<number_of_calls[call_list_any]; i++) {
               fputs(main_call_lists[call_list_any][i]->menu_name, abridge_file);
               fputs("\n", abridge_file);
            }

            if (fclose(abridge_file))
               gg->fatal_error_exit(1, "Can't close abridgement file");

            gg->init_step(tick_end, 0);

            // That's all!
            close_init_file();
            return true;
         }
      }

      // Now the array "main_call_lists[call_list_any]"
      //    has the stuff for the calls that we will actually use.
      // The remaining tasks are to make the subcall lists for other setups (e.g.
      //    those calls legal from columns), and initialize the menus with the
      //    subsetted text.

      // This is the universal menu.
      gg->create_menu(call_list_any);
      gg->init_step(do_tick, 1);

      // We are going to try to use the cache file, if we have one.
      // The cache file mechanism will check file sizes and creation
      // times for us.  But then we will do some additional checks.
      // There are 6 words that we use for this:
      //
      // (1) The length of the call list that the index arrays point to.
      //     That is, the total number of calls, taking abridgement into
      //     account.  This is the most sensitive and important test.
      //     If the array being indexed into doesn't match, it's not
      //     likely that the indices will be correct.
      // (2) The current level.  This really shouldn't fail, since the
      //     level was encoded into the cache file name.  Furthermore,
      //     a wrong level should result in a wrong number of calls.
      //     But we check anyway.
      // (3) The total number of levels that the program recognizes.
      //     If we change the enumeration, all bets are off.
      // (4) The total number of formations for which we make menus.
      //     If that changes, we can't possibly use the cache.
      // (5) A quick checksum of the call schemata, after sorting.
      //     This should catch changes in the sorting method, or in
      //     the schema assignments.
      // (6) The database version number.  This really shouldn't fail,
      //     since a version change requires a database recompilation,
      //     which changes the file modification time.
      //     But we check anyway.

      int callchecksum = 1234;

      for (i=0; i<number_of_calls[call_list_any]; i++) {
         callchecksum = (((int) main_call_lists[call_list_any][i]->the_defn.schema) -
                         callchecksum * 272279397) & 0x7FFFFFFF;
      }

      call_list_kind cl;

      // What we wanted from the cache.
      int cache_keys[6];

      cache_keys[0] = number_of_calls[call_list_any];
      cache_keys[1] = (int) calling_level;
      cache_keys[2] = (int) l_dontshow;
      cache_keys[3] = (int) call_list_extent;
      cache_keys[4] = callchecksum;
      cache_keys[5] = DATABASE_FORMAT_VERSION;

      global_cache_miss_reason[0] = 0;

      if (!mapped_cache) {
         global_cache_miss_reason[0] = 9;
         global_cache_miss_reason[1] = (int) cache_stuff.get_miss_reason();
      }
      else {
         for (int jj=0 ; jj<6 ; jj++) {
            if (mapped_cache[jj] != cache_keys[jj]) {
               global_cache_miss_reason[0] = jj+1;
               global_cache_miss_reason[1] = mapped_cache[jj];
               global_cache_miss_reason[2] = cache_keys[jj];
            }
         }
      }

      if (global_cache_miss_reason[0] == 0) {
         int cache_menu_words = 6;

         for (cl = call_list_1x8; cl < call_list_extent ; cl = (call_list_kind) (cl+1)) {
            // Read the menu length.
            number_of_calls[cl] = mapped_cache[cache_menu_words++];
            main_call_lists[cl] =
               (call_with_name **) get_mem(number_of_calls[cl] * sizeof(call_with_name *));
            // Read the menu itself.
            memcpy(main_call_lists[cl],
                   mapped_cache+cache_menu_words,
                   number_of_calls[cl]*sizeof(int));
            cache_menu_words += number_of_calls[cl];
         }
      }
      else {
         // The cache read didn't work.  We need to make the menus
         // by testing all calls in all common setups.
         // This is the thing that takes all the time.

         // In all of these setups in which people are facing, they are normal couples.
         // In general, we use the "Callerlab #0" arrangement for things like lines and waves.
         // This makes initialization work for things like star thru, ladies chain, curlique,
         // and half breed thru from waves.
         //
         // But the setup for starting DPT has the appropriate sex for triple star thru.

#define WEST (d_west|PERSON_MOVED|ROLL_IS_L)
#define EAST (d_east|PERSON_MOVED|ROLL_IS_L)
#define NORT (d_north|PERSON_MOVED|ROLL_IS_L)
#define SOUT (d_south|PERSON_MOVED|ROLL_IS_L)

         // RH tidal wave
         test_starting_setup(call_list_1x8,
                             setup(s1x8, 0, 
                                   0600|NORT,ID3_B4, 0500|SOUT,ID3_G3, 0400|SOUT,ID3_B3, 0700|NORT,ID3_G4,
                                   0200|SOUT,ID3_B2, 0100|NORT,ID3_G1, 0000|NORT,ID3_B1, 0300|SOUT,ID3_G2));

         // LH tidal wave
         test_starting_setup(call_list_l1x8,
                             setup(s1x8, 0,
                                   0600|SOUT,ID3_B4, 0500|NORT,ID3_G3, 0400|NORT,ID3_B3, 0700|SOUT,ID3_G4,
                                   0200|NORT,ID3_B2, 0100|SOUT,ID3_G1, 0000|SOUT,ID3_B1, 0300|NORT,ID3_G2));



         // DPT
         test_starting_setup(call_list_dpt,
                             setup(s2x4, 0,
                                   0300|EAST,ID3_G2, 0400|EAST,ID3_B3, 0500|WEST,ID3_G3, 0200|WEST,ID3_B2,
                                   0700|WEST,ID3_G4, 0000|WEST,ID3_B1, 0100|EAST,ID3_G1, 0600|EAST,ID3_B4));

         // completed DPT
         test_starting_setup(call_list_cdpt,
                             setup(s2x4, 0,
                                   0700|WEST,ID3_G4, 0500|WEST,ID3_G3, 0400|EAST,ID3_B3, 0600|EAST,ID3_B4,
                                   0300|EAST,ID3_G2, 0100|EAST,ID3_G1, 0000|WEST,ID3_B1, 0200|WEST,ID3_B2));

         // RCOL
         test_starting_setup(call_list_rcol,
                             setup(s2x4, 0,
                                   0600|EAST,ID3_B4, 0500|EAST,ID3_G3, 0400|EAST,ID3_B3, 0700|EAST,ID3_G4,
                                   0200|WEST,ID3_B2, 0100|WEST,ID3_G1, 0000|WEST,ID3_B1, 0300|WEST,ID3_G2));

         // LCOL
         test_starting_setup(call_list_lcol,
                             setup(s2x4, 0,
                                   0300|WEST,ID3_G2, 0000|WEST,ID3_B1, 0100|WEST,ID3_G1, 0200|WEST,ID3_B2,
                                   0700|EAST,ID3_G4, 0400|EAST,ID3_B3, 0500|EAST,ID3_G3, 0600|EAST,ID3_B4));

         // 8CH
         test_starting_setup(call_list_8ch,
                             setup(s2x4, 0,
                                   0600|EAST,ID3_B4, 0500|WEST,ID3_G3, 0400|EAST,ID3_B3, 0700|WEST,ID3_G4,
                                   0200|WEST,ID3_B2, 0100|EAST,ID3_G1, 0000|WEST,ID3_B1, 0300|EAST,ID3_G2));

         // TBY
         test_starting_setup(call_list_tby,
                             setup(s2x4, 0,
                                   0500|WEST,ID3_G3, 0600|EAST,ID3_B4, 0700|WEST,ID3_G4, 0400|EAST,ID3_B3,
                                   0100|EAST,ID3_G1, 0200|WEST,ID3_B2, 0300|EAST,ID3_G2, 0000|WEST,ID3_B1));

         // LIN
         test_starting_setup(call_list_lin,
                             setup(s2x4, 0,
                                   0300|SOUT,ID3_G2, 0000|SOUT,ID3_B1, 0100|SOUT,ID3_G1, 0200|SOUT,ID3_B2,
                                   0700|NORT,ID3_G4, 0400|NORT,ID3_B3, 0500|NORT,ID3_G3, 0600|NORT,ID3_B4));

         // LOUT
         test_starting_setup(call_list_lout,
                             setup(s2x4, 0,
                                   0600|NORT,ID3_B4, 0500|NORT,ID3_G3, 0400|NORT,ID3_B3, 0700|NORT,ID3_G4,
                                   0200|SOUT,ID3_B2, 0100|SOUT,ID3_G1, 0000|SOUT,ID3_B1, 0300|SOUT,ID3_G2));

         // RWV
         test_starting_setup(call_list_rwv,
                             setup(s2x4, 0,
                                   0600|NORT,ID3_B4, 0500|SOUT,ID3_G3, 0700|NORT,ID3_G4, 0400|SOUT,ID3_B3,
                                   0200|SOUT,ID3_B2, 0100|NORT,ID3_G1, 0300|SOUT,ID3_G2, 0000|NORT,ID3_B1));

         // LWV
         test_starting_setup(call_list_lwv,
                             setup(s2x4, 0,
                                   0600|SOUT,ID3_B4, 0500|NORT,ID3_G3, 0700|SOUT,ID3_G4, 0400|NORT,ID3_B3,
                                   0200|NORT,ID3_B2, 0100|SOUT,ID3_G1, 0300|NORT,ID3_G2, 0000|SOUT,ID3_B1));

         // R2FL
         test_starting_setup(call_list_r2fl,
                             setup(s2x4, 0,
                                   0600|NORT,ID3_B4, 0500|NORT,ID3_G3, 0700|SOUT,ID3_G4, 0400|SOUT,ID3_B3,
                                   0200|SOUT,ID3_B2, 0100|SOUT,ID3_G1, 0300|NORT,ID3_G2, 0000|NORT,ID3_B1));

         // L2FL
         test_starting_setup(call_list_l2fl,
                             setup(s2x4, 0,
                                   0500|SOUT,ID3_G3, 0600|SOUT,ID3_B4, 0400|NORT,ID3_B3, 0700|NORT,ID3_G4,
                                   0100|NORT,ID3_G1, 0200|NORT,ID3_B2, 0000|SOUT,ID3_B1, 0300|SOUT,ID3_G2));

         // tidal column
         create_misc_call_lists(call_list_gcol);
         // qtag
         create_misc_call_lists(call_list_qtag);

         // Write the cache file;

         int cache_menu_words = 6;

         for (cl = call_list_1x8; cl < call_list_extent ; cl = (call_list_kind) (cl+1))
            cache_menu_words += number_of_calls[cl]+1;    // Extra 1 for the menu size

         cache_stuff.map_for_writing(cache_menu_words*4);
         int *cache_write_segment = cache_stuff.map_address();

         if (cache_write_segment) {
            // Write the header.
            cache_write_segment[0] = number_of_calls[call_list_any];
            cache_write_segment[1] = (int) calling_level;
            cache_write_segment[2] = (int) l_dontshow;
            cache_write_segment[3] = (int) call_list_extent;
            cache_write_segment[4] = callchecksum;
            cache_write_segment[5] = DATABASE_FORMAT_VERSION;

            cache_menu_words = 6;

            for (cl = call_list_1x8; cl < call_list_extent ; cl = (call_list_kind) (cl+1)) {
               // Write the menu length.
               cache_write_segment[cache_menu_words++] = number_of_calls[cl];

               // Write the index list.
               memcpy(cache_write_segment+cache_menu_words,
                      main_call_lists[cl],
                      number_of_calls[cl]*sizeof(int));
               cache_menu_words += number_of_calls[cl];
            }
         }
         else
            global_cache_failed_flag = true;
      }

      // Repair the damage to the call lists, that is, turn them from
      // indices into pointers.  Then create the menus.

      for (cl = call_list_1x8; cl < call_list_extent ; cl = (call_list_kind) (cl+1)) {
         for (i=0; i < number_of_calls[cl]; i++)
            main_call_lists[cl][i] = main_call_lists[call_list_any][(int) main_call_lists[cl][i]];
         gg->create_menu(cl);
      }
   }

   // This was global to the initialization, but it goes away also.
   free(global_temp_call_indices);

   // Initialize the special empty call menu.

   main_call_lists[call_list_empty] = empty_menu;
   number_of_calls[call_list_empty] = 0;

   gg->init_step(tick_end, 0);
   matcher_initialize();

   // Read in the "stats" file.
   // If the fopen fails, leave it at zero.  We won't read, but we will write it back with fresh data.
   // If the given name doesn't have a suffix, use ".txt".
   if (GLOB_stats_filename[0]) {
      start_stats_file_from_GLOB_stats_filename();
      stats_file = fopen(GLOB_decorated_stats_filename, "r");
   }

   if (stats_file) {
      char stats_call[MAX_TEXT_LINE_LENGTH];

      while (fgets(stats_call, 99, stats_file)) {
         // Remove the newline character.
         int ll;
         while ((ll = strlen(stats_call)-1) >= 0 && stats_call[ll] == '\n')
            stats_call[ll] = '\0';

         int frequency, ccount;
         char junk[MAX_TEXT_LINE_LENGTH];

         if (sscanf(stats_call, "%d %n%s", &frequency, &ccount, junk) == 2) {

            // Search through the call name list for this call.
            // Why don't we use a more efficient search, based on the fact
            // that the call list has been alphabetized?  Because it was
            // alphabetized before the '@' escapes were expanded.  It
            // is no longer in alphabetical order.
            for (i=0 ; i<number_of_calls[call_list_any] ; i++) {
               if (!strcmp(stats_call+ccount, main_call_lists[call_list_any][i]->menu_name)) {
                  main_call_lists[call_list_any][i]->the_defn.frequency = frequency;
                  break;
               }
            }

            // If that failed, search for concepts.
            if (i == number_of_calls[call_list_any]) {
               for (i=0 ; concept_descriptor_table[i].kind != concept_diagnose ; i++) {
                  if (!strcmp(stats_call+ccount, concept_descriptor_table[i].menu_name)) {
                     concept_descriptor_table[i].frequency = frequency;
                     break;
                  }
               }
            }
         }
      }

      if (fclose(stats_file))
         gg->fatal_error_exit(1, "Can't close stats file");
   }

   // Make the status bar show that we are processing accelerators.
   gg->init_step(do_accelerator, 0);

   {
      bool save_allow = allowing_all_concepts;
      allowing_all_concepts = true;

      // Process the keybindings for user-definable calls, concepts, and commands.

      if (find_init_file_region("[Accelerators]", 14)) {
         char q[INPUT_TEXTLINE_SIZE];
         while (get_accelerator_line(q))
            do_accelerator_spec(q, true);
      }
      else {
         const Cstring *q;
         for (q = concept_key_table ; *q ; q++)
            do_accelerator_spec(*q, true);
      }

      // Now do the abbreviations.

      if (find_init_file_region("[Abbreviations]", 15)) {
         char q[INPUT_TEXTLINE_SIZE];
         while (get_accelerator_line(q))
            do_accelerator_spec(q, false);
      }

      allowing_all_concepts = save_allow;
   }

   close_init_file();
   gg->final_initialize();
   return false;
}
