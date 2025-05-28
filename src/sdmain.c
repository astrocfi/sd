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

    The version of this file is as shown immediately below.  This string
    gets displayed at program startup.

    ATTENTION!  If you modify the program, we recommend that
    you change the version string below to identify the change.
    This will facilitate tracking errors, since the program version
    will be written onto every sequence.  Changing the version
    string is also required by paragraphs 2(a) and 2(c) of the GNU
    General Public License if you distribute the file.
*/

#define VERSION_STRING "29.2"

/* This defines the following functions:
   sd_version_string
   mark_parse_blocks
   release_parse_blocks_to_mark
   initialize_parse
   copy_parse_tree
   save_parse_state
   restore_parse_state
   deposit_call
   deposit_concept
   query_for_call
   write_header_stuff
   main
   write_sequence_to_file
   get_real_subcall
   sequence_is_resolved

and the following external variables:
   abs_max_calls
   max_base_calls
   base_calls
   outfile_string
   last_file_position
   global_age
   parse_state
   uims_menu_index
   database_version
   whole_sequence_low_lim
   not_interactive
   initializing_database
   testing_fidelity
   selector_for_initialize
   allowing_modifications
   allowing_all_concepts
*/


#include <stdio.h>
#include <string.h>
#include "sd.h"
#include "paths.h"
   
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
    printf("-sequence filename          base name for sequence output (def \"%s\")\n",
           SEQUENCE_FILENAME);
    printf("-db filename                calls database file (def \"%s\")\n",
           DATABASE_FILENAME);
    printf("\nIn addition, the usual window system flags are supported.\n");
    exit_program(0);
}


/* These variables are external. */

int abs_max_calls;
int max_base_calls;
callspec_block **base_calls;        /* Gets allocated as array of pointers in sdinit. */
char outfile_string[MAX_FILENAME_LENGTH] = SEQUENCE_FILENAME;
int last_file_position = -1;
int global_age;
parse_state_type parse_state;
int uims_menu_index;
char database_version[81];
int whole_sequence_low_lim;
long_boolean not_interactive = FALSE;
long_boolean initializing_database = FALSE;
long_boolean testing_fidelity = FALSE;
selector_kind selector_for_initialize;
int allowing_modifications = 0;
long_boolean allowing_all_concepts = FALSE;

/* These variables are are global to this file. */

Private uims_reply reply;
Private long_boolean reply_pending;
Private int error_flag;
Private parse_block *parse_active_list;
Private parse_block *parse_inactive_list;
Private int concept_sublist_sizes[NUM_CALL_LIST_KINDS];
Private short int *concept_sublists[NUM_CALL_LIST_KINDS];


/* Stuff for saving parse state while we resolve. */

Private parse_state_type saved_parse_state;
Private parse_block *saved_command_root;




/* This static variable is used by main. */

Private concept_descriptor centers_concept = {"centers????", concept_centers_or_ends, l_mainstream, {0, 0}};


/* This fills in concept_sublist_sizes and concept_sublists. */

/*   A "1" means the concept is allowed in this setup

                     lout, lin, tby
                           |
                           |+- 8ch, Lcol, col
            2fl, Lwv, wv -+||
                          |||+- cdpt, dpt, L1x8
        qtag, gcol, L2fl-+||||
                         |||||+- 1x8, any, junk
                         ||||||       */
#define MASK_CTR_2      0600016
#define MASK_QUAD_D     0200016
#define MASK_CPLS       0547476
#define MASK_TAND       0770362
#define MASK_SIAM       0400002

