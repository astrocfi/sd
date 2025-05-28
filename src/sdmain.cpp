/* SD -- square dance caller's helper.

    Copyright (C) 1990-2002  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The version of this file is as shown immediately below.  This string
    gets displayed at program startup.

    ATTENTION!  If you modify the program, we recommend that
    you change the version string below to identify the change.
    This will facilitate tracking errors, since the program version
    will be written onto every sequence.  Changing the version
    string is also required by paragraphs 2(a) and 2(c) of the GNU
    General Public License if you distribute the file.
*/

#define VERSION_STRING "34.7n"
#define TIME_STAMP "wba@alum.mit.edu  11 August 2002 $"

/* This defines the following functions:
   sd_version_string
   deposit_call
   deposit_concept
   query_for_call
   sdmain
and the following external variables:
   menu_names
   command_menu
   resolve_menu
   startup_menu
   last_file_position
   elide_blanks
*/


#include <stdio.h>
#include <string.h>

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"
#include "paths.h"
#include "resource.h"


/* We cause this string (that is, the concatentation of these strings) to appear
   in the binary image of the program, so that the "what" and "ident" utilities
   can print the version.

   We do not endorse those programs, or any probabilistic identification
   mechanism -- we are simply trying to be helpful to those people who use them.
   It is regrettable that those two identification mechanisms are different
   and superficially incompatible, and that many existing programs only comply
   with one or the other of them, but not both.  By choosing the contents of
   the string carefully, we believe we comply with both.

   We specifically break up the "Header" word to prevent utilities like RCS or
   SCCS, if anyone should use them to store this source file, from attempting
   to modify this.  Version control for this program is performed manually,
   not by any utility.  Furthermore, we do not believe that it is proper for
   source control utilities to alter the text in a source file. */

static const char id[] = "@(#)$He" "ader: Sd: version " VERSION_STRING "  " TIME_STAMP;
   

extern char *sd_version_string()
{
   return VERSION_STRING;
}


/* These variables are external. */

/* BEWARE!!  This list is keyed to the definition of "call_list_kind" in sd.h . */
Cstring menu_names[] = {
   "???",
   "???",
   "(any setup)",
   "tidal wave",
   "left tidal wave",
   "dpt",
   "cdpt",
   "columns",
   "left columns",
   "8 chain",
   "trade by",
   "facing lines",
   "lines-out",
   "waves",
   "left waves",
   "2-faced lines",
   "left 2-faced lines",
   "tidal column",
   "diamond/qtag"};

