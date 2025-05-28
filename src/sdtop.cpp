/* SD -- square dance caller's helper.

    Copyright (C) 1990-2003  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

/* This defines the following functions:
   compress_setup
   expand_setup
   update_id_bits
   get_directions
   touch_or_rear_back
   do_matrix_expansion
   initialize_sdlib
   check_for_concept_group
   crash_print
   fail
   fail_no_retry
   fail2
   failp
   specialfail
   warn
   verify_restriction
   assoc
   find_calldef
   clear_people
   clear_result
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
   install_scatter
   process_final_concepts
   really_skip_one_concept
   fix_n_results
   configuration::calculate_resolve
   warnings_are_unacceptable
   normalize_setup
   toplevelmove
   finish_toplevelmove
   deposit_call_tree
   do_subcall_query
   find_proper_call_list
and the following external variables:
   gg
   text_line_count
   error_message1
   error_message2
   collision_person1
   collision_person2
   last_magic_diamond
   configuration::history
   configuration::history_ptr
   configuration::whole_sequence_low_lim
   history_allocation
   written_history_items
   written_history_nopic
   higher_acceptable_level
   the_topcallflags
   there_is_a_call
   base_calls
   ui_options
   enable_file_writing
   cardinals
   ordinals
   direction_names
   getout_strings
   writechar_block
   num_command_commands
   command_commands
   command_command_values
   num_startup_commands
   startup_commands
   startup_command_values
   resolve_command_values
   number_of_resolve_commands
   resolve_command_strings
   glob_abridge_mode
   abs_max_calls
   max_base_calls
   tagger_menu_list
   selector_menu_list
   circcer_menu_list
   tagger_calls
   circcer_calls
   number_of_taggers
   number_of_circcers
   parse_state
   current_options
   uims_menu_index
   no_search_warnings
   conc_elong_warnings
   dyp_each_warnings
   useless_phan_clw_warnings
   allowing_all_concepts
   allowing_minigrand
   using_active_phantoms
   last_direction_kind
   interactivity
   database_version
   testing_fidelity
   level_threshholds
   allowing_modifications
   hashed_randoms
   concept_sublist_sizes
   concept_sublists
   good_concept_sublist_sizes
   good_concept_sublists
   null_options
   configuration::null_resolve
   verify_options
   verify_used_number
   verify_used_direction
   verify_used_selector
*/


#include <string.h>

#include "sd.h"



iobase *gg;
int text_line_count = 0;
char error_message1[MAX_ERR_LENGTH];
char error_message2[MAX_ERR_LENGTH];
uint32 collision_person1;
uint32 collision_person2;
parse_block *last_magic_diamond;
configuration *configuration::history = (configuration *) 0; /* Will be allocated in sdmain */
int configuration::history_ptr;
int history_allocation;

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

/* This list tells what level calls will be put in the menu and hence made available.
   In some cases, we make calls available that are higher than the requested level.
   When we use such a call, a warning is printed. */

/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
dance_level higher_acceptable_level[] = {
   l_mainstream,
   l_plus,
   l_a1,
   l_a2,
   l_c1,
   l_c2,
   l_c3a,
   l_c3x,     /* If c3 is given, we allow c3x. */
   l_c3x,
   l_c4a,
   l_c4x,     /* If c4 is given, we allow c4x. */
   l_c4x,
   l_dontshow,
   l_nonexistent_concept};


uint32 the_topcallflags;
bool there_is_a_call;
call_with_name **base_calls;        /* Gets allocated as array of pointers in sdinit. */

ui_option_type ui_options;
int configuration::whole_sequence_low_lim;
bool enable_file_writing;

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

/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
Cstring getout_strings[] = {
   "Mainstream",
   "Plus",
   "A1",
   "A2",
   "C1",
   "C2",
   "C3A",
   "C3",
   "C3X",
   "C4A",
   "C4",
   "C4X",
   "all",
   ""};

writechar_block_type writechar_block;
int num_command_commands;     // Size of the command menu.
Cstring *command_commands;
command_kind *command_command_values;
int num_startup_commands;     // Size of the startup menu.
Cstring *startup_commands;
start_select_kind *startup_command_values;
int number_of_resolve_commands;
Cstring *resolve_command_strings;
resolve_command_kind *resolve_command_values;
abridge_mode_t glob_abridge_mode;
int abs_max_calls;
int max_base_calls;
Cstring *tagger_menu_list[NUM_TAGGER_CLASSES];
Cstring *selector_menu_list;
Cstring *circcer_menu_list;
call_with_name **tagger_calls[NUM_TAGGER_CLASSES];
call_with_name **circcer_calls;
uint32 number_of_taggers[NUM_TAGGER_CLASSES];
uint32 number_of_circcers;
parse_state_type parse_state;
call_conc_option_state current_options;
int uims_menu_index;
warning_info no_search_warnings;
warning_info conc_elong_warnings;
warning_info dyp_each_warnings;
warning_info useless_phan_clw_warnings;
bool allowing_all_concepts = false;
bool allowing_minigrand = false;
bool using_active_phantoms = false;
int last_direction_kind = direction_zagzag;
interactivity_state interactivity = interactivity_normal;
char database_version[81];
bool testing_fidelity = false;

/* This list tells what level calls will be accepted for the "pick level call"
   operation.  When doing a "pick level call, we don't actually require calls
   to be exactly on the indicated level, as long as it's plausibly close. */
/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
dance_level level_threshholds[] = {
   l_mainstream,
   l_plus,
   l_a1,
   l_a1,      /* If a2 is given, an a1 call is OK. */
   l_c1,
   l_c2,
   l_c3a,
   l_c3a,     /* If c3 is given, a c3a call is OK. */
   l_c3a,     /* If c3x is given, a c3a call is OK. */
   l_c3x,     /* If c4a is given, a c3x call is OK. */
   l_c3x,     /* If c4 is given, a c3x call is OK. */
   l_c3x,     /* If c4x is given, a c3x call is OK. */
   l_dontshow,
   l_nonexistent_concept};

int allowing_modifications = 0;
int hashed_randoms;


/* These two direct the generation of random concepts when we are searching.
   We make an attempt to generate somewhat plausible concepts, depending on the
   setup we are in.  If we just generated evenly weighted concepts from the entire
   concept list, we would hardly ever get a legal one. */
int concept_sublist_sizes[call_list_extent];
short int *concept_sublists[call_list_extent];
/* These two are similar, but contain only "really nice" concepts that
   we are willing to use in exhaustive searches.  Concepts in these
   lists are very "expensive", in that each one causes an exhaustive search
   through all calls (with nearly-exhaustive enumeration of numbers/selectors, etc).
   Also, these concepts will appear in suggested resolves very early on.  So
   we don't want anything the least bit ugly in these lists. */
int good_concept_sublist_sizes[call_list_extent];
short int *good_concept_sublists[call_list_extent];
const call_conc_option_state null_options = {
   selector_uninitialized,
   direction_uninitialized,
   0, 0, 0, 0, 0};
call_conc_option_state verify_options;
bool verify_used_number;
bool verify_used_direction;
bool verify_used_selector;



void expand::compress_setup(const expand::thing *thing, setup *stuff) THROW_DECL
{
   setup temp = *stuff;

   stuff->kind = thing->inner_kind;
   clear_people(stuff);
   gather(stuff, &temp, thing->source_indices, thing->size-1, thing->rot * 011);
   stuff->rotation -= thing->rot;
   canonicalize_rotation(stuff);
}