Private void initialize_concept_sublists(void)
{
   int number_of_concepts;
   int concepts_at_level;
   call_list_kind test_call_list_kind;
   unsigned long int setup_mask;

   for (       number_of_concepts = 0, concepts_at_level = 0;
               concept_descriptor_table[number_of_concepts].kind != marker_end_of_list;
               number_of_concepts++) {
      if (concept_descriptor_table[number_of_concepts].level <= calling_level)
         concepts_at_level++;
   }
   general_concept_size = number_of_concepts - general_concept_offset;

   concept_sublist_sizes[call_list_any] = concepts_at_level;
   concept_sublists[call_list_any] = (short int *) get_mem(concepts_at_level*sizeof(short int));

   /* Make the concept sublists, one per setup. */

   for (test_call_list_kind = call_list_qtag; test_call_list_kind > call_list_any; test_call_list_kind--) {
      for (       number_of_concepts = 0, concepts_at_level = 0;
                  concept_descriptor_table[number_of_concepts].kind != marker_end_of_list;
                  number_of_concepts++) {
         concept_descriptor *p = &concept_descriptor_table[number_of_concepts];

         if (p->level <= calling_level) {
            setup_mask = ~0;
            /* This concept is legal at this level.  see if it is appropriate for this setup.
               If we don't know, the default value of setup_mask will make it legal. */
            switch (p->kind) {
               case concept_centers_or_ends: case concept_centers_and_ends:
                  if (p->value.arg1 >= 2) {
                     setup_mask = MASK_CTR_2;    /* This is a 6 and 2 type of concept. */
                  }
                  break;
               case concept_tandem: case concept_some_are_tandem:
                  if (!(p->value.arg2 | p->value.arg3)) {
                     switch (p->value.arg4) {
                        case 0:     /* tandem */
                           setup_mask = MASK_TAND; break;
                        case 1:     /* couples */
                           setup_mask = MASK_CPLS; break;
                        default:    /* siamese */
                           setup_mask = MASK_SIAM; break;
                     }
                  }
                  break;
               case concept_quad_diamonds: case concept_quad_diamonds_together:
                  setup_mask = MASK_QUAD_D;
                  break;
            }

            if ((1 << ((int) test_call_list_kind)) & setup_mask)
               concept_sublists[call_list_any][concepts_at_level++] = number_of_concepts;
         }
      }

      concept_sublist_sizes[test_call_list_kind] = concepts_at_level;
      concept_sublists[test_call_list_kind] = (short int *) get_mem(concepts_at_level*sizeof(short int));
      (void) memcpy(concept_sublists[test_call_list_kind], concept_sublists[call_list_any], concepts_at_level*sizeof(short int));
   }

   for (       number_of_concepts = 0, concepts_at_level = 0;
               concept_descriptor_table[number_of_concepts].kind != marker_end_of_list;
               number_of_concepts++) {
      if (concept_descriptor_table[number_of_concepts].level <= calling_level)
         concept_sublists[call_list_any][concepts_at_level++] = number_of_concepts;
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




Private parse_block *get_parse_block(void)
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
   item->selector = selector_uninitialized;
   item->number = 0;
   item->subsidiary_root = (parse_block *) 0;
   item->next = (parse_block *) 0;

   return(item);
}





extern void initialize_parse(void)
{
   parse_state.concept_write_base = &history[history_ptr+1].command_root;
   parse_state.concept_write_ptr = parse_state.concept_write_base;
   *parse_state.concept_write_ptr = (parse_block *) 0;
   parse_state.call_list_to_use = find_proper_call_list(&history[history_ptr].state);
   history[history_ptr+1].centersp = 0;
   history[history_ptr+1].warnings.bits[0] = 0;
   history[history_ptr+1].warnings.bits[1] = 0;
   history[history_ptr+1].draw_pic = FALSE;

   if (written_history_items > history_ptr)
      written_history_items = history_ptr;

   parse_state.parse_stack_index = 0;
   parse_state.specialprompt = "";
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
      new_item->selector = old_item->selector;
      new_item->number = old_item->number;

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
      new_item->selector = old_item->selector;
      new_item->number = old_item->number;

      /* Chop off branches that don't belong. */

      if (!old_item->subsidiary_root)
         new_item->subsidiary_root = 0;
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
   could have happened will add to the tree but never delete anything.  This way,
   after we have saved and restored things, they are all in their original,
   locations, so that the pointers in the parse stack will still be valid. */
extern long_boolean restore_parse_state(void)
{
   parse_state = saved_parse_state;

   if (saved_command_root)
      reset_parse_tree(saved_command_root, history[history_ptr+1].command_root);
   else
      history[history_ptr+1].command_root = 0;

   return (FALSE);
}





/* Deposit a call into the parse state.  A returned value of TRUE
   means that the user refused to click on a required number or selector,
   and so we have taken no action.  This can only occur if interactive.
   If not interactive, stuff will be chosen by random number. */

extern long_boolean deposit_call(callspec_block *call)
{
   parse_block *new_block;
   selector_kind sel = selector_uninitialized;
   int number = 0;

   /* Put in selector and/or number as required. */

   if (call->callflags1 & CFLAG1_REQUIRES_SELECTOR) {
      int j;

      if (initializing_database)
         sel = selector_for_initialize;              
      else if (not_interactive)
         sel = (selector_kind) generate_random_number(last_selector_kind)+1;
      else if ((j = uims_do_selector_popup()) != 0)
         sel = (selector_kind) j;
      else
         return(TRUE);
   }

   if (call->callflags1 & CFLAG1_REQUIRES_NUMBER) {
      if (initializing_database)
         /* If this wants a number, give it 1.  0 won't work for the call 1/4 the alter. */
         number = 1;
      else if (not_interactive)
         number = generate_random_number(4)+1;
      else if ((number = uims_do_quantifier_popup()) != 0)
         ;
      else
         return(TRUE);
   }

   new_block = get_parse_block();
   new_block->concept = &mark_end_of_list;
   new_block->call = call;
   new_block->selector = sel;
   new_block->number = number;

   parse_state.topcallflags1 = call->callflags1;
   *parse_state.concept_write_ptr = new_block;

   return(FALSE);
}


/* Deposit a concept into the parse state.  A returned value of TRUE
   means that the user refused to click on a required number or selector,
   and so we have taken no action.  This can only occur if interactive.
   If not interactive, stuff will be chosen by random number. */

extern long_boolean deposit_concept(concept_descriptor *conc)
{
   parse_block *new_block;
   selector_kind sel = selector_uninitialized;
   int number = 0;

   if (concept_table[conc->kind].concept_prop & CONCPROP__USE_SELECTOR) {
      int j;

      if (not_interactive)
         sel = (selector_kind) generate_random_number(last_selector_kind)+1;
      else if ((j = uims_do_selector_popup()) != 0)
         sel = (selector_kind) j;
      else
         return(TRUE);
   }

   if (concept_table[conc->kind].concept_prop & CONCPROP__USE_NUMBER) {

      if (not_interactive)
         number = generate_random_number(4)+1;
      else if ((number = uims_do_quantifier_popup()) != 0)
         ;
      else
         return(TRUE);

      if (concept_table[conc->kind].concept_prop & CONCPROP__USE_TWO_NUMBERS) {
         int second_num;
         if (not_interactive)
            second_num = generate_random_number(4)+1;
         else if ((second_num = uims_do_quantifier_popup()) != 0)
            ;
         else {
            return(TRUE);
         }
         number |= second_num << 16;
      }
   }

   new_block = get_parse_block();
   new_block->concept = conc;
   new_block->selector = sel;
   new_block->number = number;

   *parse_state.concept_write_ptr = new_block;

   /* See if we need to parse a second call for this concept. */

   if (concept_table[conc->kind].concept_prop & CONCPROP__SECOND_CALL) {
      /* Set up recursion, reading the first call and its concepts into the same parse block. */

      if (parse_state.parse_stack_index == 39) specialfail("Excessive number of concepts.");
      parse_state.parse_stack[parse_state.parse_stack_index].save_concept_kind = conc->kind;
      parse_state.parse_stack[parse_state.parse_stack_index++].concept_write_save_ptr = parse_state.concept_write_ptr;
      parse_state.specialprompt = "";
      parse_state.topcallflags1 = 0;          /* Erase anything we had -- it is meaningless now. */
   }

   parse_state.call_list_to_use = call_list_any;

   /* Advance the write pointer. */
   parse_state.concept_write_ptr = &(new_block->next);

   return(FALSE);
}


/* False result means OK.  Otherwise, user clicked on something special,
   such as "abort" or "undo", and the reply tells what it was. */

extern long_boolean query_for_call(void)
{
   uims_reply local_reply;
   callspec_block *result;
   int old_error_flag;

   recurse_entry:

   *parse_state.concept_write_ptr = (parse_block *) 0;   /* We should actually re-use anything there. */

   if (allowing_modifications)
      parse_state.call_list_to_use = call_list_any;
   
   redisplay:

   if (!not_interactive) {
      /* We are operating in interactive mode.  Update the
         display and query the user. */

      /* Error codes are:
         1 - 1-line error message, text is in error_message1.
         2 - 2-line error message, text is in error_message1 and error_message2.
         3 - collision error, message is that people collided, they are in collision_person1 and collision_person2.
         4 - "resolve" or similar command was called in inappropriate context, text is in error_message1.
         5 - clicked on something inappropriate in subcall reader.
         6 - unable-to-execute error, person is in collision_person1.
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
      
      display_initial_history(history_ptr, 2);

      if (sequence_is_resolved()) {
         newline();
         writestuff("     resolve is:");
         newline();
         writestuff(resolve_names[history[history_ptr].resolve_flag.kind]);
         if (history[history_ptr].resolve_flag.kind != resolve_at_home ||
               (history[history_ptr].resolve_flag.distance & 7) != 0)
            writestuff(resolve_distances[history[history_ptr].resolve_flag.distance & 7]);
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
   
            char person_string[3];
   
            writestuff("Some people (");
            if ((collision_person1 | collision_person2) & BIT_VIRTUAL) {
               writestuff("virtual) on same spot.");
            }
            else {
               person_string[0] = ((collision_person1 >> 7) & 3) + '1';
               person_string[1] = (collision_person1 & 0100) ? 'G' : 'B';
               person_string[2] = '\0';
               writestuff(person_string);
               writestuff(" and ");
               person_string[0] = ((collision_person2 >> 7) & 3) + '1';
               person_string[1] = (collision_person2 & 0100) ? 'G' : 'B';
               writestuff(person_string);
               writestuff(", for example) on same spot.");
            }
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

            char person_string[3];
   
            writestuff("Some people (");
            if (collision_person1 & BIT_VIRTUAL) {
               writestuff("virtual) can't execute their part of this call.");
            }
            else {
               person_string[0] = ((collision_person1 >> 7) & 3) + '1';
               person_string[1] = (collision_person1 & 0100) ? 'G' : 'B';
               person_string[2] = '\0';
               writestuff(person_string);
               writestuff(", for example) can't execute their part of this call.");
            }
         }
         else           /* Must be 5, special signal for aborting out of subcall reader. */
            writestuff("You can't select that here.");
      
         newline();
      }

      old_error_flag = error_flag; /* save for refresh command */
      error_flag = 0;

      try_again:

      /* Display the call index number, and the partially entered call and/or prompt, as appropriate. */

      {
         char indexbuf[200];

         /* See if there are partially entered concepts.  If so, print the index number
            and those concepts on a separate line. */

         if (parse_state.concept_write_ptr != &history[history_ptr+1].command_root) {
            sprintf (indexbuf, "%d: ", history_ptr-whole_sequence_low_lim+2);
            /* This prints the concepts entered so far, with a "header" consisting of the index number.
               This partial concept tree is incomplete, so write_history_line has to be (and is) very careful. */
            write_history_line(history_ptr+1, indexbuf, FALSE, file_write_no);

            if (parse_state.specialprompt[0] != '\0') {
               writestuff(parse_state.specialprompt);
               newline();
            }
         }
         else {
            /* No partially entered concepts.  Put the sequence number in front of any "specialprompt". */
            sprintf (indexbuf, "%d: %s", history_ptr-whole_sequence_low_lim+2, parse_state.specialprompt);

            writestuff(indexbuf);
            newline();
         }
      }

      local_reply = uims_get_command(mode_normal, &parse_state.call_list_to_use);
   }
   else {

      /* We are operating in automatic mode.
         We must insert a concept or a call.  Decide which. */

      if (generate_random_concept_p()) {
         local_reply = ui_concept_select;
         uims_menu_index = concept_sublists[parse_state.call_list_to_use][generate_random_number(concept_sublist_sizes[parse_state.call_list_to_use])];
      }
      else {
         local_reply = ui_call_select;
      }
   }
   
   /* Now see what kind of command we have. */

   if (local_reply == ui_command_select) {
      if (uims_menu_index == command_refresh) {
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
         return(TRUE);
      }
   }
   else if (local_reply == ui_concept_select) {
      /* A concept is required.  Its index has been stored in uims_menu_index. */
      (void) deposit_concept(&concept_descriptor_table[uims_menu_index]);

      /* We ignore the value returned by deposit_concept.
         If the user refused to enter a selector, no action is taken. */
      goto recurse_entry;
   }
   else if (local_reply != ui_call_select) {
      reply = local_reply;     /* Save this -- top level will need it. */
      return(TRUE);
   }

   /* We have a call.  Get the actual call and deposit it into the concept list. */

   if (not_interactive) {
      if (initializing_database)
         result = base_calls[1];     /* Get "nothing". */
      else {
         int i = generate_random_number(number_of_calls[parse_state.call_list_to_use]);
         result = main_call_lists[parse_state.call_list_to_use][i];
      }

      /* Why don't we just call the random number generator again if the call is inappropriate?
         Wouldn't that be much faster?  There are two reasons:  First, we would need to take
         special precautions against an infinite loop.  Second, and more importantly, if we
         just called the random number generator again, it would screw up the hash numbers,
         which would make the uniquefication fail, so we could see the same thing twice. */
   
      if (result->callflags1 & CFLAG1_DONT_USE_IN_RESOLVE) fail("This shouldn't get printed.");
   }
   else {
      result = main_call_lists[parse_state.call_list_to_use][uims_menu_index];
   }

   if (deposit_call(result)) goto recurse_entry;

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
            parse_state.specialprompt = "ENTER CALL FOR ENDS";
            break;
         case concept_on_your_own:
            parse_state.specialprompt = "ENTER SECOND (CENTERS) CALL";
            break;
         default:
            parse_state.specialprompt = "ENTER SECOND CALL";
            break;
      }
   
      parse_state.topcallflags1 = 0;          /* Erase anything we had -- it is meaningless now. */
      goto recurse_entry;
   }

   /* Advance the write pointer. */
   parse_state.concept_write_ptr = &((*parse_state.concept_write_ptr)->next);

   return(FALSE);
}



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
            main_call_lists[call_list_any][i]->callflagsh |= 0x80000000;
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

   return(remainder);
}