command_list_menu_item command_menu[] = {
   {"exit the program",               command_quit, ID_FILE_EXIT},
   {"quit the program",               command_quit, -1},
   {"simple modifications",           command_simple_mods, -1},
   {"allow modifications",            command_all_mods, -1},
   {"toggle concept levels",          command_toggle_conc_levels, ID_COMMAND_TOGGLE_CONC},
   {"toggle active phantoms",         command_toggle_act_phan, ID_COMMAND_TOGGLE_PHAN},
   {"toggle minigrand getouts",       command_toggle_minigrand, -1},
   {"toggle retain after error",      command_toggle_retain_after_error, -1},
   {"toggle nowarn mode",             command_toggle_nowarn_mode, -1},
   {"toggle singleclick mode",        command_toggle_singleclick_mode, -1},
   {"choose font for printing",       command_select_print_font, ID_FILE_CHOOSE_FONT},
   {"print current file",             command_print_current, ID_FILE_PRINTTHIS},
   {"print any file",                 command_print_any, ID_FILE_PRINTFILE},
   {"undo last call",                 command_undo, ID_COMMAND_UNDO},
   {"discard entered concepts",       command_erase, ID_COMMAND_DISCARD_CONCEPT},
   {"abort this sequence",            command_abort, ID_COMMAND_ABORTTHISSEQUENCE},
   {"insert a comment",               command_create_comment, ID_COMMAND_COMMENT},
   {"change output file",             command_change_outfile, ID_COMMAND_CH_OUTFILE},
   {"change title",                   command_change_header, ID_COMMAND_CH_TITLE},
   {"write this sequence",            command_getout, -1},
   {"end this sequence",              command_getout, ID_COMMAND_ENDTHISSEQUENCE},
   {"cut to clipboard",               command_cut_to_clipboard, -1},
   {"clipboard cut",                  command_cut_to_clipboard, ID_COMMAND_CLIPBOARD_CUT},
   {"delete entire clipboard",        command_delete_entire_clipboard, -1},
   {"clipboard delete all",           command_delete_entire_clipboard, ID_COMMAND_CLIPBOARD_DEL_ALL},
   {"delete one call from clipboard", command_delete_one_call_from_clipboard, -1},
   {"clipboard delete one",           command_delete_one_call_from_clipboard, ID_COMMAND_CLIPBOARD_DEL_ONE},
   {"paste one call",                 command_paste_one_call, -1},
   {"clipboard paste one",            command_paste_one_call, ID_COMMAND_CLIPBOARD_PASTE_ONE},
   {"paste all calls",                command_paste_all_calls, ID_COMMAND_CLIPBOARD_PASTE_ALL},
   {"clipboard paste all",            command_paste_all_calls, -1},
   {"keep picture",                   command_save_pic, ID_COMMAND_KEEP_PICTURE},
   {"refresh display",                command_refresh, -1},
   {"resolve",                        command_resolve, ID_COMMAND_RESOLVE},
   {"normalize",                      command_normalize, ID_COMMAND_NORMALIZE},
   {"standardize",                    command_standardize, ID_COMMAND_STANDARDIZE},
   {"reconcile",                      command_reconcile, ID_COMMAND_RECONCILE},
   {"pick random call",               command_random_call, ID_COMMAND_PICK_RANDOM},
   {"pick simple call",               command_simple_call, ID_COMMAND_PICK_SIMPLE},
   {"pick concept call",              command_concept_call, ID_COMMAND_PICK_CONCEPT},
   {"pick level call",                command_level_call, ID_COMMAND_PICK_LEVEL},
   {"pick 8 person level call",       command_8person_level_call, ID_COMMAND_PICK_8P_LEVEL},
   {"create waves",                   command_create_waves, ID_COMMAND_CREATE_WAVES},
   {"create 2fl",                     command_create_2fl, ID_COMMAND_CREATE_2FL},
   {"create lines in",                command_create_li, ID_COMMAND_CREATE_LINESIN},
   {"create lines out",               command_create_lo, ID_COMMAND_CREATE_LINESOUT},
   {"create inverted lines",          command_create_inv_lines, ID_COMMAND_CREATE_INVLINES},
   {"create 3x1 lines",               command_create_3and1_lines, ID_COMMAND_CREATE_3N1LINES},
   {"create any lines",               command_create_any_lines, ID_COMMAND_CREATE_ANYLINES},
   {"create columns",                 command_create_col, ID_COMMAND_CREATE_COLUMNS},
   {"create magic columns",           command_create_magic_col, ID_COMMAND_CREATE_MAGCOL},
   {"create dpt",                     command_create_dpt, ID_COMMAND_CREATE_DPT},
   {"create cdpt",                    command_create_cdpt, ID_COMMAND_CREATE_CDPT},
   {"create 8 chain",                 command_create_8ch, ID_COMMAND_CREATE_8CH},
   {"create trade by",                command_create_tby, ID_COMMAND_CREATE_TRBY},
   {"create any columns",             command_create_any_col, ID_COMMAND_CREATE_ANYCOLS},
   {"create tidal wave",              command_create_tidal_wave, ID_COMMAND_CREATE_GWV},
   {"create any tidal setup",         command_create_any_tidal, ID_COMMAND_CREATE_ANY_TIDAL},
   {"create diamonds",                command_create_dmd, ID_COMMAND_CREATE_DMD},
   {"create 1/4 tag",                 command_create_qtag, ID_COMMAND_CREATE_QTAG},
   {"create 3/4 tag",                 command_create_3qtag, ID_COMMAND_CREATE_3QTAG},
   {"create 1/4 line",                command_create_qline, ID_COMMAND_CREATE_QLINE},
   {"create 3/4 line",                command_create_3qline, ID_COMMAND_CREATE_3QLINE},
   {"create any 1/4 tag",             command_create_any_qtag, ID_COMMAND_CREATE_ANY_QTAG},
   {"help manual",                    command_help_manual, ID_HELP_SDHELP},
   {(Cstring) 0}};

resolve_list_menu_item resolve_menu[] = {
   {"abort the search",       resolve_command_abort},
   {"exit the search",        resolve_command_abort},
   {"quit the search",        resolve_command_abort},
   {"undo the search",        resolve_command_abort},
   {"find another",           resolve_command_find_another},
   {"next",                   resolve_command_goto_next},
   {"previous",               resolve_command_goto_previous},
   {"accept current choice",  resolve_command_accept},
   {"raise reconcile point",  resolve_command_raise_rec_point},
   {"lower reconcile point",  resolve_command_lower_rec_point},
   {"write this sequence",    resolve_command_write_this},
   {(Cstring) 0}};

