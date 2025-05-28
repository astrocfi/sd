/* SD -- square dance caller's helper.

    Copyright (C) 1990-1997  William B. Ackerman.

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

#define VERSION_STRING "31.92"
#define TIME_STAMP "wba@apollo.hp.com  28 Dec 97 $"

/* This defines the following functions:
   sd_version_string
   mark_parse_blocks
   release_parse_blocks_to_mark
   get_parse_block
   initialize_parse
   copy_parse_tree
   save_parse_state
   restore_parse_state
   deposit_call
   deposit_concept
   query_for_call
   write_header_stuff
   main
   sequence_is_resolved

and the following external variables:
   abs_max_calls
   max_base_calls
   base_calls
   number_of_taggers
   tagger_calls
   number_of_circcers
   circcer_calls
   outfile_string
   header_comment
   need_new_header_comment
   sequence_number
   last_file_position
   global_age
   parse_state
   uims_menu_index
   database_version
   whole_sequence_low_lim
   interactivity
   testing_fidelity
   selector_for_initialize
   number_for_initialize
   null_options
   verify_options
   verify_used_number
   verify_used_selector
   allowing_modifications
   allowing_all_concepts
   using_active_phantoms
   elide_blanks
   retain_after_error
   alternate_person_glyphs
   singing_call_mode
   diagnostic_mode
   current_options
   selector_iterator
   direction_iterator
   number_iterator
   tagger_iterator
   circcer_iterator
   no_search_warnings
   conc_elong_warnings
   dyp_each_warnings
   useless_phan_clw_warnings
*/


#include <stdio.h>
#include <string.h>
#include "sd.h"
#include "paths.h"

/* We cause this string (that is, the concatentaion of these strings) to appear
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

static Const char id[] = "@(#)$He" "ader: Sd: version " VERSION_STRING "  " TIME_STAMP;
   

extern char *sd_version_string(void)
{
   return VERSION_STRING;
}

Private void display_help(void)
{
   printf("Sd version %s : ui%s\n",
   sd_version_string(), uims_version_string());
   printf("Usage: sd [flags ...] level\n");
   printf("  legal flags:\n");
   printf("-write_list filename        write out list for this level\n");
   printf("-write_full_list filename   write this list and all lower\n");
   printf("-abridge filename           do not use calls in this file\n");
   printf("-singlespace                single space the output file\n");
   printf("-no_warnings                do not display or print any warning messages\n");
   printf("-retain_after_error         retain pending concepts after error\n");
#ifdef OLD_ELIDE_BLANKS_JUNK
   printf("-ignoreblanks               allow user to omit spaces when typing in\n");
#endif
   printf("-active_phantoms            use active phantoms for \"assume\" operations\n");
   printf("-sequence filename          base name for sequence output (def \"%s\")\n",
          SEQUENCE_FILENAME);
   printf("-db filename                calls database file (def \"%s\")\n",
          DATABASE_FILENAME);

   uims_display_help();          /* Get any others that the UI wants to tell us about. */
   exit_program(0);
}


/* These variables are external. */

int abs_max_calls;
int max_base_calls;
callspec_block **base_calls;        /* Gets allocated as array of pointers in sdinit. */
int number_of_taggers[4];
callspec_block **tagger_calls[4];
int number_of_circcers;
callspec_block **circcer_calls;
char outfile_string[MAX_FILENAME_LENGTH] = SEQUENCE_FILENAME;
char header_comment[MAX_TEXT_LINE_LENGTH];
long_boolean need_new_header_comment = FALSE;
call_list_mode_t glob_call_list_mode;
int sequence_number = -1;
int last_file_position = -1;
int global_age;
parse_state_type parse_state;
int uims_menu_index;
char database_version[81];
int whole_sequence_low_lim;
interactivity_state interactivity = interactivity_normal;
long_boolean testing_fidelity = FALSE;
selector_kind selector_for_initialize;
int number_for_initialize;
Const call_conc_option_state null_options = {
   selector_uninitialized,
   direction_uninitialized,
   0, 0, 0, 0, 0};
call_conc_option_state verify_options;
long_boolean verify_used_number;
long_boolean verify_used_selector;
int allowing_modifications = 0;
long_boolean allowing_all_concepts = FALSE;
long_boolean using_active_phantoms = FALSE;
#ifdef OLD_ELIDE_BLANKS_JUNK
long_boolean elide_blanks = FALSE;
#endif
long_boolean retain_after_error = FALSE;
int alternate_person_glyphs = 0;
int singing_call_mode = 0;
long_boolean diagnostic_mode = FALSE;
call_conc_option_state current_options;
uint32 selector_iterator = 0;
uint32 direction_iterator = 0;
uint32 number_iterator = 0;
uint32 tagger_iterator = 0;
uint32 circcer_iterator = 0;
warning_info no_search_warnings = {{0, 0, 0}};
warning_info conc_elong_warnings = {{0, 0, 0}};
warning_info dyp_each_warnings = {{0, 0, 0}};
warning_info useless_phan_clw_warnings = {{0, 0, 0}};

/* These variables are are global to this file. */

static uims_reply reply;
static long_boolean reply_pending;
static int error_flag;
static parse_block *parse_active_list;
static parse_block *parse_inactive_list;
/* These two direct the generation of random concepts when we are searching.
   We make an attempt to generate somewhat plausible concepts, depending on the
   setup we are in.  If we just generated evenly weighted concepts from the entire
   concept list, we would hardly ever get a legal one. */
static int concept_sublist_sizes[NUM_CALL_LIST_KINDS];
static short int *concept_sublists[NUM_CALL_LIST_KINDS];
static int resolve_scan_start_point;
static int resolve_scan_current_point;
static command_kind search_goal;

/* Stuff for saving parse state while we resolve. */

static parse_state_type saved_parse_state;
static parse_block *saved_command_root;

/* This static variable is used by main. */

Private concept_descriptor centers_concept = {"centers????", concept_centers_or_ends, TRUE, l_mainstream, {0, selector_centers, FALSE}};


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
#define MASK_CPLS       0547476
#define MASK_TAND       0770362
#define MASK_SIAM       0400002
#define MASK_2X4        0177762