/* This is not automatic to keep it from being lost by the longjmp.
   It is also read by "write_header_stuff". */
Private call_list_mode_t call_list_mode;


extern void write_header_stuff(void)
{
   /* log creation version info */
   writestuff("Sd");
   writestuff(sd_version_string());
   writestuff(":db");
   writestuff(database_version);
   writestuff("     ");

   /* log level info */
   writestuff(getout_strings[calling_level]);
   if (call_list_mode == call_list_mode_abridging)
       writestuff(" (abridged)");

   newline();
}



void main(int argc, char *argv[])
{
   int argno;

   if (argc >= 2  && strcmp(argv[1], "-help") == 0)
       display_help();		/* does not return */

   /* This lets the X user interface intercept command line arguments that it is
      interested in.  It also handles the flags "-seq" and "-db". */
   uims_process_command_line(&argc, &argv);

   /* Do general initializations, which currently consist only of
      seeding the random number generator. */
   general_initialize();

   calling_level = l_mainstream;    /* The default. */
   call_list_mode = call_list_mode_none;

   for (argno=1; argno<argc; argno++) {
      if (argv[argno][0] == '-') {
         /* Special flag: must be one of
            -write_list <filename>  -- write out the call list for the
                  indicated level INSTEAD OF running the program
            -write_full_list <filename>  -- write out the call list for the
                  indicated level and all lower levels INSTEAD OF running the program
            -abridge <filename>  -- read in the file, strike all the calls
                  contained therein off the menus, and proceed. */

         if (strcmp(&argv[argno][1], "write_list") == 0)
            call_list_mode = call_list_mode_writing;
         else if (strcmp(&argv[argno][1], "write_full_list") == 0)
            call_list_mode = call_list_mode_writing_full;
         else if (strcmp(&argv[argno][1], "abridge") == 0)
            call_list_mode = call_list_mode_abridging;
	 /*
	  * These options may be handled by the UI, but if not
	  * be sure it gets done.
	  */
         else if (strcmp(&argv[argno][1], "sequence") == 0) {
	     if (argno+1 < argc)
		 strncpy(outfile_string, argv[argno+1], MAX_FILENAME_LENGTH);
	 }
         else if (strcmp(&argv[argno][1], "db") == 0) {
	     if (argno+1 < argc)
		 database_filename = argv[argno+1];
	 }
         else
            uims_bad_argument("Unknown flag:", argv[argno], NULL);

         argno++;
         if (argno>=argc)
            uims_bad_argument("This flag must be followed by a file name:", argv[argno-1], NULL);

	 if (call_list_mode != call_list_mode_none)
	     if (open_call_list_file(call_list_mode, argv[argno]))
		 exit_program(1);
      }
      else if (argv[argno][0] == 'm') calling_level = l_mainstream;
      else if (argv[argno][0] == 'p') calling_level = l_plus;
      else if (argv[argno][0] == 'a') {
         if (argv[argno][1] == '1' && !argv[argno][2]) calling_level = l_a1;
         else if (argv[argno][1] == '2' && !argv[argno][2]) calling_level = l_a2;
         else if (argv[argno][1] == 'l' && argv[argno][2] == 'l' && !argv[argno][3]) calling_level = l_dontshow;
         else
            goto bad_level;
      }
      else if (argv[argno][0] == 'c' && argv[argno][1] == '3' && argv[argno][2] == 'a' && !argv[argno][3])
         calling_level = l_c3a;
      else if (argv[argno][0] == 'c' && argv[argno][1] == '3' && argv[argno][2] == 'x' && !argv[argno][3])
         calling_level = l_c3x;
      else if (argv[argno][0] == 'c' && argv[argno][1] == '4' && argv[argno][2] == 'a' && !argv[argno][3])
         calling_level = l_c4a;
      else if (argv[argno][0] == 'c' && !argv[argno][2]) {
         if (argv[argno][1] == '1') calling_level = l_c1;
         else if (argv[argno][1] == '2') calling_level = l_c2;
         else if (argv[argno][1] == '3') calling_level = l_c3;
         else if (argv[argno][1] == '4') calling_level = l_c4;
         else
            goto bad_level;
      }
      else
         goto bad_level;
   }

   /* initialize outfile_string to calling-level-specific default outfile */

   (void) strncat(outfile_string, filename_strings[calling_level], MAX_FILENAME_LENGTH);

   initializing_database = TRUE;
   testing_fidelity = FALSE;
   not_interactive = TRUE;
   parse_active_list = (parse_block *) 0;
   parse_inactive_list = (parse_block *) 0;

   if (history == 0) {
      history_allocation = 15;
      history = (configuration *) get_mem(history_allocation * sizeof(configuration));
   }
   
   initialize_menus(call_list_mode);    /* This sets up max_base_calls. */

   /* If we wrote a call list file, that's all we do. */
   if (call_list_mode == call_list_mode_writing || call_list_mode == call_list_mode_writing_full)
      goto normal_exit;

   initialize_concept_sublists();

   uims_postinitialize();

   global_age = 1;

   /* Create the top level error handler. */

   longjmp_ptr = &longjmp_buffer;          /* point the global pointer at it. */
   error_flag = setjmp(longjmp_buffer.the_buf);

   if (error_flag) {

      /* The call we were trying to do has failed.  Abort it and display the error message. */
   
      if (initializing_database) {
         init_error(error_message1);
         goto normal_exit;
      }

      history[0] = history[history_ptr+1];     /* So failing call will get printed. */
      history[0].warnings.bits[0] = 0;         /* But without any warnings we may have collected. */
      history[0].warnings.bits[1] = 0;
   
      if (error_flag == 5) {
         /* Special signal -- user clicked on special thing while trying to get subcall. */
         if ((reply == ui_command_select) &&
              ((uims_menu_index == command_quit) ||
               (uims_menu_index == command_undo) ||
               (uims_menu_index == command_abort)))
            reply_pending = TRUE;
            goto start_with_pending_reply;
      }
   
      goto start_cycle;
   }

   /* The tandem module wants to initialize some static tables. */

   initialize_tandem_tables();
   
   initializing_database = FALSE;
   not_interactive = FALSE;

   /* HERE IS (APPROXIMATELY) WHERE THE PROGRAM STARTS. */
   
   clear_screen();

   writestuff("SD -- square dance caller's helper.");
   newline();
   writestuff("Copyright (c) 1991, 1992, 1993 William B. Ackerman and Stephen Gildea.");
   newline();
   newline();
   writestuff("SD comes with ABSOLUTELY NO WARRANTY; for details see the license.");
   newline();
   writestuff("This is free software, and you are welcome to redistribute it ");
   writestuff("under certain conditions; for details see the license.");
   newline();
   writestuff("You should have received a copy of the GNU General Public License ");
   writestuff("along with this program, in the file \"COPYING\" or with the manual; if not, write to ");
   writestuff("the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA ");
   writestuff("02139, USA.");
   newline();
   newline();

   show_banner:

   writestuff("Version ");
   write_header_stuff();
   writestuff("Output file is \"");
   writestuff(outfile_string);
   writestuff("\"");
   newline();
   
   new_sequence:
   
   /* Here to start a fresh sequence.  If first time, or if we got here by clicking on "abort",
      the screen has been cleared.  Otherwise, it shows the last sequence that we wrote. */

   /* Replace all the parse blocks left from the last sequence. */
   release_parse_blocks_to_mark((parse_block *) 0);
   
   /* Query for the starting setup. */
   
   reply = uims_get_command(mode_startup, (call_list_kind *) 0);

   if (reply == ui_command_select && uims_menu_index == command_quit) goto normal_exit;
   if (reply != ui_start_select || uims_menu_index == 0) goto normal_exit;           /* Huh? */
   
   history_ptr = 1;              /* Clear the position history. */

   whole_sequence_low_lim = 2;
   if (!startinfolist[uims_menu_index].into_the_middle) whole_sequence_low_lim = 1;

   history[1].warnings.bits[0] = 0;
   history[1].warnings.bits[1] = 0;
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
   
   restart_after_backup:

   simple_restart:

   if ((!reply_pending) && (!query_for_call())) {
      /* User specified a call (and perhaps concepts too). */

      /* The call to toplevelmove may make a call to "fail", which will get caught by the cleanup handler
         above, reset history_ptr, and go to start_cycle with the error message displayed. */

      toplevelmove();
      
      /* Call successfully completed and has been stored in history. */
      
      history_ptr++;

      goto start_cycle;
   }
   
   /* If get here, query_for_call exitted without completing its parse, because the operator
      selected something like "quit", "undo", or "resolve", or because we have such a command
      already pending. */

   if (reply == ui_command_select) {
      switch (uims_menu_index) {
         case command_quit:
            if (uims_do_abort_popup() != POPUP_ACCEPT) goto simple_restart;
            goto normal_exit;
         case command_abort:
            if (uims_do_abort_popup() != POPUP_ACCEPT) goto simple_restart;
            clear_screen();
            goto show_banner;
         case command_undo:
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
                     *last_ptr = (parse_block *) 0;
                     reply_pending = FALSE;
                     goto restart_after_backup;
                  }
      
                  if ((*last_ptr)->concept->kind <= marker_end_of_list) break;
               }
      
               /* We did not find our place, so we undo the whole line. */
               if (history_ptr > 1) history_ptr--;
               /* Going to start_cycle will make sure written_history_items does not exceed history_ptr. */
               goto start_cycle;
            }
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
                  if ( (newfile_string[0] != '\0') && (strcmp(outfile_string, newfile_string) != 0) ) {
                     char confirm_message[MAX_FILENAME_LENGTH+25];
      
                     if (probe_file(newfile_string)) {
                        (void) strncpy(outfile_string, newfile_string, MAX_FILENAME_LENGTH);
                        (void) strncpy(confirm_message, "Output file changed to \"", 25);
                        (void) strncat(confirm_message, outfile_string, MAX_FILENAME_LENGTH);
                        (void) strncat(confirm_message, "\"", 2);
                        last_file_position = -1;
                        specialfail(confirm_message);
                     }
                     else {
                        specialfail("No write access to that file, no action taken.");
                     }
                  }
               }
               goto start_cycle;
            }
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
                  main_call_lists[call_list_any][i]->callflagsh &= ~0x80000000;
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
                  if (main_call_lists[call_list_any][i]->callflagsh & 0x80000000) {
                     writestuff(main_call_lists[call_list_any][i]->name);
                     writestuff(", ");
                  }
               }
               newline();
      
               error_flag = 0;
               
               local_reply = uims_get_command(mode_normal, &dummy);
            
               if (local_reply == ui_call_select) {
                  /* Age this call. */
                  main_call_lists[call_list_any][uims_menu_index]->age = global_age;
                  goto start_neglect;
               }
               else
                  goto start_cycle;
            }
         case command_resolve: case command_reconcile: case command_anything: case command_nice_setup:
            {
               search_kind goal;
         
               switch (uims_menu_index) {
                  case command_resolve:
                     goal = search_resolve;
                     break;
                  case command_reconcile:
                     goal = search_reconcile;
                     break;
                  case command_anything:
                     goal = search_anything;
                     break;
                  case command_nice_setup:
                     goal = search_nice_setup;
                     break;
               }
            
               reply = full_resolve(goal);
         
               /* If full_resolve refused to operate (for example, we clicked on "reconcile"
                  when in an hourglass), it returned "ui_search_accept", which will cause
                  us simply to go to start_cycle. */
            
               /* If user clicked on something random, treat it as though he clicked on "accept"
                  followed by whatever it was. */
            
               if (reply == ui_command_select) {
                  switch (uims_menu_index) {
                     case command_quit: case command_abort: case command_getout:
                     case command_resolve: case command_reconcile: case command_anything: case command_nice_setup:
                        allowing_modifications = 0;
                        history[history_ptr+1].draw_pic = FALSE;
                        parse_state.concept_write_base = &history[history_ptr+1].command_root;
                        parse_state.concept_write_ptr = parse_state.concept_write_base;
                        *parse_state.concept_write_ptr = (parse_block *) 0;
                        reply_pending = TRUE;
                        /* Going to start_with_pending_reply will make sure written_history_items does not exceed history_ptr. */
                        goto start_with_pending_reply;
                  }
               }
      
               goto start_cycle;
            }
         case command_getout:

            /* Check that it is really resolved. */

            if (!sequence_is_resolved())
               specialfail("This sequence is not resolved.");
            if (write_sequence_to_file() == 0)
               goto start_cycle; /* user cancelled action */

            writestuff("Sequence written to \"");
            writestuff(outfile_string);
            writestuff("\".");
            newline();
            goto new_sequence;

         default:
            goto normal_exit;
      }
   }
   else
      goto normal_exit;
   
   bad_level:

   uims_bad_argument("Unknown calling level argument:", argv[argno],
      "Known calling levels: m, p, a1, a2, c1, c2, c3a, c3, c3x, c4a, or c4.");

   normal_exit:
   
   exit_program(0);
}