startup_list_menu_item startup_menu[] = {
   {"exit from the program",       start_select_exit, ID_FILE_EXIT},
   {"heads start",                 start_select_heads_start, -1},
   {"sides start",                 start_select_sides_start, -1},
   {"heads 1p2p",                  start_select_h1p2p, -1},
   {"sides 1p2p",                  start_select_s1p2p, -1},
   {"just as they are",            start_select_as_they_are, -1},
   {"toggle concept levels",       start_select_toggle_conc, ID_COMMAND_TOGGLE_CONC},
   {"toggle active phantoms",      start_select_toggle_act, ID_COMMAND_TOGGLE_PHAN},
   {"toggle minigrand getouts",    start_select_toggle_minigrand, -1},
   {"toggle singlespace mode",     start_select_toggle_singlespace, -1},
   {"toggle retain after error",   start_select_toggle_retain, -1},
   {"toggle nowarn mode",          start_select_toggle_nowarn_mode, -1},
   {"toggle singleclick mode",     start_select_toggle_singleclick_mode, -1},
   {"toggle singing call",         start_select_toggle_singer, -1},
   {"toggle reverse singing call", start_select_toggle_singer_backward, -1},
   {"choose font for printing",    start_select_select_print_font, ID_FILE_CHOOSE_FONT},
   {"print current file",          start_select_print_current, ID_FILE_PRINTTHIS},
   {"print any file",              start_select_print_any, ID_FILE_PRINTFILE},
   {"initialize session file",     start_select_init_session_file, -1},
   {"change output file",          start_select_change_outfile, ID_COMMAND_CH_OUTFILE},
   {"change title",                start_select_change_header_comment, ID_COMMAND_CH_TITLE},
   {(Cstring) 0}};
int last_file_position = -1;





/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
static long_boolean find_tagger(uint32 tagclass, uint32 *tagg, call_with_name **tagger_call)
{
   uint32 numtaggers = number_of_taggers[tagclass];
   call_with_name **tagtable = tagger_calls[tagclass];

   if (numtaggers == 0) return TRUE;   /* We can't possibly do this. */

   if (interactivity == interactivity_normal) {
      if (user_match.valid &&
          (user_match.match.call_conc_options.tagger != 0)) {
         *tagg = user_match.match.call_conc_options.tagger;
         user_match.match.call_conc_options.tagger = 0;
      }
      else if ((*tagg = gg->do_tagger_popup(tagclass)) == 0) return TRUE;

      if ((*tagg >> 5) != tagclass) fail("bad tagger class???");
      if ((*tagg & 0x1F) > numtaggers) fail("bad tagger index???");
   }
   else if (interactivity == interactivity_verify) {
      if (verify_options.tagger != 0) {
         *tagg = verify_options.tagger;
         if ((*tagg >> 5) != tagclass) fail("bad tagger class???");
      }
      else {
         *tagg = (tagclass << 5) + 1;
      }

      if ((*tagg & 0x1F) > numtaggers) fail("bad tagger index???");
      verify_options.tagger = 0;
   }
   else if (interactivity == interactivity_database_init) {
      /* We don't generate "dont_use_in_resolve" taggers in any random search. */
      /* Using zero as the tag call index might not be right. */
      if (tagtable[0]->the_defn.callflags1 & CFLAG1_DONT_USE_IN_RESOLVE)
         fail("This shouldn't get printed.");

      *tagg = (tagclass << 5) | 1;
   }
   else {
      if (do_tagger_iteration(tagclass, tagg, numtaggers, tagtable))
         return TRUE;  /* There simply are no acceptable taggers. */
   }

   /* At this point, tagg contains 8 bits:
         3 bits of tagger list (zero-based - 0..NUM_TAGGER_CLASSES-1)
         5 bits of tagger within that list (1-based). */

   *tagger_call = tagtable[(*tagg & 0x1F)-1];

   return FALSE;
}



/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
static long_boolean find_circcer(uint32 *circcp)
{
   if (number_of_circcers == 0) return TRUE;   /* We can't possibly do this. */

   if (interactivity == interactivity_normal || interactivity == interactivity_verify) {
      if ((*circcp = gg->do_circcer_popup()) == 0)
         return TRUE;
   }
   else if (interactivity == interactivity_database_init) {
      *circcp = 1;   /* This may not be right. */
   }
   else {
      do_circcer_iteration(circcp);
   }

   return FALSE;
}




/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
static long_boolean find_selector(selector_kind *sel_p, long_boolean is_for_call)
{
   if (interactivity == interactivity_normal) {
      int j;

      if (user_match.valid &&
          (user_match.match.call_conc_options.who != selector_uninitialized)) {
         j = (int) user_match.match.call_conc_options.who;
         user_match.match.call_conc_options.who = selector_uninitialized;
      }
      else if ((j = gg->do_selector_popup()) == 0)
         return TRUE;

      *sel_p = (selector_kind) j;
   }
   else
      *sel_p = do_selector_iteration(is_for_call);

   return FALSE;
}


