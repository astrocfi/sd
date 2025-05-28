/* SD -- square dance caller's helper.

    Copyright (C) 1990-1999  William B. Ackerman.

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

    This is for version 32. */

/* This defines the following functions:
   compress_setup
   expand_setup
   update_id_bits
   touch_or_rear_back
   do_matrix_expansion
   initialize_sdlib
   check_for_concept_group
   fail
   fail2
   failp
   specialfail
   warn
   verify_restriction
   assoc
   find_calldef
   clear_people
   rotperson
   rotcw
   rotccw
   clear_person
   copy_person
   copy_rot
   swap_people
   install_person
   install_rot
   scatter
   gather
   process_final_concepts
   really_skip_one_concept
   fix_n_results
   resolve_p
   warnings_are_unacceptable
   normalize_setup
   toplevelmove
   finish_toplevelmove
   get_escape_string
   unparse_call_name
   print_recurse
and the following external variables:
   the_callback_block
   error_message1
   error_message2
   collision_person1
   collision_person2
   last_magic_diamond
   history
   history_ptr
   written_history_items
   written_history_nopic
   base_calls
   cardinals
   ordinals
   direction_names
   tagger_calls
   circcer_calls
   diagnostic_mode
   singing_call_mode
   current_options
   no_search_warnings
   conc_elong_warnings
   dyp_each_warnings
   useless_phan_clw_warnings
   allowing_all_concepts
   using_active_phantoms
   concept_sublist_sizes
   concept_sublists
   good_concept_sublist_sizes
   good_concept_sublists
*/


#include <string.h>

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"



callbackstuff the_callback_block;
char error_message1[MAX_ERR_LENGTH];
char error_message2[MAX_ERR_LENGTH];
uint32 collision_person1;
uint32 collision_person2;
parse_block *last_magic_diamond;
configuration *history = (configuration *) 0; /* allocated in sdmain */
int history_ptr;

/* This tells how much of the history text written to the UI is "safe".  If this
   is positive, the history items up to that number, inclusive, have valid
   "text_line" fields, and have had their text written to the UI.  Furthermore,
   the local variable "text_line_count" is >= any of those "text_line" fields,
   and each "text_line" field shows the number of lines of text that were
   written to the UI when that history item was written.  This variable is
   -1 when none of the history is safe or the state of the text in the UI
   is unknown. */
int written_history_items;
/* When written_history_items is positive, this tells how many of the initial lines
   did not have pictures forced on (other than having been drawn as a natural consequence
   of the "draw_pic" flag being on.)  If this number ever becomes greater than
   written_history_items, that's OK.  It just means that none of the lines had
   forced pictures. */
int written_history_nopic;

callspec_block **base_calls;        /* Gets allocated as array of pointers in sdinit. */