/* return TRUE if sequence was written */

extern long_boolean write_sequence_to_file(void)
{
   int getout_ind;
   char date[MAX_TEXT_LINE_LENGTH];
   char header[MAX_TEXT_LINE_LENGTH];
   int j;

   /* Put up the getout popup to see if the user wants to enter a header string. */

   getout_ind = uims_do_getout_popup(header);

   if (getout_ind == POPUP_DECLINE)
      return FALSE;    /* User didn't want to end this sequence after all. */

   /* User really wants this sequence.  Open the file and write it. */

   clear_screen();
   open_file();
   enable_file_writing = TRUE;
   doublespace_file();

   get_date(date);
   writestuff(date);
   writestuff("     ");
   write_header_stuff();

   if (getout_ind == POPUP_ACCEPT_WITH_STRING) {
      writestuff("             ");
      writestuff(header);
      newline();
   }

   newline();

   for (j=whole_sequence_low_lim; j<=history_ptr; j++)
      write_history_line(j, (char *) 0, FALSE, file_write_double);

   /* Echo the concepts entered so far.  */

   if (parse_state.concept_write_ptr != &history[history_ptr+1].command_root) {
      write_history_line(history_ptr+1, (char *) 0, FALSE, file_write_double);
   }

   if (sequence_is_resolved()) {
      doublespace_file();
      writestuff(resolve_names[history[history_ptr].resolve_flag.kind]);
      if (history[history_ptr].resolve_flag.kind != resolve_at_home ||
            (history[history_ptr].resolve_flag.distance & 7) != 0)
         writestuff(resolve_distances[history[history_ptr].resolve_flag.distance & 7]);
   }

   newline();
   enable_file_writing = FALSE;
   newline();

   close_file();     /* This will signal a "specialfail" if a file error occurs. */

   global_age++;
   return TRUE;
}