/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
static long_boolean find_direction(direction_kind *dir_p)
{
   if (interactivity == interactivity_normal) {
      int j;

      if (user_match.valid &&
          (user_match.match.call_conc_options.where != direction_uninitialized)) {
         j = (int) user_match.match.call_conc_options.where;
         user_match.match.call_conc_options.where = direction_uninitialized;
      }
      else if ((j = gg->do_direction_popup()) == 0)
         return TRUE;

      *dir_p = (direction_kind) j;
   }
   else
      *dir_p = do_direction_iteration();

   return FALSE;
}


/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
static long_boolean find_numbers(int howmanynumbers, long_boolean forbid_zero,
   uint32 odd_number_only, long_boolean allow_iteration, uint32 *number_list)
{
   if (interactivity == interactivity_normal) {
      *number_list = gg->get_number_fields(howmanynumbers, forbid_zero);

      if ((*number_list) == ~0UL)
         return TRUE;           /* User waved the mouse away. */
   }
   else
      do_number_iteration(howmanynumbers, odd_number_only, allow_iteration, number_list);

   return FALSE;
}



/* Deposit a call into the parse state.  A returned value of TRUE
   means that the user refused to click on a required number or selector,
   and so we have taken no action.  This can only occur if interactive.
   Well, actually, a lossage in the database ("@y" type of ATC call
   wants a tagger class which has no tagger calls available at this level)
   can cause failure during a search.
   If not interactive, stuff will be chosen by random number. */

extern long_boolean deposit_call(call_with_name *call, const call_conc_option_state *options)
{
   parse_block *new_block;
   call_with_name *tagger_call;
   uint32 tagg = 0;
   selector_kind sel = selector_uninitialized;
   direction_kind dir = direction_uninitialized;
   uint32 circc = 0;    /* Circulator index (1-based). */
   uint32 number_list = 0;
   int howmanynums = (call->the_defn.callflags1 & CFLAG1_NUMBER_MASK) / CFLAG1_NUMBER_BIT;

   // **** If call requires both a selector and a circulator (the culprit
   // is "<ANYCIRC> percolate, <ANYONE> to a wave"), we don't generate
   // it in exhaustive searches.

   if ((call->the_defn.callflagsf & (CFLAGH__CIRC_CALL_RQ_BIT|CFLAGH__REQUIRES_SELECTOR)) ==
       (CFLAGH__CIRC_CALL_RQ_BIT|CFLAGH__REQUIRES_SELECTOR) &&
       interactivity != interactivity_normal &&
       interactivity != interactivity_verify &&
       interactivity != interactivity_database_init) {
      if (in_exhaustive_search()) {
         return TRUE;
      }
   }

   /* Put in tagging call info if required. */

   if (call->the_defn.callflagsf & CFLAGH__TAG_CALL_RQ_MASK) {
      if (find_tagger(
               ((call->the_defn.callflagsf & CFLAGH__TAG_CALL_RQ_MASK) /
                CFLAGH__TAG_CALL_RQ_BIT) - 1,
               &tagg,
               &tagger_call))
         return TRUE;
   }

   /* Or circulating call index. */

   if (call->the_defn.callflagsf & CFLAGH__CIRC_CALL_RQ_BIT) {
      if (find_circcer(&circc)) return TRUE;
   }

   /* Put in selector, direction, and/or number as required. */

   if (call->the_defn.callflagsf & CFLAGH__REQUIRES_SELECTOR) {
      if (find_selector(&sel, TRUE)) return TRUE;
   }

   if (call->the_defn.callflagsf & CFLAGH__REQUIRES_DIRECTION)
      if (find_direction(&dir)) return TRUE;

   if (howmanynums != 0)
      if (find_numbers(howmanynums, FALSE, call->the_defn.callflagsf & CFLAGH__ODD_NUMBER_ONLY, TRUE, &number_list)) return TRUE;

   new_block = get_parse_block();
   new_block->concept = &mark_end_of_list;
   new_block->call = call;
   new_block->call_to_print = call;
   new_block->options = *options;
   new_block->options.who = sel;
   new_block->options.where = dir;
   new_block->options.number_fields = number_list;
   new_block->options.howmanynumbers = howmanynums;
   new_block->options.tagger = 0;
   new_block->options.circcer = 0;

   /* Filling in the tagger requires recursion! */

   if (tagg != 0) {
      parse_block **savecwp = parse_state.concept_write_ptr;

      new_block->options.tagger = tagg;
      new_block->concept = &marker_concept_mod;
      new_block->next = get_parse_block();
      new_block->next->concept = &marker_concept_mod;

      /* Deposit the index of the base tagging call.  This will of course be replaced. */

      new_block->next->call = base_calls[base_call_tagger0];
      new_block->next->call_to_print = base_calls[base_call_tagger0];

      parse_state.concept_write_ptr = &new_block->next->subsidiary_root;
      if (deposit_call(tagger_call, &null_options))
         throw error_flag_type(error_flag_wrong_command);     // User waved the mouse away while getting subcall.
      parse_state.concept_write_ptr = savecwp;
   }

   /* Filling in the circcer does too, but it isn't serious. */

   if (circc != 0) {
      parse_block **savecwp = parse_state.concept_write_ptr;

      new_block->options.circcer = circc;
      new_block->concept = &marker_concept_mod;
      new_block->next = get_parse_block();
      new_block->next->concept = &marker_concept_mod;

      /* Deposit the index of the base circcing call.  This will of course be replaced. */

      new_block->next->call = base_calls[base_call_circcer];
      new_block->next->call_to_print = base_calls[base_call_circcer];

      if (circc > number_of_circcers) fail("bad circcer index???");

      parse_state.concept_write_ptr = &new_block->next->subsidiary_root;
      if (deposit_call(circcer_calls[circc-1], &null_options))
         throw error_flag_type(error_flag_wrong_command);     // User waved the mouse away while getting subcall.
      parse_state.concept_write_ptr = savecwp;
   }

   parse_state.topcallflags1 = call->the_defn.callflags1;
   *parse_state.concept_write_ptr = new_block;

   return FALSE;
}