Private void initialize_concept_sublists(void)
{
   int concept_index;
   int concepts_at_level;
   call_list_kind test_call_list_kind;
   concept_kind end_marker = concept_diagnose;

   /* Decide whether we allow the "diagnose" concept, by deciding
      when we will stop the concept list scan. */
   if (diagnostic_mode) end_marker = marker_end_of_list;

   for (       concept_index = 0, concepts_at_level = 0;
               concept_descriptor_table[concept_index].kind != end_marker;
               concept_index++) {
      if (!(concept_descriptor_table[concept_index].concparseflags & CONCPARSE_MENU_DUP) && concept_descriptor_table[concept_index].level <= calling_level)
         concepts_at_level++;
   }

   general_concept_size = concept_index - general_concept_offset;  /* Our friends in the UI will need this. */

   concept_sublists[call_list_any] = (short int *) get_mem(concepts_at_level*sizeof(short int));

   /* Make the concept sublists, one per setup.  We do them in downward order, with "any setup" last.
      This is because we put our results into the "any setup" slot while we are working, and then
      copy them to the correct slot for each setup other than "any". */

   for (test_call_list_kind = call_list_qtag; test_call_list_kind >= call_list_any; test_call_list_kind--) {
      for (       concept_index = 0, concepts_at_level = 0;
                  concept_descriptor_table[concept_index].kind != end_marker;
                  concept_index++) {
         concept_descriptor *p = &concept_descriptor_table[concept_index];

         if (!(p->concparseflags & CONCPARSE_MENU_DUP) && p->level <= calling_level) {
            uint32 setup_mask = ~0;      /* Default is to accept the concept. */

            /* This concept is legal at this level.  see if it is appropriate for this setup.
               If we don't know, the default value of setup_mask will make it legal. */

            switch (p->kind) {
               case concept_centers_or_ends:
               case concept_centers_and_ends:
                  switch (p->value.arg1) {
                     case selector_center6:
                     case selector_outer6:
                     case selector_center2:
                     case selector_outer2:
                        setup_mask = MASK_CTR_2;    /* This is a 6 and 2 type of concept. */
                        break;
                     default:
                        break;
                  }

                  break;
               case concept_tandem:
               case concept_some_are_tandem:
               case concept_frac_tandem:
               case concept_some_are_frac_tandem:
                  switch (p->value.arg4) {
                     case 0:     /* tandem */
                        setup_mask = MASK_TAND; break;
                     case 1:     /* couples */
                        setup_mask = MASK_CPLS; break;
                     case 2:     /* siamese */
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
                  /* We never allow the "assume_waves" concept.  In the early days,
                     it was actually dangerous.  It isn't dangerous any more,
                     but it's a fairly stupid thing to generate in a search. */
                  setup_mask = 0;
                  break;
            }

            if ((1 << (((int) test_call_list_kind) - ((int) call_list_empty))) & setup_mask)
               concept_sublists[call_list_any][concepts_at_level++] = concept_index;
         }
      }

      concept_sublist_sizes[test_call_list_kind] = concepts_at_level;

      if (test_call_list_kind != call_list_any) {
         concept_sublists[test_call_list_kind] = (short int *) get_mem(concepts_at_level*sizeof(short int));
         (void) memcpy(concept_sublists[test_call_list_kind], concept_sublists[call_list_any], concepts_at_level*sizeof(short int));
      }
   }
}




extern parse_block *mark_parse_blocks(void)
{
   return parse_active_list;
}


extern void release_parse_blocks_to_mark(parse_block *mark_point)
{
   while (parse_active_list && parse_active_list != mark_point) {
      parse_block *item = parse_active_list;

      parse_active_list = item->gc_ptr;
      item->gc_ptr = parse_inactive_list;
      parse_inactive_list = item;

      /* Clear pointers so we will notice if it gets erroneously re-used. */
      item->concept = &mark_end_of_list;
      item->call = base_calls[1];
      item->subsidiary_root = (parse_block *) 0;
      item->next = (parse_block *) 0;
   }
}




extern parse_block *get_parse_block(void)
{
   parse_block *item;

   if (parse_inactive_list) {
      item = parse_inactive_list;
      parse_inactive_list = item->gc_ptr;
   }
   else {
      item = (parse_block *) get_mem(sizeof(parse_block));
   }

   item->gc_ptr = parse_active_list;
   parse_active_list = item;

   item->concept = (concept_descriptor *) 0;
   item->call = (callspec_block *) 0;
   item->options = null_options;
   item->replacement_key = 0;
   item->no_check_call_level = 0;
   item->subsidiary_root = (parse_block *) 0;
   item->next = (parse_block *) 0;

   return item;
}





extern void initialize_parse(void)
{
   int i;

   parse_state.concept_write_base = &history[history_ptr+1].command_root;
   parse_state.concept_write_ptr = parse_state.concept_write_base;
   *parse_state.concept_write_ptr = (parse_block *) 0;
   parse_state.call_list_to_use = find_proper_call_list(&history[history_ptr].state);
   history[history_ptr+1].centersp = 0;
   for (i=0 ; i<WARNING_WORDS ; i++) history[history_ptr+1].warnings.bits[i] = 0;
   history[history_ptr+1].draw_pic = FALSE;

   if (written_history_items > history_ptr)
      written_history_items = history_ptr;

   parse_state.parse_stack_index = 0;
   parse_state.specialprompt[0] = '\0';
   parse_state.topcallflags1 = 0;
}






extern parse_block *copy_parse_tree(parse_block *original_tree)
{
   parse_block *new_item, *old_item, *new_root;

   old_item = original_tree;

   if (!old_item) return NULL;

   new_item = get_parse_block();
   new_root = new_item;
   for (;;) {
      new_item->concept = old_item->concept;
      new_item->call = old_item->call;
      new_item->options = old_item->options;
      new_item->replacement_key = old_item->replacement_key;

      if (old_item->subsidiary_root)
         new_item->subsidiary_root = copy_parse_tree(old_item->subsidiary_root);

      if (!old_item->next) break;

      new_item->next = get_parse_block();
      new_item = new_item->next;
      old_item = old_item->next;
   }

   return new_root;
}



/* Save the entire parse stack, and make a copy of the dynamic blocks that
   comprise the parse state. */
extern void save_parse_state(void)
{
   saved_parse_state = parse_state;
   saved_command_root = copy_parse_tree(history[history_ptr+1].command_root);
}


Private void reset_parse_tree(
   parse_block *original_tree,
   parse_block *final_head)

{
   parse_block *new_item, *old_item;

   new_item = final_head;
   old_item = original_tree;
   for (;;) {
      new_item->concept = old_item->concept;
      new_item->call = old_item->call;
      new_item->options = old_item->options;

      /* Chop off branches that don't belong. */

      if (!old_item->subsidiary_root)
         new_item->subsidiary_root = (parse_block *) 0;
      else
         reset_parse_tree(old_item->subsidiary_root, new_item->subsidiary_root);

      if (!old_item->next) {
         new_item->next = 0;
         break;
      }

      new_item = new_item->next;
      old_item = old_item->next;
   }
}


/* Restore the parse state.  We write directly over the original dynamic blocks
   of the current parse state (making use of the fact that the alterations that
   could have happened will add to the tree but never delete anything.)  This way,
   after we have saved and restored things, they are all in their original,
   locations, so that the pointers in the parse stack will still be valid. */
extern long_boolean restore_parse_state(void)
{
   parse_state = saved_parse_state;

   if (saved_command_root)
      reset_parse_tree(saved_command_root, history[history_ptr+1].command_root);
   else
      history[history_ptr+1].command_root = 0;

   return FALSE;
}



/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
Private long_boolean find_tagger(int tagclass, int *tagg, callspec_block **tagger_call)
{
   if (number_of_taggers[tagclass] == 0) return TRUE;   /* We can't possibly do this. */

   if (interactivity == interactivity_normal || interactivity == interactivity_verify) {
      if ((*tagg = uims_do_tagger_popup(tagclass)) == 0) return TRUE;
      if ((*tagg >> 5) != tagclass) fail("bad tagger class???");
      if ((*tagg & 0x1F) > number_of_taggers[tagclass]) fail("bad tagger index???");
   }
   else {
      int tag;

      if (interactivity == interactivity_database_init) {
         tag = 0;   /* This may not be right. */
      }
      else {
         if (interactivity == interactivity_in_first_scan || interactivity == interactivity_in_second_scan) {
            tag = tagger_iterator;

            /* But we don't allow any call that takes a another tag call (e.g. "revert"),
               or any other iteratable modifier.  It's just too complicated to iterate
               over the space of all calls when things like this happen.
               We also reject any that are marked not to be used in resolve. */

            while (     tag < number_of_taggers[tagclass] &&
                        (  (tagger_calls[tagclass][tag]->callflags1 &
                                 (CFLAG1_DONT_USE_IN_RESOLVE|
                                 CFLAG1_NUMBER_MASK)) ||
                           (tagger_calls[tagclass][tag]->callflagsf &
                                 (CFLAGH__TAG_CALL_RQ_MASK|
                                 CFLAGH__CIRC_CALL_RQ_BIT|
                                 CFLAGH__REQUIRES_SELECTOR|
                                 CFLAGH__REQUIRES_DIRECTION))))
               tag++;

            if (tag == number_of_taggers[tagclass] && tagger_iterator == 0)
               return TRUE;  /* There simply are no acceptable taggers. */

            if ((selector_iterator | direction_iterator | number_iterator) == 0) {
               tagger_iterator = tag+1;

               while (     tagger_iterator < number_of_taggers[tagclass] &&
                           (  (tagger_calls[tagclass][tagger_iterator]->callflags1 &
                                    (CFLAG1_DONT_USE_IN_RESOLVE|
                                    CFLAG1_NUMBER_MASK)) ||
                              (tagger_calls[tagclass][tagger_iterator]->callflagsf &
                                    (CFLAGH__TAG_CALL_RQ_MASK|
                                    CFLAGH__CIRC_CALL_RQ_BIT|
                                    CFLAGH__REQUIRES_SELECTOR|
                                    CFLAGH__REQUIRES_DIRECTION))))
                  tagger_iterator++;

               /* See if we have exhausted all possible taggers. */
               if (tagger_iterator == number_of_taggers[tagclass])
                  tagger_iterator = 0;
            }
         }
         else {
            tag = generate_random_number(number_of_taggers[tagclass]);
         }

         hash_nonrandom_number(tag);
      }

      /* We don't generate "dont_use_in_resolve" taggers in any random search. */
      if (tagger_calls[tagclass][tag]->callflags1 & CFLAG1_DONT_USE_IN_RESOLVE)
         fail("This shouldn't get printed.");

      *tagg = (tagclass << 5) | (tag+1);
   }

   /* At this point, tagg contains 8 bits:
         3 bits of tagger list (zero-based - 0/1/2/3)
         5 bits of tagger within that list (1-based). */

   *tagger_call = tagger_calls[tagclass][(*tagg & 0x1F)-1];

   return FALSE;
}



/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
Private long_boolean find_circcer(int *circcp)
{
   if (number_of_circcers == 0) return TRUE;   /* We can't possibly do this. */

   if (interactivity == interactivity_normal || interactivity == interactivity_verify) {
      if ((*circcp = uims_do_circcer_popup()) == 0)
         return TRUE;
   }
   else if (interactivity == interactivity_database_init) {
      *circcp = 1;   /* This may not be right. */
   }
   else {
      if (interactivity == interactivity_in_first_scan || interactivity == interactivity_in_second_scan) {
         *circcp = circcer_iterator+1;

         if ((selector_iterator | direction_iterator | number_iterator | tagger_iterator) == 0) {
            circcer_iterator++;

            /* See if we have exhausted all possible circcers. */
            if (circcer_iterator == number_of_circcers)
               circcer_iterator = 0;
         }
      }
      else {
         *circcp = generate_random_number(number_of_circcers)+1;
      }

      hash_nonrandom_number(*circcp - 1);
   }

   return FALSE;
}




/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
Private long_boolean find_selector(selector_kind *sel_p, long_boolean allow_iteration)
{
   static selector_kind selector_iterator_table[] = {
      selector_boys,
      selector_girls,
      selector_centers,
      selector_ends,
      selector_leads,
      selector_trailers,
      selector_beaus,
      selector_belles,
      selector_uninitialized};

   if (interactivity == interactivity_normal) {
      int j;

      if ((j = uims_do_selector_popup()) == 0)
         return TRUE;
      else
         *sel_p = (selector_kind) j;
   }
   else if (interactivity == interactivity_database_init || interactivity == interactivity_verify) {
      if (verify_options.who == selector_uninitialized) {
         *sel_p = selector_for_initialize;
         verify_used_selector = 1;
      }
      else
         *sel_p = verify_options.who;
   }
   else {
      int j;

      if (     allow_iteration &&
               (interactivity == interactivity_in_first_scan || interactivity == interactivity_in_second_scan)) {
         j = (int) selector_iterator_table[selector_iterator];

         selector_iterator++;

         /* See if we have exhausted all possible selectors.
               We only look for "boys", "girls", "centers", and "ends" in the first scan. */
         if (selector_iterator_table[selector_iterator] ==
                     (  (interactivity == interactivity_in_first_scan) ?
                        selector_leads :
                        (  (calling_level < beau_belle_level) ?
                           selector_beaus :
                           selector_uninitialized)))
            selector_iterator = 0;
      }
      else {
         /* We don't generate unsymmetrical selectors when searching.  It generates
              too many "couple #3 u-turn-back" calls. */
         j = generate_random_number(unsymm_selector_start-1)+1;
      }

      hash_nonrandom_number(j - 1);
      *sel_p = (selector_kind) j;
   }

   return FALSE;
}


/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
Private long_boolean find_direction(direction_kind *dir_p)
{
   static direction_kind direction_iterator_table[] = {
      direction_left,
      direction_right,
      direction_in,
      direction_out,
      direction_uninitialized};

   if (interactivity == interactivity_normal) {
      int j;

      if ((j = uims_do_direction_popup()) == 0)
         return TRUE;
      else
         *dir_p = (direction_kind) j;
   }
   else if (interactivity == interactivity_database_init || interactivity == interactivity_verify) {
      *dir_p = direction_right;   /* This may not be right. */
   }
   else {
      int j;

      if (interactivity == interactivity_in_first_scan || interactivity == interactivity_in_second_scan) {
         j = (int) direction_iterator_table[direction_iterator];

         if (selector_iterator == 0) {
            direction_iterator++;

            /* See if we have exhausted all possible directions.
               We only look for "left" and "right" in the first scan. */
            if (     direction_iterator_table[direction_iterator] == 
                     ((interactivity == interactivity_in_first_scan) ?
                        direction_in :
                        direction_uninitialized))
               direction_iterator = 0;
         }
      }
      else {
         j = generate_random_number(last_direction_kind)+1;
      }

      hash_nonrandom_number(j - 1);
      *dir_p = (direction_kind) j;
   }

   return FALSE;
}


/* Returns TRUE if it fails, meaning that the user waved the mouse away. */
Private long_boolean find_numbers(int howmanynumbers, long_boolean forbid_zero, uint32 odd_number_only, long_boolean allow_iteration, uint32 *number_list)
{
   if (interactivity == interactivity_normal) {
      *number_list = uims_get_number_fields(howmanynumbers, forbid_zero);

      if ((*number_list) == ~0)
         return TRUE;           /* User waved the mouse away. */
   }
   else {
      int i;

      *number_list = 0;

      for (i=0 ; i<howmanynumbers ; i++) {
         uint32 this_num;

         if (interactivity == interactivity_database_init || interactivity == interactivity_verify) {
            if (verify_options.howmanynumbers == 0) {
               this_num = number_for_initialize;
               verify_used_number = 1;
            }
            else {
               this_num = verify_options.number_fields & 0xF;
               verify_options.number_fields >>= 4;
               verify_options.howmanynumbers--;
            }
         }
         else {
            if (  allow_iteration &&
                  (interactivity == interactivity_in_first_scan || interactivity == interactivity_in_second_scan)) {
               this_num = ((number_iterator >> (i*2)) & 3) + 1;
            }
            else if (odd_number_only)
               this_num = (generate_random_number(2)<<1)+1;
            else
               this_num = generate_random_number(4)+1;

            hash_nonrandom_number(this_num-1);
         }

         *number_list |= (this_num << (i*4));
      }

      if (     (interactivity == interactivity_in_first_scan || interactivity == interactivity_in_second_scan) &&
               (selector_iterator | direction_iterator) == 0) {
         number_iterator++;
         /* If the call requires it, or if doing the first scan and the call
            takes multiple numbers, we iterate over odd numbers only.  The
            reason for the latter clause is that we don't want to wast a lot of
            time enumerating 256 combinations of i-j-k-l quarter the deucey.
            We will get to them on the second scan in any case. */
         if (     odd_number_only ||
                  (howmanynumbers >= 2 && interactivity == interactivity_in_first_scan)) {
            while (number_iterator & 0x55555555)
               number_iterator += number_iterator & ~(number_iterator-1);
         }

         /* See if we have exhausted all possible numbers. */
         if (number_iterator >> (howmanynumbers*2)) number_iterator = 0;
      }
   }

   return FALSE;
}



/* Deposit a call into the parse state.  A returned value of TRUE
   means that the user refused to click on a required number or selector,
   and so we have taken no action.  This can only occur if interactive.
   If not interactive, stuff will be chosen by random number. */

extern long_boolean deposit_call(callspec_block *call, Const call_conc_option_state *options)
{
   parse_block *new_block;
   callspec_block *tagger_call;
   int tagg = 0;
   selector_kind sel = selector_uninitialized;
   direction_kind dir = direction_uninitialized;
   int circc = -1;    /* Circulator index (1-based). */
   uint32 number_list = 0;
   int howmanynums = (call->callflags1 & CFLAG1_NUMBER_MASK) / CFLAG1_NUMBER_BIT;

   /* Put in tagging call info if required. */

   if (call->callflagsf & CFLAGH__TAG_CALL_RQ_MASK) {
      if (find_tagger(
               ((call->callflagsf & CFLAGH__TAG_CALL_RQ_MASK) / CFLAGH__TAG_CALL_RQ_BIT) - 1,
               &tagg,
               &tagger_call))
         return TRUE;
   }

   /* Or circulating call index. */

   if (call->callflagsf & CFLAGH__CIRC_CALL_RQ_BIT) {
      if (find_circcer(&circc)) return TRUE;
   }

   /* Put in selector, direction, and/or number as required. */

   if (call->callflagsf & CFLAGH__REQUIRES_SELECTOR) {
      if (find_selector(&sel, TRUE)) return TRUE;
   }

   if (call->callflagsf & CFLAGH__REQUIRES_DIRECTION)
      if (find_direction(&dir)) return TRUE;

   if (howmanynums != 0)
      if (find_numbers(howmanynums, FALSE, call->callflagsf & CFLAGH__ODD_NUMBER_ONLY, TRUE, &number_list)) return TRUE;

   new_block = get_parse_block();
   new_block->concept = &mark_end_of_list;
   new_block->call = call;
   new_block->options = *options;
   new_block->options.who = sel;
   new_block->options.where = dir;
   new_block->options.number_fields = number_list;
   new_block->options.howmanynumbers = howmanynums;
   new_block->options.tagger = 0;
   new_block->options.circcer = 0;

   /* Filling in the tagger requires recursion! */

   if (tagg > 0) {
      parse_block **savecwp = parse_state.concept_write_ptr;

      new_block->options.tagger = tagg;
      new_block->concept = &marker_concept_mod;
      new_block->next = get_parse_block();
      new_block->next->concept = &marker_concept_mod;

      /* Deposit the index of the base tagging call.  This will of course be replaced. */

      new_block->next->call = base_calls[BASE_CALL_TAGGER0];

      parse_state.concept_write_ptr = &new_block->next->subsidiary_root;
      if (deposit_call(tagger_call, &null_options))
         longjmp(longjmp_ptr->the_buf, 5);     /* User waved the mouse away while getting subcall. */
      parse_state.concept_write_ptr = savecwp;
   }

   /* Filling in the circcer does too, but it isn't serious. */

   if (circc > 0) {
      parse_block **savecwp = parse_state.concept_write_ptr;

      new_block->options.circcer = circc;
      new_block->concept = &marker_concept_mod;
      new_block->next = get_parse_block();
      new_block->next->concept = &marker_concept_mod;

      /* Deposit the index of the base circcing call.  This will of course be replaced. */

      new_block->next->call = base_calls[BASE_CALL_CIRCCER];

      if (circc > number_of_circcers) fail("bad circcer index???");

      parse_state.concept_write_ptr = &new_block->next->subsidiary_root;
      if (deposit_call(circcer_calls[circc-1], &null_options))
         longjmp(longjmp_ptr->the_buf, 5);     /* User waved the mouse away while getting subcall. */
      parse_state.concept_write_ptr = savecwp;
   }

   parse_state.topcallflags1 = call->callflags1;
   *parse_state.concept_write_ptr = new_block;

   return FALSE;
}



/* Deposit a concept into the parse state.  A returned value of TRUE
   means that the user refused to click on a required number or selector,
   and so we have taken no action.  This can only occur if interactive.
   If the interactivity is special, the "number_fields" argument is ignored, and
   necessary stuff will be chosen by random number.  If it is off, the appropriate
   numbers (as indicated by the "CONCPROP__USE_NUMBER" stuff) must be provided. */

extern long_boolean deposit_concept(concept_descriptor *conc)
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



Private void print_error_person(unsigned int person, long_boolean example)
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

extern long_boolean query_for_call(void)
{
   uims_reply local_reply;
   callspec_block *result;
   int old_error_flag;
   int concepts_deposited = 0;

   recurse_entry:

   *parse_state.concept_write_ptr = (parse_block *) 0;   /* We should actually re-use anything there. */

   if (allowing_modifications)
      parse_state.call_list_to_use = call_list_any;

   redisplay:

   if (interactivity == interactivity_normal) {
      /* We are operating in interactive mode.  Update the
         display and query the user. */

      /* Error codes are:
         1 - 1-line error message, text is in error_message1.
         2 - 2-line error message, text is in error_message1 and error_message2.
         3 - collision error, message is that people collided, they are in collision_person1 and collision_person2.
         4 - "resolve" or similar command was called in inappropriate context, text is in error_message1.
         5 - clicked on something inappropriate in subcall reader.
         6 - unable-to-execute error, person is in collision_person1, text is in error_message1.
         7 - wants to display stale call statistics. */

      if (error_flag == 7) {
         clear_screen();
         writestuff("***** LEAST RECENTLY USED 2% OF THE CALLS ARE:");
         newline();
         writestuff("filibuster     peel the toptivate");
         newline();
         writestuff("spin chain and circulate the gears    spin chain and exchange the gears");
         newline();
         error_flag = 0;
         goto try_again;
      }
      
      /* First, update the output area to the current state, with error messages, etc.
         We draw a picture for the last two calls. */
      
      display_initial_history(history_ptr, (diagnostic_mode ? 1 : 2));

      if (sequence_is_resolved()) {
         newline();
         writestuff("     resolve is:");
         newline();
         write_resolve_text(FALSE);
         newline();
      }
      
      if (error_flag) {
      
         if (error_flag != 4) {
            writestuff("Can't do this call:");
            newline();
            write_history_line(0, (char *) 0, FALSE, file_write_no);
         }

         if (error_flag == 3) {
                  /* very special message -- no text here, two people collided
                     and they are stored in collision_person1 and collision_person2. */
   
            writestuff("Some people (");
            print_error_person(collision_person1, FALSE);
            writestuff(" and ");
            print_error_person(collision_person2, TRUE);
            writestuff(") on same spot.");
         }
         else if (error_flag == 4) {
            writestuff(error_message1);
         }
         else if (error_flag <= 2) {
            writestuff(error_message1);
            if (error_flag == 2) {
               newline();
               writestuff("   ");
               writestuff(error_message2);
            }
         }
         else if (error_flag == 6) {
                  /* very special message -- no text here, someone can't execute the
                     call, and he is stored in collision_person1. */
  
            writestuff("Some person (");
            print_error_person(collision_person1, TRUE);
            writestuff(") ");
            writestuff(error_message1);
         }
         else           /* Must be 5, special signal for aborting out of subcall reader. */
            writestuff("You can't select that here.");
      
         newline();
         newline();
      }

      old_error_flag = error_flag; /* save for refresh command */
      error_flag = 0;

      try_again:

      /* Display the call index number, and the partially entered call and/or prompt, as appropriate. */

      /* See if there are partially entered concepts.  If so, print the index number
         and those concepts on a separate line. */

      if (parse_state.concept_write_ptr != &history[history_ptr+1].command_root) {

         /* This prints the concepts entered so far, with a "header" consisting of the index number.
            This partial concept tree is incomplete, so write_history_line has to be (and is) very careful. */

         write_history_line(history_ptr+1, (char *) 0, FALSE, file_write_no);
      }
      else {
         /* No partially entered concepts.  Just do the sequence number. */

         if (!diagnostic_mode) {
            char indexbuf[200];
            (void) sprintf (indexbuf, "%2d: ", history_ptr-whole_sequence_low_lim+2);
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

      if (uims_get_call_command(&local_reply)) goto recurse_entry;

      if (local_reply == ui_command_select) {
         if (uims_menu_index == (int) command_simple_mods) {
            /* Increment "allowing_modifications" up to a maximum of 2. */
            if (allowing_modifications != 2) allowing_modifications++;
            goto check_menu;
         }
         else if (uims_menu_index == (int) command_all_mods) {
            allowing_modifications = 2;
            goto check_menu;
         }
         else if (uims_menu_index == (int) command_toggle_conc_levels) {
            allowing_all_concepts = !allowing_all_concepts;
            goto check_menu;
         }
         else if (uims_menu_index == (int) command_toggle_act_phan) {
            using_active_phantoms = !using_active_phantoms;
            goto check_menu;
         }
#ifdef OLD_ELIDE_BLANKS_JUNK
         else if (uims_menu_index == (int) command_toggle_ignoreblanks) {
            elide_blanks = !elide_blanks;
            goto check_menu;
         }
#endif
         else if (uims_menu_index == (int) command_toggle_retain_after_error) {
            retain_after_error = !retain_after_error;
            goto check_menu;
         }
         else if (uims_menu_index == (int) command_toggle_nowarn_mode) {
            nowarn_mode = !nowarn_mode;
            goto check_menu;
         }
         else if (uims_menu_index == command_refresh) {
             written_history_items = -1; /* suppress optimized display update */
             error_flag = old_error_flag; /* want to see error messages, too */
             goto redisplay;
         }
         else if (uims_menu_index == command_create_comment) {
            char comment[MAX_TEXT_LINE_LENGTH];

            if (uims_do_comment_popup(comment)) {
               char *temp_text_ptr;
               comment_block *new_comment_block;     /* ****** Kludge!!!!! */

               new_comment_block = (comment_block *) get_mem(sizeof(comment_block));
               temp_text_ptr = &new_comment_block->txt[0];
               string_copy(&temp_text_ptr, comment);

               *parse_state.concept_write_ptr = get_parse_block();
               (*parse_state.concept_write_ptr)->concept = &marker_concept_comment;

               (*parse_state.concept_write_ptr)->call = (callspec_block *) new_comment_block;
               /* Advance the write pointer. */
               parse_state.concept_write_ptr = &((*parse_state.concept_write_ptr)->next);
            }
            goto recurse_entry;
         }
         else {
            reply = local_reply;     /* Save this -- top level will need it. */
            return TRUE;
         }
      }
   }
   else {

      /* We are operating in automatic mode.
         We must insert a concept or a call.  Decide which.
         We only insert a concept if in random search, and then only occasionally. */

      local_reply = ui_call_select;

      if (interactivity == interactivity_in_random_search) {
         long_boolean force_random = FALSE;

         switch (search_goal) {
            case command_concept_call:
               if (concepts_deposited == 0) { force_random = TRUE; break; }
               /* FALL THROUGH!!!!! */
            case command_random_call:
            case command_standardize:
               force_random = generate_random_number(8) <
                     ((search_goal == command_standardize) ? STANDARDIZE_CONCEPT_PROBABILITY : CONCEPT_PROBABILITY);

               /* Since we are not going to use the random number in a one-to-one way, we run the risk
                  of not having hashed_randoms uniquely represent what is happening.  To remedy
                  the problem, we hash just the yes-no result of our decision. */

               hash_nonrandom_number(force_random);
               break;
         }

         if (force_random) {
            int j = concept_sublist_sizes[parse_state.call_list_to_use];

            if (j != 0) {    /* If no concepts are available (perhaps some clown has
                                selected "pick concept call" at mainstream) we don't
                                insert a concept. */
               j = generate_random_number(j);
               local_reply = ui_concept_select;
               uims_menu_index = concept_sublists[parse_state.call_list_to_use][j];
               concepts_deposited++;

               /* We give 0 for the number fields.  It gets taken care of later, perhaps
                  not the best way. */
               (void) deposit_concept(&concept_descriptor_table[uims_menu_index]);
            }
         }
      }
   }

   if (local_reply == ui_concept_select) {
      goto recurse_entry;
   }
   else if (local_reply != ui_call_select) {
      reply = local_reply;     /* Save this -- top level will need it. */
      return TRUE;
   }

   /* We have a call.  Get the actual call and deposit it into the parse tree, if we haven't already. */

   if (interactivity == interactivity_starting_first_scan) {
      /* Generate the random starting point for the scan, so it won't be identical
         each time we resolve from this position.  This is the only time that we use
         the random number generator during the initial scans.  Note that this random
         number will NOT be hashed.  But don't use a random number if in diagnostic mode. */

      resolve_scan_start_point =
            (diagnostic_mode) ?
            0 :
            generate_random_number(number_of_calls[parse_state.call_list_to_use]);
      resolve_scan_current_point = resolve_scan_start_point;
      interactivity = interactivity_in_first_scan;
      selector_iterator = 0;
      direction_iterator = 0;
      number_iterator = 0;
      tagger_iterator = 0;
      circcer_iterator = 0;
   }
   else if (interactivity == interactivity_in_first_scan || interactivity == interactivity_in_second_scan) {
      if ((selector_iterator | direction_iterator | number_iterator | tagger_iterator | circcer_iterator) == 0 && resolve_scan_current_point == resolve_scan_start_point) {
         /* Done with this scan.  Advance to the next thing to do. */
         if (interactivity == interactivity_in_first_scan)
            interactivity = interactivity_in_second_scan;
         else
            interactivity = interactivity_in_random_search;
      }
   }


   if (interactivity != interactivity_normal) {
      if (interactivity == interactivity_database_init || interactivity == interactivity_verify)
         result = base_calls[1];     /* Get "nothing". */
      else if (interactivity == interactivity_in_first_scan || interactivity == interactivity_in_second_scan) {
         if ((selector_iterator | direction_iterator | number_iterator | tagger_iterator | circcer_iterator) == 0) {
            resolve_scan_current_point = (resolve_scan_current_point == 0) ?
                                             number_of_calls[parse_state.call_list_to_use]-1 :
                                             resolve_scan_current_point-1;
         }

         result = main_call_lists[parse_state.call_list_to_use][resolve_scan_current_point];

         /* Fix up the "hashed randoms" stuff as though we had generated this number through the random number generator. */

         hash_nonrandom_number(resolve_scan_current_point);
      }
      else {    /* In random search. */
         int i = generate_random_number(number_of_calls[parse_state.call_list_to_use]);
         hash_nonrandom_number(i);
         result = main_call_lists[parse_state.call_list_to_use][i];
      }

      /* Why don't we just call the random number generator again if the call is inappropriate?
         Wouldn't that be much faster?  There are two reasons:  First, we would need to take
         special precautions against an infinite loop.  Second, and more importantly, if we
         just called the random number generator again, it would screw up the hash numbers,
         which would make the uniquefication fail, so we could see the same thing twice. */
   
      if (     (search_goal == command_level_call || search_goal == command_8person_level_call) &&
               ((dance_level) result->level) < level_threshholds[calling_level]) fail("Level reject.");
      if (result->callflags1 & CFLAG1_DONT_USE_IN_RESOLVE) fail("This shouldn't get printed.");
      if (deposit_call(result, &null_options)) goto recurse_entry;
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



#ifdef NEGLECT
Private int age_buckets[33];

Private int mark_aged_calls(
   int rr,
   int dd,
   int kk)
{
   int r, d, k;
   int i, j, l, hibit, sum, remainder;
   unsigned int lomask, bits, lm;

   r = rr;
   d = dd;
   k = kk;

   startover:

   lomask = (1<<k)-1;

   for (i=0; i<33; i++) age_buckets[i] = 0;

   for (i=0; i<number_of_calls[call_list_any]; i++) {
      if ((main_call_lists[call_list_any][i]->age & (~lomask)) == r) {
         bits = main_call_lists[call_list_any][i]->age & lomask;

         hibit = -1;
         for (l=31, lm=0x80000000; l>=0; l--, lm>>=1) {
            if (bits & lm) { hibit = l; break; }
         }
         age_buckets[hibit+1]++;
      }
   }

   /* The buckets are now filled. */

   sum = 0;
   for (j = -1; j < 32; j++) {
      if ((sum + age_buckets[j+1]) > d) break;
      sum += age_buckets[j+1];
   }

   /* J tells how many buckets to mark, and sum tells how many items are in them. */

   remainder = d;

   if (j == -1) j = 0;

   for (i=0; i<number_of_calls[call_list_any]; i++) {
      if ((main_call_lists[call_list_any][i]->age & (~lomask)) == r) {
         bits = main_call_lists[call_list_any][i]->age & lomask;

         hibit = -1;
         for (l=31, lm=0x80000000; l>=0; l--, lm>>=1) {
            if (bits & lm) { hibit = l; break; }
         }

         if (hibit < j) {
            BOGUS USE OF CALLFLAGSH! main_call_lists[call_list_any][i]->callflagsh |= 0x80000000;
            remainder--;
         }
      }
   }

   if (remainder > 0) {
      if (j >= 0) {
         r = r | (1<<j);
         d = remainder;
         k = j;
         goto startover;
      }
   }

   return remainder;
}
#endif




extern void write_header_stuff(long_boolean with_ui_version, uint32 act_phan_flags)
{
   if (!diagnostic_mode) {
      /* log creation version info */
      if (with_ui_version) {     /* This is the "pretty" form that we display while running. */
         writestuff("Sd ");
         writestuff(sd_version_string());
         writestuff(" : db");
         writestuff(database_version);
         writestuff(" : ui");
         writestuff(uims_version_string());
      }
      else {                     /* This is the "compact" form that goes into the file. */
         writestuff("Sd");
         writestuff(sd_version_string());
         writestuff(":db");
         writestuff(database_version);
      }
   }

   if (act_phan_flags & RESULTFLAG__ACTIVE_PHANTOMS_ON) {
      if (act_phan_flags & RESULTFLAG__ACTIVE_PHANTOMS_OFF)
         writestuff(" (AP-)");
      else
         writestuff(" (AP)");
   }

   writestuff("     ");

   /* log level info */
   writestuff(getout_strings[calling_level]);

   if (glob_call_list_mode == call_list_mode_abridging)
      writestuff(" (abridged)");
}


/* Returns TRUE if it successfully backed up one parse block. */
Private long_boolean backup_one_item(void)
{
/* User wants to undo a call.  The concept parse list is not set up
   for easy backup, so we search forward from the beginning. */

   parse_block **this_ptr = parse_state.concept_write_base;

   if ((history_ptr == 1) && startinfolist[history[1].centersp].into_the_middle) this_ptr = &((*this_ptr)->next);

   for (;;) {
      parse_block **last_ptr;

      if (!*this_ptr) break;
      last_ptr = this_ptr;
      this_ptr = &((*this_ptr)->next);

      if (this_ptr == parse_state.concept_write_ptr) {
         parse_state.concept_write_ptr = last_ptr;

         /* See whether we need to destroy a frame in the parse stack. */
         if (parse_state.parse_stack_index != 0 &&
               parse_state.parse_stack[parse_state.parse_stack_index-1].concept_write_save_ptr == last_ptr)
            parse_state.parse_stack_index--;

         *last_ptr = (parse_block *) 0;
         return TRUE;
      }

      if ((*last_ptr)->concept->kind <= marker_end_of_list) break;
   }

   /* We did not find our place. */

   return FALSE;
}


/* return TRUE if sequence was written */

Private long_boolean write_sequence_to_file(void)
{
   int getout_ind;
   char date[MAX_TEXT_LINE_LENGTH];
   char second_header[MAX_TEXT_LINE_LENGTH];
   char seqstring[20];
   int j;

   /* Put up the getout popup to see if the user wants to enter a header string. */

   getout_ind = uims_do_getout_popup(second_header);

   /* Some user interfaces (those with icons) may have an icon to abort the
      sequence, rather than just decline the comment.  Such an action comes
      back as "POPUP_DECLINE". */

   if (getout_ind == POPUP_DECLINE)
      return FALSE;    /* User didn't want to end this sequence after all. */
   else if (getout_ind != POPUP_ACCEPT_WITH_STRING) second_header[0] = '\0';

   /* Open the file and write it. */

   clear_screen();
   open_file();
   enable_file_writing = TRUE;
   doublespace_file();

   get_date(date);
   writestuff(date);
   writestuff("     ");
   write_header_stuff(FALSE, history[history_ptr].state.result_flags);

   newline();

   if (!sequence_is_resolved()) {
      writestuff("             NOT RESOLVED");
      newline();
   }

   if (singing_call_mode != 0) {
      writestuff(
         (singing_call_mode == 2) ?
         "             Singing call reverse progression" :
         "             Singing call progression");
      newline();
   }

   /* Write header comment, if it exists. */

   if (header_comment[0]) {
      writestuff("             ");
      writestuff(header_comment);
   }

   if (sequence_number >= 0) {
      (void) sprintf(seqstring, "#%d", sequence_number);
      writestuff("   ");
      writestuff(seqstring);
   }

   /* Write secondary header comment, if it exists. */

   if (second_header[0]) {
      writestuff("       ");
      writestuff(second_header);
   }

   if (header_comment[0] || second_header[0] || sequence_number >= 0) newline();

   newline();

   if (sequence_number >= 0) sequence_number++;

   for (j=whole_sequence_low_lim; j<=history_ptr; j++)
      write_history_line(j, (char *) 0, FALSE, file_write_double);

   /* Echo the concepts entered so far.  */

   if (parse_state.concept_write_ptr != &history[history_ptr+1].command_root) {
      write_history_line(history_ptr+1, (char *) 0, FALSE, file_write_double);
   }

   if (sequence_is_resolved())
      write_resolve_text(TRUE);

   newline();
   enable_file_writing = FALSE;
   newline();

   close_file();     /* This will signal a "specialfail" if a file error occurs. */

   writestuff("Sequence");

   if (sequence_number >= 0) {
      writestuff(" ");
      writestuff(seqstring);
   }

   writestuff(" written to \"");
   writestuff(outfile_string);
   writestuff("\".");
   newline();

   global_age++;
   return TRUE;
}


void main(int argc, char *argv[])
{
   int i;

   enable_file_writing = FALSE;
   singlespace_mode = FALSE;
   nowarn_mode = FALSE;
   interactivity = interactivity_database_init;
   testing_fidelity = FALSE;
   parse_active_list = (parse_block *) 0;
   parse_inactive_list = (parse_block *) 0;
   header_comment[0] = '\0';
   verify_options.who = selector_uninitialized;
   verify_options.number_fields = 0;
   verify_options.howmanynumbers = 0;

   if (argc >= 2 && strcmp(argv[1], "-help") == 0)
       display_help();		/* does not return */

   /* Do general initializations, which currently consist only of
      seeding the random number generator. */
   general_initialize();

   /* Read the command line arguments and process the initialization file.
      This will return TRUE if we are to cease execution immediately. */

   if (open_session(argc, argv)) goto normal_exit;

   /* We need to take away the "zig-zag" directions if the level is below A2. */

   if (calling_level < zig_zag_level) {
      last_direction_kind = direction_zigzag-1;
      direction_names[direction_zigzag] = (Cstring) 0;
   }

   if (glob_call_list_mode == call_list_mode_none || glob_call_list_mode == call_list_mode_abridging)
      uims_preinitialize();

   if (history == 0) {
      history_allocation = 15;
      history = (configuration *) get_mem(history_allocation * sizeof(configuration));
   }

   /* A few other modules want to initialize some static tables. */

   initialize_tandem_tables();
   initialize_getout_tables();
   initialize_restr_tables();
   initialize_conc_tables();
   initialize_map_tables();
   initialize_touch_tables();

   initialize_menus(glob_call_list_mode);    /* This sets up max_base_calls. */

   /* If we wrote a call list file, that's all we do. */
   if (glob_call_list_mode == call_list_mode_writing || glob_call_list_mode == call_list_mode_writing_full)
      goto normal_exit;

   initialize_concept_sublists();

   uims_postinitialize();

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

   global_age = 1;

   /* Create the top level error handler. */

   longjmp_ptr = &longjmp_buffer;          /* point the global pointer at it. */
   error_flag = setjmp(longjmp_buffer.the_buf);

   if (error_flag) {

      /* The call we were trying to do has failed.  Abort it and display the error message. */
   
      if (interactivity == interactivity_database_init || interactivity == interactivity_verify) {
         init_error(error_message1);
         goto normal_exit;
      }

      history[0] = history[history_ptr+1];     /* So failing call will get printed. */
      history[0].command_root = copy_parse_tree(history[0].command_root);  /* But copy the parse tree, since we are going to clip it. */
      for (i=0 ; i<WARNING_WORDS ; i++)
         history[0].warnings.bits[i] = 0;         /* But without any warnings we may have collected. */
   
      if (error_flag == 5) {
         /* Special signal -- user clicked on special thing while trying to get subcall. */
         if ((reply == ui_command_select) &&
              ((uims_menu_index == command_quit) ||
               (uims_menu_index == command_undo) ||
               (uims_menu_index == command_erase) ||
               (uims_menu_index == command_abort)))
            reply_pending = TRUE;
            goto start_with_pending_reply;
      }
   
      /* Try to remove the call from the current parse tree, but leave everything else
         in place.  This will fail if the parse tree, or our place on it, is too
         complicated.  Also, we do not do it if in diagnostic mode, or if the user
         did not specify "retain_after_error", or if the special "heads into the middle and ..."
         operation is in place. */

      if (     !diagnostic_mode && 
               retain_after_error &&
               ((history_ptr != 1) || !startinfolist[history[1].centersp].into_the_middle) &&
               backup_one_item()) {
         reply_pending = FALSE;
         for (i=0 ; i<WARNING_WORDS ; i++)                    /* Take out warnings that arose from the failed call, */
            history[history_ptr+1].warnings.bits[i] = 0;      /* since we aren't going to do that call. */
         goto simple_restart;
      }
      goto start_cycle;      /* Failed, reinitialize the whole line. */
   }
   
   interactivity = interactivity_normal;

   /* HERE IS (APPROXIMATELY) WHERE THE PROGRAM STARTS. */
   
   clear_screen();

   if (!diagnostic_mode) {
      FILE *session;

      writestuff("SD -- square dance caller's helper.");
      newline();
      newline();
      writestuff("Copyright (c) 1991-1997 William B. Ackerman");
      newline();
      writestuff("   and Stephen Gildea.");
      newline();
      writestuff("Copyright (c) 1990-1993 Alan Snyder");
      newline();
      writestuff("SD comes with ABSOLUTELY NO WARRANTY;");
      newline();
      writestuff("   for details see the license.");
      newline();
      writestuff("This is free software, and you are");
      newline();
      writestuff("   welcome to redistribute it.");
      newline();
      newline();
      uims_display_ui_intro_text();   /* Sdtty shows additional stuff about typing question mark. */

      session = fopen(SESSION_FILENAME, "r");
      if (session) {
         (void) fclose(session);
      }
      else {
         writestuff("You do not have a session control file.  If you want to create one, give the command \"initialize session file\".");
         newline();
         newline();
      }
   }

   show_banner:

   writestuff("Version ");
   write_header_stuff(TRUE, 0);
   newline();
   writestuff("Output file is \"");
   writestuff(outfile_string);
   writestuff("\"");
   newline();

   if (need_new_header_comment) {
      (void) uims_do_header_popup(header_comment);
      need_new_header_comment = FALSE;
   }
   
   new_sequence:
   
   /* Here to start a fresh sequence.  If first time, or if we got here by clicking on "abort",
      the screen has been cleared.  Otherwise, it shows the last sequence that we wrote. */

   /* Replace all the parse blocks left from the last sequence. */
   release_parse_blocks_to_mark((parse_block *) 0);
   
   /* Query for the starting setup. */
   
   reply = uims_get_startup_command();

   if (reply == ui_command_select && uims_menu_index == command_quit) goto normal_exit;
   if (reply != ui_start_select || uims_menu_index == 0) goto normal_exit;           /* Huh? */

   switch (uims_menu_index) {
      case start_select_toggle_conc:
         allowing_all_concepts = !allowing_all_concepts;
         goto new_sequence;
      case start_select_toggle_act:
         using_active_phantoms = !using_active_phantoms;
         goto new_sequence;
#ifdef OLD_ELIDE_BLANKS_JUNK
      case start_select_toggle_ignoreblank:
         elide_blanks = !elide_blanks;
         goto new_sequence;
#endif
      case start_select_toggle_retain:
         retain_after_error = !retain_after_error;
         goto new_sequence;
      case start_select_toggle_nowarn_mode:
         nowarn_mode = !nowarn_mode;
         goto new_sequence;
      case start_select_toggle_singer:
         if (singing_call_mode != 0)
            singing_call_mode = 0;    /* Turn it off. */
         else
            singing_call_mode = 1;    /* 1 for forward progression, 2 for backward. */
         goto new_sequence;
      case start_select_toggle_singer_backward:
         if (singing_call_mode != 0)
            singing_call_mode = 0;    /* Turn it off. */
         else
            singing_call_mode = 2;
         goto new_sequence;
      case start_select_init_session_file:
         {
            FILE *session = fopen(SESSION_FILENAME, "r");
            if (session) {
               (void) fclose(session);
               if (uims_do_session_init_popup() != POPUP_ACCEPT) {
                  writestuff("No action has been taken.\n");
                  newline();
                  goto new_sequence;
               }
               else if (!rename(SESSION_FILENAME, SESSION2_FILENAME)) {
                  writestuff("File '" SESSION_FILENAME "' has been saved as '" SESSION2_FILENAME "'.\n");
                  newline();
               }
            }

            if (!(session = fopen(SESSION_FILENAME, "w"))) {
               writestuff("Failed to create '" SESSION_FILENAME "'.\n");
               newline();
               goto new_sequence;
            }

            if (fputs("[Options]\n", session) == EOF) goto copy_failed;
            if (fputs("\n", session) == EOF) goto copy_failed;
            if (fputs("[Sessions]\n", session) == EOF) goto copy_failed;
            if (fputs("sequence.C1          C1               1      Sample\n", session) == EOF) goto copy_failed;
            if (fputs("\n", session) == EOF) goto copy_failed;
            (void) fclose(session);
            writestuff("The file has been initialized, and will take effect the next time the program is started.");
            newline();
            writestuff("Exit and restart the program if you want to use it now.");
            newline();
            goto new_sequence;

            copy_failed:

            writestuff("Failed to create '" SESSION_FILENAME "'\n");
            newline();
            (void) fclose(session);
            goto new_sequence;
         }
      case start_select_change_outfile:
         {
            char newfile_string[MAX_FILENAME_LENGTH];

            if (uims_do_outfile_popup(newfile_string)) {
               if (newfile_string[0]) {
                  if (install_outfile_string(newfile_string)) {
                     char confirm_message[MAX_FILENAME_LENGTH+25];
                     (void) strncpy(confirm_message, "Output file changed to \"", 25);
                     (void) strncat(confirm_message, outfile_string, MAX_FILENAME_LENGTH);
                     (void) strncat(confirm_message, "\"", 2);
                     writestuff(confirm_message);
                  }
                  else
                     writestuff("No write access to that file, no action taken.");

                  newline();
               }
            }
         }
         goto new_sequence;
      case start_select_change_header_comment:
         (void) uims_do_header_popup(header_comment);
         need_new_header_comment = FALSE;
         goto new_sequence;
   }
   
   history_ptr = 1;              /* Clear the position history. */

   whole_sequence_low_lim = 2;
   if (!startinfolist[uims_menu_index].into_the_middle) whole_sequence_low_lim = 1;

   for (i=0 ; i<WARNING_WORDS ; i++) history[1].warnings.bits[i] = 0;
   history[1].draw_pic = FALSE;
   history[1].centersp = uims_menu_index;
   history[1].resolve_flag.kind = resolve_none;
   /* Put the people into their starting position. */
   history[1].state = startinfolist[uims_menu_index].the_setup;
   written_history_items = -1;

   error_flag = 0;
   
   /* Come here to read a bunch of concepts and a call and add an item to the history. */
   
   start_cycle:

   reply_pending = FALSE;

   start_with_pending_reply:

   allowing_modifications = 0;

   /* See if we need to increase the size of the history array.
      We must have history_allocation at least equal to history_ptr+2,
      so that history items [0..history_ptr+1] will exist.
      We also need to allow for MAX_RESOLVE_SIZE extra items, so that the
      resolver can work.  Why don't we just increase the allocation
      at the start of the resolver if we are too close?  We tried that once.
      The resolver uses the current parse state, so we can do "TANDEM <resolve>".
      This means that things like "parse_state.concept_write_base", which point
      into the history array, must remain valid.  So the resolver can't reallocate
      the history array.  There is only one place where it is safe to reallocate,
      and that is right here.  Note that we are about to call "initialize_parse",
      which destroys any lingering pointers into the history array. */

   if (history_allocation < history_ptr+MAX_RESOLVE_SIZE+2) {
      configuration * t;
      history_allocation <<= 1;
      t = (configuration *) get_more_mem_gracefully(history, history_allocation * sizeof(configuration));
      if (!t) {
         /* Couldn't get memory; we are in serious trouble. */
         history_allocation >>= 1;
         /* Bring history_ptr down to safe size.  This will have the effect of
            throwing away the last call, or part or all of the last resolve. */
         history_ptr = history_allocation-MAX_RESOLVE_SIZE-2;
         specialfail("Not enough memory!");
      }
      history = t;
   }

   initialize_parse();

   /* Check for first call given to heads or sides only. */
   
   if ((history_ptr == 1) && startinfolist[history[1].centersp].into_the_middle)
      deposit_concept(&centers_concept);
   
   /* Come here to get a concept or call or whatever from the user. */
   
   /* Display the menu and make a choice!!!! */
   
   simple_restart:

   if ((!reply_pending) && (!query_for_call())) {
      /* User specified a call (and perhaps concepts too). */

      /* The call to toplevelmove may make a call to "fail", which will get caught by the cleanup handler
         above, reset history_ptr, and go to start_cycle with the error message displayed. */

      toplevelmove();
      finish_toplevelmove();
      
      /* Call successfully completed and has been stored in history. */
      
      history_ptr++;

      goto start_cycle;
   }
   
   /* If get here, query_for_call exitted without completing its parse, because the operator
      selected something like "quit", "undo", or "resolve", or because we have such a command
      already pending. */

   reply_pending = FALSE;

   if (reply == ui_command_select) {
      switch ((command_kind) uims_menu_index) {
         case command_quit:
            if (uims_do_abort_popup() != POPUP_ACCEPT) goto simple_restart;
            goto normal_exit;
         case command_abort:
            if (uims_do_abort_popup() != POPUP_ACCEPT) goto simple_restart;
            clear_screen();
            goto show_banner;
         case command_undo:
            if (backup_one_item()) {
               /* We succeeded in backing up by one concept.  Continue from that point. */
               reply_pending = FALSE;
               goto simple_restart;
            }
            else if (parse_state.concept_write_base != parse_state.concept_write_ptr ||
                     parse_state.concept_write_base != &history[history_ptr+1].command_root) {
               /* Failed to back up, but some concept exists.  This must have been inside
                  a "checkpoint" or similar complex thing.  Just throw it all away,
                  but do not delete any completed calls. */
               reply_pending = FALSE;
               goto start_cycle;
            }
            else {
               /* There were no concepts entered.  Throw away the entire preceding line. */
               if (history_ptr > 1) history_ptr--;
               /* Going to start_cycle will make sure written_history_items does not exceed history_ptr. */
               goto start_cycle;
            }
         case command_erase:
            reply_pending = FALSE;
            goto start_cycle;
         case command_save_pic:
            history[history_ptr].draw_pic = TRUE;
            /* We have to back up to BEFORE the item we just changed. */
            if (written_history_items > history_ptr-1)
               written_history_items = history_ptr-1;
            goto simple_restart;
         case command_change_outfile:
            {
               char newfile_string[MAX_FILENAME_LENGTH];

               if (uims_do_outfile_popup(newfile_string)) {
                  if (newfile_string[0]) {
                     if (install_outfile_string(newfile_string)) {
                        char confirm_message[MAX_FILENAME_LENGTH+25];
                        (void) strncpy(confirm_message, "Output file changed to \"", 25);
                        (void) strncat(confirm_message, outfile_string, MAX_FILENAME_LENGTH);
                        (void) strncat(confirm_message, "\"", 2);
                        specialfail(confirm_message);
                     }
                     else
                        specialfail("No write access to that file, no action taken.");
                  }
               }
               goto start_cycle;
            }
         case command_change_header:
            {
               char newhead_string[MAX_TEXT_LINE_LENGTH];
         
               if (uims_do_header_popup(newhead_string)) {
                  (void) strncpy(header_comment, newhead_string, MAX_TEXT_LINE_LENGTH);

                  if (newhead_string[0]) {
                     char confirm_message[MAX_TEXT_LINE_LENGTH+25];
                     (void) strncpy(confirm_message, "Header comment changed to \"", 28);
                     (void) strncat(confirm_message, header_comment, MAX_TEXT_LINE_LENGTH);
                     (void) strncat(confirm_message, "\"", 2);
                     specialfail(confirm_message);
                  }
                  else {
                     specialfail("Header comment deleted");
                  }
               }
               goto start_cycle;
            }
#ifdef NEGLECT
         case command_neglect:
            {
               char percentage_string[MAX_TEXT_LINE_LENGTH];
               uims_reply local_reply;
               char title[MAX_TEXT_LINE_LENGTH];
               int percentage, calls_to_mark, i, deficit, final_percent;
               call_list_kind dummy = call_list_any;
      
               if (uims_do_neglect_popup(percentage_string)) {
                  percentage = parse_number(percentage_string);
                  if ((percentage < 1) || (percentage > 99)) goto start_cycle;
               }
               else {
                  percentage = 25;
               }
      
               calls_to_mark = number_of_calls[call_list_any] * percentage / 100;
               if (calls_to_mark > number_of_calls[call_list_any])
                  calls_to_mark = number_of_calls[call_list_any];
      
               start_neglect:
      
               /* Clear all the marks. */
      
               for (i=0; i<number_of_calls[call_list_any]; i++) {
                  BOGUS USE OF CALLFLAGSH! main_call_lists[call_list_any][i]->callflagsh &= ~0x80000000;
               }
      
               deficit = mark_aged_calls(0, calls_to_mark, 31);
      
               /* Determine percentage that were actually marked. */
      
               final_percent = ((calls_to_mark-deficit) * 100) / number_of_calls[call_list_any];
               if (final_percent > 100) final_percent = 100;
      
               fill_in_neglect_percentage(title, final_percent);
               clear_screen();
               writestuff(title);
               newline();
      
               /* Print the marked calls. */
            
               for (i=0; i<number_of_calls[call_list_any]; i++) {
                  BOGUS USE OF CALLFLAGSH! if (main_call_lists[call_list_any][i]->callflagsh & 0x80000000) {
                     writestuff(main_call_lists[call_list_any][i]->name);
                     writestuff(", ");
                  }
               }
               newline();
      
               error_flag = 0;
               
/* **** this call is no longer in conformance with the procedure's behavior */
               local_reply = uims_get_call_command(&dummy);
            
               if (local_reply == ui_call_select) {
                  /* Age this call. */
                  main_call_lists[call_list_any][uims_menu_index]->age = global_age;
                  goto start_neglect;
               }
               else
                  goto start_cycle;
            }
#endif
         case command_getout:
            /* Check that it is really resolved. */

            if (!sequence_is_resolved()) {
               if (uims_do_write_anyway_popup() != POPUP_ACCEPT) specialfail("This sequence is not resolved.");
            }

            if (!write_sequence_to_file())
               goto start_cycle; /* user cancelled action */
            goto new_sequence;
         default:     /* Should be some kind of search command. */
            if (((command_kind) uims_menu_index) < command_resolve) goto normal_exit;   /* It wasn't.  We have a serious problem. */
            search_goal = (command_kind) uims_menu_index;
            reply = full_resolve(search_goal);

            /* If full_resolve refused to operate (for example, we clicked on "reconcile"
               when in an hourglass), it returned "ui_search_accept", which will cause
               us simply to go to start_cycle. */

            /* If user clicked on something random, treat it as though he clicked on "accept"
               followed by whatever it was.  This includes "quit", "abort", "end this sequence",
               and any further search commands.  So a search command (e.g. "pick random call")
               will cause the last search result to be accepted, and begin another search on top
               of that result. */

            if (reply == ui_command_select) {

               switch ((command_kind) uims_menu_index) {
                  case command_quit:
                  case command_abort:
                  case command_getout:
                     break;
                  default:
                     if (((command_kind) uims_menu_index) < command_resolve) goto start_cycle;
                     break;
               }

               allowing_modifications = 0;
               history[history_ptr+1].draw_pic = FALSE;
               parse_state.concept_write_base = &history[history_ptr+1].command_root;
               parse_state.concept_write_ptr = parse_state.concept_write_base;
               *parse_state.concept_write_ptr = (parse_block *) 0;
               reply_pending = TRUE;
               /* Going to start_with_pending_reply will make sure written_history_items does not exceed history_ptr. */
               goto start_with_pending_reply;
            }

            goto start_cycle;
      }
   }
   else
      goto normal_exit;

   normal_exit:
   
   exit_program(0);
}


extern long_boolean sequence_is_resolved(void)
{
   return history[history_ptr].resolve_flag.kind != resolve_none;
}