void expand::expand_setup(const expand::thing *thing, setup *stuff) THROW_DECL
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
   const id_bit_table *ptr;
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

   static unsigned short int face_spindle[] = {
      0, d_south, 6, d_north,
      1, d_south, 5, d_north,
      2, d_south, 4, d_north,
      0, d_east, 1, d_west,
      1, d_east, 2, d_west,
      6, d_east, 5, d_west,
      5, d_east, 7, d_west,
      ~0};

   static unsigned short int face_1x4[] = {
      2, d_west, 3, d_east,
      3, d_west, 1, d_east,
      1, d_west, 0, d_east,
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

   static unsigned short int face_2x6[] = {
      5, d_west, 4, d_east,
      4, d_west, 3, d_east,
      3, d_west, 2, d_east,
      2, d_west, 1, d_east,
      1, d_west, 0, d_east,
      6, d_west, 7, d_east,
      7, d_west, 8, d_east,
      8, d_west, 9, d_east,
      9, d_west, 10, d_east,
      10, d_west, 11, d_east,
      0, d_south, 11, d_north,
      1, d_south, 10, d_north,
      2, d_south, 9, d_north,
      3, d_south, 8, d_north,
      4, d_south, 7, d_north,
      5, d_south, 6, d_north,
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

   static unsigned short int face_4x4[] = {
      13, d_west, 12,  d_east,
      14, d_west, 13,  d_east,
      0,  d_west, 14,  d_east,
      15, d_west, 10,  d_east,
      3,  d_west, 15,  d_east,
      1,  d_west,  3,  d_east,
      11, d_west,  9,  d_east,
      7,  d_west, 11,  d_east,
      2,  d_west,  7,  d_east,
      6,  d_west,  8,  d_east,
      5,  d_west,  6,  d_east,
      4,  d_west,  5,  d_east,
      12, d_south, 10, d_north,
      13, d_south, 15, d_north,
      14, d_south,  3, d_north,
      0,  d_south,  1, d_north,
      10, d_south,  9, d_north,
      15, d_south, 11, d_north,
      3,  d_south,  7, d_north,
      1,  d_south,  2, d_north,
      9,  d_south,  8, d_north,
      11, d_south,  6, d_north,
      7,  d_south,  5, d_north,
      2,  d_south,  4, d_north,
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

   for (i=0,j=1,livemask=0 ; i<=attr::slimit(ss) ; i++,j<<=1) {
      if (ss->people[i].id1) livemask |= j;
      ss->people[i].id2 &= ~BITS_TO_CLEAR;
   }

   ptr = setup_attrs[ss->kind].id_bit_table_ptr;

   switch (ss->kind) {
   case s_qtag:
      face_list = face_qtg; break;
   case s_spindle:
      face_list = face_spindle; break;
   case s1x4:
      face_list = face_1x4; break;
   case s2x4:
      face_list = face_2x4; break;
   case s1x8:
      face_list = face_1x8; break;
   case s2x2:
      face_list = face_2x2; break;
   case s2x3:
      face_list = face_2x3; break;
   case s4x4:
      face_list = face_4x4; break;
   case s4x5:
      face_list = face_4x5; break;
   case s2x6:
      face_list = face_2x6; break;
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

      for (i=0 ; i<=attr::slimit(ss) ; i++) {
         if (ss->people[i].id1 && !(ss->people[i].id2 & ID2_FACING))
            ss->people[i].id2 |= ID2_NOTFACING;
      }
   }


   /* Some setups are only recognized for ID bits with certain patterns of population.
       The bit tables make those assumptions, so we have to use the bit tables
       only if those assumptions are satisfied. */

   switch (ss->kind) {
   case s2x5:
      // We recognize "centers" or "center 4" if they are a Z within the center 6.
      if (livemask == 0x3BDUL || livemask == 0x2F7UL)
         ptr = id_bit_table_2x5_z;
      break;
   case sd2x5:
      // We recognize "centers" or "center 4" if they are a Z within the center 6.
      if (livemask != 0x37BUL && livemask != 0x1EFUL) ptr = (id_bit_table *) 0;
      break;
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
      // If this is populated appropriately, we can identify "outer pairs".
      if (livemask == 07474UL || livemask == 0x3CFUL) ptr = id_bit_table_bigdmd_wings;
      break;
   case sbigbone:
      // If this is populated appropriately, we can identify "outer pairs".
      if (livemask == 07474UL || livemask == 0x3CFUL) ptr = id_bit_table_bigbone_wings;
      break;
   case sbigdhrgl:
      // If this is populated appropriately, we can identify "outer pairs".
      if (livemask == 07474UL || livemask == 0x3CFUL) ptr = id_bit_table_bigdhrgl_wings;
      break;
   case sbighrgl:
      // If this is populated appropriately, we can identify "outer pairs".
      if (livemask == 07474UL || livemask == 0x3CFUL) ptr = id_bit_table_bighrgl_wings;
      break;
   case sbigh:
      // If it's a "double bent tidal line", we can recognize the "center 4"
      // and "outer pairs".
      if (livemask != 06363UL || livemask != 07474UL) ptr = id_bit_table_bigh_dblbent;
      // Otherwise, we recognize only the center 1x4, and only if it is full.
      else if ((livemask & 06060UL) != 06060UL) ptr = (id_bit_table *) 0;
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

      // There are three special things we can recognize from here.
      // If the setup is populated as an "H", we use a special table
      // (*NOT* the usual one picked up from the setup_attrs list)
      // that knows about the center 2 and the outer 6 and all that.
      // If the center 2x3 is occupied, we use a table for that.
      // If the setup is populated as offset lines/columns/whatever,
      // we use the table from the setup_attrs list, that knows about
      // the "outer pairs".
      // If the corners are occupied but the ends of the center line
      // are not, use a table that recognizes those corners as "ends".
      // If all else fails, we use the default table. */

      if (livemask == 07171UL) ptr = id_bit_table_3x4_h;
      else if ((livemask & 04646UL) == 04646UL) ptr = id_bit_table_3x4_ctr6;
      else if (livemask == 07474UL || livemask == 06363UL) ptr = id_bit_table_3x4_offset;
      else if ((livemask & 03131UL) == 01111UL) ptr = id_bit_table_3x4_corners;
      break;
   case s_d3x4:
      if ((livemask & 01616UL) != 01616UL) ptr = (id_bit_table *) 0;
      break;
   case s4x4:
      /* We recognize centers and ends if this is populated as a butterfly. */
      /* Otherwise, we recognize "center 4" only if those 4 spots are occupied. */
      if (livemask == 0x9999UL)
         ptr = id_bit_table_butterfly;
      else if (livemask == 0xC9C9UL || livemask == 0x9C9CUL ||
               livemask == 0xB8B8UL || livemask == 0x8B8BUL)
         ptr = id_bit_table_4x4_outer_pairs;
      else if ((livemask & 0x8888UL) != 0x8888UL)
         ptr = (id_bit_table *) 0;
      break;
   case sdeepxwv:
      // We recognize outer pairs if they are fully populated.
      if ((livemask & 00303UL) != 00303UL) ptr = (id_bit_table *) 0;
      break;
   case swqtag:
      // We recognize "center 6" if the center 2 spots are empty,
      // so that there is a "short 6".
      // Otherwise, we recognize the center 1x6 if it is fully populated.
      if (livemask == 0x1EF)
         ptr = id_bit_table_wqtag_hollow;
      else if ((livemask & 0x39CUL) != 0x39CUL)
         ptr = (id_bit_table *) 0;
      break;
   case s3dmd:
      // The standard table requires all points, and centers of center diamond only, occupied.
      // But first we look for a few other configurations.

      // Look for center 1x6 occupied.
      if ((livemask & 0xE38UL) == 0xE38UL) ptr = id_bit_table_3dmd_ctr1x6;
      // Look for center 1x6 having center 1x4 occupied.
      else if ((livemask & 0xC30UL) == 0xC30UL) ptr = id_bit_table_3dmd_ctr1x4;
      // Look for center 1x4 and outer points.
      else if (livemask == 06565UL) ptr = id_bit_table_3dmd_in_out;
      // Otherwise, see whether to accept default or reject everything.
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
   case shsqtag:
      // Only allow it if outer pairs are together.
      if (livemask != 0xF3CUL && livemask != 0xCF3UL) ptr = (id_bit_table *) 0;
      break;
   }

   if (!ptr) return;

   for (i=0; i<=attr::slimit(ss); i++) {
      if (ss->people[i].id1 & BIT_PERSON)
         ss->people[i].id2 |= ptr[i][ss->people[i].id1 & 3];
   }
}



full_expand::thing *full_expand::touch_hash_table1[full_expand::NUM_TOUCH_HASH_BUCKETS];
full_expand::thing *full_expand::touch_hash_table2[full_expand::NUM_TOUCH_HASH_BUCKETS];
full_expand::thing *full_expand::touch_hash_table3[full_expand::NUM_TOUCH_HASH_BUCKETS];

expand::thing *expand::expand_hash_table[expand::NUM_EXPAND_HASH_BUCKETS];
expand::thing *expand::compress_hash_table[expand::NUM_EXPAND_HASH_BUCKETS];

void expand::initialize()
{
   thing *tabp;
   int i;

   for (i=0 ; i<NUM_EXPAND_HASH_BUCKETS ; i++) {
      expand_hash_table[i] = (thing *) 0;
      compress_hash_table[i] = (thing *) 0;
   }

   for (tabp = init_table ; tabp->inner_kind != nothing ; tabp++) {
      uint32 hash_num = (tabp->inner_kind * 25) & (NUM_EXPAND_HASH_BUCKETS-1);
      tabp->next_expand = expand_hash_table[hash_num];
      expand_hash_table[hash_num] = tabp;

      hash_num = (tabp->outer_kind * 25) & (NUM_EXPAND_HASH_BUCKETS-1);
      tabp->next_compress = compress_hash_table[hash_num];
      compress_hash_table[hash_num] = tabp;
   }
}

void full_expand::initialize_touch_tables()
{
   thing *tabp;
   int i;

   for (i=0 ; i<NUM_TOUCH_HASH_BUCKETS ; i++) {
      touch_hash_table1[i] = (thing *) 0;
      touch_hash_table2[i] = (thing *) 0;
      touch_hash_table3[i] = (thing *) 0;
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
}


full_expand::thing *full_expand::search_table_1(setup_kind kind,
                                                uint32 livemask,
                                                uint32 directions)
{
   uint32 hash_num = ((kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

   for (thing *tptr = touch_hash_table1[hash_num] ; tptr ; tptr = tptr->next) {
      if (tptr->kind == kind &&
          tptr->live == livemask &&
          ((tptr->dir ^ directions) & tptr->dirmask) == 0) return tptr;
   }

   return (thing *) 0;
}


full_expand::thing *full_expand::search_table_2(setup_kind kind,
                                                uint32 livemask,
                                                uint32 directions)
{
   uint32 hash_num = ((kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

   for (thing *tptr = touch_hash_table2[hash_num] ; tptr ; tptr = tptr->next) {
      if (tptr->kind == kind &&
          tptr->live == livemask &&
          ((tptr->dir ^ directions) & tptr->dirmask) == 0) return tptr;
   }

   return (thing *) 0;
}


full_expand::thing *full_expand::search_table_3(setup_kind kind,
                                                uint32 livemask,
                                                uint32 directions,
                                                uint32 touchflags)
{
   uint32 hash_num = ((kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

   for (thing *tptr = touch_hash_table3[hash_num] ; tptr ; tptr = tptr->next) {
      // If it has the evil "32" bit on, don't allow it except for special
      // fan-the-top calls, indicated by CFLAG1_STEP_TO_WAVE_4_PEOPLE.
      if (tptr->kind == kind &&
          tptr->live == livemask &&
          ((tptr->dir ^ directions) & tptr->dirmask) == 0 &&
          (!(tptr->forbidden_elongation & 32) || touchflags == CFLAG1_STEP_TO_WAVE_4_PEOPLE))
         return tptr;
   }

   return (thing *) 0;
}


extern void get_directions(
   setup *ss,
   uint32 & directions,
   uint32 & livemask)
{
   directions = 0;
   livemask = 0;

   for (int i=0; i<=attr::slimit(ss); i++) {
      uint32 p = ss->people[i].id1;
      directions = (directions<<2) | (p&3);
      livemask <<= 2;
      if (p) livemask |= 3;
   }
}


extern void touch_or_rear_back(
   setup *scopy,
   bool did_mirror,
   int callflags1) THROW_DECL
{
   uint32 directions, livemask;
   const full_expand::thing *tptr;
   const expand::thing *zptr;

   // We don't understand absurd setups.
   if (attr::slimit(scopy) < 0) return;

   // We don't do this if doing the last half of a call.
   if (scopy->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_LASTHALF)) return;

   if (!(callflags1 & (CFLAG1_STEP_REAR_MASK | CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)))
      return;

   remove_z_distortion(scopy);

   get_directions(scopy, directions, livemask);

   uint32 touchflags = (callflags1 & CFLAG1_STEP_REAR_MASK);
   call_restriction new_assume = cr_none;

   switch (touchflags) {
   case CFLAG1_REAR_BACK_FROM_QTAG:
   case CFLAG1_REAR_BACK_FROM_R_WAVE:
   case CFLAG1_REAR_BACK_FROM_EITHER:
      if (touchflags != CFLAG1_REAR_BACK_FROM_QTAG &&
          !scopy->cmd.cmd_final_flags.test_heritbits(INHERITFLAG_MXNMASK)) {
         // Check for rearing back from a wave.
         tptr = full_expand::search_table_1(scopy->kind, livemask, directions);
         if (tptr) goto found_tptr;

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

      if (touchflags != CFLAG1_REAR_BACK_FROM_R_WAVE) {
         // Check for rearing back from a 1/4 tag.
         tptr = full_expand::search_table_2(scopy->kind, livemask, directions);
         if (tptr) goto found_tptr;
      }

      break;

   case CFLAG1_STEP_TO_WAVE:
   case CFLAG1_STEP_TO_NONPHAN_BOX:
   case CFLAG1_STEP_TO_WAVE_4_PEOPLE:

      // Special stuff:  If lines facing, but people are incomplete,
      // we honor an "assume facing lines" command.
      // Or if columns, we honor "assume dpt".

      if (scopy->cmd.cmd_assume.assump_col == 0 &&
          scopy->cmd.cmd_assume.assump_both == 1 &&
          scopy->cmd.cmd_assume.assumption == cr_li_lo) {
         if (scopy->kind == s2x4 && directions == (livemask & 0xAA00)) {
            new_assume = cr_wave_only;  // Turn into "assume right-handed waves" --
            livemask = 0xFFFF;          // assump_col and assump_both are still OK.
            directions = 0xAA00;
         }
         else if (scopy->kind == s2x3 && directions == (livemask & 0xA80)) {
            livemask = 0xFFF;
            directions = 0xA80;
         }
      }
      else if (scopy->cmd.cmd_assume.assump_col == 1 &&
          scopy->cmd.cmd_assume.assump_both == 1 &&
          scopy->cmd.cmd_assume.assumption == cr_2fl_only) {
         if (scopy->kind == s2x4 && directions == (livemask & 0x5FF5)) {
            livemask = 0xFFFF;
            directions = 0x5FF5;
         }
      }

      tptr = full_expand::search_table_3(scopy->kind, livemask, directions, touchflags);
      if (tptr) goto found_tptr;

      // A few setups are special -- we allow any combination at all in livemask,
      // though we are careful.

      bool step_ok =
         touchflags == CFLAG1_STEP_TO_WAVE ||
         touchflags == CFLAG1_STEP_TO_WAVE_4_PEOPLE;

      switch (scopy->kind) {
      case s2x4:
         if (livemask != 0) {
            if ((step_ok || livemask == 0xFFFFUL) &&
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
             (step_ok || livemask == 0xFF)) {
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
             ((step_ok || livemask == 0xFFFFUL) &&
              ((directions ^ 0xA802UL) & livemask) == 0)) {
            tptr = &step_spindle_pair;
            goto found_tptr;
         }
         break;
      case s_trngl:
         if ((0x0FUL & ~livemask) == 0 &&
             ((directions ^ 0x17UL) & livemask) == 0 &&
             !(callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)) {
            tptr = &step_tgl_pair;
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
      case s_ptpd:
         if ((0x3333UL & ~livemask) == 0 &&
             ((directions ^ 0xA00AUL) & livemask) == 0 &&
             !(callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)) {
            tptr = &step_ptpd_pair;
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

      break;
   }

   // We didn't find anything.  But we still need to raise an error
   // if the caller said "left spin the top" when we were in a right-hand wave.

   if ((callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) && did_mirror) {
      uint32 aa;
      uint32 bb = ~0UL;
      bool other_test = true;

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

   // Make sure we alert the user (Hi, Clark!) if we call "Fan the Top" where only
   // the centers would touch.  Case is a starting DPT with ends 1/4 left.
   // People are supposed to step to right hands, but the centers on a Fan the Top
   // normally step to left hands.  What are the dancers supposed to do?
   if ((tptr->forbidden_elongation & 64) && touchflags == CFLAG1_STEP_TO_WAVE_4_PEOPLE)
      warn(warn__some_touch_evil);
   else
      warn(tptr->warning);  // Or give whatever warning the table says, but not both.

   if ((tptr->forbidden_elongation & 4) && (scopy->cmd.cmd_misc_flags & CMD_MISC__DOING_ENDS))
      scopy->cmd.prior_elongation_bits =
         (scopy->cmd.prior_elongation_bits & (~3)) | ((scopy->rotation+1) & 3);

   if ((scopy->cmd.prior_elongation_bits & tptr->forbidden_elongation & 3) &&
       (!(scopy->cmd.cmd_misc_flags & CMD_MISC__NO_CHK_ELONG)))
      fail("People are too far away to work with each other on this call.");

   zptr = tptr->expand_lists;
   scopy->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   setup stemp = *scopy;
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

   // Assumptions are no longer valid, except for a few special cases.
   scopy->cmd.cmd_assume.assumption = new_assume;
   canonicalize_rotation(scopy);
}



bool expand::expand_from_hash_table(setup *ss,
                                    uint32 needpropbits,
                                    uint32 livemask) THROW_DECL
{

   uint32 hash_num = (ss->kind * 25) & (NUM_EXPAND_HASH_BUCKETS-1);
   const thing *eptr;

   for (eptr=expand_hash_table[hash_num] ; eptr ; eptr=eptr->next_expand) {
      if (eptr->inner_kind == ss->kind &&
          (eptr->expandconcpropmask & needpropbits) &&
          (livemask & eptr->lillivemask) == 0) {
         warn(eptr->expwarning);
         expand_setup(eptr, ss);
         return true;
      }
   }

   return false;
}





extern void do_matrix_expansion(
   setup *ss,
   uint32 concprops,
   bool recompute_id) THROW_DECL
{
   uint32 needprops = concprops & CONCPROP__NEED_MASK;
   if (needprops == 0) return;
   uint32 needpropbits = NEEDMASK(needprops);

   for (;;) {
      int i;
      uint32 livemask = little_endian_live_mask(ss);

      // Search for simple things in the hash table.

      if (expand::expand_from_hash_table(ss,
                                         needpropbits,
                                         livemask))
         goto expanded;

      if (ss->kind == s4x4) {
         if (needpropbits &
             (NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) | NEEDMASK(CONCPROP__NEEDK_4DMD))) {
            if (livemask == 0x1717UL) {
               expand::expand_setup(&s_4x4_4dma, ss);
               goto expanded;
            }
            else if (livemask == 0x7171UL) {
               expand::expand_setup(&s_4x4_4dmb, ss);
               goto expanded;
            }
         }
         else if (needpropbits &
                  (NEEDMASK(CONCPROP__NEEDK_TWINDMD) | NEEDMASK(CONCPROP__NEEDK_TWINQTAG))) {
            uint32 ctrs = ss->people[3].id1 | ss->people[7].id1 |
               ss->people[11].id1 | ss->people[15].id1;

            if (ctrs != 0 && (ctrs & 011) != 011) {
               if (needprops == CONCPROP__NEEDK_TWINQTAG) ctrs ^= 1;
               expand::expand_setup((ctrs & 1) ? &s_4x4_4x6b : &s_4x4_4x6a, ss);
               goto expanded;
            }
         }
      }

      /* If get here, we did NOT see any concept that requires a setup expansion. */

      return;

      expanded:

      /* Most global selectors are disabled if we expanded the matrix. */

      for (i=0; i<MAX_PEOPLE; i++)
         ss->people[i].id2 &= (~GLOB_BITS_TO_CLEAR | (ID2_FACEFRONT|ID2_FACEBACK|ID2_HEADLINE|ID2_SIDELINE));

      /* Put in position-identification bits (leads/trailers/beaus/belles/centers/ends etc.) */
      if (recompute_id) update_id_bits(ss);
   }
}


class restriction_tester {

private:

   enum chk_type {
      chk_none,
      chk_wave,
      chk_groups,
      chk_anti_groups,
      chk_box,
      chk_box_dbl,
      chk_indep_box,
      chk_star,
      chk_dmd_qtag,
      chk_dmd_qtag_new,
      chk_qtag,
      chk_qbox,
      chk_peelable,
      chk_spec_directions,
      chk_sex,
      chk_inroller
   };

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make the initialization arrays be statically initialized.

   struct restr_initializer {
      setup_kind k;
      call_restriction restr;
      int size;
      veryshort map1[17];
      veryshort map2[17];
      veryshort map3[8];
      veryshort map4[8];
      bool ok_for_assume;
      chk_type check;
      restr_initializer *next;
   };

   // Must be a power of 2.
   enum { NUM_RESTR_HASH_BUCKETS = 32 };

   static restr_initializer restr_init_table0[];
   static restr_initializer restr_init_table1[];
   static restr_initializer restr_init_table4[];
   static restr_initializer restr_init_table9[];

   static restr_initializer *restr_hash_table0[NUM_RESTR_HASH_BUCKETS];
   static restr_initializer *restr_hash_table1[NUM_RESTR_HASH_BUCKETS];
   static restr_initializer *restr_hash_table4[NUM_RESTR_HASH_BUCKETS];
   static restr_initializer *restr_hash_table9[NUM_RESTR_HASH_BUCKETS];

public:

   static restr_initializer *get_restriction_thing(setup_kind k, assumption_thing t);
   static void initialize_tables();

   friend restriction_test_result verify_restriction(
      setup *ss,
      assumption_thing tt,
      bool instantiate_phantoms,
      bool *failed_to_instantiate) THROW_DECL;

};

restriction_tester::restr_initializer
   *restriction_tester::restr_hash_table0[restriction_tester::NUM_RESTR_HASH_BUCKETS];
restriction_tester::restr_initializer
   *restriction_tester::restr_hash_table1[restriction_tester::NUM_RESTR_HASH_BUCKETS];
restriction_tester::restr_initializer
   *restriction_tester::restr_hash_table4[restriction_tester::NUM_RESTR_HASH_BUCKETS];
restriction_tester::restr_initializer
   *restriction_tester::restr_hash_table9[restriction_tester::NUM_RESTR_HASH_BUCKETS];


restriction_tester::restr_initializer restriction_tester::restr_init_table0[] = {
   {s2x4, cr_inroller_is_cw,   99,    {3, 7, 0, 4, 0, 4, 3, 7},
    {012, 010, 012, 010, 010, 012, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s2x4, cr_inroller_is_ccw,   99,    {4, 0, 7, 3, 7, 3, 4, 0},
    {010, 012, 010, 012, 012, 010, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s2x4, cr_outroller_is_cw,   99,    {3, 7, 0, 4, 0, 4, 3, 7},
    {010, 012, 010, 012, 012, 010, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s2x4, cr_outroller_is_ccw,   99,    {4, 0, 7, 3, 7, 3, 4, 0},
    {012, 010, 012, 010, 010, 012, 0, 0},
    {0}, {0}, false,  chk_inroller},

   {s_qtag, cr_inroller_is_cw,   99,    {4, 0, 1, 5, 1, 5, 4, 0},
    {003, 001, 003, 001, 001, 003, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s_qtag, cr_inroller_is_ccw,   99,    {5, 1, 0, 4, 0, 4, 5, 1},
    {001, 003, 001, 003, 003, 001, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s_qtag, cr_outroller_is_cw,   99,    {4, 0, 1, 5, 1, 5, 4, 0},
    {001, 003, 001, 003, 003, 001, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s_qtag, cr_outroller_is_ccw,   99,    {5, 1, 0, 4, 0, 4, 5, 1},
    {003, 001, 003, 001, 001, 003, 0, 0},
    {0}, {0}, false,  chk_inroller},

   {s2x4, cr_judge_is_cw,   0,    {7, 3, 0, 4, 0, 4, 3, 7},
    {012, 010, 012, 010, 010, 012, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s2x4, cr_judge_is_ccw,   0,    {0, 4, 7, 3, 7, 3, 0, 4},
    {012, 010, 012, 010, 010, 012, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s2x4, cr_socker_is_cw,   0,    {3, 7, 4, 0, 4, 0, 3, 7},
    {012, 010, 012, 010, 010, 012, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s2x4, cr_socker_is_ccw,   0,    {4, 0, 3, 7, 3, 7, 4, 0},
    {012, 010, 012, 010, 010, 012, 0, 0},
    {0}, {0}, false,  chk_inroller},

   {s_qtag, cr_judge_is_cw,   0,    {0, 4, 1, 5, 1, 5, 0, 4},
    {003, 001, 003, 001, 001, 003, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s_qtag, cr_judge_is_ccw,   0,    {1, 5, 0, 4, 0, 4, 1, 5},
    {003, 001, 003, 001, 001, 003, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s_qtag, cr_socker_is_cw,   0,    {4, 0, 5, 1, 5, 1, 0, 4},
    {003, 001, 003, 001, 001, 003, 0, 0},
    {0}, {0}, false,  chk_inroller},
   {s_qtag, cr_socker_is_ccw,   0,    {5, 1, 4, 0, 4, 0, 1, 5},
    {003, 001, 003, 001, 001, 003, 0, 0},
    {0}, {0}, false,  chk_inroller},

   {s1x2, cr_opposite_sex, 2, {0}, {0}, {0}, {1}, false, chk_sex},
   {s4x4,      cr_wave_only,    1,
    {0, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 0, 2, 0, 2, 0},
    {2, 0, 2, 0, 0, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 0}, {0}, {0}, true,  chk_box},
   {s4x4,      cr_2fl_only,     1,
    {0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 0, 2},
    {2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2}, {0}, {0}, true,  chk_box},
   {s_qtag,    cr_qtag_mwv,     8, {0, 1, 3, 2, 5, 4, 6, 7, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s_qtag,    cr_qtag_mag_mwv, 8, {0, 1, 2, 3, 5, 4, 7, 6, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s_spindle, cr_dmd_ctrs_mwv, 6, {0, 6, 1, 5, 2, 4, -1},
    {1, 3, 0}, {0}, {0}, false, chk_spec_directions},
   {s_short6,  cr_dmd_ctrs_mwv, 4, {0, 2, 5, 3, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {sdmd,      cr_dmd_ctrs_mwv, 2, {1, 3, -1},
    {1, 3, 0}, {0}, {0}, false, chk_spec_directions},
   {s_star,    cr_dmd_ctrs_mwv, 2, {1, 1}, {1, 3},
    {1, 0}, {1, 2},              false, chk_star},
   {s_2stars,  cr_dmd_ctrs_mwv, 2, {2, 0, 1}, {2, 4, 5},
    {2, 3, 6}, {2, 2, 7},         false, chk_star},
   {s_trngl,   cr_dmd_ctrs_mwv, 2, {1, 2, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s_trngl4,  cr_dmd_ctrs_mwv, 6, {2, 3, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s_bone6,   cr_dmd_ctrs_mwv, 4, {0, 4, 1, 3, -1},
    {1, 3, 0}, {0}, {0}, false, chk_spec_directions},
   {s1x4,      cr_dmd_ctrs_mwv, 2, {1, 3, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s2x4,      cr_dmd_ctrs_mwv, 4, {1, 2, 6, 5, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s2x3,      cr_dmd_ctrs_mwv, 2, {1, 4, -1},
    {1, 3, 0}, {0}, {0}, false, chk_spec_directions},
   {s_qtag,    cr_dmd_ctrs_mwv, 4, {3, 2, 6, 7, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s3dmd,     cr_dmd_ctrs_mwv, 6, {4, 3, 11, 5, 9, 10, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s4dmd,     cr_dmd_ctrs_mwv, 8, {5, 4, 7, 6, 14, 15, 12, 13, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s4x6,      cr_dmd_ctrs_mwv, 8, {1, 10, 4, 7, 22, 13, 19, 16, -1},
    {1, 3, 0}, {0}, {0}, false, chk_spec_directions},
   {swqtag,    cr_dmd_ctrs_mwv, 6, {3, 2, 9, 4, 7, 8, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {sdeep2x1dmd, cr_dmd_ctrs_mwv, 8, {0, 1, 3, 4, 6, 5, 9, 8, -1},
    {0, 2, 0}, {0}, {0}, false, chk_spec_directions},
   {s_ptpd,    cr_dmd_ctrs_mwv, 4, {1, 3, 7, 5, -1},
    {1, 3, 0}, {0}, {0}, false, chk_spec_directions},
   {sdmd,      cr_dmd_ctrs_1f, 2, {1, 3, -1},
    {1, 3, 1}, {0}, {0}, false, chk_spec_directions},
   {s1x4,      cr_dmd_ctrs_1f, 2, {1, 3, -1},
    {0, 2, 1}, {0}, {0}, false, chk_spec_directions},

   {s1x8, cr_wave_only, 8, {0, 1, 3, 2, 6, 7, 5, 4},
    {0}, {0}, {0}, true, chk_wave},
   {s1x8, cr_1fl_only, 4, {0, 4, 1, 5, 2, 6, 3, 7},
    {2}, {0}, {0}, true,  chk_groups},
   {s1x8, cr_all_facing_same, 8, {0, 1, 2, 3, 4, 5, 6, 7},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x8, cr_2fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},
    {0}, {0}, {0}, false, chk_wave},
   {s1x8, cr_2fl_per_1x4, 2, {0, 1, 4, 5, 2, 3, 6, 7},
    {2}, {0}, {0}, false,  chk_anti_groups},
   {s2x4, cr_4x4couples_only, 4, {0, 4, 1, 5, 2, 6, 3, 7},
    {2}, {0}, {0}, true,  chk_groups},
   {s1x8, cr_4x4couples_only, 4, {0, 4, 1, 5, 2, 6, 3, 7},
    {2}, {0}, {0}, true,  chk_groups},
   {s1x8, cr_4x4_2fl_only, 8, {0, 4, 1, 5, 2, 6, 3, 7},
    {0}, {0}, {0}, false, chk_wave},
   {s1x8, cr_couples_only, 2, {0, 2, 4, 6, 1, 3, 5, 7},
    {4}, {0}, {0}, true,  chk_groups},
   {s1x8, cr_miniwaves, 1, {0, 2, 4, 6, 1, 3, 5, 7},
    {4}, {0}, {0}, true,  chk_anti_groups},
   {s1x3, cr_1fl_only, 3, {0, 1, 2},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x3, cr_3x3couples_only, 3, {0, 1, 2},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x3, cr_wave_only, 3, {0, 1, 2},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x4, cr_wave_only, 4, {0, 1, 3, 2},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x4, cr_2fl_only, 4, {0, 2, 1, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x4, cr_1fl_only, 4, {0, 1, 2, 3},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x4, cr_4x4couples_only, 4, {0, 1, 2, 3},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x4, cr_all_facing_same, 4, {0, 1, 2, 3},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x4, cr_couples_only, 2, {0, 2, 1, 3},
    {2}, {0}, {0}, true,  chk_groups},
   {s1x4, cr_miniwaves, 1, {0, 2, 1, 3},
    {2}, {0}, {0}, true,  chk_anti_groups},
   {s1x4, cr_magic_only, 4, {0, 1, 2, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x4, cr_all_ns, 4, {4, 0, 1, 2, 3},
    {0},  {0},  {0},  false, chk_dmd_qtag},
   {s1x4, cr_all_ew, 4, {0},
    {4, 0, 1, 2, 3}, {0}, {0}, false, chk_dmd_qtag},
   {s2x4, cr_all_ns, 4, {8, 0, 1, 2, 3, 4, 5, 6, 7},
    {0}, {0}, {0}, false, chk_dmd_qtag},
   {s2x4, cr_all_ew, 4, {0},
    {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0}, {0}, false, chk_dmd_qtag},
   {s1x8, cr_all_ns, 4, {8, 0, 1, 2, 3, 4, 5, 6, 7},
    {0}, {0}, {0}, false, chk_dmd_qtag},
   {s1x8, cr_all_ew, 4, {0},
    {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0}, {0}, false, chk_dmd_qtag},
   {s1x6, cr_miniwaves, 1, {0, 2, 4, 1, 5, 3},
    {3}, {0}, {0}, true,  chk_anti_groups},
   {s1x6, cr_wave_only, 6, {0, 1, 2, 5, 4, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x6, cr_1fl_only, 3, {0, 3, 1, 4, 2, 5},
    {2}, {0}, {0}, true,  chk_groups},
   {s1x6, cr_all_facing_same, 6, {0, 1, 2, 3, 4, 5},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x6, cr_3x3_2fl_only, 6, {0, 3, 1, 4, 2, 5},
    {0}, {0}, {0}, false, chk_wave},
   {s1x6, cr_3x3couples_only, 3, {0, 3, 1, 4, 2, 5},
    {2}, {0}, {0}, true,  chk_groups},
   {s2x3, cr_3x3couples_only, 3, {0, 3, 1, 4, 2, 5},
    {2}, {0}, {0}, true,  chk_groups},
   {s1x10, cr_wave_only, 10, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
    {0},         {0}, {0}, true, chk_wave},
   {s1x12, cr_wave_only, 12, {0, 1, 2, 3, 4, 5, 7, 6, 9, 8, 11, 10},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x14, cr_wave_only, 14, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x16, cr_wave_only, 16, {0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10, 13, 12, 15, 14},
    {0}, {0}, {0}, true, chk_wave},
   {s1x16, cr_couples_only, 2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15},
    {8}, {0}, {0}, true,  chk_groups},
   {s1x16, cr_miniwaves, 1, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15},
    {8}, {0}, {0}, true,  chk_anti_groups},
   {s2x3, cr_all_facing_same, 6, {0, 1, 2, 3, 4, 5},
    {1}, {0}, {0}, true,  chk_groups},
   {s2x3, cr_1fl_only, 3, {0, 3, 1, 4, 2, 5},
    {2}, {0}, {0}, true,  chk_groups},
   {s2x3, cr_li_lo, 6, {3, 0, 4, 1, 5, 2},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_2fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_2fl_per_1x4, 2, {0, 1, 4, 5, 2, 3, 6, 7},
    {2}, {0}, {0}, false,  chk_anti_groups},
   {s3x4, cr_ctr_2fl_only, 4, {10, 4, 11, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_wave_only, 8, {0, 1, 2, 3, 5, 4, 7, 6},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x3, cr_wave_only, 6, {0, 1, 2, 3, 4, 5},
    {0}, {0}, {0}, false, chk_wave},
   {s2x3, cr_magic_only, 6, {0, 1, 2, 3, 4, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_magic_only, 8, {0, 1, 3, 2, 5, 4, 6, 7},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_li_lo, 8, {4, 0, 5, 1, 6, 2, 7, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_ctrs_in_out, 4, {5, 1, 6, 2},
    {0}, {0}, {0}, false, chk_wave},
   {s2x4, cr_indep_in_out, 0, {3, 2, 3, 2, 1, 0, 1, 0},
    {0}, {0}, {0}, false, chk_indep_box},
   {s2x4, cr_all_facing_same, 8, {0, 1, 2, 3, 4, 5, 6, 7},
    {1}, {0}, {0}, true,  chk_groups},
   {s2x4, cr_1fl_only, 4, {0, 4, 1, 5, 2, 6, 3, 7},
    {2}, {0}, {0}, true,  chk_groups},
   {s2x4, cr_couples_only, 2, {0, 2, 4, 6, 1, 3, 5, 7},
    {4}, {0}, {0}, true,  chk_groups},
   {s2x4, cr_miniwaves, 1, {0, 2, 4, 6, 1, 3, 5, 7},
    {4}, {0}, {0}, true,  chk_anti_groups},
   {s_qtag, cr_wave_only, 4, {6, 7, 3, 2},
    {0}, {0}, {0}, true,  chk_wave},
   {s_qtag, cr_2fl_only, 4, {6, 2, 7, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s_qtag, cr_dmd_not_intlk, 4, {6, 7, 3, 2},
    {0}, {0}, {0}, true,  chk_wave},
   {s_qtag, cr_dmd_intlk, 4, {6, 2, 7, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s_ptpd, cr_dmd_not_intlk, 4, {0, 2, 6, 4},
    {0}, {0}, {0}, true,  chk_wave},
   {s_ptpd, cr_dmd_intlk, 4, {0, 6, 2, 4},
    {0}, {0}, {0}, true,  chk_wave},
   {s_qtag, cr_miniwaves, 1, {6, 2, 7, 3},
    {2}, {0}, {0}, true,  chk_anti_groups},
   {s_qtag, cr_couples_only, 2, {6, 2, 7, 3},
    {2}, {0}, {0}, true,  chk_groups},
   {sbigdmd, cr_2fl_only, 4, {3, 9, 2, 8},
    {8}, {0}, {0}, true,  chk_wave},
   {s_galaxy, cr_wave_only, 4, {1, 3, 7, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s_bone, cr_miniwaves, 1, {6, 2, 7, 3},
    {2}, {0}, {0}, true,  chk_anti_groups},
   {s_bone, cr_couples_only, 2, {6, 2, 7, 3},
    {2}, {0}, {0}, true,  chk_groups},
   {s_bone6, cr_wave_only, 6, {0, 1, 2, 3, 4, 5},
    {0}, {0}, {0}, true, chk_wave},
   {s2x8, cr_wave_only, 16, {0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10, 13, 12, 15, 14},
    {0}, {0}, {0}, true, chk_wave},
   {s2x8, cr_4x4_2fl_only, 16, {0, 4, 1, 5, 2, 6, 3, 7, 12, 8, 13, 9, 14, 10, 15, 11},
    {0},{0},{0}, false,chk_wave},
   {s1x16, cr_4x4couples_only, 4, {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15},
    {4}, {0}, {0}, true, chk_groups},
   {s2x8, cr_4x4couples_only, 4, {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15},
    {4}, {0}, {0}, true,  chk_groups},
   {s2x8, cr_couples_only, 2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15},
    {8}, {0}, {0}, true,  chk_groups},
   {s2x8, cr_miniwaves, 1, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15},
    {8}, {0}, {0}, false, chk_anti_groups},
   {s2x6, cr_wave_only, 12, {0, 1, 2, 3, 4, 5, 7, 6, 9, 8, 11, 10},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x6, cr_3x3_2fl_only, 12, {0, 3, 1, 4, 2, 5, 9, 6, 10, 7, 11, 8},
    {0}, {0}, {0}, false, chk_wave},
   {s2x6, cr_3x3couples_only, 3, {0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11},
    {4}, {0}, {0}, true,  chk_groups},
   {s1x12, cr_3x3couples_only, 3, {0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11},
    {4}, {0}, {0}, true,  chk_groups},
   {s1x2, cr_wave_only, 2, {0, 1},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x2, cr_2fl_only, 2, {0, 1},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x2, cr_couples_only, 2, {0, 1},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x2, cr_all_facing_same, 2, {0, 1},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x2, cr_miniwaves, 1, {0, 1},
    {1}, {0}, {0}, true,  chk_anti_groups},
   {s2x2, cr_couples_only, 0, {1, 0, 3, 2},
    {3, 2, 1, 0}, {0}, {0}, false, chk_box_dbl},
   {s2x2, cr_miniwaves, 2, {1, 0, 3, 2, 3, 0, 0, 3},
    {3, 2, 1, 0, 3, 3, 0, 0}, {0}, {0}, false, chk_box_dbl},
   {s2x2, cr_peelable_box, 0, {3, 2, 1, 0},
    {1, 0, 3, 2}, {0}, {0}, false, chk_box_dbl},
   {s2x4, cr_peelable_box, 2, {0, 1, 2, 3, 7, 6, 5, 4},
    {4}, {0}, {0}, false, chk_groups},
   {s2x4, cr_reg_tbone, 1, { 3, 2, 0, 1, 1, 0, 2, 3},
    {0, 3, 1, 2, 2, 1, 3, 0}, {0}, {0}, false, chk_box},
   {s2x2, cr_reg_tbone, 1, {2, 1, 0, 3},
    {2, 3, 0, 1}, {0}, {0}, false, chk_box},
   {s1x4, cr_reg_tbone, 1, {3, 2, 1, 0},
    {0, 3, 2, 1}, {0}, {0}, false, chk_box},
   {s3x4, cr_wave_only, 12, {0, 1, 2, 3, 5, 4, 7, 6, 9, 8, 10, 11},
    {0}, {0}, {0}, true,  chk_wave},
   {s3x4, cr_2fl_only, 12, {0, 2, 1, 3, 8, 4, 9, 5, 10, 6, 11, 7},
    {0}, {0}, {0}, true,  chk_wave},
   {s_thar, cr_wave_only, 8, {0, 1, 2, 3, 5, 4, 7, 6},
    /* NOTE THE 4 --> */{4}, {0}, {0}, true,  chk_wave},
   {s_thar, cr_2fl_only, 8, {0, 5, 2, 7, 1, 4, 3, 6},
    /* NOTE THE 4 --> */{4}, {0}, {0}, true,  chk_wave},
   {s_thar, cr_1fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},
    {0}, {0}, {0}, false, chk_wave},
   {s_thar, cr_magic_only, 8, {0, 1, 3, 2, 5, 4, 6, 7},
    /* NOTE THE 4 --> */{4}, {0}, {0}, true,  chk_wave},
   {s_crosswave, cr_wave_only, 8, {0, 1, 2, 3, 5, 4, 7, 6},
    /* NOTE THE 4 --> */{4}, {0}, {0}, true,  chk_wave},
   {s_crosswave, cr_2fl_only, 8, {0, 5, 2, 7, 1, 4, 3, 6},
    /* NOTE THE 4 --> */{4}, {0}, {0}, true,  chk_wave},
   {s_crosswave, cr_1fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},
    {0}, {0}, {0}, false, chk_wave},
   {s_crosswave, cr_magic_only, 8, {0, 1, 3, 2, 5, 4, 6, 7},
    /* NOTE THE 4 --> */{4}, {0}, {0}, true,  chk_wave},
   {s3x1dmd, cr_wave_only, 6, {0, 1, 2, 6, 5, 4},
    {0}, {0}, {0}, true,  chk_wave},
   {s_2x1dmd, cr_wave_only, 4, {0, 1, 4, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s_qtag, cr_real_1_4_tag, 4, {6, 7, 3, 2},
    {4, 4, 0, 5, 1}, {0}, {0}, true,  chk_qtag},
   {s_qtag, cr_real_3_4_tag, 4, {6, 7, 3, 2},
    {4, 0, 4, 1, 5}, {0}, {0}, true,  chk_qtag},
   {s_qtag, cr_real_1_4_line, 4, {6, 3, 7, 2},
    {4, 4, 0, 5, 1}, {0}, {0}, true,  chk_qtag},
   {s_qtag, cr_real_3_4_line, 4, {6, 3, 7, 2},
    {4, 0, 4, 1, 5}, {0}, {0}, true,  chk_qtag},
   {s_trngl, cr_wave_only, 2, {1, 2},
    {0}, {0}, {0}, true,  chk_wave},
   {s_trngl, cr_miniwaves, 2, {1, 2},
    {0}, {0}, {0}, true,  chk_wave},
   {s_trngl, cr_couples_only, 2, {1, 2},
    {1}, {0}, {0}, true,  chk_groups},
   {s_trngl, cr_2fl_only, 3, {0, 1, 2},
    {1}, {0}, {0}, true,  chk_groups},
   {s_trngl, cr_tgl_tandbase, 0, {0},
    {2, 1, 2}, {2, 1, 2}, {0}, true,  chk_dmd_qtag},
   {s_short6, cr_nice_wv_triangles, 0, {4, 0, 2, 3, 5},
    {2, 1, 4}, {3, 0, 1, 5}, {3, 2, 3, 4}, false,  chk_dmd_qtag_new},
   {s_bone6, cr_nice_wv_triangles, 0, {2, 2, 5},
    {4, 0, 1, 3, 4}, {3, 0, 1, 2}, {3, 3, 4, 5}, false,  chk_dmd_qtag_new},
   {s_trngl, cr_nice_wv_triangles, 0, {2, 1, 2},
    {1, 0}, {1, 1}, {2, 0, 2}, false,  chk_dmd_qtag_new},
   {s_short6, cr_nice_tnd_triangles, 0, {0},
    {6, 0, 1, 2, 3, 4, 5}, {3, 1, 3, 5}, {3, 0, 2, 4}, false,  chk_dmd_qtag_new},
   {s_bone6, cr_nice_tnd_triangles, 0, {6, 0, 1, 2, 3, 4, 5},
    {0}, {3, 0, 2, 4}, {3, 1, 3, 5}, false,  chk_dmd_qtag_new},
   {s_trngl, cr_nice_tnd_triangles, 0, {0},
    {3, 0, 1, 2}, {2, 1, 2}, {1, 0}, false,  chk_dmd_qtag_new},
   {nothing}};

restriction_tester::restr_initializer restriction_tester::restr_init_table1[] = {
   {s1x2, cr_opposite_sex, 2, {0},
    {0}, {0}, {0}, false, chk_sex},
   {s2x4, cr_quarterbox_or_col, 0, {3, 0, 1, 2},
    {3, 4, 5, 6}, {3, 1, 2, 3}, {3, 5, 6, 7}, false, chk_qbox},
   {s2x4, cr_quarterbox_or_magic_col, 0, {3, 1, 2, 7},
    {3, 3, 5, 6}, {3, 1, 2, 4}, {3, 0, 5, 6}, false, chk_qbox},
   {s2x3, cr_quarterbox_or_col, 0, {2, 0, 1},
    {2, 3, 4}, {2, 1, 2}, {2, 4, 5}, false, chk_qbox},
   {s2x3, cr_quarterbox_or_magic_col, 0, {2, 1, 5},
    {2, 2, 4}, {2, 1, 3}, {2, 0, 4}, false, chk_qbox},
   {s2x3, cr_wave_only, 6, {0, 3, 1, 4, 2, 5},
    {0}, {0}, {0}, true, chk_wave},
   {s2x3, cr_magic_only, 6, {0, 1, 2, 3, 4, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x3, cr_peelable_box, 3, {0, 1, 2},
    {3, 4, 5}, {0}, {0}, false, chk_peelable},
   {s2x3, cr_all_facing_same, 6, {0, 1, 2, 3, 4, 5},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x4, cr_all_facing_same, 4, {0, 1, 2, 3},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x4, cr_2fl_only, 4, {0, 2, 1, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x4, cr_li_lo, 4, {0, 1, 3, 2},
    {0}, {0}, {0}, true,  chk_wave},
   {s1x6, cr_all_facing_same, 6, {0, 1, 2, 3, 4, 5},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x8, cr_all_facing_same, 8, {0, 1, 2, 3, 4, 5, 6, 7},
    {1}, {0}, {0}, true,  chk_groups},
   {s2x4, cr_li_lo, 8, {0, 1, 2, 3, 5, 4, 7, 6},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_ctrs_in_out, 4, {1, 2, 6, 5},
    {0}, {0}, {0}, false, chk_wave},
   {s2x4, cr_2fl_only, 8, {0, 3, 1, 2, 6, 5, 7, 4},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_wave_only, 8, {0, 4, 1, 5, 2, 6, 3, 7},
    {0}, {0}, {0}, true,  chk_wave},
   {s_galaxy, cr_wave_only, 4, {1, 7, 3, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_magic_only, 8, {0, 1, 3, 2, 5, 4, 6, 7},
    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_peelable_box, 4, {0, 1, 2, 3},
    {4, 5, 6, 7}, {0}, {0}, false, chk_peelable},
   {s2x4, cr_couples_only, 2, {0, 1, 2, 3, 7, 6, 5, 4},
    {4}, {0}, {0}, true,  chk_groups},
   {s2x4, cr_miniwaves, 1, {0, 1, 2, 3, 7, 6, 5, 4},
    {4}, {0}, {0}, true,  chk_anti_groups},
   {s2x4, cr_all_facing_same, 8, {0, 1, 2, 3, 4, 5, 6, 7},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x2, cr_all_facing_same, 2, {0, 1},
    {1}, {0}, {0}, true,  chk_groups},
   {s1x2, cr_li_lo, 2, {0, 1},
    {0}, {0}, {0}, true, chk_wave},
   {s2x4, cr_reg_tbone, 1, {2, 1, 2, 1, 0, 3, 0, 3},
    {2, 3, 2, 3, 0, 1, 0, 1}, {0}, {0}, false, chk_box},
   {s2x6, cr_wave_only, 12, {0, 6, 1, 7, 2, 8, 3, 9, 4, 10, 5, 11},
    {0}, {0}, {0}, true, chk_wave},
   {s2x6, cr_peelable_box, 6, {0, 1, 2, 3, 4, 5},
    {6, 7, 8, 9, 10, 11}, {0}, {0}, false, chk_peelable},
   {s2x6, cr_couples_only, 3, {0, 1, 2, 3, 4, 5, 11, 10, 9, 8, 7, 6},
    {4}, {0}, {0}, true,  chk_groups},
   {s2x5, cr_wave_only, 10, {0, 5, 1, 6, 2, 7, 3, 8, 4, 9},
    {0}, {0}, {0}, true, chk_wave},
   {s2x5, cr_peelable_box, 5, {0, 1, 2, 3, 4},
    {5, 6, 7, 8, 9}, {0}, {0}, false, chk_peelable},
   {s2x8, cr_wave_only, 16, {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15},
    {0}, {0}, {0}, true, chk_wave},
   {s2x8, cr_peelable_box, 8, {0, 1, 2, 3, 4, 5, 6, 7},
    {8, 9, 10, 11, 12, 13, 14, 15}, {0}, {0}, false, chk_peelable},
   {s2x8, cr_couples_only, 2, {0, 1, 2, 3, 4, 5, 6, 7, 15, 14, 13, 12, 11, 10, 9, 8},
    {8}, {0}, {0}, true,  chk_groups},
   {s_qtag, cr_wave_only, 4, {2, 3, 7, 6},
    {0}, {0}, {0}, false, chk_wave},
   {s_short6, cr_tall6, 6, {4, 0, 2, 3, 5},
    {2, 1, 4}, {3, 0, 1, 5}, {3, 2, 3, 4}, true,  chk_dmd_qtag},
   {s_short6, cr_wave_only, 6, {1, 0, 3, 2, 5, 4},
    {0}, {0}, {0}, true,  chk_wave},
   {s_trngl, cr_wave_only, 2, {1, 2},
    {0}, {0}, {0}, true,  chk_wave},   // Isn't this bogus?  It checks for TANDEM-BASE.
   {sdmd, cr_wave_only, 2, {1, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {sdmd, cr_miniwaves, 2, {1, 3},
    {0}, {0}, {0}, true,  chk_wave},
   {s_ptpd, cr_wave_only, 4, {1, 3, 7, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s_ptpd, cr_miniwaves, 1, {1, 7, 3, 5},
    {2}, {0}, {0}, true,  chk_anti_groups},
   {nothing}};

restriction_tester::restr_initializer restriction_tester::restr_init_table4[] = {
   {sdmd, cr_jright, 4, {0, 2, 1, 3},                     {0}, {0}, {0}, true,  chk_wave},
   {sdmd, cr_jleft, 4, {0, 2, 3, 1},                      {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_conc_iosame, 8, {6, 1, 0, 3, 5, 2, 7, 4},    {0}, {0}, {0}, true,  chk_wave},
   {s2x4, cr_conc_iodiff, 8, {1, 6, 0, 3, 2, 5, 7, 4},    {0}, {0}, {0}, true,  chk_wave},
   {s_ptpd, cr_jright, 8, {0, 2, 1, 3, 6, 4, 7, 5},       {0}, {0}, {0}, true,  chk_wave},
   {s_ptpd, cr_jleft, 8, {0, 2, 3, 1, 6, 4, 5, 7},        {0}, {0}, {0}, true,  chk_wave},
   {nothing}};

restriction_tester::restr_initializer restriction_tester::restr_init_table9[] = {
   {s_qtag, cr_jleft, 8, {2, 3, 0, 4, 7, 6, 1, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s_qtag, cr_jright, 8, {3, 2, 0, 4, 6, 7, 1, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s_c1phan,cr_jleft,16, {2, 0, 1, 3, 5, 7, 4, 6, 8, 10, 11, 9, 15, 13, 14, 12},
    {0}, {0}, {0}, false, chk_wave},
   {s_c1phan,cr_jright,16,{0, 2, 1, 3, 7, 5, 4, 6, 10, 8, 11, 9, 13, 15, 14, 12},
    {0}, {0}, {0}, false, chk_wave},
   {s_qtag, cr_ijleft, 8, {2, 6, 0, 4, 3, 7, 1, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s_qtag, cr_ijright, 8, {6, 2, 0, 4, 7, 3, 1, 5},
    {0}, {0}, {0}, true,  chk_wave},
   {s_qtag, cr_diamond_like, 4, {4, 2, 3, 6, 7},
    {4, 0, 1, 4, 5}, {0}, {0}, false, chk_dmd_qtag},
   {s_qtag, cr_qtag_like, 4, {8, 0, 1, 2, 3, 4, 5, 6, 7},
    {0}, {2, 4, 5}, {2, 0, 1}, false, chk_dmd_qtag},
   {s_qtag, cr_pu_qtag_like, 0, {0},
    {8, 0, 1, 2, 3, 4, 5, 6, 7}, {2, 0, 1}, {2, 4, 5}, false, chk_dmd_qtag},
   {s3dmd, cr_diamond_like, 0, {6, 3, 4, 5, 9, 10, 11},
    {6, 0, 1, 2, 6, 7, 8}, {0}, {0}, false, chk_dmd_qtag},
   {s3dmd, cr_qtag_like, 0, {12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    {0}, {3, 6, 7, 8}, {3, 0, 1, 2}, false, chk_dmd_qtag},
   {s3dmd, cr_pu_qtag_like, 0, {0},
    {12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    {3, 0, 1, 2}, {3, 6, 7, 8}, false, chk_dmd_qtag},
   {s4dmd, cr_jleft, 16, {4, 5, 0, 8, 6, 7, 1, 9, 15, 14, 2, 10, 13, 12, 3, 11},
    {0}, {0}, {0}, true, chk_wave},
   {s4dmd, cr_jright, 16, {5, 4, 0, 8, 7, 6, 1, 9, 14, 15, 2, 10, 12, 13, 3, 11},
    {0}, {0}, {0}, true, chk_wave},
   {s4dmd, cr_diamond_like, 8, {8, 4, 5, 6, 7, 12, 13, 14, 15},
    {8, 0, 1, 2, 3, 8, 9, 10, 11}, {0}, {0}, false, chk_dmd_qtag},
   {s4dmd, cr_qtag_like, 4, {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {0}, {4, 8, 9, 10, 11}, {4, 0, 1, 2, 3}, false, chk_dmd_qtag},
   {s4dmd, cr_pu_qtag_like, 0, {0},
    {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {4, 0, 1, 2, 3}, {4, 8, 9, 10, 11}, false, chk_dmd_qtag},
   {sdmd, cr_jright, 4, {2, 0, 1, 3},
    {8}, {0}, {0}, true,  chk_wave},
   {sdmd, cr_jleft, 4, {2, 0, 3, 1},
    {8}, {0}, {0}, true,  chk_wave},
   {sdmd, cr_diamond_like, 4, {2, 0, 2},
    {2, 1, 3}, {0}, {0}, false, chk_dmd_qtag},
   {sdmd, cr_qtag_like, 4, {0},
    {4, 0, 1, 2, 3}, {1, 0}, {1, 2}, false, chk_dmd_qtag},
   {sdmd, cr_pu_qtag_like, 0, {4, 0, 1, 2, 3},
    {0}, {1, 0}, {1, 2}, false, chk_dmd_qtag},
   {s_ptpd, cr_jright, 8, {2, 0, 1, 3, 4, 6, 7, 5},
    {8}, {0}, {0}, true,  chk_wave},
   {s_ptpd, cr_jleft, 8, {2, 0, 3, 1, 4, 6, 5, 7},
    {8}, {0}, {0}, true,  chk_wave},
   {s_ptpd, cr_diamond_like, 4, {4, 0, 2, 4, 6},
    {4, 1, 3, 5, 7}, {0}, {0}, false, chk_dmd_qtag},
   {s_ptpd, cr_qtag_like, 4, {0},
    {8, 0, 1, 2, 3, 4, 5, 6, 7}, {2, 0, 6}, {2, 2, 4}, false, chk_dmd_qtag},
   {s4x6, cr_qtag_like, 8, {0},
    {16, 11, 1, 9, 10, 8, 4, 6, 7, 23, 13, 21, 22, 20, 16, 19, 18},
    {4, 11, 8, 18, 21}, {4, 9, 6, 23, 20}, false, chk_dmd_qtag},
   {s_ptpd, cr_pu_qtag_like, 0, {8, 0, 1, 2, 3, 4, 5, 6, 7},
    {0}, {2, 0, 6}, {2, 2, 4}, false, chk_dmd_qtag},
   {s2x4, cr_gen_qbox, 4, {0},
    {8, 0, 1, 2, 3, 4, 5, 6, 7}, {2, 1, 2}, {2, 5, 6}, false, chk_dmd_qtag},
   {s2x2, cr_wave_only, 1, {2, 0, 0, 2},
    {0, 0, 2, 2},         {0}, {0}, true,  chk_box},
   {s2x2, cr_all_facing_same, 1, {2, 2, 2, 2},
    {0, 0, 0, 0},   {0}, {0}, true,  chk_box},
   {s2x2, cr_2fl_only, 1, {2, 2, 2, 2},
    {0, 0, 0, 0},          {0}, {0}, true,  chk_box},
   {s2x2, cr_trailers_only, 1, {0, 0, 2, 2},
    {0, 2, 2, 0},     {0}, {0}, true,  chk_box},
   {s2x2, cr_leads_only, 1, {0, 0, 2, 2},
    {0, 2, 2, 0},        {0}, {0}, true,  chk_box},
   {s2x2, cr_li_lo, 1, {0, 0, 2, 2},
    {0, 2, 2, 0},             {0}, {0}, true,  chk_box},
   {s2x2, cr_indep_in_out, 0, {3, 2, 1, 0},
    {0}, {0}, {0}, false, chk_indep_box},
   {s2x2, cr_magic_only, 1, {2, 0, 2, 0},
    {0, 2, 0, 2},        {0}, {0}, true,  chk_box},
   {nothing}};



void restriction_tester::initialize_tables()
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


restriction_tester::restr_initializer *restriction_tester::get_restriction_thing(
   setup_kind k, assumption_thing t)
{
   restr_initializer **hash_table;
   uint32 hash_num = ((k + (5*t.assumption)) * 25) & (NUM_RESTR_HASH_BUCKETS-1);
   uint32 col = t.assump_col;

   // Check for a few special cases that need to be translated.

   if (k == s2x4 && t.assumption == cr_magic_only && (col & 2)) {
      // "assume inverted boxes", people are in general lines (col=2) or cols (col=3)
      col = 0;   /* This is wrong, need something that allows
                    real mag col or unsymm congruent inv boxes.
                    This is a "check mixed groups" type of thing!!!! */
   }

   // Use a specific hash table to search for easy cases.
   // Otherwise, search the general table.

   bool can_try_gen_table = true;

   do {
      switch (col) {
      case 0:  hash_table = restr_hash_table0; break;
      case 1:  hash_table = restr_hash_table1; break;
      case 4:  hash_table = restr_hash_table4; break;
      default: hash_table = restr_hash_table9;
         can_try_gen_table = false;
         break;
      }

      for ( restr_initializer *restr_hash_bucket = hash_table[hash_num] ;
            restr_hash_bucket ;
            restr_hash_bucket = restr_hash_bucket->next) {
         if (restr_hash_bucket->k == k && restr_hash_bucket->restr == t.assumption)
            return restr_hash_bucket;
      }

      col = 99;   // If we go back, only do the general table.
   }
   while (can_try_gen_table);   // Go back one more time if did a specific table.

   return (restr_initializer *) 0;
}


restriction_test_result verify_restriction(
   setup *ss,
   assumption_thing tt,
   bool instantiate_phantoms,
   bool *failed_to_instantiate) THROW_DECL
{
   int idx, limit, i, j;
   uint32 t = 0;
   uint32 qa0, qa1, qa2, qa3;
   uint32 qaa[4];
   uint32 pdir, qdir, pdirodd, qdirodd;
   uint32 dirtest[2];
   const veryshort *p, *q;
   int phantom_count = 0;
   restriction_tester::restr_initializer *rr;
   int k = 0;
   int local_negate = tt.assump_negate;

   if (tt.assumption == cr_not_all_sel || tt.assumption == cr_some_sel)
      local_negate = 1;

   switch (tt.assumption) {
   case cr_alwaysfail:
      return restriction_fails;
   case cr_give_fudgy_warn:
      warn(warn__may_be_fudgy);
      return restriction_passes;

   case cr_ptp_unwrap_sel:
      k ^= 022002200 ^ 014001400;
   case cr_nor_unwrap_sel:
      k ^= 014001400;
      goto cr_none_sel_label;

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
      default: goto bad;
      }
      /* FALL THROUGH!!!!!! */
   case cr_not_all_sel:
   case cr_all_sel:
      /* FELL THROUGH!!!!!! */
      k = ~k;
      /* FALL THROUGH!!!!!! */

   case cr_some_sel:
   case cr_none_sel:
   cr_none_sel_label:
      /* FELL THROUGH!!!!!! */
      for (idx=0 ; idx<=attr::slimit(ss) ; idx++,k>>=1) {
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

   case cr_nice_diamonds:
      // B=1 => right-handed; B=2 => left-handed.
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
   case cr_conc_iosame:
   case cr_conc_iodiff:
      tt.assump_col = 4;
      break;
   case cr_leads_only:
      tt.assump_both = 2;
      break;
   case cr_trailers_only:
      tt.assump_both = 1;
      break;
   case cr_levelplus:
      if (calling_level < l_plus) return restriction_bad_level;
      goto good;
   case cr_levela1:
      if (calling_level < l_a1) return restriction_bad_level;
      goto good;
   case cr_levela2:
      if (calling_level < l_a2) return restriction_bad_level;
      goto good;
   case cr_levelc1:
      if (calling_level < l_c1) return restriction_bad_level;
      goto good;
   case cr_levelc2:
      if (calling_level < l_c2) return restriction_bad_level;
      goto good;
   case cr_levelc3:
      if (calling_level < l_c3) return restriction_bad_level;
      goto good;
   case cr_levelc4:
      if (calling_level < l_c4) return restriction_bad_level;
      goto good;
   case cr_siamese_in_quad:
      t ^= 2;
      // FALL THROUGH!!!!!
   case cr_not_tboned_in_quad:      // WARNING!!  WE FELL THROUGH!!
      t ^= 1;
      // This is independent of whether we are line-like or column-like.
      for (idx=0 ; idx<4 ; idx++) {
         int ii;
         int perquad = (attr::slimit(ss)+1) >> 2;
         qa0 = 0; qa1 = 0; qa2 = 0; qa3 = 0;
         // Test one quadrant.
         for (ii=0 ; ii<perquad ; ii++) {
            uint32 tp;
            if ((tp = ss->people[perquad*idx+ii].id1) != 0) {
               qa0 |= (tp^1);
               qa1 |= (tp^3);
               qa2 |= (tp^2);
               qa3 |= (tp);
            }
         }
         if ((qa0&t) && (qa1&t) && (qa2&t) && (qa3&t)) goto bad;
      }
      goto good;
   }

   rr = restriction_tester::get_restriction_thing(ss->kind, tt);
   if (!rr) return restriction_no_item;

   dirtest[0] = 0;
   dirtest[1] = 0;

   *failed_to_instantiate = true;

   veryshort *map1item;
   int szlim;

   switch (rr->check) {
   case restriction_tester::chk_spec_directions:
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
   case restriction_tester::chk_wave:
      qaa[0] = tt.assump_both;
      qaa[1] = tt.assump_both << 1;

      for (idx=0; idx<rr->size; idx++) {
         if ((t = ss->people[rr->map1[idx]].id1) != 0) {
            qaa[idx&1] |=  t;
            qaa[(idx&1)^1] |= t^2;
         }
         else if (local_negate || tt.assump_live) goto bad;    /* All live people
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
            *failed_to_instantiate = false;

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
   case restriction_tester::chk_box:
      p = rr->map1;
      q = rr->map2;

      for (i=0; i<rr->size; i++) {
         qa0 = (tt.assump_both << 1) & 2;
         qa1 = tt.assump_both & 2;
         qa2 = qa1;
         qa3 = qa0;

         for (idx=0 ; idx<=attr::slimit(ss) ; idx++) {
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

            for (i=0 ; i<=attr::slimit(ss) ; i++) {
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

            *failed_to_instantiate = false;
         }
      }

      goto good;
   case restriction_tester::chk_box_dbl:
      // Check everyone's lateral partner, independently of headlinerness.
      for (idx=0 ; idx<=attr::slimit(ss) ; idx++) {
         uint32 u;

         if ((t = ss->people[idx].id1) & 010) p = rr->map1;
         else if ((t = ss->people[idx].id1) & 1) p = rr->map2;
         else continue;

         p += idx;

         if ((u = ss->people[p[0]].id1) && ((t ^ u ^ rr->size) & 3)) goto bad;

         if (rr->restr == cr_miniwaves) {
            if (((((t>>1)&1)+1) ^ p[4]) & tt.assump_both) goto bad;
         }
      }

      goto good;
   case restriction_tester::chk_indep_box:
      qa0 = (tt.assump_both << 1) & 2;
      qa1 = tt.assump_both & 2;
      qa2 = 0;
      qa3 = 0;

      for (idx=0 ; idx<=attr::slimit(ss) ; idx++) {
         if ((t = ss->people[idx].id1) != 0) {
            qa2 |= t + rr->map1[idx];
            qa3 |= t + rr->map1[idx] + 2;
         }
      }

      if (((qa0 & qa2) | (qa1 & qa3)) != 0) goto bad;

      goto good;
   case restriction_tester::chk_groups:
      limit = rr->map2[0];

      for (idx=0; idx<limit; idx++) {
         qa0 = 0; qa1 = 0;

         for (i=0,j=idx; i<rr->size; i++,j+=limit) {
            if ((t = ss->people[rr->map1[j]].id1) != 0) { qa0 |= t; qa1 |= t^2; }
            else if (local_negate || tt.assump_live) goto bad;    /* All live people were demanded. */
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

               *failed_to_instantiate = false;
            }
         }
      }
      goto good;
   case restriction_tester::chk_anti_groups:
      limit = rr->map2[0];

      map1item = rr->map1;
      szlim = rr->size*limit;

      for (idx=0; idx<limit; idx++) {
         qa0 = 0; qa1 = 0;

         for (int jjj=0 ; jjj<rr->size ; jjj++) {
            if ((t = ss->people[map1item[0]].id1) != 0)     { qa0 |= t;   qa1 |= t^2; }
            else if (local_negate) goto bad;    /* All live people were demanded. */
            if ((t = ss->people[map1item[szlim]].id1) != 0) { qa0 |= t^2; qa1 |= t;   }
            else if (local_negate) goto bad;    /* All live people were demanded. */
            map1item++;
         }

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

               *failed_to_instantiate = false;
            }
         }
      }
      goto good;
   case restriction_tester::chk_peelable:
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
   case restriction_tester::chk_qbox:
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
   case restriction_tester::chk_star:
      qa0 = tt.assump_both;
      qa1 = ~(tt.assump_both << 1);

      for (idx=0; idx<rr->map1[0]; idx++) {
         if ((t = ss->people[rr->map1[idx+1]].id1) & 1) { qa0 |= t; qa1 &= t; }
      }

      for (idx=0; idx<rr->map2[0]; idx++) {
         if ((t = ss->people[rr->map2[idx+1]].id1) & 1) { qa0 |= ~t; qa1 &= ~t; }
      }

      for (idx=0; idx<rr->map3[0]; idx++) {
         if ((t = ss->people[rr->map3[idx+1]].id1) & 010) { qa0 |= t; qa1 &= t; }
      }

      for (idx=0; idx<rr->map4[0]; idx++) {
         if ((t = ss->people[rr->map4[idx+1]].id1) & 010) { qa0 |= ~t; qa1 &= ~t; }
      }

      if (qa0 & ~qa1 & 2)
         goto bad;

      goto good;
   case restriction_tester::chk_dmd_qtag:
   case restriction_tester::chk_dmd_qtag_new:
      qa0 = qa1 = qa2 = qa3 = 0;

      for (idx=0; idx<rr->map1[0]; idx++)
         qa1 |= ss->people[rr->map1[idx+1]].id1;

      for (idx=0; idx<rr->map2[0]; idx++)
         qa0 |= ss->people[rr->map2[idx+1]].id1;

      for (idx=0; idx<rr->map3[0]; idx++) {
         if ((t = ss->people[rr->map3[idx+1]].id1) != 0)
            { qa2 |= t; qa3 |= ~t; }
      }

      for (idx=0; idx<rr->map4[0]; idx++) {
         if ((t = ss->people[rr->map4[idx+1]].id1) != 0)
            { qa2 |= ~t; qa3 |= t; }
      }

      if ((qa1 & 001) != 0 || (qa0 & 010) != 0)
         goto bad;

      if (rr->check == restriction_tester::chk_dmd_qtag) {
         if (tt.assump_both) {
            // The "live" modifier means that we need a definitive person
            // to distinguish "in" or "out".
            if (tt.assump_live && !(qa2 | qa3))
               goto bad;

            if ((qa2 & (tt.assump_both << 1) & 2) != 0 ||
                (qa3 & tt.assump_both & 2) != 0)
               goto bad;
         }
      }
      else {
         // The "live" flag means exactly what it says.
         if (tt.assump_live) {
            for (idx=0 ; idx<=attr::slimit(ss) ; idx++) {
               if (ss->people[idx].id1 == 0) goto bad;
            }
         }

         // If "both" is off, we demand either consistent handedness.
         // If it's on, we demand that handedness.
         if (tt.assump_both) {
            if ((qa2 & (tt.assump_both << 1) & 2) != 0 ||
                (qa3 & tt.assump_both & 2) != 0)
               goto bad;
         }
         else if ((qa2 & qa3 & 2) != 0) goto bad;
      }

      goto good;
   case restriction_tester::chk_qtag:
      qaa[0] = tt.assump_both;
      qaa[1] = tt.assump_both << 1;

      for (idx=0; idx<rr->size; idx++) {
         if ((t = ss->people[rr->map1[idx]].id1) != 0) {
            qaa[idx&1] |=  t;
            qaa[(idx&1)^1] |= t^2;
         }
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
            *failed_to_instantiate = false;

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

   case restriction_tester::chk_sex:
      qaa[0] = tt.assump_both;
      qaa[1] = tt.assump_both << 1;

      for (i=0; i<2; i++) {
         if ((t = ss->people[i].id1)) {
            uint32 northified = (i ^ (t>>1)) & 1;
            dirtest[i] = t;
            if (t & ID1_PERM_BOY) qaa[northified] |= 2;
            else if (t & ID1_PERM_GIRL) qaa[northified^1] |= 2;
            else goto bad;
         }
      }

      if ((dirtest[0] & dirtest[1]) & (ID1_PERM_BOY | ID1_PERM_GIRL))
         goto bad;

      // If this is a couple, check the "left" or "right" bits.
      if (rr->map4[0] && (qaa[0] & qaa[1] & 2)) goto bad;

      goto good;

   case restriction_tester::chk_inroller:

      // This "size" field tells whether this is an inroll/outroll
      // type (size != 0) or a judge/socker type (size == 0).

      if (ss->kind == s2x4 && rr->size != 0) {
         // For these assumptions, finding a single usable person
         // will suffice to determine the outcome.
         // Now it happens that, if assump_both has something,
         // we could determine the handedness, and hence the
         // in/out roll direction, without even one person.
         // But we won't be called if there isn't even one person.
         // ***** Not so!  We might be called with one person in the
         // center, but no ends.  Too bad.
         switch (ss->cmd.cmd_assume.assumption) {
         case cr_wave_only:
            if ((ss->people[0].id1 & 013) == (uint32) rr->map2[1] ||
                (ss->people[1].id1 & 013) == (uint32) rr->map2[0] ||
                (ss->people[2].id1 & 013) == (uint32) rr->map2[1] ||
                (ss->people[3].id1 & 013) == (uint32) rr->map2[0] ||
                (ss->people[4].id1 & 013) == (uint32) rr->map2[0] ||
                (ss->people[5].id1 & 013) == (uint32) rr->map2[1] ||
                (ss->people[6].id1 & 013) == (uint32) rr->map2[0] ||
                (ss->people[7].id1 & 013) == (uint32) rr->map2[1])
               goto good;
            break;
         case cr_2fl_only:
            if ((ss->people[0].id1 & 013) == (uint32) rr->map2[1] ||
                (ss->people[2].id1 & 013) == (uint32) rr->map2[0] ||
                (ss->people[1].id1 & 013) == (uint32) rr->map2[1] ||
                (ss->people[3].id1 & 013) == (uint32) rr->map2[0] ||
                (ss->people[4].id1 & 013) == (uint32) rr->map2[0] ||
                (ss->people[6].id1 & 013) == (uint32) rr->map2[1] ||
                (ss->people[5].id1 & 013) == (uint32) rr->map2[0] ||
                (ss->people[7].id1 & 013) == (uint32) rr->map2[1])
               goto good;
            break;
         }
      }

      if ((ss->people[rr->map1[0]].id1 & 013) == (uint32) rr->map2[0] &&
          (ss->people[rr->map1[1]].id1 & 013) == (uint32) rr->map2[1] &&
          (ss->people[rr->map1[2]].id1 & 013) != (uint32) rr->map2[2] &&
          (ss->people[rr->map1[3]].id1 & 013) != (uint32) rr->map2[3])
         goto good;

      if ((ss->people[rr->map1[4]].id1 & 013) == (uint32) rr->map2[4] &&
          (ss->people[rr->map1[5]].id1 & 013) == (uint32) rr->map2[5] &&
          (ss->people[rr->map1[6]].id1 & 013) == 0 &&
          (ss->people[rr->map1[7]].id1 & 013) == 0)
         goto good;

      goto bad;

   default:
      goto bad;    // Shouldn't happen.
   }

 good:
   if (local_negate) return restriction_fails;
   else return restriction_passes;

 bad:
   if (local_negate) return restriction_passes;
   else return restriction_fails;
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


static void initialize_concept_sublists()
{
   int test_call_list_kind;
   concept_kind end_marker = concept_diagnose;

   // Decide whether we allow the "diagnose" concept, by deciding
   // when we will stop the concept list scan.
   if (ui_options.diagnostic_mode) end_marker = marker_end_of_list;

   // First, just count up all the available concepts.

   int all_legal_concepts = 0;
   const conzept::concept_descriptor *p;

   for (p = concept_descriptor_table ; p->kind != end_marker ; p++) {
      if (p->level <= calling_level) all_legal_concepts++;
   }

   concept_sublists[call_list_any] =
      (short int *) get_mem(all_legal_concepts*sizeof(short int));
   good_concept_sublists[call_list_any] =
      (short int *) get_mem(all_legal_concepts*sizeof(short int));

   // Make the concept sublists, one per setup.  We do them in downward order,
   // with "any setup" last.  This is because we put our results into the
   // "any setup" slot while we are working, and then copy them to the
   // correct slot for each setup other than "any".

   for (test_call_list_kind = (int) call_list_qtag;
        test_call_list_kind >= (int)call_list_any;
        test_call_list_kind--) {
      int concept_index, concepts_in_this_setup, good_concepts_in_this_setup;

      for (concept_index=0,concepts_in_this_setup=0,good_concepts_in_this_setup=0;
           ;
           concept_index++) {
         uint32 setup_mask = ~0;      // Default is to accept the concept.
         uint32 good_setup_mask = 0;  // Default for this is not to.
         const conzept::concept_descriptor *p = &concept_descriptor_table[concept_index];
         if (p->kind == end_marker) break;
         if (p->kind == concept_omit) continue;  // Totally stupid concept!!!!
         if (p->level > calling_level) continue;

         // This concept is legal at this level.  See if it is appropriate for this setup.
         // If we don't know, the default value of setup_mask will make it legal.

         switch (p->kind) {
         case concept_centers_or_ends:
         case concept_centers_and_ends:
            switch (p->arg1) {
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
            switch (p->arg1) {
            case schema_concentric:
            case schema_cross_concentric:
            case schema_single_concentric:
            case schema_single_cross_concentric:
               good_setup_mask = MASK_GOODCONC;
            }

            break;
         case concept_once_removed:
            if (p->arg1 == 0)
               good_setup_mask = MASK_GOODRMVD;

            break;
         case concept_tandem:
         case concept_some_are_tandem:
         case concept_frac_tandem:
         case concept_some_are_frac_tandem:
            switch (p->arg4) {
            case tandem_key_tand:
               setup_mask = MASK_TAND;

               // We allow <anyone> tandem, and we allow twosome.  That's all.
               // We specifically exclude all the "tandem in a 1/4 tag" stuff.
               if (p->arg2 != CONCPROP__NEEDK_TWINQTAG &&
                   ((p->arg3 & 0xFF) == 0 || (p->arg3 & 0xFF) == 0x10))
                  good_setup_mask = MASK_GOODTAND;

               break;
            case tandem_key_cpls:
               setup_mask = MASK_CPLS;

               // We allow <anyone> tandem, and we allow twosome.  That's all.
               // We specifically exclude all the "tandem in a 1/4 tag" stuff.
               if (p->arg2 != CONCPROP__NEEDK_TWINQTAG &&
                   ((p->arg3 & 0xFF) == 0 || (p->arg3 & 0xFF) == 0x10))
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
            // Test for quadruple C/L/W working.
            if (p->arg4 == 4) setup_mask = MASK_2X4;
            break;
         case concept_quad_diamonds:
         case concept_quad_diamonds_together:
         case concept_do_phantom_diamonds:
            setup_mask = MASK_QUAD_D;
            break;
         case concept_triple_twin_nomystic:
            // Test for divided C/L/W.
            if (p->arg3 == 10) setup_mask = MASK_2X4;
            break;
         case concept_do_phantom_2x4:
         case concept_quad_lines:
            setup_mask = MASK_2X4;          // Can actually improve on this.
            break;
         case concept_assume_waves:
            // We never allow any "assume_waves" concept.  In the early days,
            // it was actually dangerous.  It isn't dangerous any more,
            // but it's a fairly stupid thing to generate in a search.
            setup_mask = 0;
            break;
         }

         // Now we can determine whether this concept is appropriate for this setup.

         if ((1 << (test_call_list_kind - ((int) call_list_empty))) & setup_mask)
            concept_sublists[call_list_any][concepts_in_this_setup++] = concept_index;

         // And we can determine whether this concept is really nice for this setup.

         if ((1 << (test_call_list_kind - ((int) call_list_empty))) & good_setup_mask)
            good_concept_sublists[call_list_any][good_concepts_in_this_setup++] = concept_index;
      }

      concept_sublist_sizes[test_call_list_kind] = concepts_in_this_setup;
      good_concept_sublist_sizes[test_call_list_kind] = good_concepts_in_this_setup;

      if (test_call_list_kind != (int) call_list_any) {
         if (concepts_in_this_setup != 0) {
            concept_sublists[test_call_list_kind] =
               (short int *) get_mem(concepts_in_this_setup*sizeof(short int));
            (void) memcpy(concept_sublists[test_call_list_kind],
                          concept_sublists[call_list_any],
                          concepts_in_this_setup*sizeof(short int));
         }
         if (good_concepts_in_this_setup != 0) {
            good_concept_sublists[test_call_list_kind] =
               (short int *) get_mem(good_concepts_in_this_setup*sizeof(short int));
            (void) memcpy(good_concept_sublists[test_call_list_kind],
                          good_concept_sublists[call_list_any],
                          good_concepts_in_this_setup*sizeof(short int));
         }
      }
   }

   // "Any" is not considered a good setup.
   good_concept_sublist_sizes[call_list_any] = 0;
}

extern void initialize_sdlib()
{
   initialize_tandem_tables();
   restriction_tester::initialize_tables();
   select::initialize();
   conc_tables::initialize();
   merge_table::initialize();
   tglmap::initialize();
   initialize_commonspot_tables();
   map::initialize();
   full_expand::initialize_touch_tables();
   expand::initialize();
   initialize_concept_sublists();

   writechar_block.usurping_writechar = false;

   int i;

   for (i=0 ; i<warn__NUM_WARNINGS ; i++) {
      switch (warning_strings[i][0]) {
      case '#':
         useless_phan_clw_warnings.setbit((warning_index) i);
         // FALL THROUGH!!!!
      case '*':
         // FELL THROUGH!!!!
         no_search_warnings.setbit((warning_index) i); break;
      case '+':
         conc_elong_warnings.setbit((warning_index) i); break;
      case '=':
         dyp_each_warnings.setbit((warning_index) i); break;
      }
   }

   // Create the tagger menu lists.

   uint32 ui;

   for (i=0 ; i<NUM_TAGGER_CLASSES ; i++) {
      tagger_menu_list[i] = (Cstring *) get_mem((number_of_taggers[i]+1) * sizeof(char *));

      for (ui=0; ui<number_of_taggers[i]; ui++)
         tagger_menu_list[i][ui] = tagger_calls[i][ui]->menu_name;

      tagger_menu_list[i][number_of_taggers[i]] = (Cstring) 0;
   }

   // Create the selector menu list.  It is one item shorter than the enumeration,
   // because we skip the first item in the enumeration.

   selector_menu_list = (Cstring *) get_mem((selector_ENUM_EXTENT) * sizeof(char *));

   for (i=0; i<selector_ENUM_EXTENT-1; i++)
      selector_menu_list[i] = selector_list[i+1].name;

   selector_menu_list[selector_ENUM_EXTENT-1] = (Cstring) 0;

   /* Create the circcer list. */

   circcer_menu_list = (Cstring *) get_mem((number_of_circcers+1) * sizeof(char *));

   for (ui=0; ui<number_of_circcers; ui++)
      circcer_menu_list[ui] = circcer_calls[ui]->menu_name;

   circcer_menu_list[number_of_circcers] = (Cstring) 0;

   initialize_getout_tables();
}


static bool check_for_supercall(parse_block *parseptrcopy)
{
   concept_kind kk = parseptrcopy->concept->kind;

   if (kk <= marker_end_of_list) {
      // Only calls with "@0" in their name may be supercalls.
      if (kk == marker_end_of_list &&
          !parseptrcopy->next &&
          (parseptrcopy->call->the_defn.callflagsf & CFLAGH__HAS_AT_ZERO)) {
         setup_command foo2;
         by_def_item innerdef;
         innerdef.call_id = base_call_null;
         innerdef.modifiers1 = DFM1_CALL_MOD_MAND_ANYCALL;
         innerdef.modifiersh = 0;
         setup_command bar;
         bar.cmd_final_flags.clear_all_herit_and_final_bits();
         calldefn this_defn = base_calls[base_call_null]->the_defn;
         get_real_subcall(parseptrcopy, &innerdef, &bar, &this_defn, false, 0, &foo2);
      }

      if (parseptrcopy->concept->kind == concept_another_call_next_mod &&
          parseptrcopy->next &&
          (parseptrcopy->next->call == base_calls[base_call_null] ||
           parseptrcopy->next->call == base_calls[base_call_null_second]) &&
          !parseptrcopy->next->next &&
          parseptrcopy->next->subsidiary_root) {
         return true;
      }
      else
         fail("A concept is required.");
   }

   return false;
}

static parse_block pbend(&conzept::mark_end_of_list);
static parse_block pbsuper(&conzept::marker_concept_supercall);

// This writes over its 2nd and 3rd arguments.
extern bool check_for_concept_group(
   parse_block *parseptrcopy,
   parse_block * & kkk,
   uint32 & need_to_restrain,   // 1=(if not doing echo), 2=(yes, always)
   parse_block ***parseptr_skip_p) THROW_DECL
{
   concept_kind k;
   parse_block *kk;
   bool retval = false;
   parse_block *parseptr_skip;
   parse_block *next_parseptr;

   parse_block *first_arg = parseptrcopy;

   kkk = &pbend;
   need_to_restrain = 0;

 try_again:

   if (!parseptrcopy) fail("A concept is required.");

   parseptr_skip = parseptrcopy->next;

   if (parseptrcopy->concept) {
      kk = parseptrcopy;

      if (check_for_supercall(parseptrcopy))
         kk = &pbsuper;
   }
   else
      kk = &pbend;

   k = kk->concept->kind;

   if (!retval) {
      kkk = kk;

      if (k == concept_crazy ||
          k == concept_frac_crazy ||
          k == concept_n_times_const ||
          k == concept_n_times)
         need_to_restrain |= 2;
   }

   // We do these even if we aren't the first concept.

   if (k == concept_supercall ||
       k == concept_fractional ||
       (k == concept_meta && parseptrcopy->concept->arg1 == meta_key_initially) ||
       (k == concept_meta && parseptrcopy->concept->arg1 == meta_key_finally) ||
       (k == concept_meta && parseptrcopy->concept->arg1 == meta_key_initially_and_finally) ||
       (k == concept_meta && parseptrcopy->concept->arg1 == meta_key_piecewise) ||
       (k == concept_meta && parseptrcopy->concept->arg1 == meta_key_echo) ||
       (k == concept_meta && parseptrcopy->concept->arg1 == meta_key_rev_echo) ||
       (k == concept_meta && parseptrcopy->concept->arg1 == meta_key_revorder) ||
       (k == concept_meta && parseptrcopy->concept->arg1 == meta_key_finish))
      need_to_restrain |= 1;


   // If skipping "phantom", maybe it's "phantom tandem", so we need to skip both.
   // Similarly with "parallelogram split phantom C/L/W/B" or
   // "offset C/L/W split phantom C/L/W/B".     <-- this one not done.

   if (k == concept_c1_phantom) {
      final_and_herit_flags junk_concepts;

      junk_concepts.clear_all_herit_and_final_bits();

      next_parseptr =
         process_final_concepts(parseptr_skip, false, &junk_concepts, true, __FILE__, __LINE__);

      if ((next_parseptr->concept->kind == concept_tandem ||
           next_parseptr->concept->kind == concept_frac_tandem) &&
          (junk_concepts.test_herit_and_final_bits()) == 0) {
         parseptrcopy = next_parseptr;
         retval = true;
         goto try_again;
      }
   }
   else if (k == concept_parallelogram ||
            (k == concept_distorted &&
             parseptrcopy->concept->arg1 == disttest_offset &&
             parseptrcopy->concept->arg3 == 0 &&
             parseptrcopy->concept->arg4 == 0)) {
      final_and_herit_flags junk_concepts;

      junk_concepts.clear_all_herit_and_final_bits();

      next_parseptr =
         process_final_concepts(parseptr_skip, false, &junk_concepts, true, __FILE__, __LINE__);

      if ((next_parseptr->concept->kind == concept_do_phantom_2x4 ||
           next_parseptr->concept->kind == concept_do_phantom_boxes) &&
          next_parseptr->concept->arg3 == MPKIND__SPLIT &&
          (junk_concepts.test_herit_and_final_bits()) == 0) {
         parseptrcopy = next_parseptr;
         retval = true;
         goto try_again;
      }
   }
   else if (k == concept_meta) {
      meta_key_kind subkey = (meta_key_kind) parseptrcopy->concept->arg1;

      if (subkey == meta_key_random || subkey == meta_key_rev_random ||
          subkey == meta_key_piecewise || subkey == meta_key_nth_part_work ||
          subkey == meta_key_first_frac_work || subkey == meta_key_initially ||
          subkey == meta_key_finally || subkey == meta_key_initially_and_finally ||
          subkey == meta_key_echo || subkey == meta_key_rev_echo) {
         next_parseptr = parseptr_skip;
         parseptrcopy = next_parseptr;
         retval = true;
         goto try_again;
      }
   }
   else if (k == concept_so_and_so_only &&
            ((selective_key) parseptrcopy->concept->arg1) == selective_key_work_concept) {
      next_parseptr = parseptr_skip;
      parseptrcopy = next_parseptr;
      retval = true;
      goto try_again;
   }

   if (parseptr_skip_p) {
      if (k == concept_supercall)
         *parseptr_skip_p = &parseptrcopy->next->subsidiary_root;
      else if (retval)
         *parseptr_skip_p = &next_parseptr->next;
      else if (parseptrcopy->concept->kind == concept_special_sequential)
         *parseptr_skip_p = &first_arg->subsidiary_root;
      else
         *parseptr_skip_p = &first_arg->next;
   }

   return retval;
}



static void debug_print_parse_block(int level, const parse_block *p, char *tempstring_text, int &n)
{
   for ( ; p ; p = p->next) {
      if (level > 0) n += sprintf(tempstring_text+n, "(%d) ", level);
      if (p->concept) n += sprintf(tempstring_text+n, "  concept %s  ", p->concept->name);
      if (p->call) n += sprintf(tempstring_text+n, "  call %s  ", p->call->name);
      n += sprintf(tempstring_text+n, " %d %d %d %d %d \n",
             p->options.who,
             p->options.where,
             p->options.howmanynumbers,
             (int) p->options.number_fields,
             (int) p->options.tagger);
      if (p->subsidiary_root)
         debug_print_parse_block(level+1, p->subsidiary_root, tempstring_text, n);
   }
}

extern void crash_print(const char *filename, int linenum) THROW_DECL
{
   char tempstring_text[10000];  // If this isn't big enough, I guess we lose.

   int n = sprintf(tempstring_text, "++++++++ CRASH - PLEASE REPORT THIS ++++++++\n");
   n += sprintf(tempstring_text+n, "%s %d\n", filename, linenum);

   int hsize = configuration::history_ptr+1;
   for (int jjj = 2 ; jjj <= hsize ; jjj++) {
      debug_print_parse_block(0, configuration::history[jjj].command_root, tempstring_text, n);
   }

   gg->serious_error_print(tempstring_text);
   fail("Crash.");
}





extern void fail(const char s[]) THROW_DECL
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   throw error_flag_type(error_flag_1_line);
}


extern void fail_no_retry(const char s[]) THROW_DECL
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   throw error_flag_type(error_flag_no_retry);
}


extern void fail2(const char s1[], const char s2[]) THROW_DECL
{
   (void) strncpy(error_message1, s1, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   (void) strncpy(error_message2, s2, MAX_ERR_LENGTH);
   error_message2[MAX_ERR_LENGTH-1] = '\0';
   throw error_flag_type(error_flag_2_line);
}


extern void failp(uint32 id1, const char s[]) THROW_DECL
{
   collision_person1 = id1;
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   throw error_flag_type(error_flag_cant_execute);
}


extern void specialfail(const char s[]) THROW_DECL
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   throw error_flag_type(error_flag_wrong_resolve_command);
}


extern void warn(warning_index w)
{
   if (w == warn__none) return;

   // If this is an "each 1x4" type of warning, and we already have
   // such a warning, don't enter the new one.  The first one takes precedence.

   if (dyp_each_warnings.testbit(w) && configuration::test_multiple_warnings(dyp_each_warnings))
         return;

   configuration::set_one_warning(w);
}


extern callarray *assoc(begin_kind key, setup *ss, callarray *spec) THROW_DECL
{
   for (callarray *p = spec ; p ; p = p->next) {
      uint32 i, k, t, u, w, mask;
      assumption_thing tt;
      int idx, plaini;
      bool booljunk;

      // First, we demand that the starting setup be correct.
      // Also, if a qualifier number was specified, it must match.

      if ((begin_kind) p->start_setup != key) continue;

      tt.assump_negate = 0;

      /* During initialization, we will be called with a null pointer for ss.
         We need to be careful, and err on the side of acceptance. */

      if (!ss) return p;

      /* The bits of the "qualifierstuff" field have the following meaning
                  (see definitions in database.h):
         8000  left/out only (put 2 into assump_both)
         4000  right/in only (put 1 into assump_both)
         2000  must be live
         1000  must be T-boned
         0800  must not be T-boned (both together means explicit assumption)
         0780  if these 4 bits are nonzero, they must match the number plus 1
         007F  the qualifier itself (we allow 127 qualifiers) */

      if ((p->qualifierstuff & QUALBIT__NUM_MASK) != 0) {
         number_used = true;
         if (((unsigned int) (p->qualifierstuff & QUALBIT__NUM_MASK) / QUALBIT__NUM_BIT) !=
             (current_options.number_fields & 0xF)+1)
            continue;
      }

      bool require_explicit = false;

      if ((p->qualifierstuff & (QUALBIT__TBONE|QUALBIT__NTBONE)) != 0) {
         if ((p->qualifierstuff & (QUALBIT__TBONE|QUALBIT__NTBONE)) == (QUALBIT__TBONE|QUALBIT__NTBONE)) {
            require_explicit = true;
         }
         else {
            u = 0;

            for (plaini=0; plaini<=attr::slimit(ss); plaini++)
               u |= ss->people[plaini].id1;

            if ((u & 011) == 011) {
               // They are T-boned.  The "QUALBIT__NTBONE" bit says to reject.
               if ((p->qualifierstuff & QUALBIT__NTBONE) != 0) continue;
            }
            else {
               // They are not T-boned.  The "QUALBIT__TBONE" bit says to reject.
               if ((p->qualifierstuff & QUALBIT__TBONE) != 0) continue;
            }
         }
      }

      call_restriction ssA = ss->cmd.cmd_assume.assumption;
      uint32 ssB = ss->cmd.cmd_assume.assump_both;
      setup_kind ssK = ss->kind;

      call_restriction this_qualifier = (call_restriction) (p->qualifierstuff & QUALBIT__QUAL_CODE);

      if (this_qualifier == cr_none) {
         if ((p->qualifierstuff / QUALBIT__LIVE) & 1) {   // All live people were demanded.
            for (plaini=0; plaini<=attr::slimit(ss); plaini++) {
               if ((ss->people[plaini].id1) == 0) goto bad;
            }
         }

         goto good;
      }

      // Note that we have to examine setups larger than the setup the
      // qualifier is officially defined for.  If a qualifier were defined
      // as being legal only on 1x4's (so that, in the database, we only had
      // specifications of the sort "setup 1x4 1x4 qualifier wave_only") we could
      // still find ourselves here with ss->kind equal to s2x4.  Why?  Because
      // the setup could be a 2x4 and the splitter could be trying to decide
      // whether to split the setup into parallel 1x4's.  This happens when
      // trying to figure out whether to split a 2x4 into 1x4's or 2x2's for
      // the call "recycle".

      k = 0;   // Many tests will find these values useful.
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
         switch (ssA) {
         case cr_1fl_only:
         case cr_2fl_only:
         case cr_wave_only:
         case cr_li_lo:
            goto bad;
         }

         switch (ssK) {
         case s2x4: case s1x8:
            if (key == b_1x4) tt.assumption = cr_miniwaves;
            break;
         }

         goto fix_col_line_stuff;
      case cr_li_lo:
         switch (ssA) {
         case cr_magic_only: case cr_wave_only:
            goto bad;
         }

         switch (ssK) {
         case s1x2: case s1x4: case s2x2: case s2x3: case s2x4:
            goto fix_col_line_stuff;
         default:
            goto good;           /* We don't understand the setup -- we'd better accept it. */
         }
      case cr_opposite_sex:
         switch (ssK) {
         case s1x2:
            goto fix_col_line_stuff;
         default:
            goto good;           // Accept this all the way down to 1x2.
         }
      case cr_indep_in_out:
         switch (ssK) {
         case s2x2: case s2x4:
            goto fix_col_line_stuff;
         default:
            goto good;           /* We don't understand the setup -- we'd better accept it. */
         }
      case cr_ctrs_in_out:
         switch (ssK) {
         case s2x4:
            if (!((ss->people[1].id1 | ss->people[2].id1 |
                   ss->people[5].id1 | ss->people[6].id1)&010))
               tt.assump_col = 1;
            goto check_tt;
         default:
            goto good;           /* We don't understand the setup -- we'd better accept it. */
         }

      case cr_all_facing_same:
         switch (ssA) {
         case cr_wave_only:
         case cr_magic_only:
            goto bad;
         }

         // If we are not looking at the whole setup (that is, we are deciding
         // whether to split the setup into smaller ones), let it pass.

         if (setup_attrs[ssK].keytab[0] != key &&
             setup_attrs[ssK].keytab[1] != key)
            goto good;

         goto fix_col_line_stuff;
      case cr_1fl_only:        // 1x3/1x4/1x6/1x8 - a 1FL, that is, all 3/4/6/8 facing same;
                               // 2x3/2x4 - individual 1FL's
         switch (ssA) {
         case cr_1fl_only:
            goto good;
         case cr_wave_only:
         case cr_magic_only:
            goto bad;
         }

         goto check_tt;
      case cr_2fl_only:
         goto do_2fl_stuff;

      case cr_ctr_2fl_only:
         switch (ssA) {
         case cr_li_lo: case cr_1fl_only: case cr_wave_only:
         case cr_miniwaves: case cr_magic_only: goto bad;
         }

         goto check_tt;
      case cr_3x3_2fl_only:
         switch (ssA) {
         case cr_1fl_only: case cr_wave_only:
         case cr_miniwaves: case cr_magic_only: goto bad;
         }

         tt.assumption = cr_3x3_2fl_only;
         goto fix_col_line_stuff;
      case cr_4x4_2fl_only:
         switch (ssA) {
         case cr_1fl_only: case cr_wave_only:
         case cr_miniwaves: case cr_magic_only: goto bad;
         }

         tt.assumption = cr_4x4_2fl_only;
         goto fix_col_line_stuff;
      case cr_couples_only:
         switch (ssA) {
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

         switch (ssK) {
         case s1x2: case s1x4: case s1x6: case s1x8: case s1x16: case s2x4:
         case sdmd: case s_trngl: case s_qtag: case s_ptpd: case s_bone:
         case s2x2:
            goto fix_col_line_stuff;
         default:
            goto good;                 /* We don't understand the setup --
                                          we'd better accept it. */
         }
      case cr_3x3couples_only:      // 1x3/1x6/2x3/2x6/1x12 - each group of 3 people
                                    // are facing the same way
         switch (ssA) {
         case cr_1fl_only:
            goto good;
         case cr_wave_only:
         case cr_magic_only:
            goto bad;
         }

         goto check_tt;
      case cr_4x4couples_only:      // 1x4/1x8/2x4/2x8/1x16 - each group of 4 people
                                    // are facing the same way
         switch (ssA) {
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
      case cr_miniwaves:                    // miniwaves everywhere
         /* **** FELL THROUGH!!!!!! */
         switch (ssA) {
         case cr_li_lo:
         case cr_1fl_only:
         case cr_2fl_only:
            goto bad;
         case cr_wave_only:
         case cr_miniwaves:
         case cr_magic_only:
            goto good;
         }

         tt.assumption = cr_miniwaves;

         switch (ssK) {
         case s1x2: case s1x4: case s1x6: case s1x8: case s1x16: case s2x4:
         case sdmd: case s_trngl: case s_qtag: case s_ptpd: case s_bone:
         case s2x2:
            goto fix_col_line_stuff;
         default:
            goto good;                 /* We don't understand the setup --
                                          we'd better accept it. */
         }
      case cr_tgl_tandbase:
         tt.assumption = cr_tgl_tandbase;
         switch (ssK) {
         case s_trngl:
            goto fix_col_line_stuff;
         default:
            goto good;
         }
      case cr_ctrwv_end2fl:
         /* Note that this qualifier is kind of strict.  We won't permit the call "with
            confidence" do be done unless everyone can trivially determine which
            part to do. */
         if (ssK == s_crosswave) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 0 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 0 &&
                ((ss->people[2].id1 | ss->people[3].id1) == 0 ||
                 ((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 2) &&
                ((ss->people[6].id1 | ss->people[7].id1) == 0 ||
                 ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 2))
               goto good;
            goto bad;
         }
         else if (ssK == s1x6) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 0 &&
                ((ss->people[3].id1 ^ ss->people[4].id1) & d_mask) == 0 &&
                ((ss->people[1].id1 ^ ss->people[2].id1) & d_mask) == 2 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 2)
               goto good;
            goto bad;
         }
         else if (ssK == s1x8) {
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
         if (ssK == s_crosswave) {
            if (((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 0 &&
                ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 0 &&
                ((ss->people[0].id1 | ss->people[1].id1) == 0 ||
                 ((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 2) &&
                ((ss->people[4].id1 | ss->people[5].id1) == 0 ||
                 ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 2))
               goto good;
            goto bad;
         }
         else if (ssK == s1x6) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 2 &&
                ((ss->people[3].id1 ^ ss->people[4].id1) & d_mask) == 2 &&
                ((ss->people[1].id1 ^ ss->people[2].id1) & d_mask) == 0 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 0)
               goto good;
            goto bad;
         }
         else if (ssK == s1x8) {
            if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 2 &&
                ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 2 &&
                ((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 0 &&
                ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 0)
               goto good;
            goto bad;
         }
         else
            goto good;         /* We don't understand the setup -- we'd better accept it. */
      case cr_siamese_in_quad:
      case cr_not_tboned_in_quad:
         goto check_tt;
      case cr_true_Z_cw:
         k ^= 033U ^ 066U;
         mask ^= CMD_MISC2__IN_Z_CCW ^ CMD_MISC2__IN_Z_CW;
         // **** FALL THROUGH!!!!
      case cr_true_Z_ccw:
         // **** FELL THROUGH!!!!!!
         k ^= ~033U;
         mask ^= CMD_MISC2__IN_Z_CCW;

         if (ss->cmd.cmd_misc2_flags & mask)
             goto good;

         if (ssK == s2x3) {
            // In this case, we actually check the shear direction of the Z.

            mask = 0;

            for (plaini=0, w=1; plaini<=attr::slimit(ss); plaini++, w<<=1) {
               if (ss->people[plaini].id1) mask |= w;
            }

            if ((mask & k) == 0 && (mask & (k^033U^066U)) != 0) goto good;
            goto bad;
         }
         else
            goto good;         // We don't understand the setup -- we'd better accept it.
      case cr_true_PG_cw:
         k ^= 01717U ^ 07474U;
         // **** FALL THROUGH!!!!
      case cr_true_PG_ccw:
         // **** FELL THROUGH!!!!!!
         k ^= ~01717U;

         if (ssK == s2x6) {
            // In this case, we actually check the shear direction of the parallelogram.

            mask = 0;

            for (plaini=0, w=1; plaini<=attr::slimit(ss); plaini++, w<<=1) {
               if (ss->people[plaini].id1) mask |= w;
            }

            if ((mask & k) == 0 && (mask & (k^01717U^07474U)) != 0) goto good;
            goto bad;
         }
         else if (ssK == s4x6 || ssK == s2x12)
            goto good;         // We don't understand the setup, and it's one from which we
                               // could subdivide into 2x6's -- we'd better accept it.

         goto bad;   // If it's a 2x4, for example, it can't be a parallelogram.
      case cr_lateral_cols_empty:
         mask = 0;
         t = 0;

         for (plaini=0, k=1; plaini<=attr::slimit(ss); plaini++, k<<=1) {
            if (ss->people[plaini].id1) { mask |= k; t |= ss->people[plaini].id1; }
         }

         if (ssK == s3x4 && (t & 1) == 0 &&
             ((mask & 04646) == 0 ||
              (mask & 04532) == 0 || (mask & 03245) == 0 ||
              (mask & 02525) == 0 || (mask & 03232) == 0 ||
              (mask & 04651) == 0 || (mask & 05146) == 0))
            goto good;
         else if (ssK == s3x6 && (t & 1) == 0 &&
                  ((mask & 0222222) == 0))
            goto good;
         else if (ssK == s4x4 && (t & 1) == 0 &&
                  ((mask & 0xE8E8) == 0 ||
                   (mask & 0xA3A3) == 0 || (mask & 0x5C5C) == 0 ||
                   (mask & 0xA857) == 0 || (mask & 0x57A8) == 0))
            goto good;
         else if (ssK == s4x4 && (t & 010) == 0 &&
                  ((mask & 0x8E8E) == 0 ||
                   (mask & 0x3A3A) == 0 || (mask & 0xC5C5) == 0 ||
                   (mask & 0x857A) == 0 || (mask & 0x7A85) == 0))
            goto good;
         else if (ssK == s4x6 && (t & 010) == 0) goto good;
         else if (ssK == s3x8 && (t & 001) == 0) goto good;
         goto bad;
      case cr_dmd_same_pt:                   /* dmd or pdmd - centers would circulate
                                                to same point */
         if (((ss->people[1].id1 & 01011) == d_east) &&         /* faces either east or west */
             (!((ss->people[3].id1 ^ ss->people[1].id1) & d_mask)))  /* and both face same way */
            goto good;
         goto bad;
      case cr_nice_diamonds:
      case cr_dmd_facing:
      case cr_conc_iosame:
      case cr_conc_iodiff:
      case cr_nice_wv_triangles:
         goto check_tt;
      case cr_dmd_ctrs_mwv:
         switch (ssA) {
         case cr_dmd_ctrs_mwv:
            if (((tt.assump_both+1) & 2) && ssB == tt.assump_both)
               goto good;
            break;
         case cr_jleft: case cr_jright: case cr_ijleft: case cr_ijright:
            if ((tt.assump_both+1) & 2) {
               if ((ssB ^ tt.assump_both) == 3)
                  goto good;
               else if ((ssB ^ tt.assump_both) == 0)
                  goto bad;
            }
            break;
         }
         goto check_tt;
      case cr_qtag_like:
         switch (ssA) {
         case cr_diamond_like: case cr_pu_qtag_like:
            goto bad;
         case cr_jleft: case cr_jright: case cr_ijleft: case cr_ijright:
            // If tt.B is 1 or 2, demand ss.B be the other.
            if (((tt.assump_both-1) & ~1) == 0) {
               if (ssB == (tt.assump_both ^ 3))
                  goto good;
               else
                  goto bad;
            }
         }
         goto check_tt;
      case cr_diamond_like:
         switch (ssA) {
         case cr_qtag_like: case cr_pu_qtag_like:
            goto bad;
         }
         goto check_tt;
      case cr_pu_qtag_like:
         switch (ssA) {
         case cr_qtag_like: case cr_diamond_like:
            goto bad;
         }
         goto check_tt;
      case cr_dmd_intlk:
         switch (ssA) {
         case cr_ijright: case cr_ijleft: case cr_real_1_4_line: case cr_real_3_4_line:
            goto good;
         case cr_jright: case cr_jleft: case cr_real_1_4_tag: case cr_real_3_4_tag:
            goto bad;
         }

         goto check_tt;
      case cr_dmd_not_intlk:
         switch (ssA) {
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
         if (ss->cmd.cmd_final_flags.test_finalbit(FINAL__SPLIT_DIXIE_APPROVED)) goto good;
         goto bad;
      case cr_didnt_say_tgl:
         tt.assump_negate = 1;
         /* **** FALL THROUGH!!!! */
      case cr_said_tgl:
         /* **** FELL THROUGH!!!!!! */
         if (ss->cmd.cmd_misc_flags & CMD_MISC__SAID_TRIANGLE) goto good;
         goto bad;
      case cr_occupied_as_h:
         if (ssK != s3x4 ||
             (ss->people[1].id1 | ss->people[2].id1 |
              ss->people[7].id1 | ss->people[8].id1)) goto bad;
         goto good;
      case cr_occupied_as_stars:
         if (ssK != s_c1phan ||
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
      case cr_occupied_as_clumps:
         if (ssK != s4x4 ||
             ((ss->people[0].id1 | ss->people[1].id1 |
               ss->people[14].id1 | ss->people[3].id1 |
               ss->people[8].id1 | ss->people[9].id1 |
               ss->people[6].id1 | ss->people[11].id1) &&
              (ss->people[4].id1 | ss->people[5].id1 |
               ss->people[2].id1 | ss->people[7].id1 |
               ss->people[12].id1 | ss->people[13].id1 |
               ss->people[10].id1 | ss->people[15].id1)))
            goto bad;
         goto good;
      case cr_occupied_as_blocks:
         if (ssK != s4x4 ||
             ((ss->people[0].id1 | ss->people[13].id1 |
               ss->people[10].id1 | ss->people[3].id1 |
               ss->people[8].id1 | ss->people[5].id1 |
               ss->people[2].id1 | ss->people[11].id1) &&
              (ss->people[4].id1 | ss->people[9].id1 |
               ss->people[6].id1 | ss->people[7].id1 |
               ss->people[12].id1 | ss->people[1].id1 |
               ss->people[14].id1 | ss->people[15].id1)))
            goto bad;
         goto good;
      case cr_occupied_as_qtag:
         if (ssK != s3x4 ||
             (ss->people[0].id1 | ss->people[3].id1 |
              ss->people[6].id1 | ss->people[9].id1)) goto bad;
         goto good;
      case cr_occupied_as_3x1tgl:
         if (ssK == s_qtag) goto good;
         if (ssK == s3x4 && !(ss->people[1].id1 | ss->people[2].id1 |
                                   ss->people[7].id1 | ss->people[8].id1)) goto good;
         if (ssK == s3x4 && !(ss->people[0].id1 | ss->people[3].id1 |
                                   ss->people[6].id1 | ss->people[9].id1)) goto good;
         if (ssK == s3x4 && !(ss->people[1].id1 | ss->people[3].id1 |
                                   ss->people[6].id1 | ss->people[8].id1)) goto good;
         if (ssK == s3x4 && !(ss->people[0].id1 | ss->people[2].id1 |
                                   ss->people[7].id1 | ss->people[9].id1)) goto good;
         if (ssK == s2x6 && !(ss->people[0].id1 | ss->people[2].id1 |
                                   ss->people[6].id1 | ss->people[8].id1)) goto good;
         if (ssK == s2x6 && !(ss->people[3].id1 | ss->people[5].id1 |
                                   ss->people[9].id1 | ss->people[11].id1)) goto good;
         if (ssK == s2x3 && !(ss->people[0].id1 | ss->people[2].id1)) goto good;
         if (ssK == s2x3 && !(ss->people[3].id1 | ss->people[5].id1)) goto good;
         goto good;    // Must be evaluating a division of a 4x5 or whatever.
      case cr_reg_tbone:
         if (ssK == s2x4 && key == b_2x2) tt.assump_col = 1;
         /* **** FALL THROUGH!!!! */
      case cr_qtag_mwv:
      case cr_qtag_mag_mwv:
      case cr_dmd_ctrs_1f:
      case cr_gen_qbox:
      case cr_inroller_is_cw:
      case cr_inroller_is_ccw:
      case cr_outroller_is_cw:
      case cr_outroller_is_ccw:
      case cr_judge_is_cw:
      case cr_judge_is_ccw:
      case cr_socker_is_cw:
      case cr_socker_is_ccw:
         /* **** FELL THROUGH!!!!!! */
         goto check_tt;
      case cr_ctr_pts_rh:
      case cr_ctr_pts_lh:
         {
            uint32 z = 1;

            switch (ssA) {
            case cr_jright: case cr_ijright:
               z = 2;
            case cr_jleft: case cr_ijleft:
               if (z & ssB) {
                  if (this_qualifier == cr_ctr_pts_rh) goto bad;
                  else goto good;
               }
               else if ((z ^ 3) & ssB) {
                  if (this_qualifier == cr_ctr_pts_rh) goto good;
                  else goto bad;
               }
            }

            uint32 t1;
            uint32 t2;
            uint32 ndir = d_north;
            uint32 sdir = d_south;
            uint32 tb;
            bool b1 = true;
            bool b2 = true;

            switch (ssK) {
            case s_qtag:
            case s_hrglass:
            case s_dhrglass:
            case s_bone:
               t1 = 6; t2 = 2; break;
            case s_2x1dmd:
               t1 = 0; t2 = 3; break;
            case s_bone6:
               t1 = 5; t2 = 2; break;
            case s_short6:
               // This one has the people facing sideways.
               ndir = d_east; sdir = d_west;
               t1 = 1; t2 = 4; break;
            case s_star:
               // This has to look at headliner-sideliner-ness.  It
               // will screw up if people are T-boned.
               tb = (ss->people[0].id1 | ss->people[1].id1 |
                  ss->people[2].id1 | ss->people[3].id1) & 011;
               if (tb == 1) {
                  ndir = d_east; sdir = d_west;
                  t1 = 1; t2 = 3; break;
               }
               else if (tb == 010) {
                  t1 = 0; t2 = 2; break;
               }
               else goto bad;
            default:
               goto bad;
            }

            t1 = ss->people[t1].id1;
            t2 = ss->people[t2].id1;

            if (t1 && (t1 & d_mask)!=ndir) b1 = false;
            if (t2 && (t2 & d_mask)!=sdir) b1 = false;
            if (t1 && (t1 & d_mask)!=sdir) b2 = false;
            if (t2 && (t2 & d_mask)!=ndir) b2 = false;

            if (b1 == b2) goto bad;

            if (this_qualifier ==
                (b1 ? cr_ctr_pts_rh : cr_ctr_pts_lh))
               goto good;
         }
         goto bad;
      case cr_line_ends_looking_out:
         if (ssK != s2x4) goto bad;

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
         if (ssK != s2x4) goto bad;
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
         k ^= 0x090555;
         mask = 0;

         for (plaini=0, w=1; plaini<=attr::slimit(ss); plaini++, w<<=1) {
            if (selectp(ss, plaini)) mask |= w;
         }

         if (ssK == s1x4) {
            if (mask == (k & 0xF) || mask == ((k>>4) & 0xF) || mask == ((k>>8) & 0xF)) goto good;
            goto bad;
         }
         else if (ssK == s1x6) {
            if (mask == ((k>>16) & 0x3F)) goto good;
            goto bad;
         }

         goto good;     // Huh?
      case cr_people_1_and_5_real:
         if (ss->people[1].id1 & ss->people[5].id1) goto good;
         goto bad;

      case cr_ptp_unwrap_sel:
      case cr_nor_unwrap_sel:
      case cr_ends_sel:
      case cr_ctrs_sel:
      case cr_all_sel:
      case cr_not_all_sel:
      case cr_some_sel:
      case cr_none_sel:

         // If we are not looking at the whole setup (that is, we are deciding
         // whether to split the setup into smaller ones), let it pass.

         if (setup_attrs[ssK].keytab[0] != key &&
             setup_attrs[ssK].keytab[1] != key)
            goto good;

         goto check_tt;
      default:
         break;
      }

      goto bad;

   do_2fl_stuff:
      tt.assumption = cr_2fl_only;

      switch (ssA) {
      case cr_li_lo: case cr_1fl_only: case cr_wave_only:
      case cr_miniwaves: case cr_magic_only: goto bad;
      }

      switch (ssK) {
      case s2x4:
      case s1x8:
         if (key == b_1x4) tt.assumption = cr_2fl_per_1x4;
         break;
      }

      goto fix_col_line_stuff;

   do_wave_stuff:
      switch (ssA) {
      case cr_1fl_only: case cr_2fl_only: case cr_couples_only: case cr_magic_only: goto bad;
      case cr_li_lo: goto bad;
      case cr_ijright: case cr_ijleft: case cr_real_1_4_line: case cr_real_3_4_line: goto bad;
      case cr_wave_only: case cr_jright:
         if (ssB == 2 && tt.assump_both == 1) goto bad;
         if (ssB == 1 && tt.assump_both == 2) goto bad;
         break;
      case cr_jleft:
         if (ssB == 2 && tt.assump_both == 2) goto bad;
         if (ssB == 1 && tt.assump_both == 1) goto bad;
         break;
      }

      tt.assumption = cr_wave_only;

      switch (ssK) {
      case s3x4:         // This only handles lines; not columns --
                         // we couldn't have "wavy" columns that were symmetric.
      case s_bone6:
      case s_trngl:
      case s_qtag:
      case s3x1dmd:
         goto check_tt;
      case s_galaxy:
         u = ss->people[1].id1 | ss->people[3].id1 |
            ss->people[5].id1 | ss->people[7].id1;
         if (!(u&010)) tt.assump_col = 1;
         else if (u&001) goto bad;
         goto check_tt;
      case sdmd:
      case s_ptpd:
      case s_short6:
         tt.assump_col = 1;
         goto check_tt;
      case s2x4:
         // If the setup is a 2x4, but we are deciding whether
         // to split into 2x2's, let it pass.
         if (key == b_2x2) goto good;
         else if (key == b_1x4) tt.assumption = cr_miniwaves;
         break;
      }

   fix_col_line_stuff:

      switch (ssK) {
      case s1x3:
         if (tt.assump_both) goto bad;   // We can't check a 1x3 for right-or-left-handedness.
         /* FALL THROUGH!!! */
      case s1x6: case s1x8: case s1x10:
      case s1x12: case s1x14: case s1x16:
      case s2x2: case s4x4: case s_thar: case s_crosswave: case s_qtag:
      case s3x1dmd: case s_2x1dmd: case sbigdmd:
      case s_trngl: case s_bone:
         /* FELL THROUGH!!! */
         goto check_tt;
      case sdmd: case s_ptpd:
         tt.assump_col = 1;
         goto check_tt;
      case s1x4:  // Note that 1x6, 1x8, etc should be here also.  This will
                  // make "cr_2fl_only" and such things work in 4x1.
      case s1x2:
      case s2x4:
      case s2x6:
      case s2x8:
      case s2x3:
         for (idx=0,u=0 ; idx<=attr::slimit(ss) ; idx++)
            u |= ss->people[idx].id1;
         if (!(u&010)) tt.assump_col = 1;

         // This line commented out.  We now recognize "handedness" of the waves of 3
         // in a 2x3.  OK?  This is needed for 1/2 circulate from lines of 3 with
         // centers facing and ends back-to-back.  See test vg05/vg02.
         //         else if (tt.assump_both && ssK == s2x3) goto bad;
         //    /* We can't check 2x3 lines for right-or-left-handedness. */
         goto check_tt;
      default:
         /* ****** Try to change this (and other things) to "goto bad". */
         goto good;                 /* We don't understand the setup -- we'd better accept it. */
      }

   check_tt:

      // If the "explicit" flag was on, we don't accept anything unless an assumption was in place.

      if (require_explicit && ssA == cr_none) goto bad;

      if (verify_restriction(ss, tt, false, &booljunk) == restriction_passes) return p;
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
   int northified_index) THROW_DECL
{
   if (!tdef) crash_print(__FILE__, __LINE__);

   unsigned short *calldef_array;
   uint32 z;

   if (tdef->callarray_flags & CAF__PREDS) {
      for (predptr_pair *predlistptr = tdef->stuff.prd.predlist ;
           predlistptr ;
           predlistptr = predlistptr->next) {
         if ((*(predlistptr->pred->predfunc))
             (scopy, real_index, real_direction,
              northified_index, predlistptr->pred->extra_stuff)) {
            calldef_array = predlistptr->arr;
            goto got_it;
         }
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


extern void clear_result_flags(setup *z)
{
   z->result_flags.misc = 0;
   z->result_flags.clear_split_info();
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
      throw error_flag_type(error_flag_collision);
   }
}


extern void install_rot(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace, int rotamount) THROW_DECL
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
         throw error_flag_type(error_flag_collision);
      }
   }
}


extern void scatter(setup *resultpeople, const setup *sourcepeople,
                    const veryshort *resultplace, int countminus1, int rotamount) THROW_DECL
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


extern void install_scatter(setup *resultpeople, int num, const veryshort *placelist,
                            const setup *sourcepeople, int rot) THROW_DECL
{
   for (int j=0; j<num; j++)
      (void) install_rot(resultpeople, placelist[j], sourcepeople, j, rot);
}


/* WARNING!!!!  This procedure appears verbatim in sdtop.c and dbcomp.c . */

/* These combinations are not allowed. */

#define FORBID1 (INHERITFLAG_FRACTAL|INHERITFLAG_YOYO)
#define FORBID2 (INHERITFLAG_SINGLEFILE|INHERITFLAG_SINGLE)
#define FORBID3 (INHERITFLAG_MXNMASK|INHERITFLAG_NXNMASK)
#define FORBID4 (INHERITFLAG_12_MATRIX|INHERITFLAG_16_MATRIX)


static bool do_heritflag_merge(final_and_herit_flags *dest, uint32 source)
{
   uint32 revertsource = source & INHERITFLAG_REVERTMASK;

   if (revertsource) {
      // If the source is a revert/reflect bit, things are complicated.

      uint32 revertdest = dest->test_heritbit(INHERITFLAG_REVERTMASK);

      if (!revertdest) {
         goto good;
      }
      else if (revertsource == INHERITFLAGRVRTK_REVERT &&
               revertdest == INHERITFLAGRVRTK_REFLECT) {
         dest->clear_heritbit(INHERITFLAG_REVERTMASK);
         dest->set_heritbit(INHERITFLAGRVRTK_RFV);
         return false;
      }
      else if (revertsource == INHERITFLAGRVRTK_REFLECT &&
               revertdest == INHERITFLAGRVRTK_REVERT) {
         dest->clear_heritbit(INHERITFLAG_REVERTMASK);
         dest->set_heritbit(INHERITFLAGRVRTK_RVF);
         return false;
      }
      else if (revertsource == INHERITFLAGRVRTK_REFLECT &&
               revertdest == INHERITFLAGRVRTK_REFLECT) {
         dest->clear_heritbit(INHERITFLAG_REVERTMASK);
         dest->set_heritbit(INHERITFLAGRVRTK_RFF);
         return false;
      }
      else if (revertsource == INHERITFLAGRVRTK_REVERT &&
               revertdest == INHERITFLAGRVRTK_RVF) {
         dest->clear_heritbit(INHERITFLAG_REVERTMASK);
         dest->set_heritbit(INHERITFLAGRVRTK_RVFV);
         return false;
      }
      else if (revertsource == INHERITFLAGRVRTK_REFLECT &&
               revertdest == INHERITFLAGRVRTK_RFV) {
         dest->clear_heritbit(INHERITFLAG_REVERTMASK);
         dest->set_heritbit(INHERITFLAGRVRTK_RFVF);
         return false;
      }
      else
         return true;
   }

   // Check for plain redundancy.  If this is a bit in one of the complex
   // fields, this simple test may not catch the error, but the next one will.

   if (dest->test_heritbits(source))
      return true;

   if ((dest->test_heritbits(FORBID1) && (source & FORBID1)) ||
       (dest->test_heritbits(FORBID2) && (source & FORBID2)) ||
       (dest->test_heritbits(FORBID3) && (source & FORBID3)) ||
       (dest->test_heritbits(FORBID4) && (source & FORBID4)))
      return true;

   good:

   dest->set_heritbits(source);

   return false;
}



// Take a concept pointer and scan for all "final" concepts,
// returning an updated concept pointer and a mask of all such concepts found.
// "Final" concepts are those that modify the execution of a call but
// do not cause it to be executed in a virtual or distorted setup.
// This has a side-effect that is occasionally used:  When it passes over
// any "magic" or "interlocked" concept, it drops a pointer to where the
// last such occurred into the external variable "last_magic_diamond". */
//
// If not checking for errors, we set the "final" bits correctly
// (we can always do that, since the complex bit field stuff isn't
// used in this), and we just set anything in the "herit" field.
// In fact, we set something in the "herit" field even if this item
// was "final".  Doing all this gives the required behavior, which is:
//
// (1) If any concept at all, final or herit, the herit field
//    will be nonzero.
// (2) The herit field is otherwise messed up.  Only zero/nonzero matters.
// (3) In any case the "final" field will be correct. */

extern parse_block *process_final_concepts(
   parse_block *cptr,
   bool check_errors,
   final_and_herit_flags *final_concepts,
   bool forbid_unfinished_parse,
   const char *filename,
   int linenum) THROW_DECL
{
   for ( ; cptr ; cptr=cptr->next) {
      finalflags the_final_bit;
      uint32 forbidfinalbit = 0;
      heritflags heritsetbit = (heritflags) 0;
      uint32 forbidheritbit = 0;

      switch (cptr->concept->kind) {
      case concept_comment:
         continue;               // Skip comments.
      case concept_triangle:
         the_final_bit = FINAL__TRIANGLE;
         forbidfinalbit = FINAL__LEADTRIANGLE;
         goto new_final;
      case concept_leadtriangle:
         the_final_bit = FINAL__LEADTRIANGLE;
         forbidfinalbit = FINAL__TRIANGLE;
         goto new_final;
      case concept_magic:
         last_magic_diamond = cptr;
         heritsetbit = INHERITFLAG_MAGIC;
         forbidheritbit = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
         break;
      case concept_interlocked:
         last_magic_diamond = cptr;
         heritsetbit = INHERITFLAG_INTLK;
         forbidheritbit = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
         break;
      case concept_grand:
         heritsetbit = INHERITFLAG_GRAND;
         forbidheritbit = INHERITFLAG_SINGLE;
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
         heritsetbit = (heritflags) cptr->concept->arg1; break;
      case concept_split:
         the_final_bit = FINAL__SPLIT;
         goto new_final;
      case concept_12_matrix:
         if (check_errors && (final_concepts->test_herit_and_final_bits()))
            fail("Matrix modifier must appear first.");
         heritsetbit = INHERITFLAG_12_MATRIX;
         break;
      case concept_16_matrix:
         if (check_errors && (final_concepts->test_herit_and_final_bits()))
            fail("Matrix modifier must appear first.");
         heritsetbit = INHERITFLAG_16_MATRIX;
         break;
      case concept_diamond:
         heritsetbit = INHERITFLAG_DIAMOND;
         forbidheritbit = INHERITFLAG_SINGLE;
         break;
      case concept_funny:
         heritsetbit = INHERITFLAG_FUNNY; break;
      default:
         goto exit5;
      }

      // At this point we have a "herit" concept.

      if (check_errors) {
         if (final_concepts->test_heritbits(forbidheritbit))
            fail("Illegal order of call modifiers.");

         if (do_heritflag_merge(final_concepts, heritsetbit))
            fail("Illegal combination of call modifiers.");
      }
      else {
         /* If not checking for errors, we just have to set the "herit" field
            nonzero. */

         final_concepts->set_heritbits(heritsetbit);
      }

      goto check_level;

   new_final:

      // This is a "final" concept.  It is more straightforward, because the word
      // is not broken into fields that need special checking.

      if (check_errors) {
         if ((final_concepts->final & (the_final_bit|forbidfinalbit)))
            fail("Redundant call modifier.");
      }
      else {
         // Put the bit into the "herit" stuff also, so that "herit" will tell
         // whether any modifier at all was seen.  Only the zero/nonzero nature
         // of the "herit" word will be looked at if we are not checking for errors.

         final_concepts->set_heritbit(INHERITFLAG_DIAMOND);
      }

      final_concepts->set_finalbit(the_final_bit);

   check_level:

      if (check_errors && cptr->concept->level > calling_level)
         warn(warn__bad_concept_level);
   }

   if (forbid_unfinished_parse) fail_no_retry("Incomplete parse.");

   exit5:

   return cptr;
}


// This writes over its 2nd and 3rd arguments.
extern parse_block *really_skip_one_concept(
   parse_block *incoming,
   parse_block * & kkk,
   uint32 & need_to_restrain,   // 1=(if not doing echo), 2=(yes, always)
   parse_block ***parseptr_skip_p) THROW_DECL
{
   final_and_herit_flags junk_concepts;
   parse_block *parseptrcopy;

   while (incoming->concept->kind == concept_comment)
      incoming = incoming->next;

   junk_concepts.clear_all_herit_and_final_bits();

   parseptrcopy = process_final_concepts(incoming, false, &junk_concepts, true, __FILE__, __LINE__);

   // Find out whether the next concept (the one that will be "random" or whatever)
   // is a modifier or a "real" concept.

   if (junk_concepts.test_herit_and_final_bits()) {
      parseptrcopy = incoming;
   }
   else if (parseptrcopy->concept) {
      concept_kind kk = parseptrcopy->concept->kind;

      if (check_for_supercall(parseptrcopy)) {
         *parseptr_skip_p = &parseptrcopy->next->subsidiary_root;
         kkk = &pbsuper;

         /* We don't restrain for echo with supercalls, because echo doesn't pull parts
            apart, and supercalls don't work with multiple-part calls as the target
            for which they are restraining the concept. */

         need_to_restrain = 1;
         return parseptrcopy;
      }

      if (concept_table[kk].concept_action == 0)
         fail("Sorry, can't do this with this concept.");

      if ((concept_table[kk].concept_prop & CONCPROP__SECOND_CALL) &&
          parseptrcopy->concept->kind != concept_special_sequential)
         fail("Can't use a concept that takes a second call.");
   }

   (void) check_for_concept_group(parseptrcopy, kkk, need_to_restrain, parseptr_skip_p);

   return parseptrcopy;
}



/* s_2x2_2x4 is duplicated in the big table. */
const expand::thing s_2x2_2x4 = {{1, 2, 5, 6}, 4, s2x2, s2x4, 0};
const expand::thing s_2x4_qtg = {{3, 4, -1, -1, 7, 0, -1, -1}, 8, s_qtag, s2x4, 3};

// Prepare several setups to be assembled into one, by making them all have
// the same kind and rotation.  If there is a question about what ending
// setup to opt for (because of lots of phantoms), use "goal".

// Goal of -1 means opt for 1x4.
// Goal of +9 means opt for dmd.
// Goal of -2 means preserve the internal setups.

// No external client actually uses -2.  We change -1 to -2 here.
// The program used to work much harder (too hard, in fact) at
// opting for a 1x4 rather than a diamond.  It's much smarter
// now, but the old "-1" code is left in for reference.
// One of the hard test cases is, from a tidal wave,
// Split Phantom Diamonds Diamond Chain Thru.
// There is no code to track the phantoms through the cast off 3/4.
// Instead, merge_setups notices that it is merging "nothing" with
// diamond points, and changes the diamond points to ends of lines.
// It's not clear that this is the wisest way to do this.

extern bool fix_n_results(int arity, int goal, setup z[],
                          uint32 & rotstates,
                          uint32 & pointclip) THROW_DECL
{
   int i;

   if (goal == -1) goal = -2;

   int lineflag = 0;
   bool dmdflag = false;
   bool qtflag = false;
   bool boxrectflag = false;
   bool miniflag = false;
   int deadconcindex = -1;
   setup_kind kk = nothing;
   rotstates = 0xFFF;
   pointclip = 0;
   static uint16 rotstate_table[16] = {
      0x111, 0x222, 0x404, 0x808,
      0x421, 0x812, 0x104, 0x208,
      0x111, 0x222, 0x404, 0x808,
      0x421, 0x812, 0x104, 0x208};

   // There are 3 things that make this task nontrivial.  First, some setups could
   // be "nothing", in which case we turn them into the same type of setup as
   // their neighbors, with no people.  Second, some types of "grand working"
   // can leave setups confused about whether they are lines or diamonds,
   // because only the ends/points are occupied.  We turn those setups into
   // whatever matches their more-fully-occupied neighbors.  (If completely in
   // doubt, we opt for 1x4's.)  Third, some 1x4's may have been shrunk to 1x2's.
   // This can happen in some cases of "triple box patch the so-and-so", which lead
   // to triple lines in which the outer lines collapsed from 1x4's to 1x2's while
   // the center line is fully occupied.  In this case we repopulate the outer lines
   // to 1x4's.

   for (i=0; i<arity; i++) {

      // First, check that all setups have the same "eighth rotation" stuff,
      // and don't have any "imprecise rotation".

      if ((z[i].result_flags.misc & RESULTFLAG__IMPRECISE_ROT) ||
          (i > 0 && (z[0].result_flags.misc ^ z[i].result_flags.misc) & RESULTFLAG__PLUSEIGHTH_ROT))
         goto lose;

      if (z[i].kind == s_normal_concentric) {

         // We definitely have a problem.  A common way for this to happen
         // is if a concentric call was done and there were no ends,
         // so we don't know what the setup really is.
         // Example:  from normal columns, do a split phantom lines hocus-pocus.
         // Do we leave space for the phantoms?  Humans would probably say yes
         // because they know where the phantoms went, but this program has no
         // idea in general.  If a call is concentrically executed and the
         // outsides are all phantoms, we don't know what the setup is.
         // Concentric_move signifies this by creating a "concentric" setup
         // with "nothing" for the outer setup.  So we raise an error that is
         // somewhat descriptive.

         if (z[i].outer.skind == nothing)
            continue;  // Defer this until later; we may be able to figure something out.
         else if (z[i].inner.skind == nothing && z[i].outer.skind == s1x2) {

            // We can fix this up.  Just turn it into a 1x4 with the ends missing.
            // (If a diamond is actually required, that will get fixed up below.)
            // The test case for this is split phantom lines cross to a diamond from 2FL.

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
         continue;  // Defer this until later; we may be able to figure something out.
      }

      if (z[i].kind != nothing) {
         int zisrot;
         canonicalize_rotation(&z[i]);
         zisrot = z[i].rotation & 3;

         if (z[i].kind == s1x2)
            miniflag = true;
         else if (z[i].kind == s1x4 && (z[i].people[1].id1 | z[i].people[3].id1) == 0)
            lineflag |= 1;
         else if (z[i].kind == sdmd && (z[i].people[1].id1 | z[i].people[3].id1) == 0)
            lineflag |= 2;
         else {
            if (kk == nothing) kk = z[i].kind;

            if (kk != z[i].kind) {
               // We may have a minor problem -- differently oriented
               // 2x4's and qtag's with just the ends present might want to be
               // like each other.  We will turn them into qtags.
               // Or 1x4's with just the centers present might want
               // to become diamonds.

               if (((kk == s2x4 && z[i].kind == s_qtag) ||
                    (kk == s_qtag && z[i].kind == s2x4))) {
                  qtflag = true;
                  zisrot ^= 1;
               }
               else if (((kk == s2x4 && z[i].kind == s2x2) ||
                         (kk == s2x2 && z[i].kind == s2x4))) {
                  boxrectflag = true;
               }
               else if (((kk == s1x4 && z[i].kind == sdmd) ||
                         (kk == sdmd && z[i].kind == s1x4))) {
                  dmdflag = true;
                  zisrot ^= 1;
               }
               else
                  goto lose;
            }
         }

         // If the setups are "trngl" or "trngl4", the rotations have
         // to alternate by 180 degrees.

         if (z[i].kind == s2x2)
            rotstates &= 0x0FF;
         else {
            int shit = i;
            if (arity == 4 && (i&2)) shit ^= 1;

            if (z[i].kind == s_trngl || z[i].kind == s_trngl4)
               rotstates &= 0xF00;
            else
               rotstates &= 0x033;

            rotstates &= rotstate_table[((shit & 3) << 2) | zisrot];
         }
      }
   }

   if (boxrectflag) kk = s2x4;

   if (kk == nothing) {
      // If client really needs a diamond, return a diamond.
      // Otherwise opt for 1x4.
      if (lineflag != 0) {
         if (goal == 9) kk = sdmd;
         else if (goal == -1) kk = s1x4;
         else if (lineflag == 1)  // But a goal of -2 means use whatever the setups were.
            kk = s1x4;
         else if (lineflag == 2)
            kk = sdmd;
         else fail("Can't do this: don't know where the phantoms went.");
      }
      else if (miniflag) kk = s1x2;
   }
   else if (kk == s2x2) {
      rotstates &= 0x011;
      // If the arity is 3 or more, we shut off the second digit.
      // The second digit is for things that are presumed to be nonisotropic.
      // A bunch of 2x2's can't be nonisotropic.
      if (arity >=2) rotstates &= 1;
   }

   if (arity == 1) rotstates &= 0x3;
   if (!rotstates) goto lose;

   // Now deal with any setups that we may have deferred.

   if (dmdflag && kk == s1x4) {
      rotstates ^= 3;
      kk = sdmd;
   }
   else if (qtflag && kk == s2x4) {
      rotstates ^= 3;
      kk = s_qtag;
   }

   for (i=0; i<arity; i++) {
      if (z[i].kind == s_dead_concentric ||
          (z[i].kind == s_normal_concentric && z[i].outer.skind == nothing)) {
         int rr;

         if (z[i].inner.skind == s2x2 && kk == s2x4) {
            // Turn the 2x2 into a 2x4.  Need to make it have same rotation as the others;
            // that is, rotation = rr.  (We know that rr has something in it by now.)

            // We might have a situation with alternating rotations, or we
            // might have homogeneous rotations.  If the state says we could
            // have both, change it to just homogeneous.  That is, we default
            // to same rotation unless live people force mixed rotation.

            if (rotstates & 0x0F) rotstates &= 0x03;     // That does the defaulting.
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
            // Turn the 1x4 into a qtag.
            if (rotstates & 0x0F) rotstates &= 0x03;     // That does the defaulting.
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
                  (deadconcindex < 0 ||
                   (z[i].inner.skind == z[deadconcindex].inner.skind &&
                    z[i].inner.srotation == z[deadconcindex].inner.srotation))) {
            deadconcindex = i;
         }
         else
            fail("Can't do this: don't know where the phantoms went.");
      }
      else if (qtflag && z[i].kind == s2x4) {
         // Turn the 2x4 into a qtag.
         if (z[i].people[1].id1 | z[i].people[2].id1 |
             z[i].people[5].id1 | z[i].people[6].id1) goto lose;
         expand::compress_setup(&s_2x4_qtg, &z[i]);
      }
      else if (dmdflag && z[i].kind == s1x4) {
         // Turn the 1x4 into a diamond.
         if (z[i].people[0].id1 | z[i].people[2].id1) goto lose;
         expand::compress_setup(&s_1x4_dmd, &z[i]);
         pointclip |= 1 << i;
      }
      else if (boxrectflag && z[i].kind == s2x2) {
         if (rotstates & 2) {
            z[i].rotation--;
            canonicalize_rotation(&z[i]);
         }

         expand::expand_setup(&s_2x2_2x4, &z[i]);
      }

      canonicalize_rotation(&z[i]);
   }

   if (deadconcindex >= 0) {
      kk = z[deadconcindex].kind;
      rotstates = 0x001;
   }

   if (kk == nothing) return true;

   // If something wasn't sure whether it was points of a diamond or
   // ends of a 1x4, that's OK if something else had a clue.
   if (lineflag != 0 && kk != s1x4 && kk != sdmd) goto lose;

   // If something was a 1x2, that's OK if something else was a 1x4.
   if (miniflag && kk != s1x4 && kk != s1x2) goto lose;

   for (i=0; i<arity; i++) {
      if (z[i].kind == nothing)
         clear_people(&z[i]);
      else if (z[i].kind == s1x2 && kk == s1x4) {
         // We have to expand a 1x2 to the center spots of a 1x4.
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

      // We know rotstates has a nonzero bit in an appropriate field.

      if (z[i].kind == s_trngl || z[i].kind == s_trngl4) {
         z[i].rotation = i << 1;
         if (rotstates & 0xC00) z[i].rotation += 2;
         if (rotstates & 0xA00) z[i].rotation++;
         z[i].rotation &= 3;
      }
      else
         z[i].rotation = (rotstates >> 1) & 1;
   }

   return false;

   lose:

   fail("This is an inconsistent shape or orientation changer.");
   return false;
}


// Some resolves are only legal at certain levels.  We define
// these short names to keep the table entries from being unwieldy.
#define MS l_mainstream
#define XB cross_by_level
#define DX dixie_grand_level
#define EX extend_34_level

const resolve_tester configuration::null_resolve = {
   resolve_none, MS, 0, {0,0,0,0,0,0,0,0}, 0};

// Notes for "distance" field:
// Add 0x10 bit for singer-only; these must be last.
// Also, last item in each table has 0x10 only.
// Add 0x20 bit to indicate that we demand only nonzero distances.
// Add 0x40 bit to make the resolver never find this, though
//    we will display it if user gets here.

static const resolve_tester test_thar_stuff[] = {
   {resolve_rlg,            MS, 2,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8A31A813},
   {resolve_minigrand,      MS, 4,   {5, 0, 3, 6, 1, 4, 7, 2},     0x8833AA11},
   {resolve_prom,           MS, 6,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8833AA11},
   {resolve_slipclutch_rlg, MS, 1,   {5, 2, 3, 0, 1, 6, 7, 4},     0x8138A31A},
   {resolve_la,             MS, 5,   {5, 2, 3, 0, 1, 6, 7, 4},     0xA31A8138},
   {resolve_slipclutch_la,  MS, 6,   {5, 4, 3, 2, 1, 0, 7, 6},     0xA8138A31},
   {resolve_xby_rlg,        XB, 1,   {4, 3, 2, 1, 0, 7, 6, 5},     0x8138A31A},
   {resolve_revprom,        MS, 4,   {2, 3, 0, 1, 6, 7, 4, 5},     0x118833AA},
   {resolve_xby_la,         XB, 4,   {2, 3, 0, 1, 6, 7, 4, 5},     0x138A31A8},
   {resolve_dixie_grand,    DX, 0,   {4, 1, 2, 7, 0, 5, 6, 3},     0xAA118833},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_4x4_stuff[] = {
   // "circle left/right" from squared-set, normal.
   {resolve_circle,         MS, 6,   {2, 1, 14, 13, 10, 9, 6, 5},  0x33AA1188},
   // "circle left/right" from squared-set, sashayed.
   {resolve_circle,         MS, 7,   {5, 2, 1, 14, 13, 10, 9, 6},  0x833AA118},

   // From vertical 8-chain in "O".
   {resolve_rlg,            MS, 3,   {5, 2, 1, 14, 13, 10, 9, 6},  0x8A8AA8A8},
   {resolve_la,             MS, 6,   {2, 1, 14, 13, 10, 9, 6, 5},  0xA8AA8A88},

   // From horizontal 8-chain in "O".
   {resolve_rlg,            MS, 3,   {5, 2, 1, 14, 13, 10, 9, 6},  0x13313113},
   {resolve_la,             MS, 6,   {2, 1, 14, 13, 10, 9, 6, 5},  0x33131131},

   // Weird ones from squared set.
   {resolve_rlg,            MS, 3,   {5, 2, 1, 14, 13, 10, 9, 6},  0x8A31A813},
   {resolve_rlg,            MS, 3,   {5, 2, 1, 14, 13, 10, 9, 6},  0x138A31A8},
   {resolve_la,             MS, 6,   {2, 1, 14, 13, 10, 9, 6, 5},  0x38A31A81},
   {resolve_la,             MS, 6,   {2, 1, 14, 13, 10, 9, 6, 5},  0xA31A8138},

   // From squared set, around the corner.
   {resolve_rlg,            MS, 3,   {5, 2, 1, 14, 13, 10, 9, 6},  0x1A8138A3},
   {resolve_la,             MS, 6,   {2, 1, 14, 13, 10, 9, 6, 5},  0xA8138A31},

   // From squared set, facing directly.
   {resolve_rlg,            MS, 2,   {2, 1, 14, 13, 10, 9, 6, 5},  0x8A31A813},
   {resolve_la,             MS, 7,   {5, 2, 1, 14, 13, 10, 9, 6},  0x38A31A81},

   // From ladder columns, facing directly.
   {resolve_rlg,            MS, 3,   {7, 2, 0, 14, 15, 10, 8, 6},  0x13313113},
   {resolve_rlg,            MS, 1,   {3, 14, 12, 10, 11, 6, 4, 2}, 0x8AA8A88A},
   {resolve_rlg,            MS, 3,   {5, 4, 1, 3, 13, 12, 9, 11},  0x13313113},
   {resolve_rlg,            MS, 1,   {1, 0, 13, 15, 9, 8, 5, 7},   0x8AA8A88A},
   {resolve_la,             MS, 6,   {2, 0, 14, 15, 10, 8, 6, 7},  0x33131131},
   {resolve_la,             MS, 4,   {14, 12, 10, 11, 6, 4, 2, 3}, 0xAA8A88A8},
   {resolve_la,             MS, 6,   {4, 1, 3, 13, 12, 9, 11, 5},  0x33131131},
   {resolve_la,             MS, 4,   {0, 13, 15, 9, 8, 5, 7, 1},   0xAA8A88A8},

   // From pinwheel, facing directly.
   {resolve_rlg,            MS, 3,   {7, 2, 3, 14, 15, 10, 11, 6}, 0x138A31A8},
   {resolve_rlg,            MS, 3,   {5, 7, 1, 3, 13, 15, 9, 11},  0x8A31A813},

   // From pinwheel, all in miniwaves.
   {resolve_rlg,            MS, 3,   {7, 5, 3, 1, 15, 13, 11, 9},  0x138A31A8},
   {resolve_rlg,            MS, 3,   {7, 2, 3, 14, 15, 10, 11, 6}, 0x8A31A813},

   // From pinwheel, some of each.
   {resolve_rlg,            MS, 3,   {7, 2, 3, 14, 15, 10, 11, 6}, 0x13313113},
   {resolve_rlg,            MS, 3,   {5, 7, 3, 1, 13, 15, 11, 9},  0x8A8AA8A8},

   // From pinwheel, others of each.
   {resolve_rlg,            MS, 3,   {7, 2, 3, 14, 15, 10, 11, 6}, 0x8A8AA8A8},
   {resolve_rlg,            MS, 3,   {7, 5, 1, 3, 15, 13, 9, 11},  0x13313113},

   // From clumps.
   {resolve_rlg,            MS, 2,   {3, 1, 14, 0, 11, 9, 6, 8},   0x8A8AA8A8},
   {resolve_rlg,            MS, 4,   {5, 4, 7, 2, 13, 12, 15, 10}, 0x8A8AA8A8},
   {resolve_rlg,            MS, 2,   {1, 0, 3, 14, 9, 8, 11, 6},   0x31311313},
   {resolve_rlg,            MS, 4,   {7, 5, 2, 4, 15, 13, 10, 12}, 0x13133131},
   {resolve_la,             MS, 5,   {3, 0, 14, 9, 11, 8, 6, 1},   0xA8AA8A88},
   {resolve_la,             MS, 7,   {5, 2, 7, 12, 13, 10, 15, 4}, 0xA8AA8A88},
   {resolve_la,             MS, 5,   {1, 14, 3, 8, 9, 6, 11, 0},   0x13113133},
   {resolve_la,             MS, 7,   {7, 4, 2, 13, 15, 12, 10, 5}, 0x31331311},

   // From blocks.
   {resolve_rlg,            MS, 3,   {5, 2, 3, 0, 13, 10, 11, 8},  0x8A8AA8A8},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_4x6_stuff[] = {
   {resolve_rlg,            MS, 2,   {23, 6, 3, 2, 11, 18, 15, 14},0x8A31A813},
   {resolve_la,             MS, 7,   {14, 23, 6, 3, 2, 11, 18, 15},0x38A31A81},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_c1phan_stuff[] = {
   // From phantoms, all facing.
   {resolve_rlg,            MS, 3,   {10, 8, 6, 4, 2, 0, 14, 12},  0x138A31A8},
   {resolve_rlg,            MS, 3,   {9, 11, 5, 7, 1, 3, 13, 15},  0x8A31A813},
   // From phantoms, all in miniwaves.
   {resolve_rlg,            MS, 3,   {11, 9, 7, 5, 3, 1, 15, 13},  0x138A31A8},
   {resolve_rlg,            MS, 3,   {10, 8, 6, 4, 2, 0, 14, 12},  0x8A31A813},
   // From phantoms, some of each.
   {resolve_rlg,            MS, 3,   {10, 8, 6, 4, 2, 0, 14, 12},  0x13313113},
   {resolve_rlg,            MS, 3,   {9, 11, 7, 5, 1, 3, 15, 13},  0x8A8AA8A8},
   // From phantoms, others of each.
   {resolve_rlg,            MS, 3,   {10, 8, 6, 4, 2, 0, 14, 12},  0x8A8AA8A8},
   {resolve_rlg,            MS, 3,   {11, 9, 5, 7, 3, 1, 13, 15},  0x13313113},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_galaxy_stuff[] = {
   {resolve_rlg,            MS, 2,   {5, 4, 3, 2, 1, 0, 7, 6},     0x1A313813},
   {resolve_rlg,            MS, 2,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8A81A8A3},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_qtag_stuff[] = {
   // From 1/4 tag.
   {resolve_dixie_grand,    DX, 2,   {5, 0, 2, 7, 1, 4, 6, 3},     0x8AAAA888},
   {resolve_ext_rlg,        MS, 5,   {7, 5, 4, 2, 3, 1, 0, 6},     0xA88A8AA8},
   {resolve_ext_la,         MS, 6,   {3, 2, 1, 0, 7, 6, 5, 4},     0xA8AA8A88},
   // From 3/4 tag.
   {resolve_rlg,            MS, 4,   {5, 4, 3, 2, 1, 0, 7, 6},     0xAA8A88A8},
   {resolve_la,             MS, 7,   {4, 2, 3, 1, 0, 6, 7, 5},     0xA8A88A8A},

   // From diamonds with points facing each other.
   {resolve_rlg,            MS, 4,   {5, 4, 3, 2, 1, 0, 7, 6},     0x138A31A8},
   {resolve_la,             MS, 7,   {4, 2, 3, 1, 0, 6, 7, 5},     0x38A31A81},

   // From "6x2 acey deucey" type of 1/4 tag.
   {resolve_rlg,            MS, 4,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8A8AA8A8},

   // Singers only.
   // Swing/prom from 3/4 tag, ends sashayed (normal case is above).
   {resolve_rlg,            MS, 0x14,{4, 5, 3, 2, 0, 1, 7, 6},     0xAA8A88A8},
   // Swing/prom from 3/4 tag, centers traded, ends normal.
   {resolve_rlg,            MS, 0x14,{5, 4, 2, 3, 1, 0, 6, 7},     0xAAA8888A},
   // Swing/prom from 3/4 tag, centers traded, ends sashayed.
   {resolve_rlg,            MS, 0x14,{4, 5, 2, 3, 0, 1, 6, 7},     0xAAA8888A},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_2x6_stuff[] = {
   // From "Z" 8-chain.
   {resolve_rlg,            MS, 3,   {7, 6, 4, 3, 1, 0, 10, 9},    0x13313113},
   {resolve_rlg,            MS, 3,   {8, 7, 5, 4, 2, 1, 11, 10},   0x13313113},
   {resolve_la,             MS, 6,   {6, 4, 3, 1, 0, 10, 9, 7},    0x33131131},
   {resolve_la,             MS, 6,   {7, 5, 4, 2, 1, 11, 10, 8},   0x33131131},
   // From outer triple boxes.
   {resolve_rlg,            MS, 3,   {7, 6, 4, 5, 1, 0, 10, 11},   0x8A8AA8A8},
   {resolve_la,             MS, 6,   {7, 5, 4, 0, 1, 11, 10, 6},   0xA8AA8A88},
   // From parallelogram waves.
   {resolve_rlg,            MS, 3,   {7, 6, 2, 3, 1, 0, 8, 9},     0x8A8AA8A8},
   {resolve_la,             MS, 6,   {7, 3, 2, 0, 1, 9, 8, 6},     0xA8AA8A88},
   {resolve_rlg,            MS, 3,   {9, 8, 4, 5, 3, 2, 10, 11},   0x8A8AA8A8},
   {resolve_la,             MS, 6,   {9, 5, 4, 2, 3, 11, 10, 8},   0xA8AA8A88},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_3x4_stuff[] = {
   // From offset waves.
   {resolve_rlg,            MS, 3,   {7, 6, 5, 4, 1, 0, 11, 10},   0x8A8AA8A8},
   {resolve_rlg,            MS, 3,   {5, 4, 2, 3, 11, 10, 8, 9},   0x8A8AA8A8},
   {resolve_la,             MS, 6,   {7, 4, 5, 0, 1, 10, 11, 6},   0xA8AA8A88},
   {resolve_la,             MS, 6,   {5, 3, 2, 10, 11, 9, 8, 4},   0xA8AA8A88},
   // From sort of skewed offset 8-chain.
   // It may have been a mistake to write these.
   //   {resolve_rlg,       MS, 2,   {6, 4, 11, 1, 0, 10, 5, 7},   0x8A8AA8A8},
   //   {resolve_rlg,       MS, 2,   {4, 3, 5, 2, 11, 8, 10, 9},   0x8A8AA8A8},
   //   {resolve_la,        MS, 5,   {4, 11, 1, 0, 10, 5, 7, 6},   0xA8AA8A88},
   //   {resolve_la,        MS, 5,   {3, 2, 5, 10, 9, 8, 11, 4},   0xAA8A88A8},
   {resolve_rlg,            MS, 1,   {4, 5, 1, 0, 10, 11, 7, 6},   0x31311313},
   {resolve_rlg,            MS, 1,   {3, 2, 11, 10, 9, 8, 5, 4},   0x31311313},
   {resolve_la,             MS, 4,   {5, 1, 0, 10, 11, 7, 6, 4},   0x13113133},
   {resolve_la,             MS, 4,   {2, 11, 10, 9, 8, 5, 4, 3},   0x13113133},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_4dmd_stuff[] = {
   // These test for people in a miniwave as centers of the outer diamonds,
   // while the others are facing each other as points.  There are three of each
   // to allow for the outsides to be symmetrical (points of the center diamonds)
   // or unsymmetrical (points of a center diamond and the adjacent outer diamond).
   {resolve_la,             MS, 7,   {8, 4, 5, 1, 0, 12, 13, 9},   0x38A31A81},
   {resolve_la,             MS, 7,   {9, 4, 5, 2, 1, 12, 13, 10},  0x38A31A81},
   {resolve_la,             MS, 7,   {10, 4, 5, 3, 2, 12, 13, 11}, 0x38A31A81},
   {resolve_rlg,            MS, 4,   {9, 8, 5, 4, 1, 0, 13, 12},   0x138A31A8},
   {resolve_rlg,            MS, 4,   {10, 9, 5, 4, 2, 1, 13, 12},  0x138A31A8},
   {resolve_rlg,            MS, 4,   {11, 10, 5, 4, 3, 2, 13, 12}, 0x138A31A8},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_bigdmd_stuff[] = {
   // From  miniwaves in "common point diamonds".
   {resolve_rlg,            MS, 2,   {7, 6, 3, 2, 1, 0, 9, 8},     0x8A31A813},
   {resolve_rlg,            MS, 2,   {4, 5, 3, 2, 10, 11, 9, 8},   0x8A31A813},
   {resolve_la,             MS, 5,   {7, 2, 3, 0, 1, 8, 9, 6},     0xA31A8138},
   {resolve_la,             MS, 5,   {4, 2, 3, 11, 10, 8, 9, 5},   0xA31A8138},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_deepqtg_stuff[] = {
   {resolve_circle,         MS, 6,   {11, 2, 1, 0, 5, 8, 7, 6},    0x33AA1188},
   {resolve_circle,         MS, 7,   {6, 11, 2, 1, 0, 5, 8, 7},    0x833AA118},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_deepxwv_stuff[] = {
   {resolve_rlg,            MS, 4,   {5, 8, 7, 6, 11, 2, 1, 0},    0x138A31A8},
   {resolve_la,             MS, 7,   {8, 6, 7, 11, 2, 0, 1, 5},    0x38A31A81},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_3x6_stuff[] = {
   {resolve_rlg,            MS, 4,   {12, 11, 7, 6, 3, 2, 16, 15}, 0x138A31A8},
   {resolve_la,             MS, 7,   {11, 6, 7, 3, 2, 15, 16, 12}, 0x38A31A81},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_spindle_stuff[] = {
   // These test for people looking around the corner.
   {resolve_rlg,            MS, 3,   {4, 3, 2, 1, 0, 7, 6, 5},     0x13313113},
   {resolve_rlg,            MS, 3,   {4, 3, 2, 1, 0, 7, 6, 5},     0x1A313813},
   {resolve_la,             MS, 7,   {4, 3, 2, 1, 0, 7, 6, 5},     0x33131131},
   {resolve_la,             MS, 7,   {4, 3, 2, 1, 0, 7, 6, 5},     0x38131A31},
   {resolve_rlg,            MS, 2,   {3, 2, 1, 0, 7, 6, 5, 4},     0x31311313},
   {resolve_rlg,            MS, 2,   {3, 2, 1, 0, 7, 6, 5, 4},     0x8131A313},
   {resolve_la,             MS, 6,   {3, 2, 1, 0, 7, 6, 5, 4},     0x33131131},
   {resolve_la,             MS, 6,   {3, 2, 1, 0, 7, 6, 5, 4},     0xA3138131},
   {resolve_circle,         MS, 7,   {4, 3, 2, 1, 0, 7, 6, 5},     0x83AAA188},
   {resolve_circle,         MS, 5,   {3, 2, 1, 0, 7, 6, 5, 4},     0x3AAA1888},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_2x4_stuff[] = {
   // 8-chain.
   {resolve_rlg,            MS, 3,   {5, 4, 3, 2, 1, 0, 7, 6},     0x13313113},
   {resolve_minigrand,      MS, 5,   {5, 0, 3, 6, 1, 4, 7, 2},     0x11333311},
   {resolve_la,             MS, 6,   {4, 3, 2, 1, 0, 7, 6, 5},     0x33131131},
   {resolve_pth_rlg,        MS, 2,   {5, 2, 3, 0, 1, 6, 7, 4},     0x11313313},
   {resolve_pth_la,         MS, 5,   {2, 3, 0, 1, 6, 7, 4, 5},     0x13133131},
   {resolve_dixie_grand,    DX, 1,   {4, 1, 2, 7, 0, 5, 6, 3},     0x33111133},

   // Trade-by.
   {resolve_rlg,            MS, 2,   {4, 3, 2, 1, 0, 7, 6, 5},     0x11313313},
   {resolve_minigrand,      MS, 4,   {4, 7, 2, 5, 0, 3, 6, 1},     0x13333111},
   {resolve_la,             MS, 7,   {5, 4, 3, 2, 1, 0, 7, 6},     0x31131331},
   {resolve_tby_rlg,        MS, 3,   {6, 3, 4, 1, 2, 7, 0, 5},     0x11113333},
   {resolve_tby_la,         MS, 0,   {5, 6, 3, 4, 1, 2, 7, 0},     0x31111333},

   // Waves.
   {resolve_rlg,            MS, 3,   {5, 4, 2, 3, 1, 0, 6, 7},     0x8A8AA8A8},
   {resolve_minigrand,      MS, 5,   {5, 0, 2, 7, 1, 4, 6, 3},     0x8888AAAA},
   {resolve_la,             MS, 6,   {5, 3, 2, 0, 1, 7, 6, 4},     0xA8AA8A88},
   {resolve_ext_rlg,        EX, 2,   {5, 3, 2, 0, 1, 7, 6, 4},     0x8A88A8AA},
   {resolve_ext_la,         EX, 7,   {5, 4, 2, 3, 1, 0, 6, 7},     0xA8A88A8A},
   {resolve_circ_rlg,       MS, 1,   {5, 0, 2, 7, 1, 4, 6, 3},     0x8888AAAA},
   {resolve_circ_la,        MS, 0,   {5, 7, 2, 4, 1, 3, 6, 0},     0xAAA8888A},
   {resolve_xby_rlg,        XB, 2,   {4, 2, 3, 1, 0, 6, 7, 5},     0x8A88A8AA},
   {resolve_xby_la,         XB, 5,   {3, 2, 0, 1, 7, 6, 4, 5},     0xA88A8AA8},
   {resolve_dixie_grand,    DX, 0x27,{3, 6, 0, 5, 7, 2, 4, 1},     0xAA8888AA},

   // From T-bone setup, ends facing.
   {resolve_rlg,            MS, 2,   {4, 3, 2, 1, 0, 7, 6, 5},     0x8A31A813},
   {resolve_la,             MS, 7,   {5, 4, 3, 2, 1, 0, 7, 6},     0x38A31A81},

   // RLG from centers facing and ends in miniwaves.
   {resolve_rlg,            MS, 2,   {4, 3, 2, 1, 0, 7, 6, 5},     0x31311313},
   // LA from lines-out.  This has been decreed to be a horrible resolve.
   {resolve_la,             MS, 0x46,{4, 3, 2, 1, 0, 7, 6, 5},     0xA8888AAA},
   // From wacky T-bone.
   {resolve_la,             MS, 6,   {4, 3, 2, 1, 0, 7, 6, 5},     0x38131A31},

   // From 2FL.
   {resolve_prom,           MS, 7,   {5, 4, 2, 3, 1, 0, 6, 7},     0x8888AAAA},
   {resolve_revprom,        MS, 5,   {3, 2, 0, 1, 7, 6, 4, 5},     0xAA8888AA},

   // "circle left/right" from pseudo squared-set, normal.
   {resolve_circle,         MS, 6,   {4, 3, 2, 1, 0, 7, 6, 5},     0x33AA1188},
   // "circle left/right" from pseudo squared-set, sashayed.
   {resolve_circle,         MS, 7,   {5, 4, 3, 2, 1, 0, 7, 6},     0x833AA118},
   // "circle left/right" from lines-in, sashayed.
   {resolve_circle,         MS, 0x26,{4, 3, 2, 1, 0, 7, 6, 5},     0x8AAAA888},
   // "circle left/right" from lines-in, normal.
   {resolve_circle,         MS, 0x27,{5, 4, 3, 2, 1, 0, 7, 6},     0x88AAAA88},
   // From DPT.
   {resolve_dixie_grand,    DX, 2,   {5, 2, 4, 7, 1, 6, 0, 3},     0x33311113},

   // From magic column.
   {resolve_dixie_grand,    DX, 3,   {6, 3, 5, 0, 2, 7, 1, 4},     0x33331111},
   {resolve_dixie_grand,    DX, 1,   {4, 1, 3, 6, 0, 5, 7, 2},     0x33111133},

   // From columns.
   {resolve_sglfileprom,    MS, 7,   {5, 4, 3, 2, 1, 0, 7, 6},     0x11333311},
   {resolve_sglfileprom,    MS, 6,   {4, 3, 2, 1, 0, 7, 6, 5},     0x13333111},
   {resolve_revsglfileprom, MS, 7,   {5, 4, 3, 2, 1, 0, 7, 6},     0x33111133},
   {resolve_revsglfileprom, MS, 6,   {4, 3, 2, 1, 0, 7, 6, 5},     0x31111333},

   // From T-bone.
   {resolve_sglfileprom,    MS, 7,   {5, 4, 3, 2, 1, 0, 7, 6},     0x18833AA1},
   {resolve_sglfileprom,    MS, 6,   {4, 3, 2, 1, 0, 7, 6, 5},     0x8833AA11},
   {resolve_revsglfileprom, MS, 7,   {5, 4, 3, 2, 1, 0, 7, 6},     0x3AA11883},
   {resolve_revsglfileprom, MS, 6,   {4, 3, 2, 1, 0, 7, 6, 5},     0xAA118833},

   // From T-bone mixed 8-chain and waves.
   {resolve_rlg,            MS, 3,   {5, 4, 2, 3, 1, 0, 6, 7},     0x138A31A8},
   {resolve_rlg,            MS, 3,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8A31A813},
   {resolve_la,             MS, 6,   {4, 3, 2, 1, 0, 7, 6, 5},     0x38A31A81},
   {resolve_la,             MS, 6,   {5, 3, 2, 0, 1, 7, 6, 4},     0xA31A8138},

   // Singers only.
   // Swing/prom from waves, boys looking in.
   {resolve_rlg,            MS,   0x13,   {5, 4, 3, 2, 1, 0, 7, 6},     0x8AA8A88A},
   // Swing/prom from waves, girls looking in.
   {resolve_rlg,            MS,   0x13,   {4, 5, 2, 3, 0, 1, 6, 7},     0xA88A8AA8},
   // Swing/prom from lines-out.
   {resolve_rlg,            MS,   0x13,   {4, 5, 2, 3, 0, 1, 6, 7},     0xAA8888AA},
   // Same as cross-by-LA from waves (above), but it's mainstream here.
   {resolve_rlg,            MS,   0x11,   {3, 2, 0, 1, 7, 6, 4, 5},     0xA88A8AA8},
   // 8-chain, boys in center.
   {resolve_rlg,            MS,   0x13,   {5, 4, 2, 3, 1, 0, 6, 7},     0x13133131},
   // 8-chain, girls in center.
   {resolve_rlg,            MS,   0x11,   {3, 2, 0, 1, 7, 6, 4, 5},     0x31131331},
   // Trade-by, ends sashayed.
   {resolve_rlg,            MS,   0x12,   {4, 3, 1, 2, 0, 7, 5, 6},     0x11133331},
   // trade-by, centers sashayed.
   {resolve_rlg,            MS,   0x14,   {6, 5, 3, 4, 2, 1, 7, 0},     0x13113133},
   {resolve_none, MS, 0x10}};

static const resolve_tester test_hrgl_stuff[] = {
   {resolve_rlg,            MS, 5,   {6, 5, 7, 4, 2, 1, 3, 0},     0xA3138131},
   {resolve_la,             MS, 1,   {6, 5, 7, 4, 2, 1, 3, 0},     0x8131A313},
   {resolve_none, MS, 0x10}};



void configuration::calculate_resolve()
{
   const resolve_tester *testptr;
   int i;
   uint32 singer_offset = 0;

   if (ui_options.singing_call_mode == 1) singer_offset = 0600;
   else if (ui_options.singing_call_mode == 2) singer_offset = 0200;

   switch (state.kind) {
   case s2x4:
      testptr = test_2x4_stuff; break;
   case s3x4:
      testptr = test_3x4_stuff; break;
   case s2x6:
      testptr = test_2x6_stuff; break;
   case s_qtag:
      testptr = test_qtag_stuff; break;
   case s_hrglass:
      testptr = test_hrgl_stuff; break;
   case s4dmd:
      testptr = test_4dmd_stuff; break;
   case sbigdmd:
      testptr = test_bigdmd_stuff; break;
   case sdeepqtg:
      testptr = test_deepqtg_stuff; break;
   case sdeepxwv:
      testptr = test_deepxwv_stuff; break;
   case s3x6:
      testptr = test_3x6_stuff; break;
   case s4x4:
      testptr = test_4x4_stuff; break;
   case s4x6:
      testptr = test_4x6_stuff; break;
   case s_c1phan:
      testptr = test_c1phan_stuff; break;
   case s_galaxy:
      testptr = test_galaxy_stuff; break;
   case s_crosswave: case s_thar:
      // This makes use of the fact that the person numbering
      // in crossed lines and thars is identical.
      testptr = test_thar_stuff; break;
   case s_spindle:
      testptr = test_spindle_stuff; break;
   default: goto no_resolve;
   }

   do {
      uint32 directionword;
      uint32 firstperson = state.people[testptr->locations[0]].id1 & 0700;
      if (firstperson & 0100) goto not_this_one;

      // We run the tests in descending order, because the test for i=0 is especially
      // likely to pass (since the person ID is known to match), and we want to find
      // failures as quickly as possible.

      for (i=7,directionword=testptr->directions ; i>=0 ; i--,directionword>>=4) {
         uint32 expected_id = (i << 6) + ((i&1) ? singer_offset : 0);

         // The adds of "expected_id" and "firstperson" may overflow out of the "700" bits
         // into the next 2 bits.  (One bit for each add.)

         if ((state.people[testptr->locations[i]].id1 ^
              (expected_id + firstperson + (directionword & 0xF))) &
             0777)
            goto not_this_one;
      }

      if (calling_level < testptr->level_needed ||
          (testptr->k == resolve_minigrand && !allowing_minigrand)) goto not_this_one;

      resolve_flag.the_item = testptr;
      resolve_flag.distance =
         ((state.rotation << 1) + (firstperson >> 6) + testptr->distance) & 7;
      if (resolve_flag.distance == 0 && (testptr->distance & 0x20)) goto not_this_one;
      return;

      not_this_one: ;
   }
   while (
          // always do next one if it doesn't have the singer-only mark.
          !((++testptr)->distance & 0x10) ||
          // Even if it has the mark, do it if this is a singer
          // and it isn't really the end of the table.
          (ui_options.singing_call_mode != 0 && testptr->k != resolve_none));

   // Too bad.

 no_resolve:

   init_resolve();   // Set it to the null resolve.
}


extern bool warnings_are_unacceptable(bool strict)
{
   // If we are doing a "standardize", we let ALL warnings pass.
   // We particularly want weird T-bones and other unusual sort of things.

   if (!strict) return false;

   // We only care about "bad" warnings.
   if (!configuration::test_multiple_warnings(no_search_warnings)) return false;

   // If the "allow all concepts" switch isn't on, we lose.
   if (!allowing_all_concepts) return true;

   // But if "allow all concepts" was given, and the only bad warning is
   // "bad_concept_level", we let it pass.

   // So we test for any bad warnings other than "warn__bad_concept_level".
   warning_info otherthanbadconc = configuration::save_warnings();
   otherthanbadconc.clearbit(warn__bad_concept_level);
   return otherthanbadconc.testmultiple(no_search_warnings);
}

bool expand::compress_from_hash_table(setup *ss,
                                      normalize_action action,
                                      uint32 livemask,
                                      bool noqtagcompress) THROW_DECL
{
   uint32 hash_num = (ss->kind * 25) & (expand::NUM_EXPAND_HASH_BUCKETS-1);

   const thing *cptr;

   for (cptr=compress_hash_table[hash_num] ; cptr ; cptr=cptr->next_compress) {
      if (cptr->outer_kind == ss->kind &&
          action >= cptr->action_level &&
          (cptr->biglivemask & livemask) == 0) {

         // Do not compress qtag to 2x3 is switch is on.
         if (noqtagcompress && cptr->outer_kind == s_qtag && cptr->inner_kind == s2x3)
            continue;

         warn(cptr->norwarning);
         expand::compress_setup(cptr, ss);
         return true;
      }
   }

   return false;
}


const expand::thing s_dmd_hrgl = {{6, 3, 2, 7}, 4, sdmd, s_hrglass, 0};
const expand::thing s_dmd_hrgl_disc = {{6, -1, 3, 2, -1, 7}, 6, s_1x2dmd, s_hrglass, 0};
/* s_1x2_dmd is duplicated in the big table. */
const expand::thing s_1x2_dmd = {{3, 1}, 2, s1x2, sdmd, 1};
const expand::thing s_1x2_hrgl = {{7, 3}, 2, s1x2, s_hrglass, 1};
const expand::thing s_dmd_323 = {{5, 7, 1, 3}, 4, sdmd, s_323, 1};

// The "action" argument tells how hard we work to remove the outside phantoms.
// When merging the results of "on your own" or "own the so-and-so",
// we set action=normalize_before_merge to work very hard at stripping away
// outside phantoms, so that we can see more readily how to put things together.
// When preparing for an isolated call, that is, "so-and-so do your part, whatever",
// we work at it a little, so we set action=normalize_before_isolated_call.
// For normal usage, we set action=simple_normalize.
extern void normalize_setup(setup *ss, normalize_action action, bool noqtagcompress)
   THROW_DECL
{
   int i;
   uint32 livemask, tbonetest, j;
   bool did_something = false;

 startover:

   for (i=0, j=1, livemask=0, tbonetest=0; i<=attr::slimit(ss); i++, j<<=1) {
      uint32 t = ss->people[i].id1;
      tbonetest |= t;
      if (t) livemask |= j;
   }

   if (ss->kind == sfat2x8)
      ss->kind = s2x8;     /* That's all it takes! */
   else if (ss->kind == swide4x4)
      ss->kind = s4x4;     /* That's all it takes! */
   else if (ss->kind == s_dead_concentric && action >= normalize_before_isolated_call) {
      ss->kind = ss->inner.skind;
      ss->rotation += ss->inner.srotation;
      did_something = true;
      goto startover;
   }

   if (action == normalize_compress_bigdmd) {
      if (ss->kind == sbigdmd || ss->kind == sbigptpd) {
         // They're the same!
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
         expand::compress_setup(&s_dmd_323, ss);

         if (action >= normalize_before_isolated_call) {
            if (ss->kind == sdmd && !(ss->people[0].id1 | ss->people[2].id1)) {
               expand::compress_setup(&s_1x2_dmd, ss);
            }
         }

         did_something = true;
         goto startover;
      }
   }

   // Next, search for simple things in the hash table.

   setup_kind oldk = ss->kind;

   bool b = expand::compress_from_hash_table(ss,
                                             action,
                                             livemask,
                                             noqtagcompress);

   if (!b) goto difficult;

   did_something = true;
   // If we compressed a "swqtag" to a "q_tag", take no further action.
   // Do not further compress to a 2x3.
   if (oldk == swqtag && ss->kind == s_qtag) goto kinda_done;
   goto startover;

 difficult:

   // A few difficult cases.

   // **** this map is externally visible
   if (ss->kind == s4x6) {
      if (!(ss->people[0].id1 | ss->people[11].id1 |
            ss->people[18].id1 | ss->people[17].id1 |
            ss->people[5].id1 | ss->people[6].id1 |
            ss->people[23].id1 | ss->people[12].id1)) {
         expand::compress_setup(&s_4x4_4x6a, ss);
         did_something = true;
         goto startover;
      }
   }

   if (!did_something && ss->kind == s4x4 && action == normalize_after_exchange_boxes) {
      if (!(tbonetest & 0x1)) {
         if ((livemask & 0x0030) == 0x0010) swap_people(ss, 4, 5);
         if ((livemask & 0x0140) == 0x0100) swap_people(ss, 6, 8);
         if ((livemask & 0x0084) == 0x0004) swap_people(ss, 2, 7);
         if ((livemask & 0x0A00) == 0x0200) swap_people(ss, 11, 9);
         if ((livemask & 0x000A) == 0x0002) swap_people(ss, 1, 3);
         if ((livemask & 0x8400) == 0x0400) swap_people(ss, 15, 10);
         if ((livemask & 0x4001) == 0x0001) swap_people(ss, 0, 14);
         if ((livemask & 0x3000) == 0x1000) swap_people(ss, 13, 12);
         action = simple_normalize;
         goto startover;
      }
      else if (!(tbonetest & 0x8)) {
         if ((livemask & 0x0003) == 0x0001) swap_people(ss, 0, 1);
         if ((livemask & 0x0014) == 0x0010) swap_people(ss, 2, 4);
         if ((livemask & 0x4008) == 0x4000) swap_people(ss, 14, 3);
         if ((livemask & 0x00A0) == 0x0020) swap_people(ss, 7, 5);
         if ((livemask & 0xA000) == 0x2000) swap_people(ss, 13, 15);
         if ((livemask & 0x0840) == 0x0040) swap_people(ss, 11, 6);
         if ((livemask & 0x1400) == 0x1000) swap_people(ss, 12, 10);
         if ((livemask & 0x0300) == 0x0100) swap_people(ss, 9, 8);
         action = simple_normalize;
         goto startover;
      }
   }
   else if (!did_something && ss->kind == s2x6 && action == normalize_after_exchange_boxes) {
      if ((livemask & 0xC00) == 0x800 && !(ss->people[11].id1 & 0x1)) swap_people(ss, 11, 10);
      if ((livemask & 0x0C0) == 0x040 && !(ss->people[ 6].id1 & 0x1)) swap_people(ss, 6, 7);
      if ((livemask & 0x030) == 0x020 && !(ss->people[ 5].id1 & 0x1)) swap_people(ss, 4, 5);
      if ((livemask & 0x003) == 0x001 && !(ss->people[ 0].id1 & 0x1)) swap_people(ss, 0, 1);
      action = simple_normalize;
      goto startover;
   }

   if (!did_something && ss->kind == s3x4 &&
       !(ss->people[0].id1 | ss->people[3].id1 |
         ss->people[6].id1 | ss->people[9].id1)) {
      expand::compress_setup(&s_qtg_3x4, ss);
      did_something = true;
      goto startover;
   }

   if (action >= normalize_to_4 &&
       action != normalize_after_triple_squash &&
       (ss->kind == s_hrglass || ss->kind == s_dhrglass)) {
      if (action >= normalize_to_2 && (livemask & 0x77) == 0) {
         // Be sure we match what the map says -- that might get checked someday.
         ss->kind = s_hrglass;
         expand::compress_setup(&s_1x2_hrgl, ss);
         did_something = true;
         goto startover;
      }
      else if ((livemask & 0x33) == 0)  {
         // If normalizing before a merge (which might be from a "disconnected"),
         // and it is a real hourglass, be sure we leave space.
         const expand::thing *t = (action == normalize_after_disconnected && ss->kind == s_hrglass) ?
            &s_dmd_hrgl_disc : &s_dmd_hrgl;

         // Be sure we match what the map says -- that might get checked someday.
         ss->kind = s_hrglass;
         expand::compress_setup(t, ss);
         did_something = true;
         goto startover;
      }
   }

 kinda_done:

   canonicalize_rotation(ss);
}



static void check_concept_parse_tree(parse_block *conceptptr) THROW_DECL
{
   for (;;) {

      if (!conceptptr)
         fail("Incomplete parse.");

      if (conceptptr->concept->kind <= marker_end_of_list) {
         if (!conceptptr->call)
            fail("Incomplete parse.");
         break;
      }
      else {
         if (concept_table[conceptptr->concept->kind].concept_prop & CONCPROP__SECOND_CALL) {
            check_concept_parse_tree(conceptptr->subsidiary_root);
         }

         conceptptr = conceptptr->next;
      }
   }
}


// Top level move routine.

void toplevelmove() THROW_DECL
{
   int i;

   setup starting_setup = configuration::current_config().state;
   configuration & newhist = configuration::next_config();
   parse_block *conceptptr = newhist.command_root;

   // Check for an incomplete parse.  This could happen if we do
   // something like "make a pass but [?".  There's just no way to make sure
   // that the parse tree is complete.

   check_concept_parse_tree(conceptptr);

   /* Be sure that the amount of written history that we consider to be safely
      written is less than the item we are about to change. */
   if (written_history_items > configuration::history_ptr)
      written_history_items = configuration::history_ptr;

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
   starting_setup.cmd.restrained_do_as_couples = false;
   starting_setup.cmd.restrained_super9flags = 0;
   starting_setup.cmd.restrained_fraction = 0;

   newhist.init_warnings_specific();

   // If we are starting a sequence with the "so-and-so into the center and do whatever"
   // flag on, and this call is a "sequence starter", take special action.

   if (configuration::current_config().get_startinfo_specific()->into_the_middle) {

      // If the call is a special sequence starter (e.g. spin a pulley) remove the implicit
      // "centers" concept and just do it.  The setup in this case will be a squared set
      // with so-and-so moved into the middle, which is what the encoding of these calls
      // wants.
      //
      // If the call is a "split square thru" or "split dixie style" type of call, and the
      // "split" concept has been given, possibly preceded by "left" we do the same.
      // We leave the "split" concept in place.  Other mechanisms will do the rest.
      //
      // This is made much more complicated by the fact that we want the examination
      // of the call to be transparent to modifiers, certain concepts (fractional,
      // stretch, maybe others will be added to this list in the future) and to
      // "<anything> and roll" types of suffixes.

      // There's this weird stuff about using "parse_state.topcallflags1"
      // instead of the obvious call flags.  I no longer remember what that
      // is for.  But we do it.  For the first round.
      uint32 callflags1_to_examine = parse_state.topcallflags1;
      parse_block *parse_scan = conceptptr->next;
      final_and_herit_flags finaljunk;
      finaljunk.clear_all_herit_and_final_bits();
      bool did_something = true;

      // Here's the loop that strips off the stuff that we need to strip off,
      // and checks the call.
      while (did_something) {
         did_something = false;
         if (callflags1_to_examine & (CFLAG1_SEQUENCE_STARTER|CFLAG1_SEQUENCE_STARTER_ONLY)) {
            // The subject call is a sequence starter.
            // Strip the "centers" concept and exit.
            conceptptr = conceptptr->next;
            break;
         }
         else if (callflags1_to_examine &
                  (CFLAG1_SPLIT_LIKE_SQUARE_THRU | CFLAG1_SPLIT_LIKE_DIXIE_STYLE)) {
            // The subject call is a sequence starter if given the "split" modifier.
            if (finaljunk.test_finalbit(FINAL__SPLIT)) {
               conceptptr = conceptptr->next;
               break;
            }
         }

         // Now skip things.
         // First, skip any fractional concept.  Only simple "M/N".
         // Also skip "stretch" and "once removed", and so on.
         while ((parse_scan->concept->kind == concept_fractional &&
                 parse_scan->concept->arg1 == 0) ||
                parse_scan->concept->kind == concept_once_removed ||
                parse_scan->concept->kind == concept_stable ||
                parse_scan->concept->kind == concept_frac_stable ||
                parse_scan->concept->kind == concept_new_stretch ||
                parse_scan->concept->kind == concept_old_stretch) {
            parse_scan = parse_scan->next;
            did_something = true;
         }

         // Skip all simple modifiers.  If we pass a "split" modifier, remember same.
         finaljunk.clear_all_herit_and_final_bits();
         parse_block *new_parse_scan = process_final_concepts(parse_scan, false,
                                                              &finaljunk, true,
                                                              __FILE__, __LINE__);
         if (parse_scan != new_parse_scan) {
            parse_scan = new_parse_scan;
            if (parse_scan->call)
               callflags1_to_examine = parse_scan->call->the_defn.callflags1;
            did_something = true;
         }

         // If the subject call is something like "<anything> and roll",
         // look at the <anything> to see whether it is a sequence starter.
         // The test for this is "heads [split grand chain 8] and roll".
         // The subcall may have already been filled in (the user typed
         // "heads [split grand chain 8] and roll"), or it may need to be
         // filled in (the user typed "heads <anything> and roll".)
         // The concept kind will be "concept_another_call_next_mod" in the
         // former case, or "marker_end_of_list" in the latter.
         // In the latter case, "get_real_subcall" will perform the
         // query.  Either way, "thingout" will have the subcall.
         // We do this if the call is something like "<anything> and roll".
         // The test is that it is sequentially defined, and part 1 is
         // a mandatory substitution, replacing the call "nothing".
         if ((parse_scan->concept->kind == concept_another_call_next_mod ||
              parse_scan->concept->kind == marker_end_of_list) &&
             parse_scan->call &&
             parse_scan->call->the_defn.schema == schema_sequential) {
            calldefn *seqdef = &parse_scan->call->the_defn;
            by_def_item *part0item = &seqdef->stuff.seq.defarray[0];
            setup_command thingout;

            if (part0item->call_id == base_call_null &&
                (part0item->modifiers1 & DFM1_CALL_MOD_MASK) == DFM1_CALL_MOD_MAND_ANYCALL &&
                get_real_subcall(parse_scan,
                                 part0item,
                                 &starting_setup.cmd,
                                 seqdef,
                                 false,
                                 0,
                                 &thingout)) {
               parse_scan = thingout.parseptr;
               // If there is no call, then process_final_concepts will do something
               // on the next round, and we will get something on the round after that.
               callflags1_to_examine = parse_scan->call ?
                  parse_scan->call->the_defn.callflags1 : 0;
               did_something = true;
            }
         }
      }
   }
   else if (parse_state.topcallflags1 & CFLAG1_SEQUENCE_STARTER_ONLY)
      fail("This call may only be used at the beginning of a sequence.");

   // Clear a few things.  We do NOT clear the warnings, because some (namely the
   // "concept not allowed at this level" warning) may have already been logged.
   // Set the selectors to "uninitialized", so that, if we do a call like "run", we
   // will query the user to find out who is selected.
   newhist.init_centersp_specific();
   newhist.draw_pic = false;
   newhist.init_resolve();
   current_options = null_options;

   // Put in identification bits for global/unsymmetrical stuff, if possible.
   for (i=0; i<MAX_PEOPLE; i++) starting_setup.people[i].id2 &= ~GLOB_BITS_TO_CLEAR;

   if (!(starting_setup.result_flags.misc & RESULTFLAG__IMPRECISE_ROT)) {
      // Can't do it if rotation is not known.
      if (attr::slimit(&starting_setup) >= 0) {
         // Put in headliner/sideliner stuff if possible.
         for (i=0; i<=attr::slimit(&starting_setup); i++) {
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

   // Put in position-identification bits (leads/trailers/beaus/belles/centers/ends etc.)
   update_id_bits(&starting_setup);
   starting_setup.cmd.parseptr = conceptptr;
   starting_setup.cmd.callspec = (call_with_name *) 0;
   starting_setup.cmd.cmd_final_flags.clear_all_herit_and_final_bits();
   move(&starting_setup, false, &newhist.state);
   remove_tgl_distortion(&newhist.state);

   if (newhist.state.kind == s1p5x8)
      fail("Can't go into a 50% offset 1x8.");
   else if (newhist.state.kind == s1p5x4)
      fail("Can't go into a 50% offset 1x4.");
   else if (newhist.state.kind == s_dead_concentric) {
      newhist.state.kind = newhist.state.inner.skind;
      newhist.state.rotation += newhist.state.inner.srotation;
   }

   // Once rotation is imprecise, it is always imprecise.  Same for the other flags copied here.
   newhist.state.result_flags.misc |= starting_setup.result_flags.misc &
      (RESULTFLAG__IMPRECISE_ROT|RESULTFLAG__ACTIVE_PHANTOMS_ON|RESULTFLAG__ACTIVE_PHANTOMS_OFF);

   // But 1/8 rotation stuff cancels in pairs.
   if (newhist.state.result_flags.misc & starting_setup.result_flags.misc & RESULTFLAG__PLUSEIGHTH_ROT) {
      newhist.state.result_flags.misc &= ~RESULTFLAG__PLUSEIGHTH_ROT;
      newhist.state.rotation++;
      canonicalize_rotation(&newhist.state);
   }
   else
      newhist.state.result_flags.misc |= starting_setup.result_flags.misc & RESULTFLAG__PLUSEIGHTH_ROT;
}


// Do the extra things that a call requires, that are not required
// when only testing for legality.

void finish_toplevelmove() THROW_DECL
{
   configuration & newhist = configuration::next_config();

   // Remove outboard phantoms from the resulting setup.
   normalize_setup(&newhist.state, simple_normalize, false);

   for (int i=0; i<MAX_PEOPLE; i++) newhist.state.people[i].id2 &= ~GLOB_BITS_TO_CLEAR;
   newhist.calculate_resolve();
}


// This stuff is duplicated in verify_call in sdmatch.c .
bool deposit_call_tree(modifier_block *anythings, parse_block *save1, int key)
{
   /* First, if we have already deposited a call, and we see more stuff, it must be
      concepts or calls for an "anything" subcall. */

   if (save1) {
      parse_block *tt = get_parse_block();
      /* Run to the end of any already-deposited things.  This could happen if the
         call takes a tagger -- it could have a search chain before we even see it. */
      while (save1->next) save1 = save1->next;
      save1->next = tt;
      save1->concept = &conzept::marker_concept_mod;
      tt->concept = &conzept::marker_concept_mod;
      tt->call = base_calls[(key == DFM1_CALL_MOD_MAND_SECONDARY/DFM1_CALL_MOD_BIT) ?
                           base_call_null_second: base_call_null];
      tt->call_to_print = tt->call;
      tt->replacement_key = key;
      parse_state.concept_write_ptr = &tt->subsidiary_root;
   }

   save1 = (parse_block *) 0;
   user_match.match.call_conc_options = anythings->call_conc_options;

   if (anythings->kind == ui_call_select) {
      if (deposit_call(anythings->call_ptr, &anythings->call_conc_options)) return true;
      save1 = *parse_state.concept_write_ptr;
      if (!there_is_a_call) the_topcallflags = parse_state.topcallflags1;
      there_is_a_call = true;
   }
   else if (anythings->kind == ui_concept_select) {
      if (deposit_concept(anythings->concept_ptr)) return true;
   }
   else return true;   // Huh?

   if (anythings->packed_next_conc_or_subcall) {
      if (deposit_call_tree(anythings->packed_next_conc_or_subcall, save1,
                            DFM1_CALL_MOD_MAND_ANYCALL/DFM1_CALL_MOD_BIT))
         return true;
   }

   if (anythings->packed_secondary_subcall) {
      if (deposit_call_tree(anythings->packed_secondary_subcall, save1,
                            DFM1_CALL_MOD_MAND_SECONDARY/DFM1_CALL_MOD_BIT))
         return true;
   }

   return false;
}



extern bool do_subcall_query(
   int snumber,
   parse_block *parseptr,
   parse_block **newsearch,
   bool this_is_tagger,
   bool this_is_tagger_circcer,
   call_with_name *orig_call)
{
   char tempstring_text[MAX_TEXT_LINE_LENGTH];

   // Note whether we are using any mandatory substitutions, so that the menu
   // initialization will always accept this call.

   if (snumber == (DFM1_CALL_MOD_MAND_ANYCALL/DFM1_CALL_MOD_BIT) ||
       snumber == (DFM1_CALL_MOD_MAND_SECONDARY/DFM1_CALL_MOD_BIT)) {
      // In some types of pick operations, the picker simply doesn't know how
      // to choose a mandatory subcall.  In that case, the call requiring the
      // mandatory subcall (e.g. "wheel and <anything>") is simply rejected.
      if (forbid_call_with_mandatory_subcall())
         fail_no_retry("Mandatory subcall fail.");
      mandatory_call_used = true;
   }

   // Now we know that the list doesn't say anything about this call.  Perhaps we should
   // query the user for a replacement and add something to the list.  First, decide whether
   // we should consider doing so.  If we are initializing the database, the answer is
   // always "no", even for calls that require a replacement call, such as
   // "clover and anything".  This means that, for the purposes of database initialization,
   // "clover and anything" is tested as "clover and nothing", since "nothing" is the subcall
   // that appears in the database.

   // Also, when doing pick operations, the picker might not want to do a random pick.
   // It might just want to leave the default call ("clover and [nothing]") in place.
   // So we ask the picker.

   // Of course, if we are testing the fidelity of later calls during a reconcile
   // operation, we DO NOT EVER add any modifiers to the list, even if the user
   // clicked on "allow modification" before clicking on "reconcile".  It is perfectly
   // legal to click on "allow modification" before clicking on "reconcile".  It means
   // we want modifications (chosen by random number generator, since we won't be
   // interactive) for the calls that we randomly choose, but not for the later calls
   // that we test for fidelity.

   if (!(interactivity == interactivity_normal ||
         allow_random_subcall_pick()) ||
       testing_fidelity)
      return true;

   // When we are searching for resolves and the like, the situation
   // is different.  In this case, the interactivity state is set for
   // a search.  We do perform mandatory modifications, so we will
   // generate things like "clover and shakedown".  Of course, no
   // querying actually takes place.  Instead, get_subcall just uses
   // the random number generator.  Therefore, whether resolving or in
   // normal interactive mode, we are guided by the call modifier
   // flags and the "allowing_modifications" global variable. */

   // Depending on what type of substitution is involved and what the "allowing modifications"
   // level is, we may choose not to query about this subcall, but just return the default.

   switch (snumber) {
   case DFM1_CALL_MOD_ANYCALL/DFM1_CALL_MOD_BIT:
   case DFM1_CALL_MOD_ALLOW_PLAIN_MOD/DFM1_CALL_MOD_BIT:
   case DFM1_CALL_MOD_OR_SECONDARY/DFM1_CALL_MOD_BIT:
      if (!allowing_modifications) return true;
      break;
   case DFM1_CALL_MOD_ALLOW_FORCED_MOD/DFM1_CALL_MOD_BIT:
      if (allowing_modifications <= 1) return true;
      break;
   }

   // At this point, we know we should query the user about this call.

   // Set ourselves up for modification by making the null modification list
   // if necessary.  ***** Someday this null list will always be present.

   if (parseptr->concept->kind == marker_end_of_list)
      parseptr->concept = &conzept::marker_concept_mod;

   // Create a reference on the list.  "search" points to the null item at the end.

   tempstring_text[0] = '\0';           // Null string, just to be safe.

   // If doing a tagger, just get the call.

   if (snumber == 0 && this_is_tagger_circcer)
      ;

   // If the replacement is mandatory, or we are not interactive,
   // don't present the popup.  Just get the replacement call.

   else if (interactivity != interactivity_normal)
      ;
   else if (snumber == (DFM1_CALL_MOD_MAND_ANYCALL/DFM1_CALL_MOD_BIT))
      (void) sprintf(tempstring_text, "SUBSIDIARY CALL");
   else if (snumber == (DFM1_CALL_MOD_MAND_SECONDARY/DFM1_CALL_MOD_BIT))
      (void) sprintf(tempstring_text, "SECOND SUBSIDIARY CALL");
   else {

      // Need to present the popup to the operator
      // and find out whether modification is desired.

      char pretty_call_name[MAX_TEXT_LINE_LENGTH];

      // Star turn calls can have funny names like "nobox".

      unparse_call_name(
         (orig_call->the_defn.callflags1 & CFLAG1_IS_STAR_CALL) ?
         "turn the star @b" : orig_call->name,
         pretty_call_name, &current_options);

      char *line_format;

      if (this_is_tagger)
         line_format = "The \"%s\" can be replaced with a tagging call.";
      else if (this_is_tagger_circcer)
         line_format = "The \"%s\" can be replaced with a modified circulate-like call.";
      else
         line_format = "The \"%s\" can be replaced.";

      char tempstuff[200];
      sprintf(tempstuff, line_format, pretty_call_name);
      if (gg->yesnoconfirm("Replacement", tempstuff, "Do you want to replace it?", false, false)) {
         // User accepted the modification.
         // Set up the prompt and get the concepts and call.
         (void) sprintf(tempstring_text, "REPLACEMENT FOR THE %s", pretty_call_name);
      }
      else {
         // User declined the modification.  Create a null entry
         // so that we don't query again.
         *newsearch = get_parse_block();
         (*newsearch)->concept = &conzept::marker_concept_mod;
         (*newsearch)->options = current_options;
         (*newsearch)->replacement_key = snumber;
         (*newsearch)->call = orig_call;
         (*newsearch)->call_to_print = orig_call;
         return true;
      }
   }

   *newsearch = get_parse_block();
   (*newsearch)->concept = &conzept::marker_concept_mod;
   (*newsearch)->options = current_options;
   (*newsearch)->replacement_key = snumber;
   (*newsearch)->call = orig_call;
   (*newsearch)->call_to_print = orig_call;

   /* Set stuff up for reading subcall and its concepts. */

   /* Create a new parse block, point concept_write_ptr at its contents. */
   /* Create the new root at the start of the subsidiary list. */

   parse_state.concept_write_base = &(*newsearch)->subsidiary_root;
   parse_state.concept_write_ptr = parse_state.concept_write_base;

   parse_state.parse_stack_index = 0;
   parse_state.call_list_to_use = call_list_any;
   (void) strncpy(parse_state.specialprompt, tempstring_text, MAX_TEXT_LINE_LENGTH);

   /* Search for special case of "must_be_tag_call" with no other modification bits.
      That means it is a new-style tagging call. */

   if (snumber == 0 && this_is_tagger_circcer) {
      throw error_flag_type(error_flag_wrong_command);
   }
   else {
      if (query_for_call()) {
         // User clicked on something unusual like "exit" or "undo".
         throw error_flag_type(error_flag_wrong_command);
      }
   }

   return false;
}



extern call_list_kind find_proper_call_list(setup *s)
{
   if (s->kind == s1x8) {
      if      ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_1x8;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_l1x8;
      else if ((s->people[0].id1 & 015) == 1 &&
               (s->people[1].id1 & 015) == 1 &&
               (s->people[2].id1 & 015) == 1 &&
               (s->people[3].id1 & 015) == 1 &&
               (s->people[4].id1 & 015) == 1 &&
               (s->people[5].id1 & 015) == 1 &&
               (s->people[6].id1 & 015) == 1 &&
               (s->people[7].id1 & 015) == 1)
         return call_list_gcol;
   }
   else if (s->kind == s2x4) {
      if      ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_dpt;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_cdpt;
      else if ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_rcol;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_lcol;
      else if ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_8ch;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_tby;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_lin;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_lout;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_rwv;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_lwv;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_r2fl;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_l2fl;
   }
   else if (s->kind == s_qtag)
      return call_list_qtag;

   return call_list_any;
}