/* Deposit a concept into the parse state.  A returned value of TRUE
   means that the user refused to click on a required number or selector,
   and so we have taken no action.  This can only occur if interactive.
   If the interactivity is special, the "number_fields" argument is ignored, and
   necessary stuff will be chosen by random number.  If it is off, the appropriate
   numbers (as indicated by the "CONCPROP__USE_NUMBER" stuff) must be provided. */

extern long_boolean deposit_concept(const concept_descriptor *conc)
{
   parse_block *new_block;
   selector_kind sel = selector_uninitialized;
   uint32 number_list = 0;
   int howmanynumbers = 0;

   /* We hash the actual concept pointer, as though it were an integer index. */
   hash_nonrandom_number((int) conc);

   if (concept_table[conc->kind].concept_prop & CONCPROP__USE_SELECTOR) {
      if (find_selector(&sel, FALSE)) return TRUE;
   }

   if (concept_table[conc->kind].concept_prop & CONCPROP__USE_NUMBER)
      howmanynumbers = 1;
   if (concept_table[conc->kind].concept_prop & CONCPROP__USE_TWO_NUMBERS)
      howmanynumbers = 2;

   if (howmanynumbers != 0) {
      if (find_numbers(howmanynumbers, TRUE, 0, FALSE, &number_list)) return TRUE;
   }

   new_block = get_parse_block();
   new_block->concept = conc;
   new_block->options.who = sel;
   new_block->options.where = direction_uninitialized;
   new_block->options.number_fields = number_list;
   new_block->options.howmanynumbers = howmanynumbers;

   *parse_state.concept_write_ptr = new_block;

   /* See if we need to parse a second call for this concept. */

   if (concept_table[conc->kind].concept_prop & CONCPROP__SECOND_CALL) {
      /* Set up recursion, reading the first call and its concepts into the same parse block. */

      if (parse_state.parse_stack_index == 39) specialfail("Excessive number of concepts.");
      parse_state.parse_stack[parse_state.parse_stack_index].save_concept_kind = conc->kind;
      parse_state.parse_stack[parse_state.parse_stack_index++].concept_write_save_ptr = parse_state.concept_write_ptr;
      parse_state.specialprompt[0] = '\0';
      parse_state.topcallflags1 = 0;          /* Erase anything we had -- it is meaningless now. */
   }

   parse_state.call_list_to_use = call_list_any;

   /* Advance the write pointer. */
   parse_state.concept_write_ptr = &(new_block->next);

   return FALSE;
}


static void print_error_person(unsigned int person, long_boolean example)
{
   char person_string[3];

   if (person & BIT_ACT_PHAN) {
      writestuff("phantom");
   }
   else if (person & BIT_TANDVIRT) {
      writestuff("virtual");
   }
   else {
      person_string[0] = ((person >> 7) & 3) + '1';
      person_string[1] = (person & 0100) ? 'G' : 'B';
      person_string[2] = '\0';
      writestuff(person_string);
      if (example) writestuff(", for example");
   }
}



/* False result means OK.  Otherwise, user clicked on something special,
   such as "abort" or "undo", and the reply tells what it was. */