Private long_boolean debug_popup = FALSE;

extern void get_real_subcall(
   parse_block *parseptr,
   by_def_item *item,
   final_set concin,
   parse_block **concptrout,
   callspec_block **callout,
   final_set *concout)

/* ****** needs to send out alternate_concept!!! */

{
   char tempstring_text[MAX_TEXT_LINE_LENGTH];
   char *tempstringptr;
   parse_block *search;
   parse_block **newsearch;
   concept_descriptor *marker;

   /* Fill in defaults in case we choose not to get a replacement call. */

   *concptrout = parseptr;
   *callout = base_calls[item->call_id];
   *concout = concin;
   
   /* If this subcall invocation does not permit modification under any value of the
      "allowing_modifications" switch, we do nothing.  Just return the default.
      We do not search the list.  Hence, such subcalls are always protected
      from being substituted, and, if the same call appears multiple times
      in the derivation tree of a compound call, it will never be replaced.
      What do we mean by that?  Suppose we did a
         "[tally ho but [2/3 recycle]] the difference".
      After we create the table entry saying to change cast off 3/4 into 2/3
      recycle, we wouldn't want the cast off's in the difference getting
      modified.  Actually, that is not the real reason.  The casts are in
      different sublists.  The real reason is that the final part of mixed-up
      square thru is defined as 
         conc nullcall [mandatory_anycall] nullcall []
      and we don't want the unmodifiable nullcall that the ends are supposed to
      do getting modified, do we? */

   if (!(item->modifiers1 & (DFM1_MANDATORY_ANYCALL | DFM1_OR_ANYCALL | DFM1_ALLOW_FORCED_MOD)))
      return;

   /* See if we had something from before.  This avoids embarassment if a call is actually
      done multiple times.  We want to query the user just once and use that result everywhere.
      We accomplish this by keeping a subcall list showing what modifications the user
      supplied when we queried. */

   /* We ALWAYS search this list, if such exists.  Even if modifications are disabled.
      Why?  Because the reconciler re-executes calls after the point of insertion to test
      their fidelity with the new setup that results from the inserted calls.  If those
      calls have modified subcalls, we will find ourselves here, looking through the list.
      Modifications may have been enabled at the time the call was initially entered, but
      might not be now that we are being called from the reconciler. */

   if (parseptr->concept->kind == concept_another_call_next_mod) {
      newsearch = &parseptr->next;

      while ((search = *newsearch) != NULL) {
         if (base_calls[item->call_id] == search->call) {
            /* Found a reference to this call. */
            parse_block *subsidiary_ptr = search->subsidiary_root;

            /* If the pointer is nil, we already asked about this call,
               and the reply was no. */
            if (!subsidiary_ptr) return;

            *concptrout = subsidiary_ptr;
            *callout = NULLCALLSPEC;             /* ****** not right????. */
            *concout = 0;                        /* ****** not right????. */
            return;
         }

         newsearch = &search->next;
      }
   }

   /* Now we know that the list doesn't say anything about this call.  Perhaps we should
      query the user for a replacement and add something to the list.  First, decide whether
      we should consider doing so.  If we are initializing the
      database, the answer is always "no", even for calls that require a replacement call, such as
      "clover and anything".  This means that, for the purposes of database initialization,
      "clover and anything" is tested as "clover and nothing", since "nothing" is the subcall
      that appears in the database. */
      
   /* Of course, if we are testing the fidelity of later calls during a reconcile
      operation, we DO NOT EVER add any modifiers to the list, even if the user
      clicked on "allow modification" before clicking on "reconcile".  It is perfectly
      legal to click on "allow modification" before clicking on "reconcile".  It means
      we want modifications (chosen by random number generator, since we won't be
      interactive) for the calls that we randomly choose, but not for the later calls
      that we test for fidelity. */

   if (initializing_database | testing_fidelity) return;

   /* When we are searching for resolves and the like, the situation is different.  In this case,
      "initializing_database" is off but "not_interactive" is on.  We do perform mandatory
      modifications, so we will generate things like "clover and shakedown".  Of course, no
      querying actually takes place.  Instead, get_subcall just uses the random number generator.
      Therefore, whether resolving or in normal interactive mode, we are guided by the
      call modifier flags and the "allowing_modifications" global variable. */

   if (      (item->modifiers1 & DFM1_MANDATORY_ANYCALL) ||
             ((item->modifiers1 & DFM1_OR_ANYCALL) && (allowing_modifications)) ||
             ((item->modifiers1 & DFM1_ALLOW_FORCED_MOD) && (allowing_modifications > 1)))
      ;
   else
      return;     /* Do not query about this subcall.  Just return the default. */

   /* At this point, we know we should query the user about this call. */
      
   /* Set ourselves up for modification by making the null modification list
      if necessary.  ***** Someday this null list will always be present. */

   if (parseptr->concept->kind == marker_end_of_list) {
      parseptr->concept = &marker_concept_mod;
      newsearch = &parseptr->next;
   }
   else if (parseptr->concept->kind != concept_another_call_next_mod)
      fail("wrong marker in get_real_subcall???");

   /* Create a reference on the list.  "search" points to the null item at the end. */

   marker = &marker_concept_mod;
   if (DFM1_ALLOW_FORCED_MOD & item->modifiers1) marker = &marker_concept_force;
   else if (DFM1_MUST_BE_SCOOT_CALL & item->modifiers1) marker = &marker_concept_modreact;
   else if (DFM1_MUST_BE_TAG_CALL & item->modifiers1) marker = &marker_concept_modtag;

   tempstringptr = tempstring_text;
   *tempstringptr = 0;           /* Null string, just to be safe. */

   /* If the replacement is mandatory, or we are not interactive,
      don't present the popup.  Just get the replacement call. */

   if (not_interactive)
      ;
   else if (DFM1_MANDATORY_ANYCALL & item->modifiers1) {
      string_copy(&tempstringptr, "SUBSIDIARY CALL");
   }
   else {

      /* Need to present the popup to the operator and find out whether modification
         is desired. */

      modify_popup_kind kind;

      if (item->modifiers1 & DFM1_MUST_BE_TAG_CALL) kind = modify_popup_only_tag;
      else if (item->modifiers1 & DFM1_MUST_BE_SCOOT_CALL) kind = modify_popup_only_scoot;
      else kind = modify_popup_any;

      if (debug_popup || uims_do_modifier_popup(base_calls[item->call_id]->name, kind)) {
         /* User accepted the modification.
            Set up the prompt and get the concepts and call. */
      
         string_copy(&tempstringptr, "REPLACEMENT FOR THE ");
         string_copy(&tempstringptr, base_calls[item->call_id]->name);
      }
      else {
         /* User declined the modification.  Create a null entry so that we don't query again. */
         *newsearch = get_parse_block();
         (*newsearch)->concept = marker;
         (*newsearch)->call = base_calls[item->call_id];
         return;
      }
   }

   *newsearch = get_parse_block();
   (*newsearch)->concept = marker;
   (*newsearch)->call = base_calls[item->call_id];

   /* Set stuff up for reading subcall and its concepts. */

   /* Create a new parse block, point concept_write_ptr at its contents. */
   /* Create the new root at the start of the subsidiary list. */

   parse_state.concept_write_base = &(*newsearch)->subsidiary_root;
   parse_state.concept_write_ptr = parse_state.concept_write_base;

   parse_state.parse_stack_index = 0;
   parse_state.call_list_to_use = call_list_any;
   parse_state.specialprompt = tempstring_text;

   if (query_for_call())
      longjmp(longjmp_ptr->the_buf, 5);     /* User clicked on something unusual like "exit" or "undo". */

   *concptrout = (*newsearch)->subsidiary_root;
   *callout = NULLCALLSPEC;              /* We THROW AWAY the alternate call, because we want our user to get it from the concept list. */
   *concout = 0;
}

extern long_boolean sequence_is_resolved(void)
{
   return history[history_ptr].resolve_flag.kind != resolve_none;
}