Cstring cardinals[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", (Cstring) 0};
Cstring ordinals[] = {"0th", "1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", "10th", "11th", "12th", "13th", "14th", "15th", (Cstring) 0};

/* BEWARE!!  This list is keyed to the definition of "direction_kind" in sd.h,
   and to the necessary stuff in SDUI. */
/* This array, and the variable "last_direction_kind" below, get manipulated
   at startup in order to remove the "zig-zag" items below A2. */
Cstring direction_names[] = {
   "???",
   "(no direction)",
   "left",
   "right",
   "in",
   "out",
   "back",
   "zig-zag",
   "zag-zig",
   "zig-zig",
   "zag-zag",
   (Cstring) 0};

callspec_block **tagger_calls[NUM_TAGGER_CLASSES];
callspec_block **circcer_calls;


long_boolean diagnostic_mode = FALSE;
int singing_call_mode = 0;
call_conc_option_state current_options;
warning_info no_search_warnings = {{0, 0, 0}};
warning_info conc_elong_warnings = {{0, 0, 0}};
warning_info dyp_each_warnings = {{0, 0, 0}};
warning_info useless_phan_clw_warnings = {{0, 0, 0}};
long_boolean allowing_all_concepts = FALSE;
long_boolean using_active_phantoms = FALSE;


/* These two direct the generation of random concepts when we are searching.
   We make an attempt to generate somewhat plausible concepts, depending on the
   setup we are in.  If we just generated evenly weighted concepts from the entire
   concept list, we would hardly ever get a legal one. */
int concept_sublist_sizes[NUM_CALL_LIST_KINDS];
short int *concept_sublists[NUM_CALL_LIST_KINDS];
/* These two are similar, but contain only "really nice" concepts that
   we are willing to use in exhaustive searches.  Concepts in these
   lists are very "expensive", in that each one causes an exhaustive search
   through all calls (with nearly-exhaustive enumeration of numbers/selectors, etc).
   Also, these concepts will appear in suggested resolves very early on.  So
   we don't want anything the least bit ugly in these lists. */
int good_concept_sublist_sizes[NUM_CALL_LIST_KINDS];
short int *good_concept_sublists[NUM_CALL_LIST_KINDS];



extern void compress_setup(const expand_thing *thing, setup *stuff)
{
   setup temp = *stuff;

   stuff->kind = thing->inner_kind;
   clear_people(stuff);
   gather(stuff, &temp, thing->source_indices, thing->size-1, thing->rot * 011);
   stuff->rotation -= thing->rot;
   canonicalize_rotation(stuff);
}


extern void expand_setup(const expand_thing *thing, setup *stuff)
{
   setup temp = *stuff;

   stuff->kind = thing->outer_kind;
   clear_people(stuff);
   scatter(stuff, &temp, thing->source_indices, thing->size-1, thing->rot * 033);
   stuff->rotation += thing->rot;
   canonicalize_rotation(stuff);
}


extern void update_id_bits(setup *ss)
{
   int i;
   uint32 livemask, j;
   id_bit_table *ptr;
   unsigned short int *face_list = (unsigned short int *) 0;

   static unsigned short int face_qtg[] = {
      3, d_west, 7, d_east,
      2, d_west, 3, d_east,
      7, d_west, 6, d_east,
      1, d_west, 0, d_east,
      4, d_west, 5, d_east,
      1, d_south, 3, d_north,
      3, d_south, 4, d_north,
      0, d_south, 7, d_north,
      7, d_south, 5, d_north,
      ~0};

   static unsigned short int face_2x4[] = {
      3, d_west, 2, d_east,
      2, d_west, 1, d_east,
      1, d_west, 0, d_east,
      4, d_west, 5, d_east,
      5, d_west, 6, d_east,
      6, d_west, 7, d_east,
      0, d_south, 7, d_north,
      1, d_south, 6, d_north,
      2, d_south, 5, d_north,
      3, d_south, 4, d_north,
      ~0};

   static unsigned short int face_2x2[] = {
      1, d_west, 0, d_east,
      2, d_west, 3, d_east,
      0, d_south, 3, d_north,
      1, d_south, 2, d_north,
      ~0};

   static unsigned short int face_1x3[] = {
      2, d_west, 1, d_east,
      1, d_west, 0, d_east,
      ~0};

   static unsigned short int face_2x3[] = {
      2, d_west, 1, d_east,
      1, d_west, 0, d_east,
      3, d_west, 4, d_east,
      4, d_west, 5, d_east,
      0, d_south, 5, d_north,
      1, d_south, 4, d_north,
      2, d_south, 3, d_north,
      ~0};

   static unsigned short int face_1x8[] = {
      1, d_west, 0, d_east,
      2, d_west, 3, d_east,
      4, d_west, 5, d_east,
      7, d_west, 6, d_east,
      3, d_west, 1, d_east,
      5, d_west, 7, d_east,
      6, d_west, 2, d_east,
      ~0};

   static unsigned short int face_4x5[] = {
      1, d_west, 0, d_east,
      2, d_west, 1, d_east,
      3, d_west, 2, d_east,
      4, d_west, 3, d_east,
      8, d_west, 9, d_east,
      7, d_west, 8, d_east,
      6, d_west, 7, d_east,
      5, d_west, 6, d_east,
      16, d_west, 15, d_east,
      17, d_west, 16, d_east,
      18, d_west, 17, d_east,
      19, d_west, 18, d_east,
      13, d_west, 14, d_east,
      12, d_west, 13, d_east,
      11, d_west, 12, d_east,
      10, d_west, 11, d_east,
      0, d_south, 9, d_north,
      1, d_south, 8, d_north,
      2, d_south, 7, d_north,
      3, d_south, 6, d_north,
      4, d_south, 5, d_north,
      9, d_south, 15, d_north,
      8, d_south, 16, d_north,
      7, d_south, 17, d_north,
      6, d_south, 18, d_north,
      5, d_south, 19, d_north,
      15, d_south, 14, d_north,
      16, d_south, 13, d_north,
      17, d_south, 12, d_north,
      18, d_south, 11, d_north,
      19, d_south, 10, d_north,
      ~0};

   for (i=0,j=1,livemask=0 ; i<=setup_attrs[ss->kind].setup_limits ; i++,j<<=1) {
      if (ss->people[i].id1) livemask |= j;
      ss->people[i].id2 &= ~BITS_TO_CLEAR;
   }

   ptr = setup_attrs[ss->kind].id_bit_table_ptr;

   switch (ss->kind) {
   case s_qtag:
      face_list = face_qtg; break;
   case s2x4:
      face_list = face_2x4; break;
   case s1x8:
      face_list = face_1x8; break;
   case s2x2:
      face_list = face_2x2; break;
   case s2x3:
      face_list = face_2x3; break;
   case s4x5:
      face_list = face_4x5; break;
   case s1x3:
      face_list = face_1x3; break;
   }

   if (face_list) {
      for ( ; *face_list != ((unsigned short) ~0) ; ) {
         short idx1 = *face_list++;

         if ((ss->people[idx1].id1 & d_mask) == *face_list++) {
            short idx2 = *face_list++;
            if ((ss->people[idx2].id1 & d_mask) == *face_list++) {
               ss->people[idx1].id2 |= ID2_FACING;
               ss->people[idx2].id2 |= ID2_FACING;
            }
         }
         else
            face_list += 2;
      }

      for (i=0 ; i<=setup_attrs[ss->kind].setup_limits ; i++) {
         if (ss->people[i].id1 && !(ss->people[i].id2 & ID2_FACING))
            ss->people[i].id2 |= ID2_NOTFACING;
      }
   }


   /* Some setups are only recognized for ID bits with certain patterns of population.
       The bit tables make those assumptions, so we have to use the bit tables
       only if those assumptions are satisfied. */

   switch (ss->kind) {
   case s2x6:
      /* **** This isn't really right -- it would allow "outer pairs bingo".
         We really should only allow 2-person calls, unless we say
         "outer triple boxes".  So we're not completely sure what the right thing is. */
      if (livemask == 07474UL || livemask == 0x3CFUL)
         /* Setup is parallelogram, accept slightly stronger table. */
         ptr = id_bit_table_2x6_pg;
      break;
   case s1x10:
      /* We recognize center 4 and center 6 if this has center 6 filled, then a gap,
         then isolated people. */
      if (livemask != 0x3BDUL) ptr = (id_bit_table *) 0;
      break;
   case s3x6:
      // If the center 1x6 is fully occupied, use this better table.
      if ((livemask & 0700700UL) == 0700700UL) ptr = id_bit_table_3x6_with_1x6;
      // Or, if center 1x4 is occupied, use the standard table.
      // Otherwise, fail.
      if ((livemask & 0600600UL) != 0600600UL) ptr = (id_bit_table *) 0;
      break;
   case sbigdmd:
      /* If this is populated appropriately, we can identify "outer pairs". */
      if (livemask == 07474UL || livemask == 0x3CFUL) ptr = id_bit_table_bigdmd_wings;
      break;
   case sbigbone:
      /* If this is populated appropriately, we can identify "outer pairs". */
      if (livemask == 07474UL || livemask == 0x3CFUL) ptr = id_bit_table_bigbone_wings;
      break;
   case sbigdhrgl:
      /* If this is populated appropriately, we can identify "outer pairs". */
      if (livemask == 07474UL || livemask == 0x3CFUL) ptr = id_bit_table_bigdhrgl_wings;
      break;
   case sbighrgl:
      /* If this is populated appropriately, we can identify "outer pairs". */
      if (livemask == 07474UL || livemask == 0x3CFUL) ptr = id_bit_table_bighrgl_wings;
      break;
   case sbigh:
      // We recognize only the center 1x4, and only if it is full.
      if ((livemask & 06060UL) != 06060UL) ptr = (id_bit_table *) 0;
      break;
   case s_525:
      if (livemask == 04747UL) ptr = id_bit_table_525_nw;
      else if (livemask == 07474UL) ptr = id_bit_table_525_ne;
      break;
   case s_343:
      if ((livemask & 0xE7) == 0xE7) ptr = id_bit_table_343_outr;
      else if ((livemask & 0x318) == 0x318) ptr = id_bit_table_343_innr;
      break;
   case s_545:
      if ((livemask & 0xF9F) == 0x387 || (livemask & 0xF9F) == 0xE1C)
         ptr = id_bit_table_545_outr;
      else if ((livemask & 0x3060) == 0x3060)
         ptr = id_bit_table_545_innr;
      break;
   case s3x4:

      /* There are three special things we can recognize from here.
            If the setup is populated as an "H", we use a special table
            (*NOT* the usual one picked up from the setup_attrs list)
            that knows about the center 2 and the outer 6 and all that.
            If the center 2x3 is occupied, we use a table for that.
            If the setup is populated as offset lines/columns/whatever,
            we use the table from the setup_attrs list, that knows about
            the "outer pairs".
            If all else fails, we use the default table. */

      if (livemask == 07171UL) ptr = id_bit_table_3x4_h;
      else if ((livemask & 04646UL) == 04646UL) ptr = id_bit_table_3x4_ctr6;
      else if (livemask == 07474UL || livemask == 06363UL) ptr = id_bit_table_3x4_offset;
      break;
   case s_d3x4:
      if ((livemask & 01616UL) != 01616UL) ptr = (id_bit_table *) 0;
      break;
   case s4x4:
      /* We recognize centers and ends if this is populated as a butterfly. */
      /* Otherwise, we recognize "center 4" only if those 4 spots are occupied. */
      if (livemask == 0x9999UL)
         ptr = id_bit_table_butterfly;
      else if ((livemask & 0x8888UL) != 0x8888UL)
         ptr = (id_bit_table *) 0;
      break;
   case sdeepxwv:
      /* We recognize outer pairs if they are fully populated. */
      if ((livemask & 00303UL) != 00303UL) ptr = (id_bit_table *) 0;
      break;
   case s3dmd:
      /* The standard table requires all points, and centers of center diamond only, occupied.
         But first we look for a few other configurations. */

      /* Look for center 1x6 occupied. */
      if ((livemask & 0xE38UL) == 0xE38UL) ptr = id_bit_table_3dmd_ctr1x6;
      /* Look for center 1x6 having center 1x4 occupied. */
      else if ((livemask & 0xC30UL) == 0xC30UL) ptr = id_bit_table_3dmd_ctr1x4;
      /* Look for center 1x4 and outer points. */
      else if (livemask == 06565UL) ptr = id_bit_table_3dmd_in_out;
      /* Otherwise, see whether to accept default or reject everything. */
      else if (livemask != 04747UL) ptr = (id_bit_table *) 0;
      break;
   case s4dmd:
      // If center diamonds have only centers and outer ones only points, use this.
      if (livemask == 0xC9C9UL) ptr = id_bit_table_4dmd_cc_ee;
      // Or if center 1x4 is occupied, use standard map.
      else if ((livemask & 0xC0C0UL) != 0xC0C0UL) ptr = (id_bit_table *) 0;
      break;
   case s3ptpd:
      /* If the center diamond is full and the inboard points of each outer diamond
         is present, we can do a "triple trade". */
      if (livemask == 06666UL || livemask == 06363UL || livemask == 07272UL)
         ptr = id_bit_table_3ptpd;
      break;
   }

   if (!ptr) return;

   for (i=0; i<=setup_attrs[ss->kind].setup_limits; i++) {
      if (ss->people[i].id1 & BIT_PERSON)
         ss->people[i].id2 |= ptr[i][ss->people[i].id1 & 3];
   }
}







/* Must be a power of 2. */
#define NUM_TOUCH_HASH_BUCKETS 32

static full_expand_thing *touch_hash_table1[NUM_TOUCH_HASH_BUCKETS];
static full_expand_thing *touch_hash_table2[NUM_TOUCH_HASH_BUCKETS];
static full_expand_thing *touch_hash_table3[NUM_TOUCH_HASH_BUCKETS];

#define NUM_EXPAND_HASH_BUCKETS 32

static expand_thing *expand_hash_table[NUM_EXPAND_HASH_BUCKETS];
static expand_thing *compress_hash_table[NUM_EXPAND_HASH_BUCKETS];

extern void initialize_expand_tables(void)
{
   expand_thing *tabp;
   int i;

   for (i=0 ; i<NUM_EXPAND_HASH_BUCKETS ; i++) {
      expand_hash_table[i] = (expand_thing *) 0;
      compress_hash_table[i] = (expand_thing *) 0;
   }

   for (tabp = expand_init_table ; tabp->inner_kind != nothing ; tabp++) {
      uint32 hash_num = (tabp->inner_kind * 25) & (NUM_EXPAND_HASH_BUCKETS-1);
      tabp->next_expand = expand_hash_table[hash_num];
      expand_hash_table[hash_num] = tabp;

      hash_num = (tabp->outer_kind * 25) & (NUM_EXPAND_HASH_BUCKETS-1);
      tabp->next_compress = compress_hash_table[hash_num];
      compress_hash_table[hash_num] = tabp;
   }
}

static void initialize_touch_tables(void)
{
   full_expand_thing *tabp;
   int i;

   for (i=0 ; i<NUM_TOUCH_HASH_BUCKETS ; i++) {
      touch_hash_table1[i] = (full_expand_thing *) 0;
      touch_hash_table2[i] = (full_expand_thing *) 0;
      touch_hash_table3[i] = (full_expand_thing *) 0;
   }

   for (tabp = touch_init_table1 ; tabp->kind != nothing ; tabp++) {
      uint32 hash_num = ((tabp->kind + (5*tabp->live)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);
      tabp->next = touch_hash_table1[hash_num];
      touch_hash_table1[hash_num] = tabp;
   }

   for (tabp = touch_init_table2 ; tabp->kind != nothing ; tabp++) {
      uint32 hash_num = ((tabp->kind + (5*tabp->live)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);
      tabp->next = touch_hash_table2[hash_num];
      touch_hash_table2[hash_num] = tabp;
   }

   for (tabp = touch_init_table3 ; tabp->kind != nothing ; tabp++) {
      uint32 hash_num = ((tabp->kind + (5*tabp->live)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);
      tabp->next = touch_hash_table3[hash_num];
      touch_hash_table3[hash_num] = tabp;
   }
 
   initialize_expand_tables();
}




extern void touch_or_rear_back(
   setup *scopy,
   long_boolean did_mirror,
   int callflags1)
{
   int i;
   uint32 directions, livemask;
   setup stemp;
   const full_expand_thing *tptr;
   const expand_thing *zptr;

   /* We don't understand absurd setups. */
   if (setup_attrs[scopy->kind].setup_limits < 0) return;

   /* We don't do this if doing the last half of a call. */
   if (TEST_HERITBITS(scopy->cmd.cmd_final_flags,INHERITFLAG_LASTHALF)) return;

   if (!(callflags1 & (CFLAG1_REAR_BACK_FROM_R_WAVE|
                       CFLAG1_STEP_TO_WAVE|
                       CFLAG1_REAR_BACK_FROM_QTAG|
                       CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)))
      return;

   if (scopy->cmd.cmd_misc2_flags & CMD_MISC2__IN_Z_MASK)
      remove_z_distortion(scopy);

   directions = 0;
   livemask = 0;

   for (i=0; i<=setup_attrs[scopy->kind].setup_limits; i++) {
      uint32 p = scopy->people[i].id1;
      directions = (directions<<2) | (p&3);
      livemask <<= 2;
      if (p) livemask |= 3;
   }

   /* Check first for rearing back from a wave. */

   if ((callflags1 & (CFLAG1_REAR_BACK_FROM_R_WAVE|CFLAG1_STEP_TO_WAVE)) ==
       CFLAG1_REAR_BACK_FROM_R_WAVE) {
      uint32 hash_num = ((scopy->kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

      for (tptr = touch_hash_table1[hash_num] ; tptr ; tptr = tptr->next) {
         if (tptr->kind == scopy->kind &&
             tptr->live == livemask &&
             ((tptr->dir ^ directions) & tptr->dirmask) == 0) goto found_tptr;
      }

      // A few setups are special -- we allow any combination at all in livemask.

      if (livemask != 0) {
         switch (scopy->kind) {
         case s1x2:
            if (((directions ^ 0x2UL) & livemask) == 0) {
               tptr = &rear_1x2_pair;
               goto found_tptr;
            }
            break;
         case s1x4:
            if (((directions ^ 0x28UL) & livemask) == 0) {
               tptr = &rear_2x2_pair;
               goto found_tptr;
            }
            break;
         case s_bone:
            if (((directions ^ 0xA802UL) & livemask) == 0) {
               tptr = &rear_bone_pair;
               goto found_tptr;
            }
            break;
         }
      }
   }

   /* If we didn't find anything, check for rearing back from a qtag. */

   if ((callflags1 & (CFLAG1_REAR_BACK_FROM_QTAG|CFLAG1_STEP_TO_WAVE)) ==
       CFLAG1_REAR_BACK_FROM_QTAG) {
      uint32 hash_num = ((scopy->kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

      for (tptr = touch_hash_table2[hash_num] ; tptr ; tptr = tptr->next) {
         if (tptr->kind == scopy->kind &&
             tptr->live == livemask &&
             ((tptr->dir ^ directions) & tptr->dirmask) == 0) goto found_tptr;
      }
   }

   /* Finally, try stepping to a wave. */

   if (callflags1 & CFLAG1_STEP_TO_WAVE) {
      uint32 hash_num;

      /* Special stuff:  If lines facing, but people are incomplete, we honor
         an "assume facing lines" command. */

      if (scopy->kind == s2x4 &&
          scopy->cmd.cmd_assume.assump_col == 0 &&
          scopy->cmd.cmd_assume.assump_both == 1 &&
          scopy->cmd.cmd_assume.assumption == cr_li_lo &&
          directions == (livemask & 0xAA00)) {
         livemask = 0xFFFF;
         directions = 0xAA00;
      }

      hash_num = ((scopy->kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

      for (tptr = touch_hash_table3[hash_num] ; tptr ; tptr = tptr->next) {
         if (tptr->kind == scopy->kind &&
             tptr->live == livemask &&
             ((tptr->dir ^ directions) & tptr->dirmask) == 0) {
            goto found_tptr;
         }
      }

      /* A few setups are special -- we allow any combination at all in livemask,
         though we are careful. */

      switch (scopy->kind) {
      case s2x4:
         if (livemask != 0) {
            if (((callflags1 & CFLAG1_STEP_REAR_MASK) == CFLAG1_STEP_TO_WAVE ||
                 livemask == 0xFFFFUL) &&
                ((directions ^ 0x77DDUL) & livemask) == 0) {
               // Check for stepping to parallel waves from an 8-chain.
               tptr = &step_8ch_pair;
               goto found_tptr;
            }
            else if (((0x003C & ~livemask) == 0 || (0x3C00 & ~livemask) == 0) &&
                     ((directions ^ 0x5D75UL) & 0x7D7DUL & livemask) == 0) {
               // Check for stepping to some kind of 1/4 tag
               // from a DPT or trade-by or whatever.
               tptr = &step_qtag_pair;
               goto found_tptr;
            }
         }
         break;
      case s2x2:
         if (livemask != 0 &&
             ((callflags1 & CFLAG1_STEP_REAR_MASK) == CFLAG1_STEP_TO_WAVE ||
              livemask == 0xFF)) {
            if (((directions ^ 0x7DUL) & livemask) == 0) {
               tptr = &step_2x2h_pair;
               goto found_tptr;
            }
            else if (((directions ^ 0xA0UL) & livemask) == 0) {
               tptr = &step_2x2v_pair;
               goto found_tptr;
            }
         }
         break;
      case s_spindle:
         if (livemask != 0 &&
             (((callflags1 & CFLAG1_STEP_REAR_MASK) == CFLAG1_STEP_TO_WAVE ||
               livemask == 0xFFFFUL) &&
              ((directions ^ 0xA802UL) & livemask) == 0)) {
            tptr = &step_spindle_pair;
            goto found_tptr;
         }
         break;
      case sdmd:
         if ((0x33UL & ~livemask) == 0 &&
             ((directions ^ 0xA0UL) & livemask) == 0 &&
             !(callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)) {
            tptr = &step_dmd_pair;
            goto found_tptr;
         }
         break;
      case s_qtag:
         if ((0x0F0FUL & ~livemask) == 0 &&
             ((directions ^ 0xFD57UL) & livemask) == 0 &&
             !(callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)) {
            tptr = &step_qtgctr_pair;
            goto found_tptr;
         }
         break;
      }
   }

   /* We didn't find anything at all.  But we still need to raise an error
      if the caller said "left spin the top" when we were in a right-hand wave. */

   if ((callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) && did_mirror) {
      uint32 aa;
      uint32 bb = ~0UL;
      long_boolean other_test = TRUE;

      switch (scopy->kind) {
         case s2x2:
            other_test = (directions & livemask) != (0x5FUL & livemask);
            aa = 0x28UL;
            break;
         case s2x4:
            other_test = (directions & livemask) != (0x55FFUL & livemask);
            aa = 0x2288UL;
            break;
         case s_bone:   aa = 0x58F2UL; break;
         case s_rigger: aa = 0x58F2UL; break;
         case s1x2:     aa = 0x2UL; break;
         case s1x4:     aa = 0x28UL; break;
         case s1x8:     aa = 0x2882UL; break;
         case s_qtag:   aa = 0x0802UL; bb = 0x0F0F; break;
      }
      if ((directions & livemask & bb) != (aa & livemask) && other_test)
         fail("Setup is not left-handed.");
   }

   return;

   found_tptr:

   /* Check for things that we must not do if "step_to_box" was specified. */

#ifdef NOT_THIS_WAY
   if ((callflags1 & CFLAG1_STEP_REAR_MASK) != CFLAG1_STEP_TO_WAVE && (tptr->forbidden_elongation & 16))
      return;
#endif

   warn(tptr->warning);

   if ((tptr->forbidden_elongation & 4) && scopy->cmd.cmd_misc_flags & CMD_MISC__DOING_ENDS)
      scopy->cmd.prior_elongation_bits =
         (scopy->cmd.prior_elongation_bits & (~3)) | ((scopy->rotation+1) & 3);

   if ((scopy->cmd.prior_elongation_bits & tptr->forbidden_elongation & 3) &&
       (!(scopy->cmd.cmd_misc_flags & CMD_MISC__NO_CHK_ELONG)))
      fail("People are too far away to work with each other on this call.");

   zptr = tptr->expand_lists;
   scopy->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   stemp = *scopy;
   clear_people(scopy);

   if (tptr->forbidden_elongation & 8) {
      gather(scopy, &stemp, zptr->source_indices, zptr->size-1, zptr->rot * 011);
      scopy->rotation -= zptr->rot;
      scopy->kind = zptr->inner_kind;
   }
   else {
      scatter(scopy, &stemp, zptr->source_indices, zptr->size-1, zptr->rot * 033);
      scopy->rotation += zptr->rot;
      scopy->kind = zptr->outer_kind;
   }

   canonicalize_rotation(scopy);
}



extern void do_matrix_expansion(
   setup *ss,
   uint32 concprops,
   long_boolean recompute_id)
{
   uint32 needprops = concprops & CONCPROP__NEED_MASK;
   uint32 needpropbits = NEEDMASK(needprops);

   for (;;) {
      int i;
      uint32 livemask, j;
      const expand_thing *eptr;

      for (i=0, j=1, livemask=0; i<=setup_attrs[ss->kind].setup_limits; i++, j<<=1) {
         if (ss->people[i].id1) livemask |= j;
      }

      // Search for simple things in the hash table.

      uint32 hash_num = (ss->kind * 25) & (NUM_EXPAND_HASH_BUCKETS-1);

      for (eptr=expand_hash_table[hash_num] ; eptr ; eptr=eptr->next_expand) {
         if (eptr->inner_kind == ss->kind &&
             (eptr->expandconcpropmask & needpropbits) &&
             (livemask & eptr->lillivemask) == 0) {
            warn(eptr->expwarning);
            goto expand_me;
         }
      }

      if (ss->kind == s4x4) {
         if (needpropbits &
             (NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) | NEEDMASK(CONCPROP__NEEDK_4DMD))) {
            if (livemask == 0x1717UL) {
               eptr = &exp_4x4_4dm_stuff_a; goto expand_me;
            }
            else if (livemask == 0x7171UL) {
               eptr = &exp_4x4_4dm_stuff_b; goto expand_me;
            }
         }
         else if (needpropbits &
                  (NEEDMASK(CONCPROP__NEEDK_TWINDMD) | NEEDMASK(CONCPROP__NEEDK_TWINQTAG))) {
            uint32 ctrs = ss->people[3].id1 | ss->people[7].id1 |
               ss->people[11].id1 | ss->people[15].id1;
   
            if (ctrs != 0 && (ctrs & 011) != 011) {
               if (needprops == CONCPROP__NEEDK_TWINQTAG) ctrs ^= 1;
               eptr = (ctrs & 1) ? &exp_4x4_4x6_stuff_b : &exp_4x4_4x6_stuff_a; goto expand_me;
            }
         }
      }

      /* If get here, we did NOT see any concept that requires a setup expansion. */

      return;

      expand_me:

      /* If get here, we DID see a concept that requires a setup expansion. */

      expand_setup(eptr, ss);

      /* Most global selectors are disabled if we expanded the matrix. */

      for (i=0; i<MAX_PEOPLE; i++)
         ss->people[i].id2 &= (~GLOB_BITS_TO_CLEAR | (ID2_FACEFRONT|ID2_FACEBACK|ID2_HEADLINE|ID2_SIDELINE));

      /* Put in position-identification bits (leads/trailers/beaus/belles/centers/ends etc.) */
      if (recompute_id) update_id_bits(ss);
   }
}



/* Must be a power of 2. */
#define NUM_RESTR_HASH_BUCKETS 32

typedef struct grzlch {
   setup_kind k;
   call_restriction restr;
   int size;
   veryshort map1[17];
   veryshort map2[17];
   veryshort map3[8];
   veryshort map4[8];
   long_boolean ok_for_assume;
   chk_type check;
   struct grzlch *next;
} restr_initializer;


static restr_initializer restr_init_table0[] = {
   {s4x4,      cr_wave_only,    1,   {0, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 0, 2, 0, 2, 0},
                                 {2, 0, 2, 0, 0, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 0}, {0}, {0}, TRUE,  chk_box},
   {s4x4,      cr_2fl_only,     1,    {0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 0, 2},
                                 {2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2}, {0}, {0}, TRUE,  chk_box},
   {s_qtag,    cr_qtag_mwv,     8, {0, 1, 3, 2, 5, 4, 6, 7, -1}, {0, 2, 0},       {0}, {0}, FALSE, chk_spec_directions},
   {s_qtag,    cr_qtag_mag_mwv, 8, {0, 1, 2, 3, 5, 4, 7, 6, -1}, {0, 2, 0},       {0}, {0}, FALSE, chk_spec_directions},
   {s_spindle, cr_dmd_ctrs_mwv, 6, {0, 6, 1, 5, 2, 4, -1},    {1, 3, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s_short6,  cr_dmd_ctrs_mwv, 4, {0, 2, 5, 3, -1},          {0, 2, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {sdmd,      cr_dmd_ctrs_mwv, 2, {1, 3, -1},                {1, 3, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s_trngl,   cr_dmd_ctrs_mwv, 2, {1, 2, -1},                {0, 2, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s_trngl4,  cr_dmd_ctrs_mwv, 6, {2, 3, -1},                {0, 2, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s_bone6,   cr_dmd_ctrs_mwv, 4, {0, 4, 1, 3, -1},          {1, 3, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s1x4,      cr_dmd_ctrs_mwv, 2, {1, 3, -1},                {0, 2, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s2x4,      cr_dmd_ctrs_mwv, 4, {1, 2, 6, 5, -1},          {0, 2, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s2x3,      cr_dmd_ctrs_mwv, 2, {1, 4, -1},                {1, 3, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s_qtag,    cr_dmd_ctrs_mwv, 4, {3, 2, 6, 7, -1},          {0, 2, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {s_ptpd,    cr_dmd_ctrs_mwv, 4, {1, 3, 7, 5, -1},          {1, 3, 0},          {0}, {0}, FALSE, chk_spec_directions},
   {sdmd,      cr_dmd_ctrs_1f, 2, {1, 3, -1},                 {1, 3, 1},          {0}, {0}, FALSE, chk_spec_directions},
   {s1x4,      cr_dmd_ctrs_1f, 2, {1, 3, -1},                 {0, 2, 1},          {0}, {0}, FALSE, chk_spec_directions},

   {s1x8, cr_wave_only, 8, {0, 1, 3, 2, 6, 7, 5, 4},                              {0}, {0}, {0}, TRUE, chk_wave},
   {s1x8, cr_1fl_only, 4, {0, 4, 1, 5, 2, 6, 3, 7},                               {2}, {0}, {0}, TRUE,  chk_groups},
   {s1x8, cr_all_facing_same, 8, {0, 1, 2, 3, 4, 5, 6, 7},                        {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x8, cr_2fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},                               {0}, {0}, {0}, FALSE, chk_wave},
   {s2x4, cr_4x4couples_only, 4, {0, 4, 1, 5, 2, 6, 3, 7},                        {2}, {0}, {0}, TRUE,  chk_groups},
   {s1x8, cr_4x4couples_only, 4, {0, 4, 1, 5, 2, 6, 3, 7},                        {2}, {0}, {0}, TRUE,  chk_groups},
   {s1x8, cr_4x4_2fl_only, 8, {0, 4, 1, 5, 2, 6, 3, 7},                           {0}, {0}, {0}, FALSE, chk_wave},
   {s1x8, cr_couples_only, 2, {0, 2, 4, 6, 1, 3, 5, 7},                           {4}, {0}, {0}, TRUE,  chk_groups},
   {s1x8, cr_miniwaves, 2, {0, 2, 4, 6, 1, 3, 5, 7},                              {4}, {0}, {0}, TRUE,  chk_anti_groups},
   {s1x3, cr_1fl_only, 3, {0, 1, 2},                                              {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x3, cr_3x3couples_only, 3, {0, 1, 2},                                       {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x3, cr_wave_only, 3, {0, 1, 2},                                             {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x4, cr_wave_only, 4, {0, 1, 3, 2},                                          {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x4, cr_2fl_only, 4, {0, 2, 1, 3},                                           {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x4, cr_1fl_only, 4, {0, 1, 2, 3},                                           {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x4, cr_4x4couples_only, 4, {0, 1, 2, 3},                                    {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x4, cr_all_facing_same, 4, {0, 1, 2, 3},                                    {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x4, cr_couples_only, 2, {0, 2, 1, 3},                                       {2}, {0}, {0}, TRUE,  chk_groups},
   {s1x4, cr_miniwaves, 2, {0, 2, 1, 3},                                          {2}, {0}, {0}, TRUE,  chk_anti_groups},
   {s1x4, cr_magic_only, 4, {0, 1, 2, 3},                                         {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x4, cr_all_ns, 4, {4, 0, 1, 2, 3}, {0},  {0},  {0},  FALSE, chk_dmd_qtag},
   {s1x4, cr_all_ew, 4, {0}, {4, 0, 1, 2, 3}, {0}, {0}, FALSE, chk_dmd_qtag},
   {s2x4, cr_all_ns, 4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0}, {0}, {0}, FALSE, chk_dmd_qtag},
   {s2x4, cr_all_ew, 4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0}, {0}, FALSE, chk_dmd_qtag},
   {s1x8, cr_all_ns, 4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0}, {0}, {0}, FALSE, chk_dmd_qtag},
   {s1x8, cr_all_ew, 4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0}, {0}, FALSE, chk_dmd_qtag},
   {s1x6, cr_miniwaves, 2, {0, 2, 4, 1, 5, 3},                                    {3}, {0}, {0}, TRUE,  chk_anti_groups},
   {s1x6, cr_wave_only, 6, {0, 1, 2, 5, 4, 3},                                    {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x6, cr_1fl_only, 3, {0, 3, 1, 4, 2, 5},                                     {2}, {0}, {0}, TRUE,  chk_groups},
   {s1x6, cr_all_facing_same, 6, {0, 1, 2, 3, 4, 5},                              {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x6, cr_3x3_2fl_only, 6, {0, 3, 1, 4, 2, 5},                                 {0}, {0}, {0}, FALSE, chk_wave},
   {s1x6, cr_3x3couples_only, 3, {0, 3, 1, 4, 2, 5},                              {2}, {0}, {0}, TRUE,  chk_groups},
   {s2x3, cr_3x3couples_only, 3, {0, 3, 1, 4, 2, 5},                              {2}, {0}, {0}, TRUE,  chk_groups},
   {s1x10, cr_wave_only, 10, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},         {0},         {0}, {0}, TRUE, chk_wave},
   {s1x12, cr_wave_only, 12, {0, 1, 2, 3, 4, 5, 7, 6, 9, 8, 11, 10},              {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x14, cr_wave_only, 14, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},      {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x16, cr_wave_only, 16, {0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10, 13, 12, 15, 14},{0}, {0}, {0}, TRUE, chk_wave},
   {s1x16, cr_couples_only, 2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {8}, {0}, {0}, TRUE,  chk_groups},
   {s1x16, cr_miniwaves, 2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {8}, {0}, {0}, TRUE,  chk_anti_groups},
   {s2x3, cr_all_facing_same, 6, {0, 1, 2, 3, 4, 5},                              {1}, {0}, {0}, TRUE,  chk_groups},
   {s2x3, cr_1fl_only, 3, {0, 3, 1, 4, 2, 5},                                     {2}, {0}, {0}, TRUE,  chk_groups},
   {s2x4, cr_2fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},                               {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x4, cr_wave_only, 8, {0, 1, 2, 3, 5, 4, 7, 6},                              {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x3, cr_wave_only, 6, {0, 1, 2, 3, 4, 5},                                    {0}, {0}, {0}, FALSE, chk_wave},
   {s2x3, cr_magic_only, 6, {0, 1, 2, 3, 4, 5},                                   {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x4, cr_magic_only, 8, {0, 1, 3, 2, 5, 4, 6, 7},                             {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x4, cr_li_lo, 8, {4, 0, 5, 1, 6, 2, 7, 3},                                  {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x4, cr_ctrs_in_out, 4, {5, 1, 6, 2},                                        {0}, {0}, {0}, FALSE, chk_wave},
   {s2x4, cr_indep_in_out, 0, {3, 2, 3, 2, 1, 0, 1, 0},                           {0}, {0}, {0}, FALSE, chk_indep_box},
   {s2x4, cr_all_facing_same, 8, {0, 1, 2, 3, 4, 5, 6, 7},                        {1}, {0}, {0}, TRUE,  chk_groups},
   {s2x4, cr_1fl_only, 4, {0, 4, 1, 5, 2, 6, 3, 7},                               {2}, {0}, {0}, TRUE,  chk_groups},
   {s2x4, cr_couples_only, 2, {0, 2, 4, 6, 1, 3, 5, 7},                           {4}, {0}, {0}, TRUE,  chk_groups},
   {s2x4, cr_miniwaves, 2, {0, 2, 4, 6, 1, 3, 5, 7},                              {4}, {0}, {0}, TRUE,  chk_anti_groups},
   {s_qtag, cr_wave_only, 4, {6, 7, 3, 2},                                        {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_2fl_only, 4, {6, 2, 7, 3},                                         {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_dmd_not_intlk, 4, {6, 7, 3, 2},                                    {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_dmd_intlk, 4, {6, 2, 7, 3},                                        {0}, {0}, {0}, TRUE,  chk_wave},
   {s_ptpd, cr_dmd_not_intlk, 4, {0, 2, 6, 4},                                    {0}, {0}, {0}, TRUE,  chk_wave},
   {s_ptpd, cr_dmd_intlk, 4, {0, 6, 2, 4},                                        {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_miniwaves, 2, {6, 2, 7, 3},                                        {2}, {0}, {0}, TRUE,  chk_anti_groups},
   {s_qtag, cr_couples_only, 2, {6, 2, 7, 3},                                     {2}, {0}, {0}, TRUE,  chk_groups},
   {s_bone, cr_miniwaves, 2, {6, 2, 7, 3},                                        {2}, {0}, {0}, TRUE,  chk_anti_groups},
   {s_bone, cr_couples_only, 2, {6, 2, 7, 3},                                     {2}, {0}, {0}, TRUE,  chk_groups},
   {s_bone6, cr_wave_only, 6, {0, 1, 2, 3, 4, 5},                                     {0}, {0}, {0}, TRUE, chk_wave},
   {s2x8, cr_wave_only, 16, {0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10, 13, 12, 15, 14},{0}, {0}, {0}, TRUE, chk_wave},
   {s2x8, cr_4x4_2fl_only, 16, {0, 4, 1, 5, 2, 6, 3, 7, 12, 8, 13, 9, 14, 10, 15, 11},{0},{0},{0},FALSE,chk_wave},
   {s1x16, cr_4x4couples_only, 4, {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15}, {4}, {0}, {0}, TRUE, chk_groups},
   {s2x8, cr_4x4couples_only, 4, {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15}, {4}, {0}, {0}, TRUE,  chk_groups},
   {s2x8, cr_couples_only, 2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {8}, {0}, {0}, TRUE,  chk_groups},
   {s2x8, cr_miniwaves, 2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {8}, {0}, {0}, FALSE, chk_anti_groups},
   {s2x6, cr_wave_only, 12, {0, 1, 2, 3, 4, 5, 7, 6, 9, 8, 11, 10},               {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x6, cr_3x3_2fl_only, 12, {0, 3, 1, 4, 2, 5, 9, 6, 10, 7, 11, 8},            {0}, {0}, {0}, FALSE, chk_wave},
   {s2x6, cr_3x3couples_only, 3, {0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11},          {4}, {0}, {0}, TRUE,  chk_groups},
   {s1x12, cr_3x3couples_only, 3, {0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11},         {4}, {0}, {0}, TRUE,  chk_groups},
   {s1x2, cr_wave_only, 2, {0, 1},                                                {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x2, cr_2fl_only, 2, {0, 1},                                                 {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x2, cr_couples_only, 2, {0, 1},                                             {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x2, cr_all_facing_same, 2, {0, 1},                                          {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x2, cr_miniwaves, 2, {0, 1},                                                {1}, {0}, {0}, TRUE,  chk_anti_groups},
   {s2x2, cr_couples_only, 0, {1, 0, 3, 2},                              {3, 2, 1, 0}, {0}, {0}, FALSE, chk_box_dbl},
   {s2x2, cr_miniwaves, 2, {1, 0, 3, 2},                                 {3, 2, 1, 0}, {0}, {0}, FALSE, chk_box_dbl},
   {s2x2, cr_peelable_box, 0, {3, 2, 1, 0},                              {1, 0, 3, 2}, {0}, {0}, FALSE, chk_box_dbl},
   {s2x4, cr_reg_tbone, 1, { 3, 2, 0, 1, 1, 0, 2, 3},        {0, 3, 1, 2, 2, 1, 3, 0}, {0}, {0}, FALSE, chk_box},
   {s2x2, cr_reg_tbone, 1, {2, 1, 0, 3},                                 {2, 3, 0, 1}, {0}, {0}, FALSE, chk_box},
   {s1x4, cr_reg_tbone, 1, {3, 2, 1, 0},                                 {0, 3, 2, 1}, {0}, {0}, FALSE, chk_box},
   {s3x4, cr_wave_only, 12, {0, 1, 2, 3, 5, 4, 7, 6, 9, 8, 10, 11},               {0}, {0}, {0}, TRUE,  chk_wave},
   {s3x4, cr_2fl_only, 12, {0, 2, 1, 3, 8, 4, 9, 5, 10, 6, 11, 7},                {0}, {0}, {0}, TRUE,  chk_wave},
   {s_thar, cr_wave_only, 8, {0, 1, 2, 3, 5, 4, 7, 6},        /* NOTE THE 4 --> */{4}, {0}, {0}, TRUE,  chk_wave},
   {s_thar, cr_1fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},                             {0}, {0}, {0}, FALSE, chk_wave},
   {s_thar, cr_magic_only, 8, {0, 1, 3, 2, 5, 4, 6, 7},       /* NOTE THE 4 --> */{4}, {0}, {0}, TRUE,  chk_wave},
   {s_crosswave, cr_wave_only, 8, {0, 1, 2, 3, 5, 4, 7, 6},   /* NOTE THE 4 --> */{4}, {0}, {0}, TRUE,  chk_wave},
   {s_crosswave, cr_1fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},                        {0}, {0}, {0}, FALSE, chk_wave},
   {s_crosswave, cr_magic_only, 8, {0, 1, 3, 2, 5, 4, 6, 7},  /* NOTE THE 4 --> */{4}, {0}, {0}, TRUE,  chk_wave},
   {s3x1dmd, cr_wave_only, 6, {0, 1, 2, 6, 5, 4},                                 {0}, {0}, {0}, TRUE,  chk_wave},
   {s_2x1dmd, cr_wave_only, 4, {0, 1, 4, 3},                                      {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_real_1_4_tag, 4, {6, 7, 3, 2},                         {4, 4, 0, 5, 1}, {0}, {0}, TRUE,  chk_qtag},
   {s_qtag, cr_real_3_4_tag, 4, {6, 7, 3, 2},                         {4, 0, 4, 1, 5}, {0}, {0}, TRUE,  chk_qtag},
   {s_qtag, cr_real_1_4_line, 4, {6, 3, 7, 2},                        {4, 4, 0, 5, 1}, {0}, {0}, TRUE,  chk_qtag},
   {s_qtag, cr_real_3_4_line, 4, {6, 3, 7, 2},                        {4, 0, 4, 1, 5}, {0}, {0}, TRUE,  chk_qtag},
   {s_trngl, cr_wave_only, 2, {1, 2},                                             {0}, {0}, {0}, TRUE,  chk_wave},
   {s_trngl, cr_miniwaves, 2, {1, 2},                                             {0}, {0}, {0}, TRUE,  chk_wave},
   {s_trngl, cr_couples_only, 2, {1, 2},                                          {1}, {0}, {0}, TRUE,  chk_groups},
   {s_trngl, cr_2fl_only, 3, {0, 1, 2},                                           {1}, {0}, {0}, TRUE,  chk_groups},
   {nothing}};

static restr_initializer restr_init_table1[] = {
   {s2x4, cr_quarterbox_or_col, 0, {3, 0, 1, 2}, {3, 4, 5, 6},       {3, 1, 2, 3}, {3, 5, 6, 7}, FALSE, chk_qbox},
   {s2x4, cr_quarterbox_or_magic_col, 0, {3, 1, 2, 7}, {3, 3, 5, 6}, {3, 1, 2, 4}, {3, 0, 5, 6}, FALSE, chk_qbox},
   {s2x3, cr_quarterbox_or_col, 0, {2, 0, 1}, {2, 3, 4}, {2, 1, 2}, {2, 4, 5},                   FALSE, chk_qbox},
   {s2x3, cr_quarterbox_or_magic_col, 0, {2, 1, 5}, {2, 2, 4}, {2, 1, 3}, {2, 0, 4},             FALSE, chk_qbox},
   {s2x3, cr_wave_only, 6, {0, 3, 1, 4, 2, 5},                                     {0}, {0}, {0}, TRUE, chk_wave},
   {s2x3, cr_magic_only, 6, {0, 1, 2, 3, 4, 5},                {0},                    {0}, {0}, TRUE,  chk_wave},
   {s2x3, cr_peelable_box, 3, {0, 1, 2},                   {3, 4, 5},                  {0}, {0}, FALSE, chk_peelable},
   {s2x3, cr_all_facing_same, 6, {0, 1, 2, 3, 4, 5},                              {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x4, cr_all_facing_same, 4, {0, 1, 2, 3},                                    {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x4, cr_2fl_only, 4, {0, 2, 1, 3},                                           {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x4, cr_li_lo, 4, {0, 1, 3, 2},                                              {0}, {0}, {0}, TRUE,  chk_wave},
   {s1x6, cr_all_facing_same, 6, {0, 1, 2, 3, 4, 5},                              {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x8, cr_all_facing_same, 8, {0, 1, 2, 3, 4, 5, 6, 7},                        {1}, {0}, {0}, TRUE,  chk_groups},
   {s2x4, cr_li_lo, 8, {0, 1, 2, 3, 5, 4, 7, 6},                                  {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x4, cr_ctrs_in_out, 4, {1, 2, 6, 5},                                        {0}, {0}, {0}, FALSE, chk_wave},
   {s2x4, cr_2fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},                               {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x4, cr_wave_only, 8, {0, 4, 1, 5, 2, 6, 3, 7},                              {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x4, cr_magic_only, 8, {0, 1, 3, 2, 5, 4, 6, 7},                             {0}, {0}, {0}, TRUE,  chk_wave},
   {s2x4, cr_peelable_box, 4, {0, 1, 2, 3},                {4, 5, 6, 7},               {0}, {0}, FALSE, chk_peelable},
   {s2x4, cr_couples_only, 2, {0, 1, 2, 3, 7, 6, 5, 4},                           {4}, {0}, {0}, TRUE,  chk_groups},
   {s2x4, cr_miniwaves, 2, {0, 1, 2, 3, 7, 6, 5, 4},                              {4}, {0}, {0}, TRUE,  chk_anti_groups},
   {s2x4, cr_all_facing_same, 8, {0, 1, 2, 3, 4, 5, 6, 7},                        {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x2, cr_all_facing_same, 2, {0, 1},                                          {1}, {0}, {0}, TRUE,  chk_groups},
   {s1x2, cr_li_lo, 2, {0, 1},                                                    {0}, {0}, {0}, TRUE, chk_wave},
   {s2x4, cr_reg_tbone, 1, {2, 1, 2, 1, 0, 3, 0, 3},    {2, 3, 2, 3, 0, 1, 0, 1},      {0}, {0}, FALSE, chk_box},
   {s2x6, cr_wave_only, 12, {0, 6, 1, 7, 2, 8, 3, 9, 4, 10, 5, 11},               {0}, {0}, {0}, TRUE, chk_wave},
   {s2x6, cr_peelable_box, 6, {0, 1, 2, 3, 4, 5},          {6, 7, 8, 9, 10, 11},           {0}, {0}, FALSE, chk_peelable},
   {s2x6, cr_couples_only, 3, {0, 1, 2, 3, 4, 5, 11, 10, 9, 8, 7, 6},                 {4}, {0}, {0}, TRUE,  chk_groups},
   {s2x5, cr_wave_only, 10, {0, 5, 1, 6, 2, 7, 3, 8, 4, 9},                       {0}, {0}, {0}, TRUE, chk_wave},
   {s2x5, cr_peelable_box, 5, {0, 1, 2, 3, 4},                        {5, 6, 7, 8, 9}, {0}, {0}, FALSE, chk_peelable},
   {s2x8, cr_wave_only, 16, {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15},{0}, {0}, {0}, TRUE, chk_wave},
   {s2x8, cr_peelable_box, 8, {0, 1, 2, 3, 4, 5, 6, 7},    {8, 9, 10, 11, 12, 13, 14, 15}, {0}, {0}, FALSE, chk_peelable},
   {s2x8, cr_couples_only, 2, {0, 1, 2, 3, 4, 5, 6, 7, 15, 14, 13, 12, 11, 10, 9, 8}, {8}, {0}, {0}, TRUE,  chk_groups},
   {s_qtag, cr_wave_only, 4, {2, 3, 7, 6},                                        {0}, {0}, {0}, FALSE, chk_wave},
   {s_short6, cr_wave_only, 6, {1, 0, 3, 2, 5, 4},                                {0}, {0}, {0}, TRUE,  chk_wave},
   {s_trngl, cr_wave_only, 2, {1, 2},                                             {0}, {0}, {0}, TRUE,  chk_wave},   /* isn't this bogus?  It checks for TANDEM-BASE. */
   {sdmd, cr_wave_only, 2, {1, 3},                                                {0}, {0}, {0}, TRUE,  chk_wave},
   {sdmd, cr_miniwaves, 2, {1, 3},                                                {0}, {0}, {0}, TRUE,  chk_wave},
   {s_ptpd, cr_wave_only, 4, {1, 3, 7, 5},                                        {0}, {0}, {0}, TRUE,  chk_wave},
   {s_ptpd, cr_miniwaves, 2, {1, 7, 3, 5},                                        {2}, {0}, {0}, TRUE,  chk_anti_groups},
   {nothing}};

static restr_initializer restr_init_table4[] = {
   {sdmd, cr_jright, 4, {0, 2, 1, 3},                                             {0}, {0}, {0}, TRUE,  chk_wave},
   {sdmd, cr_jleft, 4, {0, 2, 3, 1},                                              {0}, {0}, {0}, TRUE,  chk_wave},
   {s_ptpd, cr_jright, 8, {0, 2, 1, 3, 6, 4, 7, 5},                               {0}, {0}, {0}, TRUE,  chk_wave},
   {s_ptpd, cr_jleft, 8, {0, 2, 3, 1, 6, 4, 5, 7},                                {0}, {0}, {0}, TRUE,  chk_wave},
   {nothing}};

static restr_initializer restr_init_table9[] = {
   {s_qtag, cr_jleft, 8, {2, 3, 0, 4, 7, 6, 1, 5},        {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_jright, 8, {3, 2, 0, 4, 6, 7, 1, 5},       {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_ijleft, 8, {2, 6, 0, 4, 3, 7, 1, 5},       {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_ijright, 8, {6, 2, 0, 4, 7, 3, 1, 5},      {0}, {0}, {0}, TRUE,  chk_wave},
   {s_qtag, cr_diamond_like, 4, {4, 2, 3, 6, 7}, {4, 0, 1, 4, 5},
                                                               {0}, {0}, FALSE, chk_dmd_qtag},
   {s_qtag, cr_qtag_like, 4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0},
                                                   {2, 4, 5}, {2, 0, 1}, FALSE, chk_dmd_qtag},
   {s_qtag, cr_pu_qtag_like, 0, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},
                                                   {2, 0, 1}, {2, 4, 5}, FALSE, chk_dmd_qtag},
   {s3dmd, cr_diamond_like, 0, {6, 3, 4, 5, 9, 10, 11}, {6, 0, 1, 2, 6, 7, 8},
                                                               {0}, {0}, FALSE, chk_dmd_qtag},
   {s3dmd, cr_qtag_like, 0, {12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {0},
                                             {3, 6, 7, 8}, {3, 0, 1, 2}, FALSE, chk_dmd_qtag},
   {s3dmd, cr_pu_qtag_like, 0, {0}, {12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
                                             {3, 0, 1, 2}, {3, 6, 7, 8}, FALSE, chk_dmd_qtag},
   {s4dmd, cr_jleft, 16, {4, 5, 0, 8, 6, 7, 1, 9, 15, 14, 2, 10, 13, 12, 3, 11},
                                                          {0}, {0}, {0}, TRUE, chk_wave},
   {s4dmd, cr_jright, 16, {5, 4, 0, 8, 7, 6, 1, 9, 14, 15, 2, 10, 12, 13, 3, 11},
                                                          {0}, {0}, {0}, TRUE, chk_wave},
   {s4dmd, cr_diamond_like, 8, {8, 4, 5, 6, 7, 12, 13, 14, 15},
                                {8, 0, 1, 2, 3, 8, 9, 10, 11}, {0}, {0}, FALSE, chk_dmd_qtag},
   {s4dmd, cr_qtag_like, 4, {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                {0}, {4, 8, 9, 10, 11}, {4, 0, 1, 2, 3}, FALSE, chk_dmd_qtag},
   {s4dmd, cr_pu_qtag_like, 0, {0}, {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                                     {4, 0, 1, 2, 3}, {4, 8, 9, 10, 11}, FALSE, chk_dmd_qtag},
   {sdmd, cr_jright, 4, {2, 0, 1, 3},                     {8}, {0}, {0}, TRUE,  chk_wave},
   {sdmd, cr_jleft, 4, {2, 0, 3, 1},                      {8}, {0}, {0}, TRUE,  chk_wave},
   {sdmd, cr_diamond_like, 4, {2, 0, 2}, {2, 1, 3},            {0}, {0}, FALSE, chk_dmd_qtag},
   {sdmd, cr_qtag_like, 4, {0}, {4, 0, 1, 2, 3},         {1, 0}, {1, 2}, FALSE, chk_dmd_qtag},
   {sdmd, cr_pu_qtag_like, 0, {4, 0, 1, 2, 3}, {0},      {1, 0}, {1, 2}, FALSE, chk_dmd_qtag},
   {s_ptpd, cr_jright, 8, {2, 0, 1, 3, 4, 6, 7, 5},       {8}, {0}, {0}, TRUE,  chk_wave},
   {s_ptpd, cr_jleft, 8, {2, 0, 3, 1, 4, 6, 5, 7},        {8}, {0}, {0}, TRUE,  chk_wave},
   {s_ptpd, cr_diamond_like, 4, {4, 0, 2, 4, 6}, {4, 1, 3, 5, 7},
                                                               {0}, {0}, FALSE, chk_dmd_qtag},
   {s_ptpd, cr_qtag_like, 4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},
                                                   {2, 0, 6}, {2, 2, 4}, FALSE, chk_dmd_qtag},
   {s_ptpd, cr_pu_qtag_like, 0, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0},
                                                   {2, 0, 6}, {2, 2, 4}, FALSE, chk_dmd_qtag},
   {s2x4, cr_gen_qbox, 4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},
                                                   {2, 1, 2}, {2, 5, 6}, FALSE, chk_dmd_qtag},
   {s2x2, cr_wave_only, 1, {2, 0, 0, 2}, {0, 0, 2, 2},         {0}, {0}, TRUE,  chk_box},
   {s2x2, cr_all_facing_same, 1, {2, 2, 2, 2}, {0, 0, 0, 0},   {0}, {0}, TRUE,  chk_box},
   {s2x2, cr_2fl_only, 1, {2, 2, 2, 2}, {0, 0, 0, 0},          {0}, {0}, TRUE,  chk_box},
   {s2x2, cr_trailers_only, 1, {0, 0, 2, 2}, {0, 2, 2, 0},     {0}, {0}, TRUE,  chk_box},
   {s2x2, cr_leads_only, 1, {0, 0, 2, 2}, {0, 2, 2, 0},        {0}, {0}, TRUE,  chk_box},
   {s2x2, cr_li_lo, 1, {0, 0, 2, 2}, {0, 2, 2, 0},             {0}, {0}, TRUE,  chk_box},
   {s2x2, cr_indep_in_out, 0, {3, 2, 1, 0},               {0}, {0}, {0}, FALSE, chk_indep_box},
   {s2x2, cr_magic_only, 1, {2, 0, 2, 0}, {0, 2, 0, 2},        {0}, {0}, TRUE,  chk_box},
   {nothing}};

static restr_initializer *restr_hash_table0[NUM_RESTR_HASH_BUCKETS];
static restr_initializer *restr_hash_table1[NUM_RESTR_HASH_BUCKETS];
static restr_initializer *restr_hash_table4[NUM_RESTR_HASH_BUCKETS];
static restr_initializer *restr_hash_table9[NUM_RESTR_HASH_BUCKETS];


static void initialize_restr_tables(void)
{
   restr_initializer *tabp;
   int i;

   for (i=0 ; i<NUM_RESTR_HASH_BUCKETS ; i++) {
      restr_hash_table0[i] = (restr_initializer *) 0;
      restr_hash_table1[i] = (restr_initializer *) 0;
      restr_hash_table4[i] = (restr_initializer *) 0;
      restr_hash_table9[i] = (restr_initializer *) 0;
   }

   for (tabp = restr_init_table0 ; tabp->k != nothing ; tabp++) {
      uint32 hash_num = ((tabp->k + (5*tabp->restr)) * 25) & (NUM_RESTR_HASH_BUCKETS-1);
      tabp->next = restr_hash_table0[hash_num];
      restr_hash_table0[hash_num] = tabp;
   }

   for (tabp = restr_init_table1 ; tabp->k != nothing ; tabp++) {
      uint32 hash_num = ((tabp->k + (5*tabp->restr)) * 25) & (NUM_RESTR_HASH_BUCKETS-1);
      tabp->next = restr_hash_table1[hash_num];
      restr_hash_table1[hash_num] = tabp;
   }

   for (tabp = restr_init_table4 ; tabp->k != nothing ; tabp++) {
      uint32 hash_num = ((tabp->k + (5*tabp->restr)) * 25) & (NUM_RESTR_HASH_BUCKETS-1);
      tabp->next = restr_hash_table4[hash_num];
      restr_hash_table4[hash_num] = tabp;
   }

   for (tabp = restr_init_table9 ; tabp->k != nothing ; tabp++) {
      uint32 hash_num = ((tabp->k + (5*tabp->restr)) * 25) & (NUM_RESTR_HASH_BUCKETS-1);
      tabp->next = restr_hash_table9[hash_num];
      restr_hash_table9[hash_num] = tabp;
   }
}


/* This fills in concept_sublist_sizes and concept_sublists. */

/*   A "1" means the concept is allowed in this setup

                     lout, lin, tby
                           |
                           |+- 8ch, Lcol, Rcol
          R2fl, Lwv, Rwv -+||
                          |||+- cdpt, dpt, L1x8
        qtag, gcol, L2fl-+||||
                         |||||+- R1x8, any, junk
                         ||||||       */
#define MASK_CTR_2      0600016
#define MASK_QUAD_D     0200016
#define MASK_CPLS       0547462
#define MASK_GOODCPLS   0147460
#define MASK_TAND       0770362
#define MASK_GOODTAND   0170360
#define MASK_GOODCONC   0177774
#define MASK_GOODRMVD   0177760
#define MASK_SIAM       0400002
#define MASK_2X4        0177762


static void initialize_concept_sublists(void)
{
   int concept_index;
   int all_legal_concepts;
   int test_call_list_kind;
   concept_kind end_marker = concept_diagnose;

   /* Decide whether we allow the "diagnose" concept, by deciding
      when we will stop the concept list scan. */
   if (diagnostic_mode) end_marker = marker_end_of_list;

   /* First, just count up all the available concepts. */

   for (concept_index=0,all_legal_concepts=0 ; ; concept_index++) {
      concept_descriptor *p = &concept_descriptor_table[concept_index];
      if (p->kind == end_marker) break;
      if ((p->concparseflags & CONCPARSE_MENU_DUP) || p->level > calling_level)
         continue;
      all_legal_concepts++;
   }

   /* Our friends in the UI will need this. */
   general_concept_size = concept_index - general_concept_offset;

   concept_sublists[call_list_any] =
      (short int *) (*the_callback_block.get_mem_fn)(all_legal_concepts*sizeof(short int));
   good_concept_sublists[call_list_any] =
      (short int *) (*the_callback_block.get_mem_fn)(all_legal_concepts*sizeof(short int));

   /* Make the concept sublists, one per setup.  We do them in downward order,
      with "any setup" last.  This is because we put our results into the
      "any setup" slot while we are working, and then copy them to the
      correct slot for each setup other than "any". */

   for (test_call_list_kind = (int) call_list_qtag;
        test_call_list_kind >= (int)call_list_any;
        test_call_list_kind--) {
      int concepts_in_this_setup, good_concepts_in_this_setup;

      for (concept_index=0,concepts_in_this_setup=0,good_concepts_in_this_setup=0;
           ;
           concept_index++) {
         uint32 setup_mask = ~0;      /* Default is to accept the concept. */
         uint32 good_setup_mask = 0;  /* Default for this is not to. */
         concept_descriptor *p = &concept_descriptor_table[concept_index];
         if (p->kind == end_marker) break;
         if ((p->concparseflags & CONCPARSE_MENU_DUP) || p->level > calling_level)
            continue;

         /* This concept is legal at this level.  See if it is appropriate for this setup.
            If we don't know, the default value of setup_mask will make it legal. */

         switch (p->kind) {
         case concept_centers_or_ends:
         case concept_centers_and_ends:
            switch (p->value.arg1) {
            case selector_center6:
            case selector_outer6:
            case selector_center2:
            case selector_verycenters:
            case selector_outer2:
            case selector_veryends:
               setup_mask = MASK_CTR_2;    /* This is a 6 and 2 type of concept. */
               break;
            default:
               break;
            }

            break;
         case concept_concentric:
            switch (p->value.arg1) {
            case schema_concentric:
            case schema_cross_concentric:
            case schema_single_concentric:
            case schema_single_cross_concentric:
               good_setup_mask = MASK_GOODCONC;
            }

            break;
         case concept_once_removed:
            if (p->value.arg1 == 0) 
               good_setup_mask = MASK_GOODRMVD;

            break;
         case concept_tandem:
         case concept_some_are_tandem:
         case concept_frac_tandem:
         case concept_some_are_frac_tandem:
            switch (p->value.arg4) {
            case tandem_key_tand:
               setup_mask = MASK_TAND;

               /* We allow <anyone> tandem, and we allow twosome.  That's all.
                  We specifically exclude all the "tandem in a 1/4 tag" stuff. */
               if (p->value.arg2 != CONCPROP__NEEDK_TWINQTAG &&
                   ((p->value.arg3 & 0xFF) == 0 || (p->value.arg3 & 0xFF) == 0x10))
                  good_setup_mask = MASK_GOODTAND;

               break;
            case tandem_key_cpls:
               setup_mask = MASK_CPLS;

               /* We allow <anyone> tandem, and we allow twosome.  That's all.
                  We specifically exclude all the "tandem in a 1/4 tag" stuff. */
               if (p->value.arg2 != CONCPROP__NEEDK_TWINQTAG &&
                   ((p->value.arg3 & 0xFF) == 0 || (p->value.arg3 & 0xFF) == 0x10))
                  good_setup_mask = MASK_GOODCPLS;

               break;
            case tandem_key_siam:
               setup_mask = MASK_SIAM; break;
            default:
               setup_mask = 0; break;    /* Don't waste time on the others. */
            }
            break;
         case concept_multiple_lines_tog_std:
         case concept_multiple_lines_tog:
            /* Test for quadruple C/L/W working. */
            if (p->value.arg4 == 4) setup_mask = MASK_2X4;
            break;
         case concept_quad_diamonds:
         case concept_quad_diamonds_together:
         case concept_do_phantom_diamonds:
            setup_mask = MASK_QUAD_D;
            break;
         case concept_do_phantom_2x4:
         case concept_divided_2x4:
         case concept_quad_lines:
            setup_mask = MASK_2X4;          /* Can actually improve on this. */
            break;
         case concept_assume_waves:
            /* We never allow any "assume_waves" concept.  In the early days,
               it was actually dangerous.  It isn't dangerous any more,
               but it's a fairly stupid thing to generate in a search. */
            setup_mask = 0;
            break;
         }

         /* Now we can determine whether this concept is appropriate for this setup. */

         if ((1 << (test_call_list_kind - ((int) call_list_empty))) & setup_mask)
            concept_sublists[call_list_any][concepts_in_this_setup++] = concept_index;

         /* And we can determine whether this concept is really nice for this setup. */

         if ((1 << (test_call_list_kind - ((int) call_list_empty))) & good_setup_mask)
            good_concept_sublists[call_list_any][good_concepts_in_this_setup++] = concept_index;
      }

      concept_sublist_sizes[test_call_list_kind] = concepts_in_this_setup;
      good_concept_sublist_sizes[test_call_list_kind] = good_concepts_in_this_setup;

      if (test_call_list_kind != (int) call_list_any) {
         if (concepts_in_this_setup != 0) {
            concept_sublists[test_call_list_kind] =
               (short int *) (*the_callback_block.get_mem_fn)(concepts_in_this_setup*sizeof(short int));
            (void) memcpy(concept_sublists[test_call_list_kind],
                          concept_sublists[call_list_any],
                          concepts_in_this_setup*sizeof(short int));
         }
         if (good_concepts_in_this_setup != 0) {
            good_concept_sublists[test_call_list_kind] =
               (short int *) (*the_callback_block.get_mem_fn)(good_concepts_in_this_setup*sizeof(short int));
            (void) memcpy(good_concept_sublists[test_call_list_kind],
                          good_concept_sublists[call_list_any],
                          good_concepts_in_this_setup*sizeof(short int));
         }
      }
   }

   /* "Any" is not considered a good setup. */
   good_concept_sublist_sizes[call_list_any] = 0;
}

extern void initialize_sdlib(void)
{
   initialize_tandem_tables();
   initialize_restr_tables();
   initialize_conc_tables();
   initialize_sel_tables();
   initialize_map_tables();
   initialize_touch_tables();
   initialize_concept_sublists();

   int i;

   for (i=0 ; i<NUM_WARNINGS ; i++) {
      char c = warning_strings[i][0];
      if (c == '*' || c == '#')
         no_search_warnings.bits[i>>5] |= 1 << (i & 0x1F);
      if (c == '+')
         conc_elong_warnings.bits[i>>5] |= 1 << (i & 0x1F);
      if (c == '=')
         dyp_each_warnings.bits[i>>5] |= 1 << (i & 0x1F);
      if (c == '#')
         useless_phan_clw_warnings.bits[i>>5] |= 1 << (i & 0x1F);
   }
}


static restr_initializer *get_restriction_thing(setup_kind k, assumption_thing t)
{
   restr_initializer *restr_hash_bucket;
   restr_initializer **hash_table;
   uint32 hash_num = ((k + (5*t.assumption)) * 25) & (NUM_RESTR_HASH_BUCKETS-1);
   uint32 col = t.assump_col;

   // Check for a few special cases that need to be translated.

   if (k == s2x4 && t.assumption == cr_magic_only && (col & 2)) {
      /* "assume inverted boxes", people are in general lines (col=2) or cols (col=3) */
      col = 0;   /* This is wrong, need something that allows
                    real mag col or unsymm congruent inv boxes.
                    This is a "check mixed groups" type of thing!!!! */
   }

   // First, use a specific hash table to search for easy cases.

   switch (col) {
   case 0:
      hash_table = restr_hash_table0;
      break;
   case 1:
      hash_table = restr_hash_table1;
      break;
   case 4:
      hash_table = restr_hash_table4;
      break;
   default:
      goto nothing_special;
   }

   restr_hash_bucket = hash_table[hash_num];

   for ( ; restr_hash_bucket ; restr_hash_bucket = restr_hash_bucket->next) {
      if (restr_hash_bucket->k == k && restr_hash_bucket->restr == t.assumption)
         return restr_hash_bucket;
   }

 nothing_special:

   // Now search the general hash table.

   hash_table = restr_hash_table9;
   restr_hash_bucket = hash_table[hash_num];

   for ( ; restr_hash_bucket ; restr_hash_bucket = restr_hash_bucket->next) {
      if (restr_hash_bucket->k == k && restr_hash_bucket->restr == t.assumption)
         return restr_hash_bucket;
   }

   return (restr_initializer *) 0;
}


static long_boolean check_for_supercall(const parse_block *parseptrcopy)
{
   concept_kind kk = parseptrcopy->concept->kind;

   if (kk <= marker_end_of_list) {
      if (kk == concept_another_call_next_mod &&
          parseptrcopy->next &&
          (parseptrcopy->next->call == base_calls[base_call_null] ||
           parseptrcopy->next->call == base_calls[base_call_null_second]) &&
          !parseptrcopy->next->next &&
          parseptrcopy->next->subsidiary_root) {
         return TRUE;
      }
      else
         fail("A concept is required.");
   }

   return FALSE;
}


extern long_boolean check_for_concept_group(
   Const parse_block *parseptrcopy,
   long_boolean want_all_that_other_stuff,
   concept_kind *k_p,
   uint32 *need_to_restrain_p,   /* 1=(if not doing echo), 2=(yes, always) */
   parse_block **parseptr_skip_p)
{
   concept_kind k;
   long_boolean retval = FALSE;
   parse_block *parseptr_skip;
   parse_block *next_parseptr;

   const parse_block *first_arg = parseptrcopy;

   *need_to_restrain_p = 0;

 try_again:

   parseptr_skip = parseptrcopy->next;

   if (parseptrcopy->concept) {
      k = parseptrcopy->concept->kind;

      if (check_for_supercall(parseptrcopy))
         k = concept_supercall;
   }
   else
      k = marker_end_of_list;

   if (!retval) {
      *k_p = k;

      if (k == concept_crazy ||
          k == concept_frac_crazy || 
          k == concept_twice ||
          k == concept_n_times)
         *need_to_restrain_p |= 2;
   }

   /* We do these even if we aren't the first concept. */

#ifdef NEW_ECHO
#endif

   if (allowing_all_concepts | 1) {
   if (k == concept_supercall ||
       k == concept_fractional ||
       (k == concept_meta && parseptrcopy->concept->value.arg1 == meta_key_initially) ||
       (k == concept_meta && parseptrcopy->concept->value.arg1 == meta_key_finally) ||

       (k == concept_meta && parseptrcopy->concept->value.arg1 == meta_key_echo) ||
       (k == concept_meta && parseptrcopy->concept->value.arg1 == meta_key_rev_echo) ||

       (k == concept_meta && parseptrcopy->concept->value.arg1 == meta_key_finish))
      *need_to_restrain_p |= 1;
   }
   else {
   if (k == concept_supercall ||
       k == concept_fractional ||
       (k == concept_meta && parseptrcopy->concept->value.arg1 == meta_key_initially) ||
       (k == concept_meta && parseptrcopy->concept->value.arg1 == meta_key_finally) ||
       (k == concept_meta && parseptrcopy->concept->value.arg1 == meta_key_finish))
      *need_to_restrain_p |= 1;
   }


   /* If skipping "phantom", maybe it's "phantom tandem", so we need to skip both. */

   if (k == concept_c1_phantom) {
      uint64 junk_concepts;

      junk_concepts.her8it = 0;
      junk_concepts.final = 0;

      next_parseptr =
         process_final_concepts(parseptr_skip, FALSE, &junk_concepts);

      if ((next_parseptr->concept->kind == concept_tandem ||
           next_parseptr->concept->kind == concept_frac_tandem) &&
          (junk_concepts.her8it | junk_concepts.final) == 0) {
         parseptrcopy = next_parseptr;
         retval = TRUE;
         goto try_again;
      }
   }
   else if (k == concept_meta) {
      meta_key_kind subkey = (meta_key_kind) parseptrcopy->concept->value.arg1;

      if (subkey == meta_key_random || subkey == meta_key_rev_random ||
          subkey == meta_key_piecewise || subkey == meta_key_nth_part_work ||
          subkey == meta_key_initially || subkey == meta_key_finally ||
          subkey == meta_key_echo || subkey == meta_key_rev_echo) {
         next_parseptr = parseptr_skip;
         parseptrcopy = next_parseptr;
         retval = TRUE;
         goto try_again;
      }
   }
   else if (k == concept_so_and_so_only &&
            ((selective_key) parseptrcopy->concept->value.arg1) == selective_key_work_concept) {
      next_parseptr = parseptr_skip;
      parseptrcopy = next_parseptr;
      retval = TRUE;
      goto try_again;
   }

   if (want_all_that_other_stuff) {
      if (k == concept_supercall)
         *parseptr_skip_p = parseptrcopy->next->subsidiary_root;
      else if (retval)
         *parseptr_skip_p = next_parseptr->next;
      else if (parseptrcopy->concept->kind == concept_special_sequential)
         *parseptr_skip_p = first_arg->subsidiary_root;
      else
         *parseptr_skip_p = first_arg->next;
   }

   return retval;
}


extern void fail(Const char s[])
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   (*the_callback_block.do_throw_fn)(error_flag_1_line);
}


extern void fail2(Const char s1[], Const char s2[])
{
   (void) strncpy(error_message1, s1, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   (void) strncpy(error_message2, s2, MAX_ERR_LENGTH);
   error_message2[MAX_ERR_LENGTH-1] = '\0';
   (*the_callback_block.do_throw_fn)(error_flag_2_line);
}


extern void failp(uint32 id1, Const char s[])
{
   collision_person1 = id1;
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   (*the_callback_block.do_throw_fn)(error_flag_cant_execute);
}


extern void specialfail(Const char s[])
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   (*the_callback_block.do_throw_fn)(error_flag_wrong_resolve_command);
}


extern void warn(warning_index w)
{
   if (w == warn__none) return;

   /* If this is an "each 1x4" type of warning, and we already have
      such a warning, don't enter the new one.  The first one takes precedence. */

   if (dyp_each_warnings.bits[w>>5] & (1 << (w & 0x1F))) {
      int i;

      for (i=0 ; i<WARNING_WORDS ; i++) {
         if (dyp_each_warnings.bits[i] & history[history_ptr+1].warnings.bits[i])
            return;
      }
   }

   if (w != warn__none) history[history_ptr+1].warnings.bits[w>>5] |= 1 << (w & 0x1F);
}

           

extern restriction_test_result verify_restriction(
   setup *ss,
   assumption_thing tt,
   long_boolean instantiate_phantoms,
   long_boolean *failed_to_instantiate)
{
   int idx, limit, i, j, k;
   uint32 t;
   uint32 qa0, qa1, qa2, qa3;
   uint32 qaa[4];
   uint32 pdir, qdir, pdirodd, qdirodd;
   uint32 dirtest[2];
   const veryshort *p, *q;
   int phantom_count = 0;

   switch (tt.assumption) {
   case cr_alwaysfail:
      return restriction_fails;
   case cr_nice_diamonds:
      // B=1 => right-handed; B=2 => left-handed.
      // The handedness stuff isn't documented!
      tt.assumption = cr_jright;
      tt.assump_col = 4;
      break;
   case cr_dmd_facing:
      // This isn't documented, but it should work.
      // If it does, can get rid of some special code in assoc.
      // B=1 => pts are right-handed; B=2 => pts are left-handed.
      tt.assumption = cr_jleft;
      tt.assump_col = 4;
      break;
   case cr_leads_only:
      tt.assump_both = 2;
      break;
   case cr_trailers_only:
      tt.assump_both = 1;
      break;
   }

   restr_initializer *rr = get_restriction_thing(ss->kind, tt);
   if (!rr) return restriction_no_item;

   dirtest[0] = 0;
   dirtest[1] = 0;

   *failed_to_instantiate = TRUE;

   switch (rr->check) {
   case chk_spec_directions:
      qa1 = 0;
      qa0 = 3 & (~tt.assump_both);

      p = rr->map1;
      qa2 = rr->map2[0];
      qa3 = rr->map2[1];
            
      while (*p>=0) {
         uint32 t1 = ss->people[*(p++)].id1;
         uint32 t2 = ss->people[*(p++)].id1;
         qa1 |= t1 | t2;
         if (t1 && (t1 & 3)!=qa2) qa0 &= ~2;
         if (t2 && (t2 & 3)!=qa3) qa0 &= ~2;
         if (t1 && (t1 & 3)!=qa3) qa0 &= ~1;
         if (t2 && (t2 & 3)!=qa2) qa0 &= ~1;
      }

      if (qa1) {
         if (rr->map2[2]) {
            if (!qa0) goto good;
         }
         else {
            if (qa0) goto good;
         }
      }

      goto bad;
   case chk_wave:
      qaa[0] = tt.assump_both;
      qaa[1] = tt.assump_both << 1;

      for (idx=0; idx<rr->size; idx++) {
         if ((t = ss->people[rr->map1[idx]].id1) != 0) {
            qaa[idx&1] |=  t;
            qaa[(idx&1)^1] |= t^2;
         }
         else if (tt.assump_negate || tt.assump_live) goto bad;    /* All live people
                                                                      were demanded. */
      }

      if ((qaa[0] & qaa[1] & 2) != 0)
         goto bad;

      if (rr->ok_for_assume) {
         uint32 qab[4];
         qab[0] = 0;
         qab[2] = 0;

         for (idx=0; idx<rr->size; idx++)
            qab[idx&2] |= ss->people[rr->map1[idx]].id1;

         if ((tt.assump_col | rr->map2[0]) & 4) {
            qab[2] >>= 3;
         }
         else if (tt.assump_col == 1 || (rr->map2[0] & 8)) {
            qab[0] >>= 3;
            qab[2] >>= 3;
         }

         if ((qab[0]|qab[2]) & 1) goto bad;

         if (instantiate_phantoms) {
            *failed_to_instantiate = FALSE;

            if (qaa[0] == 0) fail("Need live person to determine handedness.");

            if ((tt.assump_col | rr->map2[0]) & 4) {
               if ((qaa[0] & 2) == 0) { pdir = d_north; qdir = d_south; }
               else                   { pdir = d_south; qdir = d_north; }
               pdirodd = rotcw(pdir); qdirodd = rotcw(qdir);
            }
            else if (tt.assump_col == 1 || (rr->map2[0] & 8)) {
               if ((qaa[0] & 2) == 0) { pdir = d_east; qdir = d_west; }
               else                   { pdir = d_west; qdir = d_east; }
               pdirodd = pdir; qdirodd = qdir;
            }
            else {
               if ((qaa[0] & 2) == 0) { pdir = d_north; qdir = d_south; }
               else                   { pdir = d_south; qdir = d_north; }
               pdirodd = pdir; qdirodd = qdir;
            }

            for (i=0; i<rr->size; i++) {
               int p = rr->map1[i];
   
               if (!ss->people[p].id1) {
                  if (phantom_count >= 16) fail("Too many phantoms.");
   
                  ss->people[p].id1 =           (i&1) ?
                     ((i&2) ? qdirodd : qdir) :
                     ((i&2) ? pdirodd : pdir);
   
                  ss->people[p].id1 |= BIT_ACT_PHAN | ((phantom_count++) << 6);
                  ss->people[p].id2 = 0;
               }
               else if (ss->people[p].id1 & BIT_ACT_PHAN)
                  fail("Active phantoms may only be used once.");
            }
         }
      }

      goto good;
   case chk_box:
      p = rr->map1;
      q = rr->map2;

      for (i=0; i<rr->size; i++) {
         qa0 = (tt.assump_both << 1) & 2;
         qa1 = tt.assump_both & 2;
         qa2 = qa1;
         qa3 = qa0;

         for (idx=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++) {
            if ((t = ss->people[idx].id1) != 0) {
               qa0 |= t^(*p)^0;
               qa1 |= t^(*p)^2;
               qa2 |= t^(*q)^1;
               qa3 |= t^(*q)^3;
            }
            else if (tt.assump_live) goto bad;

            p++;
            q++;
         }

         if ((qa1&3) == 0 || (qa0&3) == 0 || (qa3&3) == 0 || (qa2&3) == 0)
            goto check_box_assume;
      }

      goto bad;

   check_box_assume:

      if (rr->ok_for_assume) {    /* Will not be true unless size = 1 */
         if (instantiate_phantoms) {
            if (!(qa0 & BIT_PERSON))
               fail("Need live person to determine handedness.");

            for (i=0 ; i<=setup_attrs[ss->kind].setup_limits ; i++) {
               if (!ss->people[i].id1) {
                  if (phantom_count >= 16)
                     fail("Too many phantoms.");
      
                  pdir = (qa0&1) ?
                     (d_east ^ ((rr->map2[i] ^ qa2) & 2)) :
                     (d_north ^ ((rr->map1[i] ^ qa0) & 2));
      
                  ss->people[i].id1 = pdir | BIT_ACT_PHAN | ((phantom_count++) << 6);
                  ss->people[i].id2 = 0;
               }
               else if (ss->people[i].id1 & BIT_ACT_PHAN)
                  fail("Active phantoms may only be used once.");
            }

            *failed_to_instantiate = FALSE;
         }
      }

      goto good;
   case chk_box_dbl:   /* Check everyone's lateral partner, independently of headlinerness. */
      for (idx=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++) {
         uint32 u;

         if ((t = ss->people[idx].id1) & 010) p = rr->map1;
         else if ((t = ss->people[idx].id1) & 1) p = rr->map2;
         else continue;

         if ((u = ss->people[p[idx]].id1) && ((t ^ u ^ rr->size) & 3)) goto bad;
      }

      goto good;
   case chk_indep_box:
      qa0 = (tt.assump_both << 1) & 2;
      qa1 = tt.assump_both & 2;
      qa2 = 0;
      qa3 = 0;

      for (idx=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++) {
         if ((t = ss->people[idx].id1) != 0) {
            qa2 |= t + rr->map1[idx];
            qa3 |= t + rr->map1[idx] + 2;
         }
      }

      if (((qa0 & qa2) | (qa1 & qa3)) != 0) goto bad;

      goto good;
   case chk_groups:
      limit = rr->map2[0];
   
      for (idx=0; idx<limit; idx++) {
         qa0 = 0; qa1 = 0;

         for (i=0,j=idx; i<rr->size; i++,j+=limit) {
            if ((t = ss->people[rr->map1[j]].id1) != 0) { qa0 |= t; qa1 |= t^2; }
            else if (tt.assump_negate || tt.assump_live) goto bad;    /* All live people were demanded. */
         }

         if ((qa0 & qa1 & 2) != 0) goto bad;

         if (rr->ok_for_assume) {
            if (tt.assump_col == 1) {
               if ((qa0 & 2) == 0) { pdir = d_east; }
               else                { pdir = d_west; }

               qa0 >>= 3;
               qa1 >>= 3;
            }
            else {
               if ((qa0 & 2) == 0) { pdir = d_north; }
               else                { pdir = d_south; }
            }

            if ((qa0|qa1)&1) goto bad;

            if (instantiate_phantoms) {
               if (qa0 == 0) fail("Need live person to determine handedness.");

               for (i=0,k=0 ; k<rr->size ; i+=limit,k++) {
                  int p = rr->map1[idx+i];
   
                  personrec *pq = &ss->people[p];
                  t = pq->id1;
      
                  if (!t) {
                     if (phantom_count >= 16) fail("Too many phantoms.");
                     pq->id1 = pdir | BIT_ACT_PHAN | ((phantom_count++) << 6);
                     pq->id2 = 0UL;
                  }
                  else if (t & BIT_ACT_PHAN)
                     fail("Active phantoms may only be used once.");
               }

               *failed_to_instantiate = FALSE;
            }
         }
      }
      goto good;
   case chk_anti_groups:
      limit = rr->map2[0];

      for (idx=0; idx<limit; idx++) {
         qa0 = 0; qa1 = 0;

         if ((t = ss->people[rr->map1[idx]].id1) != 0)       { qa0 |= t;   qa1 |= t^2; }
         else if (tt.assump_negate) goto bad;    /* All live people were demanded. */
         if ((t = ss->people[rr->map1[idx+limit]].id1) != 0) { qa0 |= t^2; qa1 |= t;   }
         else if (tt.assump_negate) goto bad;    /* All live people were demanded. */

         if ((qa0 & qa1 & 2) != 0) goto bad;

         if (rr->ok_for_assume) {
            if (tt.assump_col == 1) {
               if ((qa0 & 2) == 0) { pdir = d_east; qdir = d_west; }
               else                { pdir = d_west; qdir = d_east; }

               qa0 >>= 3;
               qa1 >>= 3;
            }
            else {
               if ((qa0 & 2) == 0) { pdir = d_north; qdir = d_south; }
               else                { pdir = d_south; qdir = d_north; }
            }

            if ((qa0|qa1)&1) goto bad;

            if (instantiate_phantoms) {
               if (qa0 == 0)
                  fail("Need live person to determine handedness.");

               for (i=0 ; i<=limit ; i += limit) {
                  int p = rr->map1[idx+i];
                  personrec *pq = &ss->people[p];
                  t = pq->id1;
      
                  if (!t) {
                     if (phantom_count >= 16) fail("Too many phantoms.");
                     pq->id1 = (i ? qdir : pdir) | BIT_ACT_PHAN | ((phantom_count++) << 6);
                     pq->id2 = 0UL;
                  }
                  else if (t & BIT_ACT_PHAN)
                     fail("Active phantoms may only be used once.");
               }

               *failed_to_instantiate = FALSE;
            }
         }
      }
      goto good;
   case chk_peelable:
      qa0 = 3; qa1 = 3;
      qa2 = 3; qa3 = 3;

      for (j=0; j<rr->size; j++) {
         if ((t = ss->people[rr->map1[j]].id1) != 0)  { qa0 &= t; qa1 &= t^2; }
         if ((t = ss->people[rr->map2[j]].id1) != 0)  { qa2 &= t; qa3 &= t^2; }
      }

      if ((((~qa0)&3) && ((~qa1)&3)) ||
          (((~qa2)&3) && ((~qa3)&3)))
         goto bad;

      goto good;
   case chk_qbox:

      qa0 = 0;
      qa1 = ~0;

      for (idx=0; idx<rr->map1[0]; idx++) {
         if ((t = ss->people[rr->map1[idx+1]].id1) & 1) qa0 |= t;
      }

      for (idx=0; idx<rr->map2[0]; idx++) {
         if ((t = ss->people[rr->map2[idx+1]].id1) & 1) qa0 |= ~t;
      }

      for (idx=0; idx<rr->map3[0]; idx++) {
         if ((t = ss->people[rr->map3[idx+1]].id1) & 1) qa1 &= t;
      }

      for (idx=0; idx<rr->map4[0]; idx++) {
         if ((t = ss->people[rr->map4[idx+1]].id1) & 1) qa1 &= ~t;
      }

      if (qa0 & ~qa1 & 2)
         goto bad;

      goto good;
   case chk_dmd_qtag:
      qa0 = 0;
      qa1 = 0;
      qa2 = 0;
      qa3 = 0;

      for (idx=0; idx<rr->map1[0]; idx++)
         qa1 |= ss->people[rr->map1[idx+1]].id1;

      for (idx=0; idx<rr->map2[0]; idx++)
         qa0 |= ss->people[rr->map2[idx+1]].id1;

      for (idx=0; idx<rr->map3[0]; idx++) {
         if ((t = ss->people[rr->map3[idx+1]].id1) != 0) {
            qa2 |= t;
            qa3 |= ~t;
         }
      }

      for (idx=0; idx<rr->map4[0]; idx++) {
         if ((t = ss->people[rr->map4[idx+1]].id1) != 0) {
            qa2 |= ~t;
            qa3 |= t;
         }
      }

      if ((qa1 & 001) != 0 || (qa0 & 010) != 0)
         goto bad;

      if (tt.assump_both) {
         /* The "live" modifier means that we need a definitive person
            to distinguish "in" or "out". */
         if (tt.assump_live && !(qa2 | qa3))
            goto bad;

         if ((qa2 & (tt.assump_both << 1) & 2) != 0 ||
             (qa3 & tt.assump_both & 2) != 0)
            goto bad;
      }

      goto good;
   case chk_qtag:
      qaa[0] = tt.assump_both;
      qaa[1] = tt.assump_both << 1;

      for (idx=0; idx<rr->size; idx++) {
         if ((t = ss->people[rr->map1[idx]].id1) != 0) { qaa[idx&1] |=  t; qaa[(idx&1)^1] |= t^2; }
      }

      if ((qaa[0] & qaa[1] & 2) != 0)
         goto bad;

      for (idx=0; idx<rr->map2[0]; idx++) {
         if ((t = ss->people[rr->map2[idx+1]].id1) != 0 && ((t ^ (idx << 1)) & 2) != 0)
            goto bad;
      }

      for (idx=0 ; idx<8 ; idx++) { if (ss->people[idx].id1 & 1) goto bad; }

      if (rr->ok_for_assume) {
         uint32 qab[4];
         qab[0] = 0;
         qab[2] = 0;

         for (idx=0; idx<rr->size; idx++)
            qab[idx&2] |= ss->people[rr->map1[idx]].id1;

         if ((qab[0]|qab[2]) & 1) goto bad;

         if (instantiate_phantoms) {
            *failed_to_instantiate = FALSE;

            if (qaa[0] == 0) fail("Need live person to determine handedness.");

            else {
               if ((qaa[0] & 2) == 0) { pdir = d_north; qdir = d_south; }
               else                   { pdir = d_south; qdir = d_north; }
               pdirodd = pdir; qdirodd = qdir;
            }

            for (i=0; i<rr->size; i++) {
               int p = rr->map1[i];
   
               if (!ss->people[p].id1) {
                  if (phantom_count >= 16) fail("Too many phantoms.");
   
                  ss->people[p].id1 =           (i&1) ?
                     ((i&2) ? qdirodd : qdir) :
                     ((i&2) ? pdirodd : pdir);
   
                  ss->people[p].id1 |= BIT_ACT_PHAN | ((phantom_count++) << 6);
                  ss->people[p].id2 = 0;
               }
               else if (ss->people[p].id1 & BIT_ACT_PHAN)
                  fail("Active phantoms may only be used once.");
            }

            for (i=0; i<rr->map2[0]; i++) {
               int p = rr->map2[i+1];
   
               if (!ss->people[p].id1) {
                  if (phantom_count >= 16) fail("Too many phantoms.");
                  ss->people[p].id1 = (i&1) ? d_south : d_north;
                  ss->people[p].id1 |= BIT_ACT_PHAN | ((phantom_count++) << 6);
                  ss->people[p].id2 = 0;
               }
               else if (ss->people[p].id1 & BIT_ACT_PHAN)
                  fail("Active phantoms may only be used once.");
            }
         }
      }

      goto good;
   default:
      goto bad;    /* Shouldn't happen. */
   }

   good: if (tt.assump_negate) return restriction_fails;
         else return restriction_passes;

   bad: if (tt.assump_negate) return restriction_passes;
        else return restriction_fails;
}


extern callarray *assoc(begin_kind key, setup *ss, callarray *spec)
{
   callarray *p;
   long_boolean booljunk;

   for (p = spec; p; p = p->next) {
      uint32 i, k, t, u, w, mask;
      assumption_thing tt;
      int idx, plaini;
      call_restriction this_qualifier;

      /* First, we demand that the starting setup be correct.  Also, if a qualifier
         number was specified, it must match. */

      if ((begin_kind) p->start_setup != key) continue;

      tt.assump_negate = 0;

      /* During initialization, we will be called with a null pointer for ss.
         We need to be careful, and err on the side of acceptance. */

      if (!ss) goto good;

      /* The bits of the "qualifierstuff" field have the following meaning
                  (see definitions in database.h):
         8000  left/out only (put 2 into assump_both)
         4000  right/in only (put 1 into assump_both)
         2000  must be live
         1000  must be T-boned
         0800  must not be T-boned
         0780  if these 4 bits are nonzero, they must match the number plus 1
         007F  the qualifier itself (we allow 127 qualifiers) */

      if ((p->qualifierstuff & QUALBIT__NUM_MASK) != 0) {
         number_used = TRUE;
         if (((p->qualifierstuff & QUALBIT__NUM_MASK) / QUALBIT__NUM_BIT) != (current_options.number_fields & 0xF)+1)
            continue;
      }

      if ((p->qualifierstuff & (QUALBIT__TBONE|QUALBIT__NTBONE)) != 0) {
         u = 0;

         for (plaini=0; plaini<=setup_attrs[ss->kind].setup_limits; plaini++)
            u |= ss->people[plaini].id1;

         if ((u & 011) == 011) {
            /* They are T-boned.  The "QUALBIT__NTBONE" bit says to reject. */
            if ((p->qualifierstuff & QUALBIT__NTBONE) != 0) continue;
         }
         else {
            /* They are not T-boned.  The "QUALBIT__TBONE" bit says to reject. */
            if ((p->qualifierstuff & QUALBIT__TBONE) != 0) continue;
         }
      }

      this_qualifier = (call_restriction) (p->qualifierstuff & QUALBIT__QUAL_CODE);

      if (this_qualifier == cr_none) {
         if ((p->qualifierstuff / QUALBIT__LIVE) & 1) {   /* All live people were demanded. */
            for (plaini=0; plaini<=setup_attrs[ss->kind].setup_limits; plaini++) {
               if ((ss->people[plaini].id1) == 0) goto bad;
            }
         }

         goto good;
      }

      /* Note that we have to examine setups larger than the setup the
         qualifier is officially defined for.  If a qualifier were defined
         as being legal only on 1x4's (so that, in the database, we only had
         specifications of the sort "setup 1x4 1x4 qualifier wave_only") we could
         still find ourselves here with ss->kind equal to s2x4.  Why?  Because
         the setup could be a 2x4 and the splitter could be trying to decide
         whether to split the setup into parallel 1x4's.  This happens when
         trying to figure out whether to split a 2x4 into 1x4's or 2x2's for
         the call "recycle". */

      k = 0;   /* Many tests will find these values useful. */
      mask = 0;
      i = 2;
      tt.assumption = this_qualifier;
      tt.assump_col = 0;
      tt.assump_cast = 0;
      tt.assump_live = (p->qualifierstuff / QUALBIT__LIVE) & 1;
      tt.assump_both = (p->qualifierstuff / QUALBIT__RIGHT) & 3;

      switch (this_qualifier) {
      case cr_wave_only:
         goto do_wave_stuff;
      case cr_magic_only:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only:
         case cr_2fl_only:
         case cr_wave_only:
            goto bad;
         }

         goto fix_col_line_stuff;
      case cr_li_lo: 
         switch (ss->kind) {
         case s1x2: case s1x4: case s2x2: case s2x4:
            goto fix_col_line_stuff;
         default:
            goto good;           /* We don't understand the setup -- we'd better accept it. */
         }
      case cr_indep_in_out:
         switch (ss->kind) {
         case s2x2: case s2x4:
            goto fix_col_line_stuff;
         default:
            goto good;           /* We don't understand the setup -- we'd better accept it. */
         }
      case cr_ctrs_in_out:
         switch (ss->kind) {
         case s2x4:
            if (!((ss->people[1].id1 | ss->people[2].id1 |
                   ss->people[5].id1 | ss->people[6].id1)&010))
               tt.assump_col = 1;
            goto check_tt;
         default:
            goto good;           /* We don't understand the setup -- we'd better accept it. */
         }
      case cr_all_facing_same:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_wave_only:
         case cr_magic_only:
            goto bad;
         }

         /* If we are not looking at the whole setup (that is, we are deciding
            whether to split the setup into smaller ones), let it pass. */

         if (setup_attrs[ss->kind].keytab[0] != key &&
             setup_attrs[ss->kind].keytab[1] != key)
            goto good;

         goto fix_col_line_stuff;
      case cr_1fl_only:        /* 1x3/1x4/1x6/1x8 - a 1FL, that is, all 3/4/6/8 facing same;
                                  2x3/2x4 - individual 1FL's */
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only:
            goto good;
         case cr_wave_only:
         case cr_magic_only:
            goto bad;
         }

         goto check_tt;
      case cr_2fl_only:        /* 1x4 or 2x4 - 2FL; 4x1 - single DPT or single CDPT */
         goto do_2fl_stuff;
      case cr_3x3_2fl_only:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only: case cr_wave_only: case cr_miniwaves: case cr_magic_only: goto bad;
         }

         tt.assumption = cr_3x3_2fl_only;
         goto fix_col_line_stuff;
      case cr_4x4_2fl_only:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only: case cr_wave_only: case cr_miniwaves: case cr_magic_only: goto bad;
         }

         tt.assumption = cr_4x4_2fl_only;
         goto fix_col_line_stuff;
      case cr_couples_only:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only:
         case cr_2fl_only:
            goto good;
         case cr_wave_only:
         case cr_miniwaves:
         case cr_magic_only:
            goto bad;
         }

         tt.assumption = cr_couples_only;  /* Don't really need this, unless implement
                                              "not_couples" (like "cr_not_miniwaves")
                                              in the future. */

         switch (ss->kind) {
         case s1x2: case s1x4: case s1x6: case s1x8: case s1x16: case s2x4:
         case sdmd: case s_trngl: case s_qtag: case s_ptpd: case s_bone:
         case s2x2:
            goto fix_col_line_stuff;
         default:
            goto good;                 /* We don't understand the setup --
                                          we'd better accept it. */
         }
      case cr_3x3couples_only:      /* 1x3/1x6/2x3/2x6/1x12 - each group of 3 people
                                       are facing the same way */
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only:
            goto good;
         case cr_wave_only:
         case cr_magic_only:
            goto bad;
         }

         goto check_tt;
      case cr_4x4couples_only:      /* 1x4/1x8/2x4/2x8/1x16 - each group of 4 people
                                       are facing the same way */
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only:
            goto good;
         case cr_wave_only:
         case cr_magic_only:
            goto bad;
         }

         goto check_tt;
      case cr_not_miniwaves:
         tt.assump_negate = 1;
         /* **** FALL THROUGH!!!! */
      case cr_miniwaves:                    /* miniwaves everywhere */
         /* **** FELL THROUGH!!!!!! */
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only:
         case cr_2fl_only:
            goto bad;
         case cr_wave_only:
         case cr_miniwaves:
         case cr_magic_only:
            goto good;
         }

         tt.assumption = cr_miniwaves;

         switch (ss->kind) {
         case s1x2: case s1x4: case s1x6: case s1x8: case s1x16: case s2x4:
         case sdmd: case s_trngl: case s_qtag: case s_ptpd: case s_bone:
         case s2x2:
            goto fix_col_line_stuff;
         default:
            goto good;                 /* We don't understand the setup --
                                          we'd better accept it. */
         }
      case cr_ctrwv_end2fl:
         /* Note that this qualifier is kind of strict.  We won't permit the call "with
            confidence" do be done unless everyone can trivially determine which
            part to do. */
         if (ss->kind == s_crosswave) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 0 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 0 &&
                ((ss->people[2].id1 | ss->people[3].id1) == 0 || ((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 2) &&
                ((ss->people[6].id1 | ss->people[7].id1) == 0 || ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 2))
               goto good;
            goto bad;
         }
         else if (ss->kind == s1x6) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 0 &&
                ((ss->people[3].id1 ^ ss->people[4].id1) & d_mask) == 0 &&
                ((ss->people[1].id1 ^ ss->people[2].id1) & d_mask) == 2 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 2)
               goto good;
            goto bad;
         }
         else if (ss->kind == s1x8) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 0 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 0 &&
                ((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 2 &&
                ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 2)
               goto good;
            goto bad;
         }
         else
            goto good;        /* We don't understand the setup -- we'd better accept it. */
      case cr_ctr2fl_endwv:
         /* Note that this qualifier is kind of strict.  We won't permit the call "with
               confidence" do be done unless everyone can trivially determine which
               part to do. */
         if (ss->kind == s_crosswave) {
            if (((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 0 &&
                ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 0 &&
                ((ss->people[0].id1 | ss->people[1].id1) == 0 || ((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 2) &&
                ((ss->people[4].id1 | ss->people[5].id1) == 0 || ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 2))
               goto good;
            goto bad;
         }
         else if (ss->kind == s1x6) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 2 &&
                ((ss->people[3].id1 ^ ss->people[4].id1) & d_mask) == 2 &&
                ((ss->people[1].id1 ^ ss->people[2].id1) & d_mask) == 0 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 0)
               goto good;
            goto bad;
         }
         else if (ss->kind == s1x8) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 2 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 2 &&
                ((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 0 &&
                ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 0)
               goto good;
            goto bad;
         }
         else
            goto good;         /* We don't understand the setup -- we'd better accept it. */
      case cr_true_Z_cw:
         k ^= 033U ^ 066U;
         mask ^= CMD_MISC2__IN_Z_CCW ^ CMD_MISC2__IN_Z_CW;
         /* **** FALL THROUGH!!!! */
      case cr_true_Z_ccw:
         /* **** FELL THROUGH!!!!!! */
         k ^= ~033U;
         mask ^= CMD_MISC2__IN_Z_CCW;

         if (ss->cmd.cmd_misc2_flags & mask)
             goto good;

         if (ss->kind == s2x3) {
            /* In this case, we actually check the shear direction of the Z. */

            mask = 0;

            for (plaini=0, w=1; plaini<=setup_attrs[ss->kind].setup_limits; plaini++, w<<=1) {
               if (ss->people[plaini].id1) mask |= w;
            }

            if ((mask & k) == 0 && (mask & (k^033U^066U)) != 0) goto good;
            goto bad;
         }
         else
            goto good;         /* We don't understand the setup -- we'd better accept it. */
      case cr_lateral_cols_empty:
         mask = 0;
         t = 0;

         for (plaini=0, k=1; plaini<=setup_attrs[ss->kind].setup_limits; plaini++, k<<=1) {
            if (ss->people[plaini].id1) { mask |= k; t |= ss->people[plaini].id1; }
         }

         if (ss->kind == s3x4 && (t & 1) == 0 &&
             ((mask & 04646) == 0 ||
              (mask & 04532) == 0 || (mask & 03245) == 0 ||
              (mask & 02525) == 0 || (mask & 03232) == 0 ||
              (mask & 04651) == 0 || (mask & 05146) == 0))
            goto good;
         else if (ss->kind == s3x6 && (t & 1) == 0 &&
                  ((mask & 0222222) == 0))
            goto good;
         else if (ss->kind == s4x4 && (t & 1) == 0 &&
                  ((mask & 0xE8E8) == 0 ||
                   (mask & 0xA3A3) == 0 || (mask & 0x5C5C) == 0 ||
                   (mask & 0xA857) == 0 || (mask & 0x57A8) == 0))
            goto good;
         else if (ss->kind == s4x4 && (t & 010) == 0 &&
                  ((mask & 0x8E8E) == 0 ||
                   (mask & 0x3A3A) == 0 || (mask & 0xC5C5) == 0 ||
                   (mask & 0x857A) == 0 || (mask & 0x7A85) == 0))
            goto good;
         else if (ss->kind == s4x6 && (t & 010) == 0) goto good;
         else if (ss->kind == s3x8 && (t & 001) == 0) goto good;
         goto bad;
      case cr_dmd_same_pt:                   /* dmd or pdmd - centers would circulate
                                                to same point */
         if (((ss->people[1].id1 & 01011) == d_east) &&         /* faces either east or west */
             (!((ss->people[3].id1 ^ ss->people[1].id1) & d_mask)))  /* and both face same way */
            goto good;
         goto bad;
      case cr_nice_diamonds:
      case cr_dmd_facing:
         goto check_tt;
      case cr_dmd_ctrs_mwv:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_jleft: case cr_jright: case cr_ijleft: case cr_ijright:
            if ((tt.assump_both+1) & 2) {
               if ((ss->cmd.cmd_assume.assump_both ^ tt.assump_both) == 3)
                  goto good;
               else if ((ss->cmd.cmd_assume.assump_both ^ tt.assump_both) == 0)
                  goto bad;
            }
         }
         goto check_tt;
      case cr_qtag_like:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_diamond_like: case cr_pu_qtag_like:
            goto bad;
         case cr_jleft: case cr_jright: case cr_ijleft: case cr_ijright:
            if (ss->cmd.cmd_assume.assump_both == 2 && tt.assump_both == 1)
               goto good;
            if (ss->cmd.cmd_assume.assump_both == 1 && tt.assump_both == 2)
               goto good;
         }
         goto check_tt;
      case cr_diamond_like:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_qtag_like: case cr_pu_qtag_like:
            goto bad;
         }
         goto check_tt;
      case cr_pu_qtag_like:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_qtag_like: case cr_diamond_like:
            goto bad;
         }
         goto check_tt;
      case cr_dmd_intlk:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_ijright: case cr_ijleft: case cr_real_1_4_line: case cr_real_3_4_line:
            goto good;
         case cr_jright: case cr_jleft: case cr_real_1_4_tag: case cr_real_3_4_tag:
            goto bad;
         }

         goto check_tt;
      case cr_dmd_not_intlk:
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_ijright: case cr_ijleft: case cr_real_1_4_line: case cr_real_3_4_line:
            goto bad;
         case cr_jright: case cr_jleft: case cr_real_1_4_tag: case cr_real_3_4_tag:
            goto good;
         }
         goto check_tt;
      case cr_not_split_dixie:
         tt.assump_negate = 1;
         /* **** FALL THROUGH!!!! */
      case cr_split_dixie:
         /* **** FELL THROUGH!!!!!! */
         if (ss->cmd.cmd_final_flags.final & FINAL__SPLIT_DIXIE_APPROVED) goto good;
         goto bad;
      case cr_didnt_say_tgl:
         tt.assump_negate = 1;
         /* **** FALL THROUGH!!!! */
      case cr_said_tgl:
         /* **** FELL THROUGH!!!!!! */
         if (ss->cmd.cmd_misc_flags & CMD_MISC__SAID_TRIANGLE) goto good;
         goto bad;
      case cr_occupied_as_h:
         if (ss->kind != s3x4 ||
             (ss->people[1].id1 | ss->people[2].id1 |
              ss->people[7].id1 | ss->people[8].id1)) goto bad;
         goto good;
      case cr_occupied_as_stars:
         if (ss->kind != s_c1phan ||
             ((ss->people[0].id1 | ss->people[1].id1 |
               ss->people[2].id1 | ss->people[3].id1 |
               ss->people[8].id1 | ss->people[9].id1 |
               ss->people[10].id1 | ss->people[11].id1) &&
              (ss->people[4].id1 | ss->people[5].id1 |
               ss->people[6].id1 | ss->people[7].id1 |
               ss->people[12].id1 | ss->people[13].id1 |
               ss->people[14].id1 | ss->people[15].id1)))
            goto bad;
         goto good;
      case cr_occupied_as_qtag:
         if (ss->kind != s3x4 ||
             (ss->people[0].id1 | ss->people[3].id1 |
              ss->people[6].id1 | ss->people[9].id1)) goto bad;
         goto good;
      case cr_occupied_as_3x1tgl:
         if (ss->kind == s_qtag) goto good;
         if (ss->kind == s3x4 && !(ss->people[1].id1 | ss->people[2].id1 |
                                   ss->people[7].id1 | ss->people[8].id1)) goto good;
         if (ss->kind == s3x4 && !(ss->people[0].id1 | ss->people[3].id1 |
                                   ss->people[6].id1 | ss->people[9].id1)) goto good;
         if (ss->kind == s3x4 && !(ss->people[1].id1 | ss->people[3].id1 |
                                   ss->people[6].id1 | ss->people[8].id1)) goto good;
         if (ss->kind == s3x4 && !(ss->people[0].id1 | ss->people[2].id1 |
                                   ss->people[7].id1 | ss->people[9].id1)) goto good;
         if (ss->kind == s2x6 && !(ss->people[0].id1 | ss->people[2].id1 |
                                   ss->people[6].id1 | ss->people[8].id1)) goto good;
         if (ss->kind == s2x6 && !(ss->people[3].id1 | ss->people[5].id1 |
                                   ss->people[9].id1 | ss->people[11].id1)) goto good;
         if (ss->kind == s2x3 && !(ss->people[0].id1 | ss->people[2].id1)) goto good;
         if (ss->kind == s2x3 && !(ss->people[3].id1 | ss->people[5].id1)) goto good;
         goto bad;
      case cr_reg_tbone:
         if (ss->kind == s2x4 && key == b_2x2) tt.assump_col = 1;
         /* **** FALL THROUGH!!!! */
      case cr_qtag_mwv:
      case cr_qtag_mag_mwv:
      case cr_dmd_ctrs_1f:
      case cr_gen_qbox:
         /* **** FELL THROUGH!!!!!! */
         goto check_tt;
      case cr_ctr_pts_rh:
      case cr_ctr_pts_lh:
         {
            call_restriction kkk;      /* gets set to the qualifier corresponding to
                                          what we have. */
            uint32 t1;
            uint32 t2;
            long_boolean b1 = TRUE;
            long_boolean b2 = TRUE;
            
            switch (ss->kind) {
            case s_qtag:
            case s_hrglass:
               t1 = ss->people[6].id1;
               t2 = ss->people[2].id1;
               break;
            case s_2x1dmd:
               t1 = ss->people[0].id1;
               t2 = ss->people[3].id1;
               break;
            case s_bone6:
               t1 = ss->people[5].id1;
               t2 = ss->people[2].id1;
               break;
            default:
               goto bad;
            }

            if (t1 && (t1 & d_mask)!=d_north) b1 = FALSE;
            if (t2 && (t2 & d_mask)!=d_south) b1 = FALSE;
            if (t1 && (t1 & d_mask)!=d_south) b2 = FALSE;
            if (t2 && (t2 & d_mask)!=d_north) b2 = FALSE;

            if (b1 == b2) goto bad;
            kkk = b1 ? cr_ctr_pts_rh : cr_ctr_pts_lh;
            if (this_qualifier == kkk) goto good;
         }
         goto bad;
      case cr_line_ends_looking_out:
         if (ss->kind != s2x4) goto bad;

            /* We demand at least one center person T-boned, so that we
               know it has to be an 8-person call. */

         if (!((  ss->people[1].id1 |
                  ss->people[2].id1 |
                  ss->people[5].id1 |
                  ss->people[6].id1) & 1)) goto bad;

         if ((t = ss->people[0].id1) && (t & d_mask) != d_north) goto bad;
         if ((t = ss->people[3].id1) && (t & d_mask) != d_north) goto bad;
         if ((t = ss->people[4].id1) && (t & d_mask) != d_south) goto bad;
         if ((t = ss->people[7].id1) && (t & d_mask) != d_south) goto bad;
         goto good;
      case cr_col_ends_lookin_in:
         if (ss->kind != s2x4) goto bad;
         if ((t = ss->people[0].id1) && (t & d_mask) != d_east) goto bad;
         if ((t = ss->people[3].id1) && (t & d_mask) != d_west) goto bad;
         if ((t = ss->people[4].id1) && (t & d_mask) != d_west) goto bad;
         if ((t = ss->people[7].id1) && (t & d_mask) != d_east) goto bad;
         goto good;
      case cr_ripple_both_centers:
         k ^= (0xAAA ^ 0xA82);
         /* **** FALL THROUGH!!!! */
      case cr_ripple_any_centers:
         /* **** FELL THROUGH!!!!!! */
         k ^= (0xA82 ^ 0x144);
         /* **** FALL THROUGH!!!! */
      case cr_ripple_one_end:
         /* **** FELL THROUGH!!!!!! */
         k ^= (0x144 ^ 0x555);
         /* **** FALL THROUGH!!!! */
      case cr_ripple_both_ends:
         /* **** FELL THROUGH!!!!!! */
         if (ss->kind != s1x4) goto good;
         k ^= 0x555;
         mask = 0;

         for (plaini=0, w=1; plaini<=setup_attrs[ss->kind].setup_limits; plaini++, w<<=1) {
            if (selectp(ss, plaini)) mask |= w;
         }

         if (mask == (k & 0xF) || mask == ((k>>4) & 0xF) || mask == ((k>>8) & 0xF)) goto good;
         goto bad;
      case cr_people_1_and_5_real:
         if (ss->people[1].id1 & ss->people[5].id1) goto good;
         goto bad;

         /* Beware!  These next four use cumulative xoring of the variable k, which
            is in all cases initialized to zero. */

      case cr_ends_sel:
         k = ~k;
         /* FALL THROUGH!!!!!! */
      case cr_ctrs_sel:
         /* FELL THROUGH!!!!!! */

         switch (ss->kind) {
         case s1x4: k ^= 0x5; break;
         case s2x4: k ^= 0x99; break;
         case s_qtag: case s_bone: k ^= 0x33; break;
         case s_rigger: k ^= 0xCC; break;
         default: k = ~1;  /* Will force an error later, unless splitting. */
         }
         /* FALL THROUGH!!!!!! */
      case cr_all_sel:
         /* FELL THROUGH!!!!!! */
         k = ~k;
         /* FALL THROUGH!!!!!! */
      case cr_none_sel:
         /* FELL THROUGH!!!!!! */

         /* If we are not looking at the whole setup (that is, we are deciding
            whether to split the setup into smaller ones), let it pass. */

         if (setup_attrs[ss->kind].keytab[0] != key &&
             setup_attrs[ss->kind].keytab[1] != key)
            goto good;

         if (k == 1) goto bad;

         for (idx=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++,k>>=1) {
            if (!ss->people[idx].id1) {
               if (tt.assump_live) goto bad;
            }
            else if (selectp(ss, idx)) {
               if (!(k&1)) goto bad;
            }
            else {
               if (k&1) goto bad;
            }
         }
         goto good;
      default:
         break;
      }

      goto bad;

   do_2fl_stuff:

      switch (ss->cmd.cmd_assume.assumption) {
      case cr_1fl_only: case cr_wave_only: case cr_miniwaves: case cr_magic_only: goto bad;
      }

      tt.assumption = cr_2fl_only;
      goto fix_col_line_stuff;

   do_wave_stuff:
      switch (ss->cmd.cmd_assume.assumption) {
      case cr_1fl_only: case cr_2fl_only: case cr_couples_only: case cr_magic_only: goto bad;
      case cr_ijright: case cr_ijleft: goto bad;
      case cr_wave_only: case cr_jright:
         if (ss->cmd.cmd_assume.assump_both == 2 && tt.assump_both == 1) goto bad;
         if (ss->cmd.cmd_assume.assump_both == 1 && tt.assump_both == 2) goto bad;
         break;
      case cr_jleft:
         if (ss->cmd.cmd_assume.assump_both == 2 && tt.assump_both == 2) goto bad;
         if (ss->cmd.cmd_assume.assump_both == 1 && tt.assump_both == 1) goto bad;
         break;
      }

      tt.assumption = cr_wave_only;

      switch (ss->kind) {
      case s3x4:         /* This only handles lines; not columns --
                            we couldn't have "wavy" columns that were symmetric. */
      case s_bone6:
      case s_trngl:
      case s_qtag:
      case s3x1dmd:
         goto check_tt;
      case sdmd:
      case s_ptpd:
      case s_short6:
         tt.assump_col = 1;
         goto check_tt;
      case s2x4:
         /* If the setup is a 2x4, but we are deciding whether
            to split into 2x2's, let it pass. */

         if (key == b_2x2) goto good;
         else if (key == b_1x4) tt.assumption = cr_miniwaves;
         break;
      }

   fix_col_line_stuff:

      switch (ss->kind) {
      case s1x3:
         if (tt.assump_both) goto bad;   /* We can't check a 1x3 for right-or-left-handedness. */
         /* FALL THROUGH!!! */
      case s1x6: case s1x8: case s1x10:
      case s1x12: case s1x14: case s1x16:
      case s2x2: case s4x4: case s_thar: case s_crosswave: case s_qtag:
      case s3x1dmd: case s_2x1dmd:
      case s_trngl: case s_bone:
         /* FELL THROUGH!!! */
         goto check_tt;
      case sdmd: case s_ptpd:
         tt.assump_col = 1;
         goto check_tt;
      case s1x4:  /* Note that 1x6, 1x8, etc should be here also.  This will
                     make "cr_2fl_only" and such things work in 4x1. */
      case s1x2:
      case s2x4:
      case s2x6:
      case s2x8:
      case s2x3:
         for (idx=0,u=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++) u |= ss->people[idx].id1;
         if (!(u&010)) tt.assump_col = 1;
         else if (tt.assump_both && ss->kind == s2x3) goto bad;   /* We can't check 2x3 lines for right-or-left-handedness. */
         goto check_tt;
      default:
         /* ****** Try to change this (and other things) to "goto bad". */
         goto good;                 /* We don't understand the setup -- we'd better accept it. */
      }

   check_tt:

      if (verify_restriction(ss, tt, FALSE, &booljunk) == restriction_passes) return p;
      continue;

   bad:

      if (tt.assump_negate) return p;
      else continue;

   good:

      if (!tt.assump_negate) return p;
      else continue;
   }

   return (callarray *) 0;
}


extern uint32 find_calldef(
   callarray *tdef,
   setup *scopy,
   int real_index,
   int real_direction,
   int northified_index)
{
   unsigned short *calldef_array;
   predptr_pair *predlistptr;
   uint32 z;

   if (tdef->callarray_flags & CAF__PREDS) {
      predlistptr = tdef->stuff.prd.predlist;
      while (predlistptr != (predptr_pair *) 0) {
         if ((*(predlistptr->pred->predfunc))(scopy, real_index, real_direction, northified_index, predlistptr->pred->extra_stuff)) {
            calldef_array = predlistptr->arr;
            goto got_it;
         }
         predlistptr = predlistptr->next;
      }
      fail(tdef->stuff.prd.errmsg);
   }
   else
      calldef_array = tdef->stuff.def;

got_it:

   z = calldef_array[northified_index];
   if (!z) failp(scopy->people[real_index].id1, "can't execute their part of this call.");

   return z;
}


extern void clear_people(setup *z)
{
   (void) memset(z->people, 0, sizeof(personrec)*MAX_PEOPLE);
}


extern uint32 rotperson(uint32 n, int amount)
{
   if (n == 0) return 0; else return (n + amount) & ~064;
}


extern uint32 rotcw(uint32 n)
{
   if (n == 0) return 0; else return (n + 011) & ~064;
}


extern uint32 rotccw(uint32 n)
{
   if (n == 0) return 0; else return (n + 033) & ~064;
}


extern void clear_person(setup *resultpeople, int resultplace)
{
   resultpeople->people[resultplace].id1 = 0;
   resultpeople->people[resultplace].id2 = 0;
}


extern uint32 copy_person(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace)
{
   resultpeople->people[resultplace] = sourcepeople->people[sourceplace];
   return resultpeople->people[resultplace].id1;
}


extern uint32 copy_rot(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace, int rotamount)
{
   uint32 newperson = sourcepeople->people[sourceplace].id1;

   if (newperson) newperson = (newperson + rotamount) & ~064;
   resultpeople->people[resultplace].id2 = sourcepeople->people[sourceplace].id2;
   return resultpeople->people[resultplace].id1 = newperson;
}


extern void swap_people(setup *ss, int oneplace, int otherplace)
{
   personrec temp = ss->people[otherplace];
   ss->people[otherplace] = ss->people[oneplace];
   ss->people[oneplace] = temp;
}


extern void install_person(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace)
{
   uint32 newperson = sourcepeople->people[sourceplace].id1;

   if (resultpeople->people[resultplace].id1 == 0)
      resultpeople->people[resultplace] = sourcepeople->people[sourceplace];
   else if (newperson) {
      collision_person1 = resultpeople->people[resultplace].id1;
      collision_person2 = newperson;
      error_message1[0] = '\0';
      error_message2[0] = '\0';
      (*the_callback_block.do_throw_fn)(error_flag_collision);
   }
}


extern void install_rot(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace, int rotamount)
{
   uint32 newperson = sourcepeople->people[sourceplace].id1;

   if (newperson) {
      if (resultplace < 0) fail("This would go into an excessively large matrix.");

      if (resultpeople->people[resultplace].id1 == 0) {
         resultpeople->people[resultplace].id1 = (newperson + rotamount) & ~064;
         resultpeople->people[resultplace].id2 = sourcepeople->people[sourceplace].id2;
      }
      else {
         collision_person1 = resultpeople->people[resultplace].id1;
         collision_person2 = newperson;
         error_message1[0] = '\0';
         error_message2[0] = '\0';
         (*the_callback_block.do_throw_fn)(error_flag_collision);
      }
   }
}


extern void scatter(setup *resultpeople, const setup *sourcepeople,
                    const veryshort *resultplace, int countminus1, int rotamount)
{
   int k, idx;
   for (k=0; k<=countminus1; k++) {
      idx = resultplace[k];

      if (idx < 0) {
         /* This could happen in "touch_or_rear_back". */
         if (sourcepeople->people[k].id1) fail("Don't understand this setup at all.");
      }
      else
         (void) copy_rot(resultpeople, idx, sourcepeople, k, rotamount);
   }
}


extern void gather(setup *resultpeople, const setup *sourcepeople,
                   const veryshort *resultplace, int countminus1, int rotamount)
{
   int k, idx;
   for (k=0; k<=countminus1; k++) {
      idx = resultplace[k];

      if (idx >= 0)
         (void) copy_rot(resultpeople, k, sourcepeople, idx, rotamount);
   }
}



/* WARNING!!!!  This procedure appears verbatim in sdutil.c and dbcomp.c . */

/* These combinations are not allowed. */

#define FORBID1 (INHERITFLAG_FRACTAL|INHERITFLAG_YOYO)
#define FORBID2 (INHERITFLAG_SINGLEFILE|INHERITFLAG_SINGLE)
#define FORBID3 (INHERITFLAG_MXNMASK|INHERITFLAG_NXNMASK)
#define FORBID4 (INHERITFLAG_12_MATRIX|INHERITFLAG_16_MATRIX)


static long_boolean do_heritflag_merge(uint32 *dest, uint32 source)
{
   uint32 revertsource = source & INHERITFLAG_REVERTMASK;

   if (revertsource) {
      /* If the source is a revert/reflect bit, things are complicated. */

      uint32 revertdest = (*dest) & INHERITFLAG_REVERTMASK;

      if (!revertdest) {
         goto good;
      }
      else if (revertsource == INHERITFLAGRVRTK_REVERT &&
               revertdest == INHERITFLAGRVRTK_REFLECT) {
         *dest &= ~INHERITFLAG_REVERTMASK;
         *dest |= INHERITFLAGRVRTK_RFV;
         return FALSE;
      }
      else if (revertsource == INHERITFLAGRVRTK_REFLECT &&
               revertdest == INHERITFLAGRVRTK_REVERT) {
         *dest &= ~INHERITFLAG_REVERTMASK;
         *dest |= INHERITFLAGRVRTK_RVF;
         return FALSE;
      }
      else if (revertsource == INHERITFLAGRVRTK_REFLECT &&
               revertdest == INHERITFLAGRVRTK_REFLECT) {
         *dest &= ~INHERITFLAG_REVERTMASK;
         *dest |= INHERITFLAGRVRTK_RFF;
         return FALSE;
      }
      else if (revertsource == INHERITFLAGRVRTK_REVERT &&
               revertdest == INHERITFLAGRVRTK_RVF) {
         *dest &= ~INHERITFLAG_REVERTMASK;
         *dest |= INHERITFLAGRVRTK_RVFV;
         return FALSE;
      }
      else if (revertsource == INHERITFLAGRVRTK_REFLECT &&
               revertdest == INHERITFLAGRVRTK_RFV) {
         *dest &= ~INHERITFLAG_REVERTMASK;
         *dest |= INHERITFLAGRVRTK_RFVF;
         return FALSE;
      }
      else
         return TRUE;
   }

   /* Check for plain redundancy.  If this is a bit in one of the complex
      fields, this simple test may not catch the error, but the next one will. */

   if ((*dest & source))
      return TRUE;

   if (((*dest & FORBID1) && (source & FORBID1)) ||
       ((*dest & FORBID2) && (source & FORBID2)) ||
       ((*dest & FORBID3) && (source & FORBID3)) ||
       ((*dest & FORBID4) && (source & FORBID4)))
      return TRUE;

   good:

   *dest |= source;

   return FALSE;
}



/* Take a concept pointer and scan for all "final" concepts,
   returning an updated concept pointer and a mask of all such concepts found.
   "Final" concepts are those that modify the execution of a call but
   do not cause it to be executed in a virtual or distorted setup.
   This has a side-effect that is occasionally used:  When it passes over
   any "magic" or "interlocked" concept, it drops a pointer to where the
   last such occurred into the external variable "last_magic_diamond". */

/* If not checking for errors, we set the "final" bits correctly
   (we can always do that, since the complex bit field stuff isn't
   used in this), and we just set anything in the "herit" field.
   In fact, we set something in the "herit" field even if this item
   was "final".  Doing all this gives the required behavior, which is:

   (1) If any concept at all, final or herit, the herit field
      will be nonzero.
   (2) The herit field is otherwise messed up.  Only zero/nonzero matters.
   (3) In any case the "final" field will be correct. */


extern parse_block *process_final_concepts(
   parse_block *cptr,
   long_boolean check_errors,
   uint64 *final_concepts)
{
   while (cptr) {
      uint32 the_final_bit;
      uint32 heritsetbit = 0;
      uint32 forbidbit = 0;

      switch (cptr->concept->kind) {
      case concept_comment:
         goto get_next;               /* Skip comments. */
      case concept_triangle:
         the_final_bit = FINAL__TRIANGLE;
         forbidbit = FINAL__LEADTRIANGLE;
         goto new_final;
      case concept_leadtriangle:
         the_final_bit = FINAL__LEADTRIANGLE;
         forbidbit = FINAL__TRIANGLE;
         goto new_final;
      case concept_magic:
         last_magic_diamond = cptr;
         heritsetbit = INHERITFLAG_MAGIC;
         forbidbit = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
         break;
      case concept_interlocked:
         last_magic_diamond = cptr;
         heritsetbit = INHERITFLAG_INTLK;
         forbidbit = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
         break;
      case concept_grand:
         heritsetbit = INHERITFLAG_GRAND;
         forbidbit = INHERITFLAG_SINGLE;
         break;
      case concept_cross:
         heritsetbit = INHERITFLAG_CROSS; break;
      case concept_reverse:
         heritsetbit = INHERITFLAG_REVERSE; break;
      case concept_fast:
         heritsetbit = INHERITFLAG_FAST; break;
      case concept_left:
         heritsetbit = INHERITFLAG_LEFT; break;
      case concept_yoyo:
         heritsetbit = INHERITFLAG_YOYO; break;
      case concept_fractal:
         heritsetbit = INHERITFLAG_FRACTAL; break;
      case concept_straight:
         heritsetbit = INHERITFLAG_STRAIGHT; break;
      case concept_twisted:
         heritsetbit = INHERITFLAG_TWISTED; break;
      case concept_single:
         heritsetbit = INHERITFLAG_SINGLE; break;
      case concept_singlefile:
         heritsetbit = INHERITFLAG_SINGLEFILE; break;
      case concept_1x2:
         heritsetbit = INHERITFLAGMXNK_1X2; break;
      case concept_2x1:
         heritsetbit = INHERITFLAGMXNK_2X1; break;
      case concept_1x3:
         heritsetbit = INHERITFLAGMXNK_1X3; break;
      case concept_3x1:
         heritsetbit = INHERITFLAGMXNK_3X1; break;
      case concept_2x2:
         heritsetbit = INHERITFLAGNXNK_2X2; break;
      case concept_3x3:
         heritsetbit = INHERITFLAGNXNK_3X3; break;
      case concept_4x4:
         heritsetbit = INHERITFLAGNXNK_4X4; break;
      case concept_5x5:
         heritsetbit = INHERITFLAGNXNK_5X5; break;
      case concept_6x6:
         heritsetbit = INHERITFLAGNXNK_6X6; break;
      case concept_7x7:
         heritsetbit = INHERITFLAGNXNK_7X7; break;
      case concept_8x8:
         heritsetbit = INHERITFLAGNXNK_8X8; break;
      case concept_revert:
         heritsetbit = cptr->concept->value.arg1; break;
      case concept_split:
         the_final_bit = FINAL__SPLIT;
         goto new_final;
      case concept_12_matrix:
         if (check_errors && (final_concepts->her8it | final_concepts->final))
            fail("Matrix modifier must appear first.");
         heritsetbit = INHERITFLAG_12_MATRIX;
         break;
      case concept_16_matrix:
         if (check_errors && (final_concepts->her8it | final_concepts->final))
            fail("Matrix modifier must appear first.");
         heritsetbit = INHERITFLAG_16_MATRIX;
         break;
      case concept_diamond:
         heritsetbit = INHERITFLAG_DIAMOND;
         forbidbit = INHERITFLAG_SINGLE;
         break;
      case concept_funny:
         heritsetbit = INHERITFLAG_FUNNY; break;
      default:
         goto exit5;
      }

      /* At this point we have a "herit" concept. */

      if (check_errors) {
         if (final_concepts->her8it & forbidbit)
            fail("Illegal order of call modifiers.");

         if (do_heritflag_merge(&final_concepts->her8it, heritsetbit))
            fail("Illegal combination of call modifiers.");
      }
      else {
         /* If not checking for errors, we just have to set the "herit" field
            nonzero. */

         final_concepts->her8it |= heritsetbit;
      }

      goto check_level;

   new_final:

      /* This is a "final" concept.  It is more straightforward, because the word
         is not broken into fields that need special checking. */

      if (check_errors) {
         if ((final_concepts->final & (the_final_bit|forbidbit)))
            fail("Redundant call modifier.");
      }
      else {
         /* Put the bit into the "herit" stuff also, so that "herit" will tell
            whether any modifier at all was seen.  Only the zero/nonzero nature
            of the "herit" word will be looked at if we are not checking for errors. */

         final_concepts->her8it |= the_final_bit;
      }

      final_concepts->final |= the_final_bit;

   check_level:

      if (check_errors && cptr->concept->level > calling_level)
         warn(warn__bad_concept_level);

   get_next:

      cptr = cptr->next;
   }

   exit5:

   return cptr;
}


extern parse_block *really_skip_one_concept(
   parse_block *incoming,
   concept_kind *k_p,
   uint32 *need_to_restrain_p,   /* 1=(if not doing echo), 2=(yes, always) */
   parse_block **parseptr_skip_p)
{
   uint64 junk_concepts;
   parse_block *parseptrcopy;

   while (incoming->concept->kind == concept_comment)
      incoming = incoming->next;

   junk_concepts.her8it = 0;
   junk_concepts.final = 0;

   parseptrcopy = process_final_concepts(incoming, FALSE, &junk_concepts);

   /* Find out whether the next concept (the one that will be "random" or whatever)
      is a modifier or a "real" concept. */

   if (junk_concepts.her8it | junk_concepts.final) {
      parseptrcopy = incoming;
   }
   else if (parseptrcopy->concept) {
      concept_kind kk = parseptrcopy->concept->kind;

      if (check_for_supercall(parseptrcopy)) {
         *parseptr_skip_p = parseptrcopy->next->subsidiary_root;
         *k_p = concept_supercall;

         /* We don't restrain for echo with supercalls, because echo doesn't pull parts
            apart, and supercalls don't work with multiple-part calls as the target
            for which they are restraining the concept. */

         *need_to_restrain_p = 1;
         return parseptrcopy;
      }

      if (concept_table[kk].concept_action == 0)
         fail("Sorry, can't do this with this concept.");

      if ((concept_table[kk].concept_prop & CONCPROP__SECOND_CALL) &&
          parseptrcopy->concept->kind != concept_special_sequential)
         fail("Can't use a concept that takes a second call.");
   }

   (void) check_for_concept_group(parseptrcopy, TRUE, k_p, need_to_restrain_p, parseptr_skip_p);

   return parseptrcopy;
}




/* Prepare several setups to be assembled into one, by making them all have
   the same kind and rotation.  If there is a question about what ending
   setup to opt for (because of lots of phantoms), use "goal". */

extern long_boolean fix_n_results(int arity, setup_kind goal, setup z[], uint32 *rotstatep)
{
   int i;
   long_boolean lineflag = FALSE;
   long_boolean dmdflag = FALSE;
   long_boolean qtflag = FALSE;
   long_boolean miniflag = FALSE;
   int deadconcindex = -1;
   setup_kind kk = nothing;
   uint16 rotstates = 0xFFF;
   static uint16 rotstate_table[16] = {
      0x111, 0x222, 0x404, 0x808,
      0x421, 0x812, 0x104, 0x208,
      0x111, 0x222, 0x404, 0x808,
      0x421, 0x812, 0x104, 0x208};

   /* There are 3 things that make this task nontrivial.  First, some setups could
      be "nothing", in which case we turn them into the same type of setup as
      their neighbors, with no people.  Second, some types of "grand working"
      can leave setups confused about whether they are lines or diamonds,
      because only the ends/points are occupied.  We turn those setups into
      whatever matches their more-fully-occupied neighbors.  (If completely in
      doubt, we opt for 1x4's.)  Third, some 1x4's may have been shrunk to 1x2's.
      This can happen in some cases of "triple box patch the so-and-so", which lead
      to triple lines in which the outer lines collapsed from 1x4's to 1x2's while
      the center line is fully occupied.  In this case we repopulate the outer lines
      to 1x4's. */

   for (i=0; i<arity; i++) {
      if (z[i].kind == s_normal_concentric) {
         /* We definitely have a problem.  A common way for this to happen is if a concentric call
            was done and there were no ends, so we don't know what the setup really is.  Example:
            from normal columns, do a split phantom lines hocus-pocus.  Do we leave space for the
            phantoms?  Humans would probably say yes because they know where the phantoms went, but
            this program has no idea in general.  If a call is concentrically executed and the
            outsides are all phantoms, we don't know what the setup is.  Concentric_move signifies
            this by creating a "concentric" setup with "nothing" for the outer setup.  So we raise
            an error that is somewhat descriptive. */
         if (z[i].outer.skind == nothing)
            continue;      /* Defer this until later; we may be able to figure something out. */
         else if (z[i].inner.skind == nothing && z[i].outer.skind == s1x2) {
            /* We can fix this up.  Just turn it into a 1x4 with the ends missing.
            (If a diamond is actually required, that will get fixed up below.)
            The test case for this is split phantom lines cross to a diamond from 2FL. */

            z[i].kind = s1x4;
            z[i].rotation = z[i].outer.srotation;
            copy_person(&z[i], 0, &z[i], 12);
            copy_person(&z[i], 2, &z[i], 13);
            clear_person(&z[i], 1);
            clear_person(&z[i], 3);
         }
         else
            fail("Don't recognize ending setup for this call.");
      }
      else if (z[i].kind == s_dead_concentric) {
         continue;      /* Defer this until later; we may be able to figure something out. */
      }

      if (z[i].kind != nothing) {
         int zisrot;
         canonicalize_rotation(&z[i]);
         zisrot = z[i].rotation & 3;

         if (z[i].kind == s1x2)
            miniflag = TRUE;
         else if ((z[i].kind == s1x4 || z[i].kind == sdmd) &&
                  (z[i].people[1].id1 | z[i].people[3].id1) == 0)
            lineflag = TRUE;
         else {
            if (kk == nothing) kk = z[i].kind;

            if (kk != z[i].kind) {
               /* We may have a minor problem -- differently oriented
                  2x4's and qtag's with just the ends present might want to be
                  like each other.  We will turn them into qtags.
                  Or 1x4's with just the centers present might want
                  to become diamonds. */

               if (((kk == s2x4 && z[i].kind == s_qtag) ||
                    (kk == s_qtag && z[i].kind == s2x4))) {
                  qtflag = TRUE;
                  zisrot ^= 1;
               }
               else if (((kk == s1x4 && z[i].kind == sdmd) ||
                         (kk == sdmd && z[i].kind == s1x4))) {
                  dmdflag = TRUE;
                  zisrot ^= 1;
               }
               else
                  goto lose;
            }
         }

         /* If the setups are "trngl" or "trngl4", the rotations have
            to alternate by 180 degrees. */

         if (z[i].kind == s_trngl || z[i].kind == s_trngl4)
            rotstates &= 0xF00;
         else
            rotstates &= 0x033;

         {
            int shit = i;

            if (arity == 4 && (i&2)) shit ^= 1;

            rotstates &= rotstate_table[((shit & 3) << 2) | zisrot];
         }
      }
   }

   if (kk == nothing) {
      /* If client really needs a diamond, return a diamond.
         Otherwise opt for 1x4. */
      if (lineflag) kk = (goal == sdmd) ? sdmd : s1x4;
      else if (miniflag) kk = s1x2;
   }

   if (arity == 1) rotstates &= 0x3;
   if (!rotstates) goto lose;

   /* Now deal with any setups that we may have deferred. */

   if (dmdflag && kk == s1x4) {
      rotstates ^= 3;
      kk = sdmd;
   }
   if (qtflag && kk == s2x4) {
      rotstates ^= 3;
      kk = s_qtag;
   }

   for (i=0; i<arity; i++) {
      if (z[i].kind == s_dead_concentric ||
          (z[i].kind == s_normal_concentric && z[i].outer.skind == nothing)) {
         int rr;

         if (z[i].inner.skind == s2x2 && kk == s2x4) {
            /* Turn the 2x2 into a 2x4.  Need to make it have same rotation as the others;
               that is, rotation = rr.  (We know that rr has something in it by now.) */

            /* We might have a situation with alternating rotations, or we
               might have homogeneous rotations.  If the state says we could
               have both, change it to just homogeneous.  That is, we default
               to same rotation unless live people force mixed rotation. */

            if (rotstates & 0x0F) rotstates &= 0x03;     /* That does the defaulting. */
            if (!rotstates) goto lose;
            rr = (((rotstates & 0x0F0) ? (rotstates >> 4) : rotstates) >> 1) & 1;

            z[i].inner.srotation -= rr;
            canonicalize_rotation(&z[i]);
            z[i].kind = kk;
            z[i].rotation = rr;
            swap_people(&z[i], 3, 6);
            swap_people(&z[i], 2, 5);
            swap_people(&z[i], 2, 1);
            swap_people(&z[i], 1, 0);
            clear_person(&z[i], 4);
            clear_person(&z[i], 7);
            clear_person(&z[i], 0);
            clear_person(&z[i], 3);
            canonicalize_rotation(&z[i]);
         }
         else if (z[i].inner.skind == s1x4 && kk == s_qtag) {
            /* Turn the 1x4 into a qtag. */
            if (rotstates & 0x0F) rotstates &= 0x03;     /* That does the defaulting. */
            if (!rotstates) goto lose;
            rr = (((rotstates & 0x0F0) ? (rotstates >> 4) : rotstates) >> 1) & 1;
            if (z[i].inner.srotation != rr) goto lose;

            z[i].kind = kk;
            z[i].rotation = rr;
            swap_people(&z[i], 0, 6);
            swap_people(&z[i], 1, 7);
            clear_person(&z[i], 0);
            clear_person(&z[i], 1);
            clear_person(&z[i], 4);
            clear_person(&z[i], 5);
            canonicalize_rotation(&z[i]);
         }
         else if (kk == nothing &&
                  (  deadconcindex < 0 ||
                     (  z[i].inner.skind == z[deadconcindex].inner.skind &&
                        z[i].inner.srotation == z[deadconcindex].inner.srotation))) {
            deadconcindex = i;
         }
         else
            fail("Can't do this: don't know where the phantoms went.");
      }
      else if (qtflag && z[i].kind == s2x4) {
         /* Turn the 2x4 into a qtag. */
         if (z[i].people[1].id1 | z[i].people[2].id1 |
             z[i].people[5].id1 | z[i].people[6].id1) goto lose;

         z[i].kind = s_qtag;
         z[i].rotation++;
         (void) copy_rot(&z[i], 5, &z[i], 0, 033);
         (void) copy_rot(&z[i], 0, &z[i], 3, 033);
         (void) copy_rot(&z[i], 1, &z[i], 4, 033);
         (void) copy_rot(&z[i], 4, &z[i], 7, 033);
         clear_person(&z[i], 3);
         clear_person(&z[i], 7);
      }
      else if (dmdflag && z[i].kind == s1x4) {
         /* Turn the 1x4 into a diamond. */
         if (z[i].people[0].id1 | z[i].people[2].id1) goto lose;

         z[i].kind = sdmd;
         z[i].rotation--;
         (void) copy_rot(&z[i], 1, &z[i], 1, 011);
         (void) copy_rot(&z[i], 3, &z[i], 3, 011);
      }
      else {
         canonicalize_rotation(&z[i]);
         continue;
      }

      canonicalize_rotation(&z[i]);
#ifdef DOESNT_DO
      /* They're not alternating any more. */

      if (rotstates & 0x0F0) {
         if (i&1) rotstates ^= 0x030;
         rotstates >>= 4;
      }
#endif
   }
  
   if (deadconcindex >= 0) {
      kk = z[deadconcindex].kind;
      rotstates = 0x001;
   }

   if (kk == nothing) return TRUE;

   /* If something wasn't sure whether it was points of a diamond or
      ends of a 1x4, that's OK if something else had a clue. */
   if (lineflag && kk != s1x4 && kk != sdmd) goto lose;

   /* If something was a 1x2, that's OK if something else was a 1x4. */
   if (miniflag && kk != s1x4 && kk != s1x2) goto lose;

   for (i=0; i<arity; i++) {
      if (z[i].kind == nothing)
         clear_people(&z[i]);
      else if (z[i].kind == s1x2 && kk == s1x4) {
         /* We have to expand a 1x2 to the center spots of a 1x4. */
         (void) copy_person(&z[i], 3, &z[i], 1);
         clear_person(&z[i], 2);
         (void) copy_person(&z[i], 1, &z[i], 0);
         clear_person(&z[i], 0);
      }

      z[i].kind = kk;

      if (kk == s_dead_concentric || kk == s_normal_concentric) {
         z[i].inner.skind = z[deadconcindex].inner.skind;
         z[i].inner.srotation = z[deadconcindex].inner.srotation;
      }

      /* We know rotstates has a nonzero bit in an appropriate field. */

      if (z[i].kind == s_trngl || z[i].kind == s_trngl4) {
         z[i].rotation = i << 1;
         if (rotstates & 0xC00) z[i].rotation += 2;
         if (rotstates & 0xA00) z[i].rotation++;
      }
      else
         z[i].rotation = (rotstates >> 1) & 1;
   }

   *rotstatep = rotstates;
   return FALSE;

   lose:

   fail("This is an inconsistent shape or orientation changer!!");
   return FALSE;
}



typedef struct {
  resolve_kind k;
  dance_level level_needed;
  short distance;      /* Add the 64 bit for singer-only; these must be last. */
  short nonzero_only;  /* If this is nonzero, we demand only nonzero distances. */
  int locations[8];
  uint32 directions;
} resolve_tester;

static resolve_tester test_thar_stuff[] = {
   {resolve_rlg,            l_mainstream,      2, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8A31A813},    /* RLG from thar. */
   {resolve_prom,           l_mainstream,      6, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8833AA11},    /* promenade from thar. */
   {resolve_slipclutch_rlg, l_mainstream,      1, 0,   {5, 2, 3, 0, 1, 6, 7, 4},     0x8138A31A},    /* slip-the-clutch-RLG from thar. */
   {resolve_la,             l_mainstream,      5, 0,   {5, 2, 3, 0, 1, 6, 7, 4},     0xA31A8138},    /* LA from thar. */
   {resolve_slipclutch_la,  l_mainstream,      6, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0xA8138A31},    /* slip-the-clutch-LA from thar. */
   {resolve_xby_rlg,        cross_by_level,    1, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x8138A31A},    /* cross-by-RLG from thar. */
   {resolve_revprom,        l_mainstream,      4, 0,   {2, 3, 0, 1, 6, 7, 4, 5},     0x118833AA},    /* reverse promenade from thar. */
   {resolve_xby_la,         cross_by_level,    4, 0,   {2, 3, 0, 1, 6, 7, 4, 5},     0x138A31A8},    /* cross-by-LA from thar. */
   {resolve_none,           l_mainstream,      64}};

static resolve_tester test_4x4_stuff[] = {
   {resolve_circle,         l_mainstream,      6, 0,   {2, 1, 14, 13, 10, 9, 6, 5},  0x33AA1188},    /* "circle left/right" from squared-set, normal. */
   {resolve_circle,         l_mainstream,      7, 0,   {5, 2, 1, 14, 13, 10, 9, 6},  0x833AA118},    /* "circle left/right" from squared-set, sashayed. */
   {resolve_rlg,            l_mainstream,      3, 0,   {5, 2, 1, 14, 13, 10, 9, 6},  0x8A8AA8A8},    /* RLG from vertical 8-chain in "O". */
   {resolve_rlg,            l_mainstream,      3, 0,   {5, 2, 1, 14, 13, 10, 9, 6},  0x13313113},    /* RLG from horizontal 8-chain in "O". */
   {resolve_la,             l_mainstream,      6, 0,   {2, 1, 14, 13, 10, 9, 6, 5},  0x33131131},    /* LA from horizontal 8-chain in "O". */
   {resolve_la,             l_mainstream,      6, 0,   {2, 1, 14, 13, 10, 9, 6, 5},  0xA8AA8A88},    /* LA from vertical 8-chain in "O". */
   {resolve_rlg,            l_mainstream,      2, 0,   {2, 1, 14, 13, 10, 9, 6, 5},  0x8A31A813},    /* RLG from squared set, facing directly. */
   {resolve_la,             l_mainstream,      7, 0,   {5, 2, 1, 14, 13, 10, 9, 6},  0x38A31A81},    /* LA from squared set, facing directly. */
   {resolve_rlg,            l_mainstream,      3, 0,   {5, 2, 1, 14, 13, 10, 9, 6},  0x1A8138A3},    /* RLG from squared set, around the corner. */
   {resolve_la,             l_mainstream,      6, 0,   {2, 1, 14, 13, 10, 9, 6, 5},  0xA8138A31},    /* LA from squared set, around the corner. */
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 2, 3, 14, 15, 10, 11, 6}, 0x138A31A8},    /* RLG from pinwheel, all facing. */
   {resolve_rlg,            l_mainstream,      3, 0,   {5, 7, 1, 3, 13, 15, 9, 11},  0x8A31A813},    /* RLG from pinwheel, all facing. */
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 5, 3, 1, 15, 13, 11, 9},  0x138A31A8},    /* RLG from pinwheel, all in miniwaves. */
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 2, 3, 14, 15, 10, 11, 6}, 0x8A31A813},    /* RLG from pinwheel, all in miniwaves. */
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 2, 3, 14, 15, 10, 11, 6}, 0x13313113},    /* RLG from pinwheel, some of each. */
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 2, 3, 14, 15, 10, 11, 6}, 0x8A8AA8A8},    /* RLG from pinwheel, others of each. */
   {resolve_rlg,            l_mainstream,      3, 0,   {5, 7, 3, 1, 13, 15, 11, 9},  0x8A8AA8A8},    /* RLG from pinwheel, some of each. */
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 5, 1, 3, 15, 13, 9, 11},  0x13313113},    /* RLG from pinwheel, others of each. */

   // From clumps.
   {resolve_rlg,            l_mainstream, 2, 0,   {3, 1, 14, 0, 11, 9, 6, 8},       0x8A8AA8A8},
   {resolve_rlg,            l_mainstream, 4, 0,   {5, 4, 7, 2, 13, 12, 15, 10},     0x8A8AA8A8},
   {resolve_rlg,            l_mainstream, 2, 0,   {1, 0, 3, 14, 9, 8, 11, 6},       0x31311313},
   {resolve_rlg,            l_mainstream, 4, 0,   {7, 5, 2, 4, 15, 13, 10, 12},     0x13133131},
   {resolve_la,             l_mainstream, 5, 0,   {3, 0, 14, 9, 11, 8, 6, 1},       0xA8AA8A88},
   {resolve_la,             l_mainstream, 7, 0,   {5, 2, 7, 12, 13, 10, 15, 4},     0xA8AA8A88},
   {resolve_la,             l_mainstream, 5, 0,   {1, 14, 3, 8, 9, 6, 11, 0},       0x13113133},
   {resolve_la,             l_mainstream, 7, 0,   {7, 4, 2, 13, 15, 12, 10, 5},     0x31331311},
   {resolve_none,           l_mainstream,      64}};

static resolve_tester test_c1phan_stuff[] = {
   {resolve_rlg,            l_mainstream,      3, 0,   {10, 8, 6, 4, 2, 0, 14, 12},  0x138A31A8},    /* RLG from phantoms, all facing. */
   {resolve_rlg,            l_mainstream,      3, 0,   {9, 11, 5, 7, 1, 3, 13, 15},  0x8A31A813},    /* RLG from phantoms, all facing. */
   {resolve_rlg,            l_mainstream,      3, 0,   {11, 9, 7, 5, 3, 1, 15, 13},  0x138A31A8},    /* RLG from phantoms, all in miniwaves. */
   {resolve_rlg,            l_mainstream,      3, 0,   {10, 8, 6, 4, 2, 0, 14, 12},  0x8A31A813},    /* RLG from phantoms, all in miniwaves. */
   {resolve_rlg,            l_mainstream,      3, 0,   {10, 8, 6, 4, 2, 0, 14, 12},  0x13313113},    /* RLG from phantoms, some of each. */
   {resolve_rlg,            l_mainstream,      3, 0,   {10, 8, 6, 4, 2, 0, 14, 12},  0x8A8AA8A8},    /* RLG from phantoms, others of each. */
   {resolve_rlg,            l_mainstream,      3, 0,   {9, 11, 7, 5, 1, 3, 15, 13},  0x8A8AA8A8},    /* RLG from phantoms, some of each. */
   {resolve_rlg,            l_mainstream,      3, 0,   {11, 9, 5, 7, 3, 1, 13, 15},  0x13313113},    /* RLG from phantoms, others of each. */
   {resolve_none,           l_mainstream,      64}};

static resolve_tester test_galaxy_stuff[] = {
   {resolve_rlg,            l_mainstream, 2, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x1A313813},
   {resolve_rlg,            l_mainstream, 2, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8A81A8A3},
   {resolve_none,           l_mainstream,      64}};

static resolve_tester test_qtag_stuff[] = {
   {resolve_dixie_grand,    dixie_grand_level, 2, 0,   {5, 0, 2, 7, 1, 4, 6, 3},     0x8AAAA888},    /* dixie grand from 1/4 tag. */
   {resolve_rlg,            l_mainstream,      4, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0xAA8A88A8},    /* RLG from 3/4 tag. */
   {resolve_ext_rlg,        l_mainstream,      5, 0,   {7, 5, 4, 2, 3, 1, 0, 6},     0xA88A8AA8},    /* extend-RLG from 1/4 tag. */
   {resolve_rlg,            l_mainstream,      4, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x138A31A8},    /* RLG from diamonds with points facing each other. */
   {resolve_rlg,            l_mainstream,      4, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8A8AA8A8},    /* RLG from "6x2 acey deucey" type of 1/4 tag. */
   {resolve_la,             l_mainstream,      7, 0,   {4, 2, 3, 1, 0, 6, 7, 5},     0xA8A88A8A},    /* LA from 3/4 tag. */
   {resolve_ext_la,         l_mainstream,      6, 0,   {3, 2, 1, 0, 7, 6, 5, 4},     0xA8AA8A88},    /* extend-LA from 1/4 tag. */
   {resolve_la,             l_mainstream,      7, 0,   {4, 2, 3, 1, 0, 6, 7, 5},     0x38A31A81},    /* LA from diamonds with points facing each other. */
   /* Singers only. */
   {resolve_rlg,            l_mainstream,   64+4, 0,   {4, 5, 3, 2, 0, 1, 7, 6},     0xAA8A88A8},    /* swing/prom from 3/4 tag, ends sashayed (normal case is above). */
   {resolve_rlg,            l_mainstream,   64+4, 0,   {5, 4, 2, 3, 1, 0, 6, 7},     0xAAA8888A},    /* swing/prom from 3/4 tag, centers traded, ends normal. */
   {resolve_rlg,            l_mainstream,   64+4, 0,   {4, 5, 2, 3, 0, 1, 6, 7},     0xAAA8888A},    /* swing/prom from 3/4 tag, centers traded, ends sashayed. */
   {resolve_none,           l_mainstream,      64}};

static resolve_tester test_2x6_stuff[] = {
   // RLG/LA from "Z" 8-chain.
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 6, 4, 3, 1, 0, 10, 9},    0x13313113},
   {resolve_rlg,            l_mainstream,      3, 0,   {8, 7, 5, 4, 2, 1, 11, 10},   0x13313113},
   {resolve_la,             l_mainstream,      6, 0,   {6, 4, 3, 1, 0, 10, 9, 7},    0x33131131},
   {resolve_la,             l_mainstream,      6, 0,   {7, 5, 4, 2, 1, 11, 10, 8},   0x33131131},
   // RLG/LA from outer triple boxes.
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 6, 4, 5, 1, 0, 10, 11},   0x8A8AA8A8},
   {resolve_la,             l_mainstream,      6, 0,   {7, 5, 4, 0, 1, 11, 10, 6},   0xA8AA8A88},
   // RLG/LA from parallelogram waves.
   {resolve_rlg,            l_mainstream,      3, 0,   {7, 6, 2, 3, 1, 0, 8, 9},     0x8A8AA8A8},
   {resolve_la,             l_mainstream,      6, 0,   {7, 3, 2, 0, 1, 9, 8, 6},     0xA8AA8A88},
   {resolve_rlg,            l_mainstream,      3, 0,   {9, 8, 4, 5, 3, 2, 10, 11},   0x8A8AA8A8},
   {resolve_la,             l_mainstream,      6, 0,   {9, 5, 4, 2, 3, 11, 10, 8},   0xA8AA8A88},
   {resolve_none,           l_mainstream,      64}};

static resolve_tester test_3x4_stuff[] = {
   /* These test for offset waves. */
   {resolve_rlg,            l_mainstream, 3, 0,   {7, 6, 5, 4, 1, 0, 11, 10},   0x8A8AA8A8},
   {resolve_rlg,            l_mainstream, 3, 0,   {5, 4, 2, 3, 11, 10, 8, 9},   0x8A8AA8A8},
   {resolve_la,             l_mainstream, 6, 0,   {7, 4, 5, 0, 1, 10, 11, 6},   0xA8AA8A88},
   {resolve_la,             l_mainstream, 6, 0,   {5, 3, 2, 10, 11, 9, 8, 4},   0xA8AA8A88},
   {resolve_none,           l_mainstream, 64}};

static resolve_tester test_4dmd_stuff[] = {
   /* These test for people in a miniwave as centers of the outer diamonds,
      while the others are facing each other as points.  There are three of each
      to allow for the outsides to be symmetrical (points of the center diamonds)
      or unsymmetrical (points of a center diamond and the adjacent outer diamond). */
   {resolve_la,             l_mainstream, 7, 0,   {8, 4, 5, 1, 0, 12, 13, 9},   0x38A31A81},
   {resolve_la,             l_mainstream, 7, 0,   {9, 4, 5, 2, 1, 12, 13, 10},  0x38A31A81},
   {resolve_la,             l_mainstream, 7, 0,   {10, 4, 5, 3, 2, 12, 13, 11}, 0x38A31A81},
   {resolve_rlg,            l_mainstream, 4, 0,   {9, 8, 5, 4, 1, 0, 13, 12},   0x138A31A8},
   {resolve_rlg,            l_mainstream, 4, 0,   {10, 9, 5, 4, 2, 1, 13, 12},  0x138A31A8},
   {resolve_rlg,            l_mainstream, 4, 0,   {11, 10, 5, 4, 3, 2, 13, 12}, 0x138A31A8},
   {resolve_none,           l_mainstream, 64}};

static resolve_tester test_bigdmd_stuff[] = {
   /* These test for miniwaves in "common point diamonds". */
   {resolve_rlg,            l_mainstream, 2, 0,   {7, 6, 3, 2, 1, 0, 9, 8},     0x8A31A813},
   {resolve_rlg,            l_mainstream, 2, 0,   {4, 5, 3, 2, 10, 11, 9, 8},   0x8A31A813},
   {resolve_la,             l_mainstream, 5, 0,   {7, 2, 3, 0, 1, 8, 9, 6},     0xA31A8138},
   {resolve_la,             l_mainstream, 5, 0,   {4, 2, 3, 11, 10, 8, 9, 5},   0xA31A8138},
   {resolve_none,           l_mainstream, 64}};

static resolve_tester test_deepqtg_stuff[] = {
   {resolve_circle,         l_mainstream, 6, 0,   {11, 2, 1, 0, 5, 8, 7, 6},  0x33AA1188},
   {resolve_circle,         l_mainstream, 7, 0,   {6, 11, 2, 1, 0, 5, 8, 7},  0x833AA118},
   {resolve_none,           l_mainstream, 64}};

static resolve_tester test_spindle_stuff[] = {
   /* These test for people looking around the corner. */
   {resolve_rlg,            l_mainstream, 3, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x13313113},
   {resolve_la,             l_mainstream, 7, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x33131131},
   {resolve_none,           l_mainstream, 64}};

static resolve_tester test_2x4_stuff[] = {
   /* 8-chain. */
   {resolve_rlg,            l_mainstream, 3, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x13313113},
   {resolve_la,             l_mainstream, 6, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x33131131},
   {resolve_pth_rlg,        l_mainstream, 2, 0,   {5, 2, 3, 0, 1, 6, 7, 4},     0x11313313},
   {resolve_pth_la,         l_mainstream, 5, 0,   {2, 3, 0, 1, 6, 7, 4, 5},     0x13133131},

   /* trade-by. */
   {resolve_rlg,            l_mainstream, 2, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x11313313},
   {resolve_la,             l_mainstream, 7, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x31131331},
   {resolve_tby_rlg,        l_mainstream, 3, 0,   {6, 3, 4, 1, 2, 7, 0, 5},     0x11113333},
   {resolve_tby_la,         l_mainstream, 0, 0,   {5, 6, 3, 4, 1, 2, 7, 0},     0x31111333},

   /* waves. */
   {resolve_rlg,            l_mainstream, 3, 0,   {5, 4, 2, 3, 1, 0, 6, 7},     0x8A8AA8A8},
   {resolve_la,             l_mainstream, 6, 0,   {5, 3, 2, 0, 1, 7, 6, 4},     0xA8AA8A88},

   {resolve_ext_rlg,        extend_34_level, 2, 0,   {5, 3, 2, 0, 1, 7, 6, 4},     0x8A88A8AA},    /* extend-RLG from waves. */
   {resolve_circ_rlg,       l_mainstream, 1, 0,   {5, 0, 2, 7, 1, 4, 6, 3},     0x8888AAAA},    /* circulate-RLG from waves. */
   {resolve_xby_rlg,        cross_by_level, 2, 0,   {4, 2, 3, 1, 0, 6, 7, 5},     0x8A88A8AA},    /* cross-by-RLG from waves. */
   {resolve_rlg,            l_mainstream, 2, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x8A31A813},    /* RLG from T-bone setup, ends facing. */
   {resolve_rlg,            l_mainstream, 2, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x31311313},    /* RLG from centers facing and ends in miniwaves. */
   {resolve_la,             l_mainstream, 6, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0xA8888AAA},    /* LA from lines-out. */
   {resolve_ext_la,         extend_34_level,   7, 0,   {5, 4, 2, 3, 1, 0, 6, 7},     0xA8A88A8A},    /* ext-LA from waves. */
   {resolve_circ_la,        l_mainstream,      0, 0,   {5, 7, 2, 4, 1, 3, 6, 0},     0xAAA8888A},    /* circulate-LA from waves. */
   {resolve_xby_la,         cross_by_level,    5, 0,   {3, 2, 0, 1, 7, 6, 4, 5},     0xA88A8AA8},    /* cross-by-LA from waves. */
   {resolve_la,             l_mainstream,      7, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x38A31A81},    /* LA from T-bone setup, ends facing. */
   {resolve_prom,           l_mainstream,      7, 0,   {5, 4, 2, 3, 1, 0, 6, 7},     0x8888AAAA},    /* promenade from 2FL. */
   {resolve_revprom,        l_mainstream,      5, 0,   {3, 2, 0, 1, 7, 6, 4, 5},     0xAA8888AA},    /* reverse promenade from 2FL. */
   {resolve_circle,         l_mainstream,      6, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x33AA1188},    /* "circle left/right" from pseudo squared-set, normal. */
   {resolve_circle,         l_mainstream,      7, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x833AA118},    /* "circle left/right" from pseudo squared-set, sashayed. */
   {resolve_circle,         l_mainstream,      6, 1,   {4, 3, 2, 1, 0, 7, 6, 5},     0x8AAAA888},    /* "circle left/right" from lines-in, sashayed. */
   {resolve_circle,         l_mainstream,      7, 1,   {5, 4, 3, 2, 1, 0, 7, 6},     0x88AAAA88},    /* "circle left/right" from lines-in, normal. */
   {resolve_dixie_grand,    dixie_grand_level, 2, 0,   {5, 2, 4, 7, 1, 6, 0, 3},     0x33311113},    /* dixie grand from DPT. */
   {resolve_dixie_grand,    dixie_grand_level, 3, 0,   {6, 3, 5, 0, 2, 7, 1, 4},     0x33331111},    /* dixie grand from magic column. */
   {resolve_dixie_grand,    dixie_grand_level, 1, 0,   {4, 1, 3, 6, 0, 5, 7, 2},     0x33111133},    /* dixie grand from other magic column. */
   {resolve_sglfileprom,    l_mainstream,      7, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x11333311},    /* single file promenade from L columns */
   {resolve_sglfileprom,    l_mainstream,      6, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x13333111},    /* single file promenade from L columns */
   {resolve_sglfileprom,    l_mainstream,      7, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x18833AA1},    /* single file promenade from T-bone */
   {resolve_sglfileprom,    l_mainstream,      6, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x8833AA11},    /* single file promenade from T-bone */
   {resolve_revsglfileprom, l_mainstream,      7, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x33111133},    /* reverse single file promenade from R columns */
   {resolve_revsglfileprom, l_mainstream,      6, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x31111333},    /* reverse single file promenade from R columns */
   {resolve_revsglfileprom, l_mainstream,      7, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x3AA11883},    /* reverse single file promenade from T-bone */
   {resolve_revsglfileprom, l_mainstream,      6, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0xAA118833},    /* reverse single file promenade from T-bone */
   {resolve_rlg,            l_mainstream,      3, 0,   {5, 4, 2, 3, 1, 0, 6, 7},     0x138A31A8},    /* RLG, T-bone mixed 8-chain and waves. */
   {resolve_rlg,            l_mainstream,      3, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8A31A813},    /* RLG, other T-bone mixed 8-chain and waves. */
   {resolve_la,             l_mainstream,      6, 0,   {4, 3, 2, 1, 0, 7, 6, 5},     0x38A31A81},    /* LA, T-bone mixed 8-chain and waves. */
   {resolve_la,             l_mainstream,      6, 0,   {5, 3, 2, 0, 1, 7, 6, 4},     0xA31A8138},    /* LA, other T-bone mixed 8-chain and waves. */
   /* Singers only. */
   {resolve_rlg,            l_mainstream,   64+3, 0,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8AA8A88A},    /* swing/prom from waves, boys looking in. */
   {resolve_rlg,            l_mainstream,   64+3, 0,   {4, 5, 2, 3, 0, 1, 6, 7},     0xA88A8AA8},    /* swing/prom from waves, girls looking in. */
   {resolve_rlg,            l_mainstream,   64+3, 0,   {4, 5, 2, 3, 0, 1, 6, 7},     0xAA8888AA},    /* swing/prom from lines-out. */
   {resolve_rlg,            l_mainstream,   64+1, 0,   {3, 2, 0, 1, 7, 6, 4, 5},     0xA88A8AA8},    /* same as cross-by-LA from waves (above), but it's mainstream here. */
   {resolve_rlg,            l_mainstream,   64+3, 0,   {5, 4, 2, 3, 1, 0, 6, 7},     0x13133131},    /* 8-chain, boys in center. */
   {resolve_rlg,            l_mainstream,   64+1, 0,   {3, 2, 0, 1, 7, 6, 4, 5},     0x31131331},    /* 8-chain, girls in center. */
   {resolve_rlg,            l_mainstream,   64+2, 0,   {4, 3, 1, 2, 0, 7, 5, 6},     0x11133331},    /* trade-by, ends sashayed. */
   {resolve_rlg,            l_mainstream,   64+4, 0,   {6, 5, 3, 4, 2, 1, 7, 0},     0x13113133},    /* trade-by, centers sashayed. */
   {resolve_none,           l_mainstream,      64}};


extern resolve_indicator resolve_p(setup *s)
{
   resolve_indicator k;
   resolve_tester *testptr;
   int i;
   uint32 singer_offset = 0;

   if (singing_call_mode == 1) singer_offset = 0600;
   else if (singing_call_mode == 2) singer_offset = 0200;

   switch (s->kind) {
   case s2x4:
      testptr = test_2x4_stuff; break;
   case s3x4:
      testptr = test_3x4_stuff; break;
   case s2x6:
      testptr = test_2x6_stuff; break;
   case s_qtag:
      testptr = test_qtag_stuff; break;
   case s4dmd:
      testptr = test_4dmd_stuff; break;
   case sbigdmd:
      testptr = test_bigdmd_stuff; break;
   case sdeepqtg:
      testptr = test_deepqtg_stuff; break;
   case s4x4:
      testptr = test_4x4_stuff; break;
   case s_c1phan:
      testptr = test_c1phan_stuff; break;
   case s_galaxy:
      testptr = test_galaxy_stuff; break;
   case s_crosswave: case s_thar:
      /* This makes use of the fact that the person numbering
         in crossed lines and thars is identical. */
      testptr = test_thar_stuff; break;
   case s_spindle:
      testptr = test_spindle_stuff; break;
   default: goto no_resolve;
   }

   do {
      uint32 directionword;
      uint32 firstperson = s->people[testptr->locations[0]].id1 & 0700;
      if (firstperson & 0100) goto not_this_one;

      /* We run the tests in descending order, because the test for i=0 is especially
         likely to pass (since the person ID is known to match), and we want to find
         failures as quickly as possible. */

      for (i=7,directionword=testptr->directions ; i>=0 ; i--,directionword>>=4) {
         uint32 expected_id = (i << 6) + ((i&1) ? singer_offset : 0);

         /* The add of "expected_id" and "firstperson" may overflow out of the "700" bits
            into the next 2 bits.  (One bit for each add.) */

         if ((s->people[testptr->locations[i]].id1                  /* The person under test */
              ^                                                     /* XOR */
              (expected_id + firstperson + (directionword & 0xF)))  /* what we check against */
             &                                                      /* but only test some bits */
             0777)    /* The bits we check -- the person ID and the direction. */
            goto not_this_one;
      }

      if (calling_level < testptr->level_needed) goto not_this_one;

      k.kind = testptr->k;
      k.distance = ((s->rotation << 1) + (firstperson >> 6) + testptr->distance) & 7;
      if (k.distance == 0 && testptr->nonzero_only != 0) goto not_this_one;
      return k;

      not_this_one: ;
   }
   while (
            !((++testptr)->distance & 64)    /* always do next one if it doesn't have the singer-only mark. */
                     ||
            (singing_call_mode != 0 && testptr->k != resolve_none)  /* Even if it has the mark, do it if    */
         );                                                         /* this is a singer and it isn't really */
                                                                    /* the end of the table.                */
   /* Too bad. */

   no_resolve:

   k.kind = resolve_none;
   k.distance = 0;    /* To get around warnings from buggy and confused compilers. */
   return k;
}


extern long_boolean warnings_are_unacceptable(long_boolean strict)
{
   int i;
   uint32 w = 0;     /* Will become nonzero if any bad warning other than "warn__bad_concept_level" appears. */
   uint32 ww = 0;    /* Will become nonzero if any bad warning appears. */

   for (i=0 ; i<WARNING_WORDS ; i++) {
      uint32 warn_word = history[history_ptr+1].warnings.bits[i] & no_search_warnings.bits[i];
      ww |= warn_word;
      if (i == (warn__bad_concept_level > 5))
         w |= warn_word & ~(1 << (warn__bad_concept_level & 0x1F));
      else
         w |= warn_word;
   }

   /* But if we are doing a "standardize", we let ALL warnings pass.  We particularly
      want weird T-bone and other unusual sort of things. */

   if (strict && ww) {
      /* But if "allow all concepts" was given, and that's the only bad warning, we let it pass. */
      if (!allowing_all_concepts || w != 0) return TRUE;
   }

   return FALSE;
}


/* The "action" argument tells how hard we work to remove the outside phantoms.
   When merging the results of "on your own" or "own the so-and-so",
   we set action=normalize_before_merge to work very hard at stripping away
   outside phantoms, so that we can see more readily how to put things together.
   When preparing for an isolated call, that is, "so-and-so do your part, whatever",
   we work at it a little, so we set action=normalize_before_isolated_call.
   For normal usage, we set action=simple_normalize. */
extern void normalize_setup(setup *ss, normalize_action action)
{
   int i;
   uint32 livemask, j;
   long_boolean did_something = FALSE;
   const expand_thing *cptr;

 startover:

   for (i=0, j=1, livemask=0; i<=setup_attrs[ss->kind].setup_limits; i++, j<<=1) {
      if (ss->people[i].id1) livemask |= j;
   }

   if (ss->kind == sfat2x8)
      ss->kind = s2x8;     /* That's all it takes! */
   else if (ss->kind == swide4x4)
      ss->kind = s4x4;     /* That's all it takes! */
   else if (ss->kind == s_dead_concentric && action >= normalize_before_isolated_call) {
      ss->kind = ss->inner.skind;
      ss->rotation += ss->inner.srotation;
      did_something = TRUE;
      goto startover;
   }

   if (action == normalize_compress_bigdmd) {
      if (ss->kind == sbigdmd) {
         if ((livemask & 00003) == 00001) swap_people(ss, 0, 1);
         if ((livemask & 00060) == 00040) swap_people(ss, 4, 5);
         if ((livemask & 00300) == 00100) swap_people(ss, 6, 7);
         if ((livemask & 06000) == 04000) swap_people(ss, 10, 11);
         action = simple_normalize;
         goto startover;
      }
      else return;   // Huh?
   }

   // A few difficult cases.

   // **** We really require that the second clause not happen unless the first did!!!
   if (ss->kind == s_323) {
      if (!(ss->people[0].id1 | ss->people[2].id1 |
            ss->people[4].id1 | ss->people[6].id1)) {
         compress_setup(&exp_dmd_323_stuff, ss);

         if (action >= normalize_before_isolated_call) {
            if (ss->kind == sdmd && !(ss->people[0].id1 | ss->people[2].id1)) {
               compress_setup(&exp_1x2_dmd_stuff, ss);
            }
         }

         did_something = TRUE;
         goto startover; 
      }
   }

   // Next, search for simple things in the hash table.

   uint32 hash_num = (ss->kind * 25) & (NUM_EXPAND_HASH_BUCKETS-1);

   for (cptr=compress_hash_table[hash_num] ; cptr ; cptr=cptr->next_compress) {
      if (cptr->outer_kind == ss->kind &&
          action >= cptr->action_level &&
          (cptr->biglivemask & livemask) == 0) {
         warn(cptr->norwarning);
         compress_setup(cptr, ss);
         did_something = TRUE;
         goto startover; 
      }
   }

   // A few difficult cases.

   // **** this map is externally visible
   if (ss->kind == s4x6) {
      if (!(ss->people[0].id1 | ss->people[11].id1 |
            ss->people[18].id1 | ss->people[17].id1 |
            ss->people[5].id1 | ss->people[6].id1 |
            ss->people[23].id1 | ss->people[12].id1)) {
         compress_setup(&exp_4x4_4x6_stuff_a, ss);
         did_something = TRUE;
         goto startover; 
      }
   }

   if (!did_something &&
       ss->kind == s3x4 &&
       !(ss->people[0].id1 | ss->people[3].id1 |
         ss->people[6].id1 | ss->people[9].id1)) {
      compress_setup(&exp_qtg_3x4_stuff, ss);
      did_something = TRUE;
      goto startover; 
   }

   if (action >= normalize_to_4 &&
       action != normalize_after_triple_squash &&
       (ss->kind == s_hrglass || ss->kind == s_dhrglass)) {
      if (action >= normalize_to_2 && (livemask & 0x77) == 0) {
         ss->kind = s_hrglass;   // Be sure we match what the map says --
         // that might get checked someday.
         compress_setup(&exp_1x2_hrgl_stuff, ss);
         did_something = TRUE;
         goto startover; 
      }
      else if ((livemask & 0x33) == 0)  {
         ss->kind = s_hrglass;   // Be sure we match what the map says --
         // that might get checked someday.
         compress_setup(&exp_dmd_hrgl_stuff, ss);
         did_something = TRUE;
         goto startover; 
      }
   }

   canonicalize_rotation(ss);
}



/* Top level move routine. */

SDLIB_API void toplevelmove(void)
{
   int i;

   setup starting_setup = history[history_ptr].state;
   configuration *newhist = &history[history_ptr+1];
   parse_block *conceptptr = newhist->command_root;

   /* Be sure that the amount of written history that we consider to be safely
      written is less than the item we are about to change. */
   if (written_history_items > history_ptr)
      written_history_items = history_ptr;

   starting_setup.cmd.cmd_misc_flags = 0;
   starting_setup.cmd.cmd_misc2_flags = 0;
   starting_setup.cmd.do_couples_her8itflags = 0;
   starting_setup.cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
   starting_setup.cmd.cmd_assume.assumption = cr_none;
   starting_setup.cmd.cmd_assume.assump_cast = 0;
   starting_setup.cmd.prior_elongation_bits = 0;
   starting_setup.cmd.prior_expire_bits = 0;
   starting_setup.cmd.skippable_concept = (parse_block *) 0;
   starting_setup.cmd.restrained_concept = (parse_block *) 0;
   starting_setup.cmd.restrained_super8flags = 0;
   starting_setup.cmd.restrained_fraction = 0;

   for (i=0 ; i<WARNING_WORDS ; i++) newhist->warnings.bits[i] = 0;

   /* If we are starting a sequence with the "so-and-so into the center and do whatever"
      flag on, and this call is a "sequence starter", take special action. */

   if (startinfolist[history[history_ptr].centersp].into_the_middle) {

      /* If the call is a special sequence starter (e.g. spin a pulley) remove the implicit
         "centers" concept and just do it.  The setup in this case will be a squared set
         with so-and-so moved into the middle, which is what the encoding of these calls
         wants.
      If the call is a "split square thru" or "split dixie style" type of call, and the
         "split" concept has been given, possibly preceded by "left" we do the same.
         We leave the "split" concept in place.  Other mechanisms will do the rest. */

      if (the_callback_block.parse_state_ptr->topcallflags1 & CFLAG1_SEQUENCE_STARTER)
         conceptptr = conceptptr->next;
      else if (the_callback_block.parse_state_ptr->topcallflags1 &
               (CFLAG1_SPLIT_LIKE_SQUARE_THRU | CFLAG1_SPLIT_LIKE_DIXIE_STYLE)) {
         uint64 finaljunk;
         parse_block *concept_scan = conceptptr->next;

         finaljunk.her8it = 0;
         finaljunk.final = 0;

         // First, skip the fractional concept.  Only simple "M/N".
         while (concept_scan->concept->kind == concept_fractional &&
                concept_scan->concept->value.arg1 == 0)
            concept_scan = concept_scan->next;

         (void) process_final_concepts(concept_scan, FALSE, &finaljunk);
         if (finaljunk.final & FINAL__SPLIT)
            conceptptr = conceptptr->next;
      }
   }

   /* Clear a few things.  We do NOT clear the warnings, because some (namely the
      "concept not allowed at this level" warning) may have already been logged. */
   newhist->centersp = 0;
   newhist->draw_pic = FALSE;
   newhist->resolve_flag.kind = resolve_none;

   /* Set the selector to "uninitialized", so that, if we do a call like "run", we
      will query the user to find out who is selected. */

   current_options.who = selector_uninitialized;

   /* Put in identification bits for global/unsymmetrical stuff, if possible. */

   for (i=0; i<MAX_PEOPLE; i++) starting_setup.people[i].id2 &= ~GLOB_BITS_TO_CLEAR;

   if (!(starting_setup.result_flags & RESULTFLAG__IMPRECISE_ROT)) {    /* Can't do it if rotation is not known. */
      if (setup_attrs[starting_setup.kind].setup_limits >= 0) {     /* Put in headliner/sideliner stuff if possible. */
         for (i=0; i<=setup_attrs[starting_setup.kind].setup_limits; i++) {
            if (starting_setup.people[i].id1 & BIT_PERSON) {
               if ((starting_setup.people[i].id1 + starting_setup.rotation) & 1)
                  starting_setup.people[i].id2 |= ID2_SIDELINE;
               else if ((starting_setup.people[i].id1 + starting_setup.rotation) & 2)
                  starting_setup.people[i].id2 |= ID2_HEADLINE|ID2_FACEFRONT;
               else
                  starting_setup.people[i].id2 |= ID2_HEADLINE|ID2_FACEBACK;
            }
         }
      }

      if (starting_setup.kind == s2x4) {
         uint32 nearbit = 0;
         uint32 farbit = 0;

         if (starting_setup.rotation & 1) {
            nearbit = ID2_NEARBOX;
            farbit = ID2_FARBOX;
         }
         else {
            uint32 tbonetest = 0;

            for (i=0; i<8; i++) tbonetest |= starting_setup.people[i].id1;

            if (!(tbonetest & 1)) {
               nearbit = ID2_NEARLINE;
               farbit = ID2_FARLINE;
            }
            else if (!(tbonetest & 010)) {
               nearbit = ID2_NEARCOL;
               farbit = ID2_FARCOL;
            }
         }

         for (i=0; i<8; i++) {
            if (starting_setup.people[i].id1 & BIT_PERSON)
               starting_setup.people[i].id2 |= ((i + (starting_setup.rotation << 1)) & 4) ? nearbit : farbit;
         }
      }
      else if (starting_setup.kind == s1x8 && starting_setup.rotation & 1) {
         uint32 nearbit = 0;
         uint32 farbit = 0;
         uint32 tbonetest = 0;

         for (i=0; i<8; i++) tbonetest |= starting_setup.people[i].id1;

         if (!(tbonetest & 1)) {
            nearbit = ID2_NEARLINE;
            farbit = ID2_FARLINE;
         }
         else if (!(tbonetest & 010)) {
            nearbit = ID2_NEARCOL;
            farbit = ID2_FARCOL;
         }

         for (i=0; i<8; i++) {
            if (starting_setup.people[i].id1 & BIT_PERSON)
               starting_setup.people[i].id2 |= ((i + 2 + (starting_setup.rotation << 1)) & 4) ? farbit : nearbit;
         }
      }
      else if (starting_setup.kind == s_c1phan) {
         uint32 nearbit = 0;
         uint32 farbit = 0;
         uint32 tbonetest[2];
         uint32 livemask;
         uint32 j;

         tbonetest[0] = 0;
         tbonetest[1] = 0;

         for (i=0, j=1, livemask = 0; i<16; i++, j<<=1) {
            if (starting_setup.people[i].id1) livemask |= j;
            tbonetest[i>>3] |= starting_setup.people[i].id1;
         }

         if (livemask == 0x5AA5UL) {
            nearbit = ID2_NEARBOX;

            if (!(tbonetest[0] & 1)) {
               farbit = ID2_FARLINE;
            }
            else if (!(tbonetest[0] & 010)) {
               farbit = ID2_FARCOL;
            }
         }
         else if (livemask == 0xA55AUL) {
            farbit = ID2_FARBOX;

            if (!(tbonetest[1] & 1)) {
               nearbit = ID2_NEARLINE;
            }
            else if (!(tbonetest[1] & 010)) {
               nearbit = ID2_NEARCOL;
            }
         }

         for (i=0; i<16; i++) {
            if (starting_setup.people[i].id1 & BIT_PERSON)
               starting_setup.people[i].id2 |= (i & 8) ? nearbit : farbit;
         }
      }
   }

   /* Put in position-identification bits (leads/trailers/beaus/belles/centers/ends etc.) */
   update_id_bits(&starting_setup);
   starting_setup.cmd.parseptr = conceptptr;
   starting_setup.cmd.callspec = NULLCALLSPEC;
   starting_setup.cmd.cmd_final_flags.final = 0;
   starting_setup.cmd.cmd_final_flags.her8it = 0;
   move(&starting_setup, FALSE, &newhist->state);

   if (newhist->state.kind == s1p5x8)
      fail("Can't go into a 50% offset 1x8.");
   else if (newhist->state.kind == s_dead_concentric) {
      newhist->state.kind = newhist->state.inner.skind;
      newhist->state.rotation += newhist->state.inner.srotation;
   }

   /* Once rotation is imprecise, it is always imprecise.  Same for the other flags copied here. */
   newhist->state.result_flags |= starting_setup.result_flags &
         (RESULTFLAG__IMPRECISE_ROT|RESULTFLAG__ACTIVE_PHANTOMS_ON|RESULTFLAG__ACTIVE_PHANTOMS_OFF);
}


/* Do the extra things that a call requires, that are not required when only testing for legality. */

SDLIB_API void finish_toplevelmove(void)
{
   int i;

   configuration *newhist = &history[history_ptr+1];

   /* Remove outboard phantoms from the resulting setup. */

   normalize_setup(&newhist->state, simple_normalize);
   for (i=0; i<MAX_PEOPLE; i++) newhist->state.people[i].id2 &= ~GLOB_BITS_TO_CLEAR;
   newhist->resolve_flag = resolve_p(&newhist->state);
}


// These are local gateways to the outside world.

static void writestuff(Const char *s)
{
   (*the_callback_block.writestuff_fn)(s);
}

static void writechar(char src)
{
   (*the_callback_block.writechar_fn)(src);
}


/* Getting blanks into all the right places in the presence of substitions,
   insertions, and elisions is way too complicated to do in the database or
   to test.  For example, consider calls like "@4keep @5busy@1",
   "fascinat@pe@q@ning@o@t", or "spin the pulley@7 but @8".  So we try to
   help by never putting two blanks together, always putting blanks adjacent
   to the outside of brackets, and never putting blanks adjacent to the
   inside of brackets.  This procedure is part of that mechanism. */
Private void write_blank_if_needed(void)
{
   if (the_callback_block.writechar_block_ptr->lastchar != ' ' && the_callback_block.writechar_block_ptr->lastchar != '[' && the_callback_block.writechar_block_ptr->lastchar != '(' && the_callback_block.writechar_block_ptr->lastchar != '-') writestuff(" ");
}

/* This examines the indicator character after an "@" escape.  If the escape
   is for something that is supposed to appear in the menu as a "<...>"
   wildcard, it returns that string.  If it is an escape that starts a
   substring that would normally be elided, as in "@7 ... @8", it returns
   a non-null pointer to a null character.  If it is an escape that is normally
   simply dropped, it returns a null pointer. */

SDLIB_API Const char *get_escape_string(char c)
{
   switch (c) {
      case '0': case 'm':
         return "<ANYTHING>";
      case 'N':
         return "<ANYCIRC>";
      case '6': case 'k':
         return "<ANYONE>";
      case 'h':
         return "<DIRECTION>";
      case '9':
         return "<N>";
      case 'a': case 'b': case 'B': case 'D':
         return "<N/4>";
      case 'u':
         return "<Nth>";
      case 'v': case 'w': case 'x': case 'y':
         return "<ATC>";
      case '7': case 'n': case 'j': case 'J': case 'E':
         return "";
      default:
         return (char *) 0;
   }
}


Private void write_nice_number(char indicator, int num)
{
   if (num < 0) {
      writestuff(get_escape_string(indicator));
   }
   else {
      switch (indicator) {
      case '9': case 'a': case 'b': case 'B': case 'D':
         if (indicator == '9')
            writestuff(cardinals[num]);
         else if (indicator == 'B' || indicator == 'D') {
            if (num == 1)
               writestuff("quarter");
            else if (num == 2)
               writestuff("half");
            else if (num == 3)
               writestuff("three quarter");
            else if (num == 4)
               writestuff("four quarter");
            else {
               writestuff(cardinals[num]);
               writestuff("/4");
            }
         }
         else if (num == 2)
            writestuff("1/2");
         else if (num == 4 && indicator == 'a')
            writestuff("full");
         else {
            writestuff(cardinals[num]);
            writestuff("/4");
         }
         break;
      case 'u':     /* Need to plug in an ordinal number. */
         writestuff(ordinals[num]);
         break;
      }
   }
}




Private void writestuff_with_decorations(call_conc_option_state *cptr, Cstring f)
{
   uint32 index = cptr->number_fields;
   int howmany = cptr->howmanynumbers;

   while (f[0]) {
      if (f[0] == '@') {
         switch (f[1]) {
         case 'a': case 'b': case 'B': case 'D': case 'u': case '9':
            // DJGPP has a problem with this, need convert to int.
            write_nice_number(f[1], (howmany <= 0) ? -1 : (int) (index & 0xF));
            index >>= 4;
            howmany--;
            break;
         case '6':
            writestuff(selector_list[cptr->who].name_uc);
            break;
         case 'k':
            writestuff(selector_list[cptr->who].sing_name_uc);
            break;
         default:   /**** maybe we should really do what "translate_menu_name"
                          does, using call to "get_escape_string". */
            break;
         }
         
         f += 2;
      }
      else
         writechar(*f++);
   }
}


SDLIB_API void unparse_call_name(Cstring name, char *s, call_conc_option_state *options)
{
   writechar_block_type saved_writeblock = *the_callback_block.writechar_block_ptr;
   the_callback_block.writechar_block_ptr->destcurr = s;
   the_callback_block.writechar_block_ptr->usurping_writechar = TRUE;

   writestuff_with_decorations(options, name);
   writechar('\0');

   *the_callback_block.writechar_block_ptr = saved_writeblock;
   the_callback_block.writechar_block_ptr->usurping_writechar = FALSE;
}


/* There are 2 bits that are meaningful in the argument to "print_recurse":
         PRINT_RECURSE_STAR
   This means to print an asterisk for a call that is missing in the
   current type-in state.
         PRINT_RECURSE_CIRC
   This means that this is a circulate-substitute call, and should have any
   @O ... @P stuff elided from it. */

#define PRINT_RECURSE_STAR 1
#define PRINT_RECURSE_CIRC 2


SDLIB_API void print_recurse(parse_block *thing, int print_recurse_arg)
{
   parse_block *local_cptr;
   parse_block *next_cptr;
   long_boolean use_left_name = FALSE;
   long_boolean use_cross_name = FALSE;
   long_boolean use_magic_name = FALSE;
   long_boolean use_intlk_name = FALSE;
   long_boolean allow_deferred_concept = TRUE;
   parse_block *deferred_concept = (parse_block *) 0;
   int deferred_concept_paren = 0;
   int comma_after_next_concept = 0;    /* 1 for comma, 2 for the word "all". */
   int did_comma = 0;                   /* Same as comma_after_next_concept. */
   long_boolean did_concept = FALSE;
   long_boolean last_was_t_type = FALSE;
   long_boolean last_was_l_type = FALSE;
   long_boolean request_final_space = FALSE;

   local_cptr = thing;

   while (local_cptr) {
      concept_kind k;
      concept_descriptor *item;

      item = local_cptr->concept;
      k = item->kind;

      if (k == concept_comment) {
         comment_block *fubb;

         fubb = (comment_block *) local_cptr->call_to_print;
         if (request_final_space) writestuff(" ");
         writestuff("{ ");
         writestuff(fubb->txt);
         writestuff(" } ");
         local_cptr = local_cptr->next;
         request_final_space = FALSE;
         last_was_t_type = FALSE;
         last_was_l_type = FALSE;
         comma_after_next_concept = 0;
      }
      else if (k > marker_end_of_list) {
         /* This is a concept. */

         long_boolean force = FALSE;
         int request_comma_after_next_concept = 0;           /* Same as comma_after_next_concept. */

         /* Some concepts look better with a comma after them. */

         if (item->concparseflags & CONCPARSE_PARSE_F_TYPE) {
            /* This is an "F" type concept. */
            comma_after_next_concept = 1;
            last_was_t_type = FALSE;
            force = did_concept && !last_was_l_type;
            last_was_l_type = FALSE;
            did_concept = TRUE;
         }
         else if (item->concparseflags & CONCPARSE_PARSE_L_TYPE) {
            /* This is an "L" type concept. */
            last_was_t_type = FALSE;
            last_was_l_type = TRUE;
         }
         else if (item->concparseflags & CONCPARSE_PARSE_G_TYPE) {
            /* This is a "leading T/trailing L" type concept, also known as a "G" concept. */
            force = last_was_t_type && !last_was_l_type;;
            last_was_t_type = FALSE;
            last_was_l_type = TRUE;
         }
         else {
            /* This is a "T" type concept. */
            if (did_concept) comma_after_next_concept = 1;
            force = last_was_t_type && !last_was_l_type;
            last_was_t_type = TRUE;
            last_was_l_type = FALSE;
            did_concept = TRUE;
         }

         if (force && did_comma == 0) writestuff(", ");
         else if (request_final_space) writestuff(" ");

         next_cptr = local_cptr->next;    /* Now it points to the thing after this concept. */

         request_final_space = FALSE;

         if (concept_table[k].concept_prop & CONCPROP__SECOND_CALL) {
            parse_block *subsidiary_ptr = local_cptr->subsidiary_root;

            if (k == concept_centers_and_ends) {
               if (item->value.arg1 == selector_center6 ||
                   item->value.arg1 == selector_center2 ||
                   item->value.arg1 == selector_verycenters)
                  writestuff(selector_list[item->value.arg1].name_uc);
               else
                  writestuff(selector_list[selector_centers].name_uc);

               writestuff(" ");
            }
            else if (k == concept_some_vs_others) {
               selective_key sk = (selective_key) item->value.arg1;

               if (sk == selective_key_dyp)
                  writestuff_with_decorations(&local_cptr->options, "DO YOUR PART, @6 ");
               else if (sk == selective_key_own)
                  writestuff_with_decorations(&local_cptr->options, "OWN THE @6, ");
               else if (sk == selective_key_plain)
                  writestuff_with_decorations(&local_cptr->options, "@6 ");
               else
                  writestuff_with_decorations(&local_cptr->options, "@6 DISCONNECTED ");
            }
            else if (k == concept_sequential) {
               writestuff("(");
            }
            else if (k == concept_replace_nth_part || k == concept_replace_last_part || k == concept_interrupt_at_fraction) {
               writestuff("DELAY: ");
               if (!local_cptr->next || !subsidiary_ptr) {
                  switch (local_cptr->concept->value.arg1) {
                     case 9:
                        writestuff("(interrupting after the ");
                        writestuff(ordinals[local_cptr->options.number_fields]);
                        writestuff(" part) ");
                        break;
                     case 8:
                        writestuff("(replacing the ");
                        writestuff(ordinals[local_cptr->options.number_fields]);
                        writestuff(" part) ");
                        break;
                     case 0:
                        writestuff("(replacing the last part) ");
                        break;
                     case 1:
                        writestuff("(interrupting before the last part) ");
                        break;
                     case 2:
                        writestuff("(interrupting after ");
                        writestuff(cardinals[local_cptr->options.number_fields & 0xF]);
                        writestuff("/");
                        writestuff(cardinals[(local_cptr->options.number_fields >> 4) & 0xF]);
                        writestuff(") ");
                        break;
                  }
               }
            }
            else {
               writestuff(item->name);
               writestuff(" ");
            }

            print_recurse(local_cptr->next, 0);

            if (!subsidiary_ptr) break;         /* Can happen if echoing incomplete input. */

            did_concept = FALSE;                /* We're starting over. */
            last_was_t_type = FALSE;
            last_was_l_type = FALSE;
            comma_after_next_concept = 0;
            request_final_space = TRUE;

            if (k == concept_centers_and_ends) {
               if (item->value.arg2)
                  writestuff(" WHILE THE ENDS CONCENTRIC");
               else
                  writestuff(" WHILE THE ENDS");
            }
            else if (k == concept_some_vs_others && (selective_key) item->value.arg1 != selective_key_own) {

               selector_kind opp = selector_list[local_cptr->options.who].opposite;
               writestuff(" WHILE THE ");
               writestuff((opp == selector_uninitialized) ? ((Cstring) "OTHERS") : selector_list[opp].name_uc);
            }
            else if (k == concept_on_your_own)
               writestuff(" AND");
            else if (k == concept_interlace ||
                     k == concept_sandwich)
               writestuff(" WITH");
            else if (k == concept_replace_nth_part ||
                     k == concept_replace_last_part ||
                     k == concept_interrupt_at_fraction) {
               writestuff(" BUT ");
               writestuff_with_decorations(&local_cptr->options, local_cptr->concept->name);
               writestuff(" WITH A [");
               request_final_space = FALSE;
            }
            else if (k == concept_sequential)
               writestuff(" ;");
            else if (k == concept_special_sequential) {
               if (item->value.arg1 == 2)
                  writestuff(" :");   /* This is "start with". */
               else
                  writestuff(",");
            }
            else
               writestuff(" BY");

            /* Note that we leave "allow_deferred_concept" on.  This means that if we say "twice"
               immediately inside the second clause of an interlace, it will get the special
               processing.  The first clause will get the special processing by virtue of the recursion. */

            next_cptr = subsidiary_ptr;

            /* Setting this means that, if the second argument uses "twice", we will put
               it in parens.  This is needed to disambiguate this situation from the
               use of "twice" before the entire "interlace". */
            if (deferred_concept_paren == 0) deferred_concept_paren = 2;
         }
         else {
            Const callspec_block *target_call = (callspec_block *) 0;
            Const parse_block *tptr;

            /* Look for special concepts that, in conjunction with calls that have certain escape codes
               in them, get deferred and inserted into the call name. */

            if (local_cptr && (k == concept_left || k == concept_cross || k == concept_magic || k == concept_interlocked)) {
   
               /* These concepts want to take special action if there are no following
                  concepts and certain escape characters are found in the name of
                  the following call. */
   
               uint64 junk_concepts;

               junk_concepts.her8it = 0;
               junk_concepts.final = 0;
               
               /* Skip all final concepts, then demand that what remains is a marker
                  (as opposed to a serious concept), and that a real call
                  has been entered, and that its name starts with "@g". */
               tptr = process_final_concepts(next_cptr, FALSE, &junk_concepts);
   
               if (tptr && tptr->concept->kind <= marker_end_of_list) target_call = tptr->call_to_print;
            }

            if (target_call &&
                k == concept_left &&
                (target_call->callflagsf & ESCAPE_WORD__LEFT)) {
               use_left_name = TRUE;
            }
            else if (target_call &&
                     k == concept_magic &&
                     (target_call->callflagsf & ESCAPE_WORD__MAGIC)) {
               use_magic_name = TRUE;
            }
            else if (target_call &&
                     k == concept_interlocked &&
                     (target_call->callflagsf & ESCAPE_WORD__INTLK)) {
               use_intlk_name = TRUE;
            }
            else if (target_call &&
                     k == concept_cross &&
                     (target_call->callflagsf & ESCAPE_WORD__CROSS)) {
               use_cross_name = TRUE;
            }
            else if (allow_deferred_concept &&
                     next_cptr &&
                     (k == concept_twice ||
                      k == concept_n_times ||
                      (k == concept_fractional && item->value.arg1 == 2))) {
               deferred_concept = local_cptr;
               comma_after_next_concept = 0;
               did_concept = FALSE;

               /* If there is another concept, we need parens. */
               if (next_cptr->concept->kind > marker_end_of_list) deferred_concept_paren |= 1;

               if (deferred_concept_paren == 3) writestuff("("/*)*/);
               if (deferred_concept_paren) writestuff("("/*)*/);
            }
            else {
               if ((k == concept_meta_one_arg && item->value.arg1 == meta_key_nth_part_work) ||
                   (k == concept_snag_mystic && item->value.arg1 == CMD_MISC2__SAID_INVERT) ||
                   (k == concept_meta && (item->value.arg1 == meta_key_initially ||
                                          item->value.arg1 == meta_key_finally))) {
                  /* This is "DO THE <Nth> PART",
                     or INVERT followed by another concept, which must be SNAG or MYSTIC,
                     or INITIALLY/FINALLY.
                     These concepts require a comma after the following concept. */
                  request_comma_after_next_concept = 1;
               }
               else if (k == concept_so_and_so_only &&
                        ((selective_key) item->value.arg1) == selective_key_work_concept) {
                  /* "<ANYONE> WORK" */
                  /* This concept requires the word "all" after the following concept. */
                  request_comma_after_next_concept = 2;
               }

               writestuff_with_decorations(&local_cptr->options, local_cptr->concept->name);
               request_final_space = TRUE;
            }

            /* For some concepts, we still permit the "defer" stuff.  But don't do it
               if others are doing the call, because that would lead to
               "<anyone> work 1-1/2, swing thru" turning into
               "<anyone> work swing thru 1-1/2". */

            if ((k != concept_so_and_so_only || item->value.arg2) &&
                k != concept_c1_phantom &&
                k != concept_tandem)
               allow_deferred_concept = FALSE;
         }

         if (comma_after_next_concept == 2) {
            writestuff(", ALL");
            request_final_space = TRUE;
         }
         else if (comma_after_next_concept == 1) {
            writestuff(",");
            request_final_space = TRUE;
         }

         did_comma = comma_after_next_concept;

         if (comma_after_next_concept == 3)
            comma_after_next_concept = 2;
         else
            comma_after_next_concept = request_comma_after_next_concept;

         if (comma_after_next_concept == 2 && next_cptr) {
            //            parse_block *junk2;
            concept_kind kjunk;
            parse_block *junk2;
            uint32 njunk;

            if (check_for_concept_group(next_cptr, FALSE, &kjunk, &njunk, &junk2))
               comma_after_next_concept = 3;    /* Will try again later. */
         }

         local_cptr = next_cptr;

         if (k == concept_sequential) {
            if (request_final_space) writestuff(" ");
            print_recurse(local_cptr, PRINT_RECURSE_STAR);
            writestuff(")");
            return;
         }
         else if (k == concept_replace_nth_part ||
                  k == concept_replace_last_part ||
                  k == concept_interrupt_at_fraction) {
            if (request_final_space) writestuff(" ");
            print_recurse(local_cptr, PRINT_RECURSE_STAR);
            writestuff("]");
            return;
         }
      }
      else {
         /* This is a "marker", so it has a call, perhaps with a selector and/or number.
            The call may be null if we are printing a partially entered line.  Beware. */

         parse_block *sub1_ptr;
         parse_block *sub2_ptr;
         parse_block *search;
         long_boolean pending_subst1, pending_subst2;

         selector_kind i16junk = local_cptr->options.who;
         direction_kind idirjunk = local_cptr->options.where;
         uint32 number_list = local_cptr->options.number_fields;
         callspec_block *localcall = local_cptr->call_to_print;
         parse_block *save_cptr = local_cptr;

         long_boolean subst1_in_use = FALSE;
         long_boolean subst2_in_use = FALSE;

         if (request_final_space) writestuff(" ");

         if (k == concept_another_call_next_mod) {
            search = save_cptr->next;
            while (search) {
               parse_block *subsidiary_ptr = search->subsidiary_root;
               long_boolean this_is_subst1 = FALSE;
               long_boolean this_is_subst2 = FALSE;
               if (subsidiary_ptr) {
                  switch (search->replacement_key) {
                     case 1:
                     case 2:
                        this_is_subst1 = TRUE;
                        break;
                     case 5:
                     case 6:
                        this_is_subst2 = TRUE;
                        break;
                  }

                  if (this_is_subst1) {
                     if (subst1_in_use) writestuff("ERROR!!!");
                     subst1_in_use = TRUE;
                     sub1_ptr = subsidiary_ptr;
                  }

                  if (this_is_subst2) {
                     if (subst2_in_use) writestuff("ERROR!!!");
                     subst2_in_use = TRUE;
                     sub2_ptr = subsidiary_ptr;
                  }
               }
               search = search->next;
            }
         }
   
         pending_subst1 = subst1_in_use;
         pending_subst2 = subst2_in_use;

         /* Now "subst_in_use" is on if there is a replacement call that goes in naturally.  During the
            scan of the name, we will try to fit that replacement into the name of the call as directed
            by atsign-escapes.  If we succeed at this, we will clear "pending_subst".
            In addition to all of this, there may be any number of forcible replacements. */

         /* Call = NIL means we are echoing input and user hasn't entered call yet. */

         if (localcall) {
            char *np = localcall->name;

            //            if (enable_file_writing) localcall->age = global_age;

            while (*np) {
               char c = *np++;

               if (c == '@') {
                  char savec = *np++;

                  switch (savec) {
                  case '6': case 'k':
                     write_blank_if_needed();
                     if (savec == '6')
                        writestuff(selector_list[i16junk].name);
                     else
                        writestuff(selector_list[i16junk].sing_name);
                     if (np[0] && np[0] != ' ' && np[0] != ']')
                        writestuff(" ");
                     break;
                  case 'v': case 'w': case 'x': case 'y':
                     write_blank_if_needed();
                     /* Find the base tag call that this is invoking. */

                     search = save_cptr->next;
                     while (search) {
                        parse_block *subsidiary_ptr = search->subsidiary_root;
                        if (subsidiary_ptr &&
                            subsidiary_ptr->call_to_print &&
                            (subsidiary_ptr->call_to_print->callflags1 & CFLAG1_BASE_TAG_CALL_MASK)) {
                           print_recurse(subsidiary_ptr, 0);
                           goto did_tagger;
                        }
                        search = search->next;
                     }

                     /* We didn't find the tagger.  It must not have been entered into
                           the parse tree.  See if we can get it from the "tagger" field. */

                     if (save_cptr->options.tagger != 0)
                        writestuff(tagger_calls
                                   [save_cptr->options.tagger >> 5]
                                   [(save_cptr->options.tagger & 0x1F)-1]->menu_name);
                     else
                        writestuff("NO TAGGER???");

                  did_tagger:

                     if (np[0] && np[0] != ' ' && np[0] != ']')
                        writestuff(" ");
                     break;
                  case 'N':
                     write_blank_if_needed();

                     /* Find the base circ call that this is invoking. */

                     search = save_cptr->next;
                     while (search) {
                        parse_block *subsidiary_ptr = search->subsidiary_root;
                        if (subsidiary_ptr &&
                            subsidiary_ptr->call_to_print &&
                            (subsidiary_ptr->call_to_print->callflags1 & CFLAG1_BASE_CIRC_CALL)) {
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_CIRC);
                           goto did_circcer;
                        }
                        search = search->next;
                     }

                     /* We didn't find the circcer.  It must not have been entered into the parse tree.
                           See if we can get it from the "circcer" field. */

                     if (save_cptr->options.circcer > 0)
                        writestuff(circcer_calls[(save_cptr->options.circcer)-1]->menu_name);
                     else
                        writestuff("NO CIRCCER???");

                  did_circcer:

                     if (np[0] && np[0] != ' ' && np[0] != ']')
                        writestuff(" ");
                     break;
                  case 'h':                   /* Need to plug in a direction. */
                     write_blank_if_needed();
                     writestuff(direction_names[idirjunk]);
                     if (np[0] && np[0] != ' ' && np[0] != ']')
                        writestuff(" ");
                     break;
                  case '9': case 'a': case 'b': case 'B': case 'D': case 'u':
                     /* Need to plug in a number. */
                     write_blank_if_needed();
                     write_nice_number(savec, number_list & 0xF);
                     number_list >>= 4;    /* Get ready for next number. */
                     break;
                  case 'e':
                     if (use_left_name) {
                        while (*np != '@') np++;
                        if (the_callback_block.writechar_block_ptr->lastchar == ']') writestuff(" ");
                        writestuff("left");
                        np += 2;
                     }
                     break;
                  case 'j':
                     if (!use_cross_name) {
                        while (*np != '@') np++;
                        np += 2;
                     }
                     break;
                  case 'C':
                     if (use_cross_name) {
                        write_blank_if_needed();
                        writestuff("cross");
                     }
                     break;
                  case 'S':                   /* Look for star turn replacement. */
                     if (save_cptr->options.star_turn_option < 0) {
                        writestuff(", don't turn the star");
                     }
                     else if (save_cptr->options.star_turn_option != 0) {
                        writestuff(", turn the star ");
                        write_nice_number('b', save_cptr->options.star_turn_option & 0xF);
                     }
                     break;
                  case 'J':
                     if (!use_magic_name) {
                        while (*np != '@') np++;
                        np += 2;
                     }
                     break;
                  case 'M':
                     if (use_magic_name) {
                        if (the_callback_block.writechar_block_ptr->lastchar != ' ' && the_callback_block.writechar_block_ptr->lastchar != '[') writechar(' ');
                        writestuff("magic");
                     }
                     break;
                  case 'E':
                     if (!use_intlk_name) {
                        while (*np != '@') np++;
                        np += 2;
                     }
                     break;
                  case 'I':
                     if (use_intlk_name) {
                        if (the_callback_block.writechar_block_ptr->lastchar == 'a' && the_callback_block.writechar_block_ptr->lastlastchar == ' ')
                           writestuff("n ");
                        else if (the_callback_block.writechar_block_ptr->lastchar != ' ' && the_callback_block.writechar_block_ptr->lastchar != '[')
                           writechar(' ');
                        writestuff("interlocked");
                     }
                     break;
                  case 'l': case 'L': case 'F': case '8': case 'o':
                     /* Just skip these -- they end stuff that we could have
                        elided but didn't. */
                     break;
                  case 'n': case 'p': case 'r': case 'm': case 't':
                     if (subst2_in_use) {
                        if (savec == 'p' || savec == 'r') {
                           while (*np != '@') np++;
                           np += 2;
                        }
                     }
                     else {
                        if (savec == 'n') {
                           while (*np != '@') np++;
                           np += 2;
                        }
                     }
   
                     if (pending_subst2 && savec != 'p' && savec != 'n') {
                        write_blank_if_needed();
                        writestuff("[");
                        print_recurse(sub2_ptr, PRINT_RECURSE_STAR);
                        writestuff("]");
         
                        pending_subst2 = FALSE;
                     }
         
                     break;
                  case 'O':
                     if (print_recurse_arg & PRINT_RECURSE_CIRC) {
                        while (*np != '@') np++;
                        np += 2;
                     }

                     break;
                  default:
                     if (subst1_in_use) {
                        if (savec == '2' || savec == '4') {
                           while (*np != '@') np++;
                           np += 2;
                        }
                     }
                     else {
                        if (savec == '7') {
                           while (*np != '@') np++;
                           np += 2;
                        }
                     }
         
                     if (pending_subst1 && savec != '4' && savec != '7') {
                        write_blank_if_needed();
                        writestuff("[");
                        print_recurse(sub1_ptr, PRINT_RECURSE_STAR);
                        writestuff("]");
         
                        pending_subst1 = FALSE;
                     }
         
                     break;
                  }
               }
               else {
                  if (the_callback_block.writechar_block_ptr->lastchar == ']' && c != ' ' && c != ']')
                     writestuff(" ");

                  if ((the_callback_block.writechar_block_ptr->lastchar != ' ' && the_callback_block.writechar_block_ptr->lastchar != '[') || c != ' ') writechar(c);
               }
            }

            if (the_callback_block.writechar_block_ptr->lastchar == ']' && *np && *np != ' ' && *np != ']')
               writestuff(" ");
         }
         else if (print_recurse_arg & PRINT_RECURSE_STAR) {
            writestuff("*");
         }

         /* Now if "pending_subst" is still on, we have to do by hand what should have been
            a natural replacement.  In any case, we have to find forcible replacements and
            report them. */
   
         if (k == concept_another_call_next_mod) {
            int first_replace = 0;

            for (search = save_cptr->next ; search ; search = search->next) {
               Const callspec_block *cc;
               parse_block *subsidiary_ptr = search->subsidiary_root;

               /* If we have a subsidiary_ptr, handle the replacement that is indicated.
                  BUT:  if the call shown in the subsidiary_ptr is a base tag call, don't
                  do anything -- such substitutions were already taken care of.
                     BUT:  only check if there is actually a call there. */

               if (!subsidiary_ptr) continue;
               cc = subsidiary_ptr->call_to_print;

               if (!cc ||    /* If no call pointer, it isn't a tag base call. */
                   (!(cc->callflags1 & CFLAG1_BASE_TAG_CALL_MASK) &&
                    (!(cc->callflags1 & CFLAG1_BASE_CIRC_CALL) ||
                     search->call_to_print != base_calls[base_call_circcer]))) {
                  callspec_block *replaced_call = search->call_to_print;

                  /* Need to check for case of replacing one star turn with another. */

                  if ((first_replace == 0) &&
                      (replaced_call->callflags1 & CFLAG1_IS_STAR_CALL) &&
                      ((subsidiary_ptr->concept->kind == marker_end_of_list) ||
                       subsidiary_ptr->concept->kind == concept_another_call_next_mod) &&
                      cc &&
                      ((cc->callflags1 & CFLAG1_IS_STAR_CALL) ||
                       cc->schema == schema_nothing)) {
                     first_replace++;

                     if (cc->schema == schema_nothing)
                        writestuff(", don't turn the star");
                     else {
                        writestuff(", ");
                        print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                     }
                  }
                  else {
                     switch (search->replacement_key) {
                     case 1:
                     case 2:
                     case 3:
                        /* This is a natural replacement.
                           It may already have been taken care of. */
                        if (pending_subst1 || search->replacement_key == 3) {
                           write_blank_if_needed();
                           if (search->replacement_key == 3)
                              writestuff("but [");
                           else
                              writestuff("[modification: ");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                     case 5:
                     case 6:
                        /* This is a secondary replacement.
                           It may already have been taken care of. */
                        if (pending_subst2) {
                           write_blank_if_needed();
                           writestuff("[modification: ");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                     default:
                        /* This is a forced replacement. */
                        write_blank_if_needed();
                        first_replace++;
                        if (first_replace == 1)
                           writestuff("BUT REPLACE ");
                        else
                           writestuff("AND REPLACE ");

                        /* Star turn calls can have funny names like "nobox". */

                        writestuff_with_decorations(
                           &search->options,
                           (replaced_call->callflags1 & CFLAG1_IS_STAR_CALL) ?
                           "turn the star @b" : replaced_call->name);

                        writestuff(" WITH [");
                        print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                        writestuff("]");
                        break;
                     }
                  }
               }
            }
         }

         break;
      }
   }

   if (deferred_concept) {
      if (deferred_concept_paren & 1) writestuff(/*(*/")");
      writestuff(" ");

      if (deferred_concept->concept->kind == concept_twice &&
          deferred_concept->concept->value.arg2 == 3)
         writestuff("3 TIMES");
      else
         writestuff_with_decorations(&deferred_concept->options, deferred_concept->concept->name);
      if (deferred_concept_paren & 2) writestuff(/*(*/")");
   }

   return;
}