extern long_boolean query_for_call()
{
   uims_reply local_reply;
   error_flag_type old_error_flag;
   int concepts_deposited = 0;

   recurse_entry:

   /* We should actually re-use anything there. */
   *parse_state.concept_write_ptr = (parse_block *) 0;

   if (allowing_modifications)
      parse_state.call_list_to_use = call_list_any;

   redisplay:

   if (interactivity == interactivity_normal) {
      /* We are operating in interactive mode.  Update the
         display and query the user. */

      if (global_error_flag == error_flag_show_stats) {
         clear_screen();
         writestuff("***** LEAST RECENTLY USED 2% OF THE CALLS ARE:");
         newline();
         writestuff("filibuster     peel the toptivate");
         newline();
         writestuff("spin chain and circulate the gears    spin chain and exchange the gears");
         newline();
         global_error_flag = (error_flag_type) 0;
         goto try_again;
      }

      /* First, update the output area to the current state, with error messages, etc.
         We draw a picture for the last two calls. */

      // Some things can only be done if we have a "nice" execution error.
      // Otherwise, the history array won't have the "last setup" that we want.

      if (global_error_flag < error_flag_wrong_command ||
          global_error_flag == error_flag_selector_changed ||
          global_error_flag == error_flag_formation_changed) {
         display_initial_history(configuration::history_ptr, (diagnostic_mode ? 1 : 2));

         if (configuration::sequence_is_resolved()) {
            newline();
            writestuff("     resolve is:");
            newline();
            write_resolve_text(FALSE);
            newline();
         }
      }

      if (global_error_flag && global_error_flag < error_flag_wrong_command) {
         writestuff("Can't do this call:");
         newline();
         write_history_line(0, false, false, file_write_no);
      }

      if (global_error_flag) {
         switch (global_error_flag) {
         case error_flag_wrong_resolve_command:
         case error_flag_1_line:
         case error_flag_no_retry:
            // Commonplace error message.
            writestuff(error_message1);
            break;
         case error_flag_2_line:
            writestuff(error_message1);
            newline();
            writestuff("   ");
            writestuff(error_message2);
            break;
         case error_flag_collision:
            // Very special message -- no text here, two people collided
            // and they are stored in collision_person1 and collision_person2.
            writestuff("Some people (");
            print_error_person(collision_person1, FALSE);
            writestuff(" and ");
            print_error_person(collision_person2, TRUE);
            writestuff(") on same spot.");
            break;
         case error_flag_cant_execute:
            // Very special message -- given text is to be prefixed with description
            // of the perpetrator, who is stored in collision_person1.
            writestuff("Some person (");
            print_error_person(collision_person1, TRUE);
            writestuff(") ");
            writestuff(error_message1);
            break;
         case error_flag_selector_changed:
            writestuff("Warning -- person identifiers were changed.");
            break;
         case error_flag_formation_changed:
            writestuff("Warning -- the formation has changed.");
            break;
         case error_flag_wrong_command:
            writestuff("You can't select that here.");
            break;
         }
      
         newline();
         newline();
      }

      old_error_flag = global_error_flag; /* save for refresh command */
      global_error_flag = (error_flag_type) 0;

      if (clipboard_size != 0) {
         int j;

         writestuff("............................");
         newline();

         /* Display at most 3 lines. */
         for (j = clipboard_size-1 ; j >= 0 && j >= clipboard_size-3 ; j--) {
            writestuff("      ");
            print_recurse(clipboard[j].command_root, 0);
            newline();
         }

         writestuff("............................");
         newline();
      }

      try_again:

      /* Display the call index number, and the partially entered call and/or prompt, as appropriate. */

      /* See if there are partially entered concepts.  If so, print the index number
         and those concepts on a separate line. */

      if (parse_state.concept_write_ptr != &configuration::next_config().command_root) {

         // This prints the concepts entered so far, with a "header"
         // consisting of the index number.  This partial concept tree
         // is incomplete, so write_history_line has to be (and is) very careful.

         write_history_line(configuration::history_ptr+1, false, true, file_write_no);
      }
      else {
         // No partially entered concepts.  Just do the sequence number.

         if (!diagnostic_mode) {
            char indexbuf[200];
            (void) sprintf (indexbuf, "%2d: ", configuration::history_ptr-configuration::whole_sequence_low_lim+2);
            writestuff(indexbuf);
            newline();
         }
      }

      if (parse_state.specialprompt[0] != '\0') {
         writestuff(parse_state.specialprompt);
         newline();
      }

      /* Returned value of true means that the user waved the mouse away at some point,
         that is, nothing was entered, and we should try again.  Otherwise, the concepts
         and call have been deposited with calls to "deposit_call" and "deposit_concept". */

      check_menu:

      if (gg->get_call_command(&local_reply)) goto recurse_entry;

      if (local_reply == ui_command_select) {
         switch ((command_kind) uims_menu_index) {
            char comment[MAX_TEXT_LINE_LENGTH];
         case command_create_comment:
            if (gg->do_comment_popup(comment)) {
               char *temp_text_ptr;
               comment_block *new_comment_block;     /* ****** Kludge!!!!! */

               new_comment_block = (comment_block *) get_mem(sizeof(comment_block));
               temp_text_ptr = &new_comment_block->txt[0];
               string_copy(&temp_text_ptr, comment);

               *parse_state.concept_write_ptr = get_parse_block();
               (*parse_state.concept_write_ptr)->concept = &marker_concept_comment;

               (*parse_state.concept_write_ptr)->call = (call_with_name *) new_comment_block;
               (*parse_state.concept_write_ptr)->call_to_print =
                  (call_with_name *) new_comment_block;
               /* Advance the write pointer. */
               parse_state.concept_write_ptr = &((*parse_state.concept_write_ptr)->next);
            }
            goto recurse_entry;
         case command_simple_mods:
            /* Increment "allowing_modifications" up to a maximum of 2. */
            if (allowing_modifications != 2) allowing_modifications++;
            goto check_menu;
         case command_all_mods:
            allowing_modifications = 2;
            goto check_menu;
         case command_toggle_conc_levels:
            allowing_all_concepts = !allowing_all_concepts;
            goto check_menu;
         case command_toggle_minigrand:
            allowing_minigrand = !allowing_minigrand;
            goto check_menu;
         case command_toggle_act_phan:
            using_active_phantoms = !using_active_phantoms;
            goto check_menu;
         case command_toggle_retain_after_error:
            retain_after_error = !retain_after_error;
            goto check_menu;
         case command_toggle_nowarn_mode:
            ui_options.nowarn_mode = !ui_options.nowarn_mode;
            goto check_menu;
         case command_toggle_singleclick_mode:
            ui_options.accept_single_click = !ui_options.accept_single_click;
            goto check_menu;
         case command_refresh:
            written_history_items = -1; /* suppress optimized display update */
            global_error_flag = old_error_flag; /* want to see error messages, too */
            goto redisplay;
         default:
            global_reply = local_reply;     /* Save this -- top level will need it. */
            return TRUE;
         }
      }
   }
   else {

      /* We are operating in automatic mode.
         We must insert a concept or a call.  Decide which.
         We only insert a concept if in random search, and then only occasionally. */

      const concept_descriptor *concept_to_use = pick_concept(concepts_deposited != 0);

      if (concept_to_use) {
         /* We give 0 for the number fields.  It gets taken care of later,
            perhaps not the best way. */
         (void) deposit_concept(concept_to_use);
         concepts_deposited++;
         local_reply = ui_concept_select;
      }
      else {
         local_reply = ui_call_select;
      }
   }

   if (local_reply == ui_concept_select) {
      goto recurse_entry;
   }
   else if (local_reply != ui_call_select) {
      global_reply = local_reply;     /* Save this -- top level will need it. */
      return TRUE;
   }

   /* We have a call.  Get the actual call and deposit it into the parse tree,
      if we haven't already. */

   if (interactivity == interactivity_database_init || interactivity == interactivity_verify)
      fail("This shouldn't get printed.");
   else if (interactivity != interactivity_normal) {
      if (deposit_call(do_pick(), &null_options))
         fail("This shouldn't get printed.");
   }

   /* Check our "stack" and see if we have recursive invocations to clean up. */

   if (parse_state.parse_stack_index != 0) {
      /* Set stuff up for reading second call and its concepts. */
   
      /* Create a new parse block, point concept_write_ptr at its contents. */
      /* Fill in the pointer to the second parse block. */
   
      parse_state.concept_write_ptr = parse_state.parse_stack[--parse_state.parse_stack_index].concept_write_save_ptr;

      (*parse_state.concept_write_ptr)->subsidiary_root = (parse_block *) 0;
      parse_state.concept_write_base = &(*parse_state.concept_write_ptr)->subsidiary_root;
      parse_state.concept_write_ptr = parse_state.concept_write_base;
      *parse_state.concept_write_ptr = (parse_block *) 0;
   
      parse_state.call_list_to_use = call_list_any;
   
      switch (parse_state.parse_stack[parse_state.parse_stack_index].save_concept_kind) {
         case concept_centers_and_ends:
            (void) strncpy(parse_state.specialprompt, "ENTER CALL FOR ENDS", MAX_TEXT_LINE_LENGTH);
            break;
         case concept_on_your_own:
            (void) strncpy(parse_state.specialprompt, "ENTER SECOND (CENTERS) CALL", MAX_TEXT_LINE_LENGTH);
            break;
         default:
            (void) strncpy(parse_state.specialprompt, "ENTER SECOND CALL", MAX_TEXT_LINE_LENGTH);
            break;
      }
   
      parse_state.topcallflags1 = 0;          /* Erase anything we had -- it is meaningless now. */
      goto recurse_entry;
   }

   /* Advance the write pointer. */
   parse_state.concept_write_ptr = &((*parse_state.concept_write_ptr)->next);

   return FALSE;
}



ui_option_type::ui_option_type() :
   no_graphics(0),
   no_intensify(0),
   reverse_video(0),
   pastel_color(0),
   color_scheme(color_by_gender),
   no_sound(0),
   sequence_num_override(-1),
   singlespace_mode(FALSE),
   nowarn_mode(FALSE),
   accept_single_click(FALSE),
   use_escapes_for_drawing_people(0),
   pn1("11223344"),
   pn2("BGBGBGBG"),
   direc("?>?<????^?V?????"),
   stddirec("?>?<????^?V?????"),
   squeeze_this_newline(0),
   drawing_picture(0)
{}


extern int sdmain(int argc, char *argv[])
{
   if (argc >= 2 && strcmp(argv[1], "-help") == 0) {
      printf("Sd version %s : ui%s\n",
             sd_version_string(), gg->version_string());
      printf("Usage: sd [flags ...] level\n");
      printf("  legal flags:\n");
      printf("-write_list filename        write out list for this level\n");
      printf("-write_full_list filename   write this list and all lower\n");
      printf("-abridge filename           do not use calls in this file\n");
      printf("-no_checkers                do not use large \"checkers\" for setup display\n");
      printf("-no_graphics                do not use special characters for setup display\n");
      printf("-reverse_video              (Sd only) display transcript in white-on-black\n");
      printf("-normal_video               (Sdtty only) display transcript in black-on-white\n");
      printf("-no_color                   do not display people in color\n");
      printf("-bold_color                 use bold colors when not coloring by couple or corner\n");
      printf("-pastel_color               use pastel colors when not coloring by couple or corner\n");
      printf("-color_by_couple            display color according to couple number, rgby\n");
      printf("-color_by_couple_rgyb       similar to color_by_couple, but with rgyb\n");
      printf("-color_by_corner            similar to color_by_couple, but make corners match\n");
      printf("-no_sound                   do not make any noise when an error occurs\n");
      printf("-no_intensify               show text in the normal shade instead of extra-bright\n");
      printf("-singlespace                single space the output file\n");
      printf("-single_click               (Sd only) act on single mouse clicks on the menu\n");
      printf("-minigrand_getouts          allow \"mini-grand\" (RLG but promenade on 3rd hand) getouts\n");
      printf("-concept_levels             allow concepts from any level\n");
      printf("-no_warnings                do not display or print any warning messages\n");
      printf("-retain_after_error         retain pending concepts after error\n");
      printf("-active_phantoms            use active phantoms for \"assume\" operations\n");
      printf("-sequence filename          name for sequence output file (def \"%s\")\n",
             SEQUENCE_FILENAME);
      printf("-db filename                calls database file (def \"%s\")\n",
             DATABASE_FILENAME);
      printf("-sequence_num n             use this initial sequence number\n");

      gg->display_help(); // Get any others that the UI wants to tell us about.
      general_final_exit(0);
   }

   enable_file_writing = FALSE;
   interactivity = interactivity_database_init;
   testing_fidelity = FALSE;
   header_comment[0] = 0;
   verify_options.who = selector_uninitialized;
   verify_options.number_fields = 0;
   verify_options.howmanynumbers = 0;
   history_allocation = 15;
   configuration::history = (configuration *) get_mem(history_allocation * sizeof(configuration));

   /* Do general initializations, which currently consist only of
      seeding the random number generator. */
   general_initialize();

   // Read the command line arguments and process the initialization file.
   // This will return TRUE if we are to cease execution immediately.
   // If not, it will have called "initialize_misc_lists" to do lots
   // of initialization of the internal workings, at the appropriate time.
   // That appropriate time is after the level has been determined
   // (so the "on-level concept lists" for the normalize command will
   // be correct) but before the database is analyzed (the internal workings
   // must be initialized before executing calls.)

   if (!open_session(argc, argv))
      run_program();

   // This does a lot more than just exit.  It updates the init file.
   // If deletion of an item in the init file was called for, "open_session"
   // returned false, and "general_final_exit" will do the actual deletion.
   general_final_exit(0);

   /* NOTREACHED */
   return 0;
}
