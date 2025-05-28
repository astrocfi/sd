/*
   sdmatch.c - command matching support

    Copyright (C) 1990-1996  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 31. */

/*
 * Copyright (C) 1993 Alan Snyder
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The author makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" WITHOUT ANY
 * WARRANTY, without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  
 *
 */

/* This file defines the following functions:
   matcher_initialize
   matcher_setup_call_menu
   match_user_input
*/

/* #define TIMING */ /* uncomment to display timing information */

#include "sd.h"
#define SCREWED_UP_REVERTS
#include "sdmatch.h"
#include <string.h> /* for strcpy */
#include <stdio.h>  /* for sprintf */
#include <ctype.h>  /* for tolower */

#ifdef TIMING
#include <time.h>
#endif

static callspec_block *empty_menu[] = {NULLCALLSPEC};
static callspec_block **call_menu_lists[NUM_CALL_LIST_KINDS];
static Cstring *selector_menu_list;

static int *concept_list; /* all concepts */
static int concept_list_length;

static int *level_concept_list; /* indices of concepts valid at current level */
static int level_concept_list_length;


typedef struct pat2_blockstruct {
   Cstring car;
   concept_descriptor *special_concept;
   match_result *folks_to_restore;
   struct pat2_blockstruct *next;
} pat2_block;

static modifier_block *modifier_active_list = (modifier_block *) 0;
static modifier_block *modifier_inactive_list = (modifier_block *) 0;

static int static_call_menu;              /* The call menu (or special negative command) that we are searching */




static void scan_concepts_and_calls(Cstring user, Cstring firstchar, pat2_block *pat2, match_result *saved_result_p, int patxi);
static void match_wildcard(Cstring user, Cstring pat, pat2_block *pat2, int patxi, concept_descriptor *special);


/* This is statically used by the match_wildcard and match_suffix_2 procedures. */

match_state static_ss;
static match_result *current_result;
static match_result everyones_real_result;


/* The following arrays must be coordinated with the Sd program */

/* BEWARE!!  This list is keyed to the definition of "start_select_kind" in sd.h . */
static Cstring startup_commands[] = {
   "exit from the program",
   "heads 1p2p",
   "sides 1p2p",
   "heads start",
   "sides start",
   "just as they are",
   "toggle concept levels",
   "toggle active phantoms",
   "toggle ignoreblanks",
   "toggle retain after error",
   "change output file",
   "change title",
   (Cstring) 0
};


static Cstring n_4_patterns[] = {
   "1/4",
   "2/4",
   "3/4",
   "4/4",
   "5/4",
   (Cstring) 0
};


/*
   Call MATCHER_INITIALIZE first.  If SHOW_COMMANDS_LAST is true, then the
   commands will be displayed last when matching against a call menu;
   otherwise, they will be displayed first.
   
   This function sets up the concept list.  The concepts are found
   in the external array concept_descriptor_table.  For each
   i, the field concept_descriptor_table[i].name has the text that we
   should display for the user.
*/

extern void matcher_initialize(void)
{
   int i;
   int concept_number;
   concept_descriptor *p;
   int *item, *level_item;
   concept_kind end_marker = concept_diagnose;

   /* Decide whether we allow the "diagnose" concept, by deciding
      when we will stop the concept list scan. */
   if (diagnostic_mode) end_marker = marker_end_of_list;

   /* initialize our special empty call menu */

   call_menu_lists[call_list_empty] = empty_menu;
   number_of_calls[call_list_empty] = 0;

   /* count the number of concepts to put in the lists */
   
   concept_list_length = 0;
   level_concept_list_length = 0;
   for (concept_number=0;;concept_number++) {
      p = &concept_descriptor_table[concept_number];
      if (p->kind == end_marker) {
         break;
      }
      else if (p->kind == concept_comment || (p->concparseflags & CONCPARSE_MENU_DUP)) {
         continue;
      }

      /* Pick out concepts that will be produced when certain function keys are pressed. */
      if (p->kind == concept_twice) twice_concept_ptr = p;
      if (p->kind == concept_sequential) two_calls_concept_ptr = p;

      concept_list_length++;

      if (p->level <= calling_level)
         level_concept_list_length++;
   }

   /* create the concept lists */

   concept_list = (int *) get_mem(sizeof(int) * concept_list_length);
   level_concept_list = (int *) get_mem(sizeof(int) * level_concept_list_length);

   item = concept_list;
   level_item = level_concept_list;
   for (concept_number=0;;concept_number++) {
      p = &concept_descriptor_table[concept_number];
      if (p->kind == end_marker) {
         break;
      }
      if (p->kind == concept_comment || (p->concparseflags & CONCPARSE_MENU_DUP)) {
         continue;
      }

      *item = concept_number;
      item++;

      if (p->level <= calling_level) {
         *level_item = concept_number;
         level_item++;
      }
   }

   /* And the selector list. */
   selector_menu_list = (Cstring *) get_mem((last_selector_kind+2) * sizeof(char *));

   for (i=0; i<last_selector_kind+1; i++)
      selector_menu_list[i] = selector_list[i].name;

   selector_menu_list[last_selector_kind+1] = (Cstring) 0;
}


extern void matcher_setup_call_menu(call_list_kind cl, callspec_block *call_name_list[])
{
   int i;

   call_menu_lists[cl] = (callspec_block **) get_mem((number_of_calls[cl]) * sizeof(callspec_block *));

   for (i=0; i<number_of_calls[cl]; i++)
      call_menu_lists[cl][i] = call_name_list[i];
}

/*
 * Call Verification
 */



/* These variables are actually local to verify_call, but they are
   expected to be preserved across the longjmp, so they must be static. */
static parse_block *parse_mark;
static call_list_kind savecl;

/*
 * Return TRUE if the specified call appears to be legal in the
 * current context.
 */

Private long_boolean verify_call(void)
{
   uint32 bits0, bits1;
   int old_history_ptr;
   long_boolean resultval;
   real_jmp_buf my_longjmp_buffer;

   /* If we are not verifying, we return TRUE immediately,
      thereby causing the item to be listed. */

   if (!static_ss.verify) return TRUE;

   interactivity = interactivity_verify;   /* so deposit_call doesn't ask user for info */
   bits0 = history[history_ptr+1].warnings.bits[0];
   bits1 = history[history_ptr+1].warnings.bits[1];
   old_history_ptr = history_ptr;

   parse_mark = mark_parse_blocks();
   save_parse_state();
   savecl = parse_state.call_list_to_use;

   /* Create a temporary error handler. */

   longjmp_ptr = &my_longjmp_buffer;
   if (setjmp(my_longjmp_buffer.the_buf)) {

      /* A call failed.  If the call had some mandatory substitution, pass it anyway. */

      if (mandatory_call_used) goto accept;

      /* Or a bad choice of selector or number may be the cause.
         Try different selectors first. */

      if (selector_used && verify_used_selector) {

         /* This call used a selector and didn't like it.  Try again with
            a different selector, until we run out of ideas. */
         switch (selector_for_initialize) {
            case selector_beaus:
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

      /* Now try a different number.  Only do this if the call actually
         consumes numbers, and the wildcard matching has not filled in all
         required numbers. */

      if (number_used && verify_used_number) {

         /* Try again with a different number, until we run out of ideas. */

         if (number_for_initialize < 4) {
            /* We try all numbers from 1 to 4.  We need to do this to get
               "exchange the boxes N/4" on the waves menu". */
            number_for_initialize++;
            goto try_another_number;
         }
      }

      goto try_again;
   }

   number_for_initialize = 1;

   try_another_number:

   /* Set the selector to try.  See the code in sdinit.c that chooses selectors
      when trying calls to make the initial menus for a discussion of how
      this is done. */

   selector_for_initialize = selector_beaus;

   try_another_selector:

   /* Do the call.  An error will signal and go to try_again. */

   selector_used = FALSE;
   number_used = FALSE;
   mandatory_call_used = FALSE;
   verify_used_number = FALSE;
   verify_used_selector = FALSE;

   (void) restore_parse_state();

   {
      long_boolean theres_a_call_in_here = FALSE;
      parse_block *save1 = (parse_block *) 0;
      modifier_block *anythings = &static_ss.result.match;

      /* This stuff is duplicated in uims_get_call_command in sdui-tty.c . */

      while (anythings) {
         call_conc_option_state save_stuff = static_ss.result.match.call_conc_options;

         /* First, if we have already deposited a call, and we see more stuff, it must be
            concepts or calls for an "anything" subcall. */

         if (save1) {
            parse_block *tt = get_parse_block();
            save1->concept = &marker_concept_mod;
            save1->next = tt;
            tt->concept = &marker_concept_mod;
            tt->call = base_calls[1];   /* "nothing" */
            tt->replacement_key = 2;    /* "mandatory_anycall" */
            parse_state.concept_write_ptr = &tt->subsidiary_root;
            save1 = (parse_block *) 0;
         }

         static_ss.result.match.call_conc_options = anythings->call_conc_options;

         if (anythings->kind == ui_call_select) {
            verify_options = anythings->call_conc_options;
            if (deposit_call(anythings->call_ptr)) goto try_again;
            save1 = *parse_state.concept_write_ptr;
            theres_a_call_in_here = TRUE;
         }
         else if (anythings->kind == ui_concept_select) {
            verify_options = anythings->call_conc_options;
            if (deposit_concept(anythings->concept_ptr)) goto try_again;
         }
         else break;   /* Huh? */

         static_ss.result.match.call_conc_options = save_stuff;
         anythings = anythings->next;
      }

      parse_state.call_list_to_use = savecl;         /* deposit_concept screwed it up */

      /* If we didn't see a call, the user is just verifying
         a string of concepts. Accept it. */

      if (!theres_a_call_in_here) goto accept;

      /* If the parse stack is nenempty, a subsidiary call is needed and hasn't been filled in.
         Therefore, the parse tree is incomplete.  We can print such parse trees, but we
         can't execute them.  So we just assume the call works. */

      if (parse_state.parse_stack_index != 0) goto accept;
   }

   toplevelmove(); /* does longjmp if error */

   accept:

   resultval = TRUE;
   goto foobar;

   try_again:

   resultval = FALSE;

   foobar:

   longjmp_ptr = &longjmp_buffer;    /* restore the global error handler */

   (void) restore_parse_state();
   release_parse_blocks_to_mark(parse_mark);

   history_ptr = old_history_ptr;
   history[history_ptr+1].warnings.bits[0] = bits0;
   history[history_ptr+1].warnings.bits[1] = bits1;
   interactivity = interactivity_normal;

   return resultval;
}


/*
 * STRN_GCP sets S1 to be the greatest common prefix of S1 and S2.
 */

static void strn_gcp(char *s1, char *s2)
{
    for (;;) {
        if (*s1 == 0) break;
        if (*s1 != *s2) {
            *s1 = 0;
            break;
        }
        ++s1;
        ++s2;
    }
}

/*
 * Record a match.  Extension is how the current input would be extended to
 * match the current pattern.  Result is the value of the
 * current pattern.  Special case: if user input is empty, extension is
 * not set.
 */
    
static void record_a_match(void)
{
   int old_yield = static_ss.lowest_yield_depth;

   if (!static_ss.showing) {
      if (static_ss.match_count == 0)
         /* this is the first match */
         strcpy(static_ss.extended_input, static_ss.extension);
      else
         strn_gcp(static_ss.extended_input, static_ss.extension);
   }

   /* Always copy the first match.
      Also, always copy, and process modifiers, if we are processing the "verify" operation.
      Also, copy the first exact match, and any exact match that isn't yielding relative to what we have so far. */

   if (     static_ss.match_count == 0 ||
            static_ss.verify ||
            (*static_ss.extension == '\0' && (
                  static_ss.exact_count == 0 || 
                  current_result->yield_depth <= old_yield))) {
      Const match_result *outbar;
      modifier_block **mod_tail = &static_ss.result.match.next;

      static_ss.result = everyones_real_result;
      static_ss.lowest_yield_depth = current_result->yield_depth;

      outbar = &static_ss.result;

      while (outbar->next) {
         modifier_block *out;

         /* We need to copy the modifiers to reasonably stable storage. */

         outbar = outbar->next;

         if (modifier_inactive_list) {
            out = modifier_inactive_list;
            modifier_inactive_list = out->gc_ptr;
         }
         else
            out = (modifier_block *) get_mem(sizeof(modifier_block));

         *out = outbar->match;
         out->next = (modifier_block *) 0;
         out->gc_ptr = modifier_active_list;
         modifier_active_list = out;
         *mod_tail = out;
         mod_tail = &out->next;
      }
   }

   if (*static_ss.extension == '\0') {
      static_ss.exact_count++;
      static_ss.result.exact = TRUE;
   }

   static_ss.match_count++;

   if (current_result->yield_depth > old_yield)
      static_ss.yielding_matches++;

   if (static_ss.showing) {
      if (verify_call())
         show_match();
   }
}


static Const call_conc_option_state null_options = {
   selector_uninitialized,
   direction_uninitialized,
   0,
   0,
   0,
   0};


/* Patxp is where the next character of the extension of the user input for the current pattern is to be written. */

static void match_suffix_2(Cstring user, Cstring pat1, pat2_block *pat2, int patxi)
{
   concept_descriptor *pat2_concept = (concept_descriptor *) 0;

   if (pat2->special_concept && !(pat2->special_concept->concparseflags & CONCPARSE_PARSE_DIRECT))
      pat2->special_concept = (concept_descriptor *) 0;

   for (;;) {
      if (*pat1 == 0) {
         /* PAT1 has run out, get a string from PAT2 */
         if (pat2_concept) {
            if (user) {
               /* We have processed a concept.  Scan for concepts and calls after same. */
               match_result saved_result = *current_result;
               current_result->match.concept_ptr = pat2_concept;
               scan_concepts_and_calls(user, " ", pat2, &saved_result, patxi);
            }
            pat2 = (pat2_block *) 0;
            pat2_concept = (concept_descriptor *) 0;
         }
         else if (pat2) {
            if (pat2->folks_to_restore) {
               /* Be sure maximum yield depth gets propagated back. */
               pat2->folks_to_restore->yield_depth = current_result->yield_depth;
               current_result = pat2->folks_to_restore;
            }
            pat1 = pat2->car;
            pat2_concept = pat2->special_concept;
            pat2 = pat2->next;
            continue;
         }
      }

#ifdef CLOVER_AND
      if (user && (*user == '\0' || (*user == ']' && *pat1 == '\0'))) {
#else
      if (user && (*user == '\0')) {
#endif
         /* we have just reached the end of the user input */
         Cstring p = pat1;

         while (p[0] == '@' && (p[1] == 'M' || p[1] == 'I' || p[1] == 'C')) p += 2;

         if (*p == ' ')
            static_ss.space_ok = TRUE;

         if (!pat2 && *pat1 == '\0') {
            /* exact match */
            static_ss.extension[patxi] = '\0';
            record_a_match();
            break;
         }

         /* we need to look at the rest of the pattern because
            if it contains wildcards, then there are multiple matches */
         user = (Cstring) 0;
      }
      else {
         char p = *pat1++;

         /* Check for expansion of wildcards.  But if we are just listing
            the matching commands, there is no point in expanding wildcards
            that are past the part that matches the user input */

         if (p == '@') {
            match_wildcard(user, pat1, pat2, patxi, pat2_concept);
   
            if (user==0) {
               /* User input has run out, just looking for more wildcards. */
   
               Cstring ep = get_escape_string(*pat1++);

               if (ep && *ep) {
                  (void) strcpy(&static_ss.extension[patxi], ep);
                  patxi += strlen(ep);
               }
               else {
                  if (ep) {
                     while (*pat1++ != '@');
                     pat1++;
                  }

                  /* Don't write duplicate blanks. */
                  if (*pat1 == ' ' && patxi > 0 && static_ss.extension[patxi-1] == ' ') pat1++;
               }
            }
            else {
               char u = *user++;
               Cstring ep = get_escape_string(*pat1++);

               if (u == '<') {
                  if (ep && *ep) {
                     int i;

                     for (i=1; ep[i]; i++) {
                        if (!user[i-1]) {
                           while (ep[i]) { static_ss.extension[patxi++] = ep[i] ; i++; }
                           user = 0;
                           goto cont;
                        }
   
                        if (user[i-1] != tolower(ep[i])) return;
                     }
   
                     user += strlen((char *) ep)-1;
                     cont: ;
                  }
                  else
                     break;
               }
               else {
                  if (ep && *ep)
                     break;   /* Pattern has "<...>" thing, but user didn't type "<". */
                  else {
                     if (ep) {
                        while (*pat1++ != '@');
                        pat1++;
                     }

                     user--;    /* Back up! */

                     /* Check for something that would cause the pattern effectively to have
                        two consecutive blanks, and compress them to one.  This can happen
                        if the pattern string has lots of confusing '@' escapes. */
                     if (*pat1 == ' ' && user[-1] == ' ') pat1++;
                  }
               }
            }
         }
         else {
            if (user==0) {
               /* User input has run out, just looking for more wildcards. */
   
               if (p) {
                  /* there is more pattern */
                  static_ss.extension[patxi++] = tolower(p);
               }
               else {
                  /* reached the end of the pattern */
   
                  static_ss.extension[patxi] = '\0';
   
                  if (!pat2) {
                     record_a_match();
                     break;
                  }
               }
            }
            else {
               char u = *user++;

               if (u != p && (p > 'Z' || p < 'A' || u != p+'a'-'A')) {
                  if (elide_blanks) {
                     if (p != '-' || u != ' ') {           /* If user said "wave based" instead of "wave-based", just continue. */
                        if (p == ' ' || p == '-') user--;  /* If user said "wavebased" (no hyphen) or "inroll" (no blank), elide. */
                        else break;
                     }
                  }
                  else
                     break;
               }
            }
         }
      }
   }
}


static void scan_concepts_and_calls(Cstring user, Cstring firstchar, pat2_block *pat2, match_result *saved_result_p, int patxi)
{
   int *item;
   int menu_length;
   int i;
   pat2_block p2b;
   match_result *saved_folksptr = current_result;

   current_result->next = saved_result_p;
   p2b.folks_to_restore = (match_result *) 0;
   p2b.next = pat2;

   /* We force any call invoked under a modifier to yield if it is ambiguous.  This way,
      if the user types "cross roll", preference will be given to the call "cross roll",
      rather than to the modifier "cross" on the call "roll".  This whole thing depends,
      of course, on our belief that the language of square dance calling never contains
      an ambiguous call/concept utterance.  For example, we do not expect anyone to
      invent a call "and turn" that can be used with the "cross" modifier. */
   saved_result_p->yield_depth++;
   saved_result_p->next = (match_result *) 0;

   /* scan concepts */

   if (allowing_all_concepts) {
      item = concept_list;
      menu_length = concept_list_length;
   }
   else {
      item = level_concept_list;
      menu_length = level_concept_list_length;
   }

   saved_result_p->match.kind = ui_concept_select;

   for (i = 0; i < menu_length; i++) {
      concept_descriptor *this_concept = &concept_descriptor_table[*item];

      p2b.car = this_concept->name;
      p2b.special_concept = this_concept;
      saved_result_p->match.concept_ptr = this_concept;
      saved_result_p->match.call_conc_options = null_options;
      current_result = saved_result_p;
      match_suffix_2(user, firstchar, &p2b, patxi);
      item++;
   }

   /* scan calls */

   p2b.special_concept = (concept_descriptor *) 0;
   saved_result_p->match.kind = ui_call_select;

   for (i = 0; i < number_of_calls[call_list_any]; i++) {
      saved_result_p->match.call_ptr = main_call_lists[call_list_any][i];
      p2b.car = call_menu_lists[call_list_any][i]->name;
      saved_result_p->match.call_conc_options = null_options;
      current_result = saved_result_p;
      match_suffix_2(user, firstchar, &p2b, patxi);
   }

   current_result = saved_folksptr;
   current_result->next = (match_result *) 0;
}



/*
 * Match_wildcard tests for and handles pattern suffixes that begin with
 * a wildcard such as "<anyone>".  A wildcard is handled only if there is
 * room in the Result struct to store the associated value.
 */

static void match_wildcard(Cstring user, Cstring pat, pat2_block *pat2, int patxi, concept_descriptor *special)
{
   Cstring prefix;
   Cstring *number_table;
   int i;
   pat2_block p2b;
   char crossname[80];
   char *crossptr;
   int save_howmanynumbers;
   uint32 save_number_fields;
   int concidx;
   Cstring pattern;
   char key = *pat++;
   p2b.car = pat;
   p2b.special_concept = special;
   p2b.folks_to_restore = (match_result *) 0;
   p2b.next = pat2;

   /* if we are just listing the matching commands, there
      is no point in expanding wildcards that are past the
      part that matches the user input.  That is why we test
      "(user == 0)". */

   if (user) {
      switch (key) {
         case '6': case 'k':
            if (current_result->match.call_conc_options.who == selector_uninitialized) {
               selector_kind save_who = current_result->match.call_conc_options.who;

               for (i=1; i<=last_selector_kind; ++i) {
                  current_result->match.call_conc_options.who = (selector_kind) i;
                  match_suffix_2(user, ((key == '6') ? selector_list[i].name : selector_list[i].sing_name), &p2b, patxi);
               }

               current_result->match.call_conc_options.who = save_who;
               return;
            }
            break;
         case '0':
            if (*user == '[') {
               pat2_block p3b;
               match_result saved_result = *current_result;

               p3b.car = "]";
               p3b.special_concept = (concept_descriptor *) 0;
               p3b.folks_to_restore = current_result;
               p3b.next = &p2b;
               scan_concepts_and_calls(user, "[", &p3b, &saved_result, patxi);
               return;
            }
            break;
         case 'h':
            if (current_result->match.call_conc_options.where == direction_uninitialized) {
               direction_kind save_where = current_result->match.call_conc_options.where;
      
               for (i=1; i<=last_direction_kind; ++i) {
                  current_result->match.call_conc_options.where = (direction_kind) i;
                  match_suffix_2(user, direction_names[i], &p2b, patxi);
               }
      
               current_result->match.call_conc_options.where = save_where;
               return;
            }
            break;
         case 'v': case 'w': case 'x': case 'y':
            if ((current_result->match.call_conc_options.tagger & 0xFF000000UL) == 0) {
               /* We allow recursion 4 levels deep.  In fact, we consider it
                  inappropriate to stack revert/reflect things.  There are special
                  calls "revert, then reflect", etc. for this purpose. */
               int tagclass;
               uint32 save_tagger = current_result->match.call_conc_options.tagger;
#ifdef SCREWED_UP_REVERTS
               uint32 old_low_tagger;
               uint32 test_mask = 0xFF;
               old_low_tagger = save_tagger & 0xFF;
#endif
         
               /* If we are already operating under a tagger field, this is a "revert" type of thing.
                  Use the tagger class from above in preference to the one that we deduce from the
                  escape letter, since the escape letters are not always correct for recursive
                  invocations of "revert" things. */
         
               if (save_tagger & 0xFF) {
                  tagclass = (save_tagger >> 5) & 3;
               }
               else {
                  tagclass = 0;
                  if (key == 'w') tagclass = 1;
                  else if (key == 'x') tagclass = 2;
                  else if (key == 'y') tagclass = 3;
               }

#ifdef SCREWED_UP_REVERTS
               while ((current_result->match.call_conc_options.tagger & test_mask) != 0) {
                  test_mask <<= 8;
                  old_low_tagger <<= 8;
               }

               current_result->match.call_conc_options.tagger |= old_low_tagger;
               current_result->match.call_conc_options.tagger &= ~0xFF;
#else
               current_result->match.call_conc_options.tagger <<= 8;
#endif
               current_result->match.call_conc_options.tagger |= tagclass << 5;
         
               for (i=0; i<number_of_taggers[tagclass]; ++i) {
                  current_result->match.call_conc_options.tagger++;
                  match_suffix_2(user, tagger_calls[tagclass][i]->name, &p2b, patxi);
               }
      
               current_result->match.call_conc_options.tagger = save_tagger;
               return;
            }
            break;
         case 'N':
            if (current_result->match.call_conc_options.circcer == 0) {
               char circname[80];
               uint32 save_circcer = current_result->match.call_conc_options.circcer;
      
               for (i=0; i<number_of_circcers; ++i) {
                  char *fromptr = circcer_calls[i]->name;
                  char *toptr = circname;
                  char c;
                  do {
                     c = *fromptr++;
                     if (c == '@') {
                        if (*fromptr++ == 'O') {
                           while (*fromptr++ != '@') ;
                           fromptr++;
                        }
                     }
                     else
                        *toptr++ = c;
                  } while (c);
         
                  current_result->match.call_conc_options.circcer++;
                  match_suffix_2(user, circname, &p2b, patxi);
               }
      
               current_result->match.call_conc_options.circcer = save_circcer;
               return;
            }
            break;
         case '9':
            if (*user < '1' || *user > '8') return;
            number_table = cardinals;
            goto do_number_stuff;
         case 'u':
            if (*user < '1' || *user > '8') return;
            number_table = ordinals;
            goto do_number_stuff;
         case 'a': case 'b': case 'B': case 'D':
            if ((*user >= '1' && *user <= '8') || *user == 'q' || *user == 'h' || *user == 't' || *user == 'f') {
               save_howmanynumbers = current_result->match.call_conc_options.howmanynumbers;
               save_number_fields = current_result->match.call_conc_options.number_fields;
      
               current_result->match.call_conc_options.howmanynumbers++;

               for (i=1 ; (prefix = n_4_patterns[i-1]) ; i++) {
                  current_result->match.call_conc_options.number_fields += 1 << (save_howmanynumbers*4);
                  if (key != 'D' || (i&1) != 0)
                     match_suffix_2(user, prefix, &p2b, patxi);
               }
         
               /* special case: allow "quarter" for 1/4 */
               current_result->match.call_conc_options.number_fields = save_number_fields + (1 << (save_howmanynumbers*4));
               match_suffix_2(user, "quarter", &p2b, patxi);
         
               /* special case: allow "half" or "1/2" for 2/4 */
               if (key != 'D') {
                  current_result->match.call_conc_options.number_fields = save_number_fields + (2 << (save_howmanynumbers*4));
                  match_suffix_2(user, "half", &p2b, patxi);
                  match_suffix_2(user, "1/2", &p2b, patxi);
               }
         
               /* special case: allow "three quarter" for 3/4 */
               current_result->match.call_conc_options.number_fields = save_number_fields + (3 << (save_howmanynumbers*4));
               match_suffix_2(user, "three quarter", &p2b, patxi);
         
               /* special case: allow "full" for 4/4 */
               if (key != 'D') {
                  current_result->match.call_conc_options.number_fields = save_number_fields + (4 << (save_howmanynumbers*4));
                  match_suffix_2(user, "full", &p2b, patxi);
               }
      
               current_result->match.call_conc_options.howmanynumbers = save_howmanynumbers;
               current_result->match.call_conc_options.number_fields = save_number_fields;
               return;
            }
            break;
      }
   }

   switch (key) {
      case 'j':
         crossptr = crossname;
         while ((*crossptr++ = *pat++) != '@');
         pat++;
         crossptr--;
         *crossptr = 0;
         pattern = crossname;
         concidx = cross_concept_index;
         goto do_cross_stuff;
      case 'C':
         pattern = " cross";
         concidx = cross_concept_index;
         goto do_cross_stuff;
      case 'J':
         crossptr = crossname;
         while ((*crossptr++ = *pat++) != '@');
         pat++;
         crossptr--;
         *crossptr = 0;
         pattern = crossname;
         concidx = magic_concept_index;
         goto do_cross_stuff;
      case 'M':
         concidx = magic_concept_index;
         pattern = " magic";
         goto do_cross_stuff;
      case 'E':
         crossptr = crossname;
         while ((*crossptr++ = *pat++) != '@');
         pat++;
         crossptr--;
         *crossptr = 0;
         pattern = crossname;
         concidx = intlk_concept_index;
         goto do_cross_stuff;
      case 'I':
         {
            char *p = static_ss.extension;
            int idx = patxi;
            long_boolean fixing_an_a = TRUE;

            for (i=0 ; i<2 ; i++) {
               idx--;
               if (idx < 0) { idx = static_ss.full_input_size-1 ; p = static_ss.full_input; }
               if (p[idx] != "a "[i]) { fixing_an_a = FALSE; break; }
            }

            if (fixing_an_a || (user && user[-1] == 'a' && user[-2] == ' '))
               pattern = "n interlocked";
            else
               pattern = " interlocked";

            concidx = intlk_concept_index;
         }
         goto do_cross_stuff;
      case 'e':
         while (*pat++ != '@');
         pat++;
         pattern = "left";
         concidx = left_concept_index;
         goto do_cross_stuff;
   }

   return;

   do_number_stuff:

   save_howmanynumbers = current_result->match.call_conc_options.howmanynumbers;
   save_number_fields = current_result->match.call_conc_options.number_fields;

   current_result->match.call_conc_options.howmanynumbers++;

   for (i=1 ; (prefix = number_table[i-1]) ; i++) {
       current_result->match.call_conc_options.number_fields += 1 << (save_howmanynumbers*4);
       match_suffix_2(user, prefix, &p2b, patxi);
   }

   current_result->match.call_conc_options.howmanynumbers = save_howmanynumbers;
   current_result->match.call_conc_options.number_fields = save_number_fields;

   return;

   do_cross_stuff:

   {
      match_result *saved_resultptr = current_result;
      match_result saved_result = *current_result;

      current_result->match.kind = ui_concept_select;
      current_result->match.call_conc_options = null_options;
      current_result->match.concept_ptr = &concept_descriptor_table[concidx];
      current_result->next = &saved_result;
      p2b.car = pat;
      current_result = &saved_result;
      match_suffix_2(user, pattern, &p2b, patxi);
      current_result = saved_resultptr;
      *current_result = saved_result;
      current_result->next = (match_result *) 0;
   }
}


/*
 * Match_pattern tests the user input against a pattern (pattern)
 * that may contain wildcards (such as "<anyone>").  Pattern matching is
 * case-sensitive.  As currently used, the user input and the pattern
 * are always given in all lower case.
 * If the input is equivalent to a prefix of the pattern,
 * a match is recorded in the search state.
 */
 
static void match_pattern(Cstring pattern, concept_descriptor *this_is_grand)
{
   char pch, uch;
   pat2_block p2b;

   /* the following special cases are purely to improve performance */

   pch = pattern[0];
   uch = static_ss.full_input[0];

   if (uch == '\0') {
      if (!static_ss.showing) {
         /* null user input matches everything (except a comment) */
         /* special case: pattern never begins with SPACE */
         /* special case: ignore wildcards, we know there are multiple matches */
         /* special case: static_ss.extension not set */
         static_ss.match_count++;
         return;
      }
   }
   else if (uch != tolower(pch)) {
      if (pch != '@') {
         /* fails to match */
         return;
      }
      else if (pattern[1] == '0' && uch != '[' && uch != '<') {
         /* Call was "<ANYTHING> and ...", but user didn't type "[" or "<". */
         return;
      }
   }

   p2b.car = pattern;
   p2b.special_concept = this_is_grand;
   p2b.folks_to_restore = (match_result *) 0;
   p2b.next = (pat2_block *) 0;

   match_suffix_2(static_ss.full_input, "", &p2b, 0);
}

static void search_menu(uims_reply kind)
{
   int i;
   Cstring *menu;
   int menu_length;

   everyones_real_result.valid = TRUE;
   everyones_real_result.exact = FALSE;
   everyones_real_result.match.kind = kind;
   everyones_real_result.match.call_conc_options = null_options;
   everyones_real_result.next = (match_result *) 0;
   everyones_real_result.yield_depth = 0;

   current_result = &everyones_real_result;

   if (kind == ui_call_select) {
      menu_length = number_of_calls[static_call_menu];

      for (i = 0; i < menu_length; i++) {
         if (static_ss.verify && verify_has_stopped) break;  /* Don't waste time after user stops us. */
         parse_state.call_list_to_use = static_call_menu;
         everyones_real_result.match.call_ptr = main_call_lists[static_call_menu][i];
         everyones_real_result.yield_depth = (call_menu_lists[static_call_menu][i]->callflags1 & CFLAG1_YIELD_IF_AMBIGUOUS) ? 1 : 0;
         match_pattern(call_menu_lists[static_call_menu][i]->name, (concept_descriptor *) 0);
      }
   }
   else if (kind == ui_concept_select) {
      int *item;

      if (allowing_all_concepts) {
         item = concept_list;
         menu_length = concept_list_length;
      }
      else {
         item = level_concept_list;
         menu_length = level_concept_list_length;
      }
   
      for (i = 0; i < menu_length; i++) {
         concept_descriptor *this_concept = &concept_descriptor_table[*item];
         parse_state.call_list_to_use = static_call_menu;
         everyones_real_result.match.concept_ptr = this_concept;
         everyones_real_result.yield_depth = (this_concept->concparseflags & CONCPARSE_YIELD_IF_AMB) ? 1 : 0;
         match_pattern(this_concept->name, this_concept);
         item++;
      }
   }
   else if (static_call_menu >= match_taggers && static_call_menu <= match_taggers+3) {
      int tagclass = static_call_menu - match_taggers;
      everyones_real_result.match.call_conc_options.tagger = tagclass << 5;

      for (i = 0; i < number_of_taggers[tagclass]; i++) {
         everyones_real_result.match.call_conc_options.tagger++;
         match_pattern(tagger_calls[tagclass][i]->name, (concept_descriptor *) 0);
      }
   }
   else if (static_call_menu == match_circcer) {
      everyones_real_result.match.call_conc_options.circcer = 0;

      for (i = 0; i < number_of_circcers; i++) {
         everyones_real_result.match.call_conc_options.circcer++;
         match_pattern(circcer_calls[i]->name, (concept_descriptor *) 0);
      }
   }
   else {
      if (kind == ui_command_select) {
         if (static_call_menu == match_resolve_extra_commands) {
            menu = extra_resolve_commands;
            menu_length = num_extra_resolve_commands;
         }
         else {
            menu = command_commands;
            menu_length = num_command_commands;
         }
      }
      else if (static_call_menu == match_directions) {
         menu = &direction_names[1];
         menu_length = last_direction_kind;
      }
      else if (static_call_menu == match_selectors) {
         menu = &selector_menu_list[1];
         menu_length = last_selector_kind;
      }
      else if (static_call_menu == match_startup_commands) {
         kind = ui_start_select;
         menu = startup_commands;
         menu_length = NUM_START_SELECT_KINDS;
      }
      else if (static_call_menu == match_resolve_commands) {
         kind = ui_resolve_select;
         menu = resolve_command_strings;
         menu_length = number_of_resolve_commands;
      }

      everyones_real_result.match.kind = kind;

      for (i = 0; i < menu_length; i++) {
         everyones_real_result.match.index = i;
         match_pattern(menu[i], (concept_descriptor *) 0);
      }
   }
}

    


/*
 * MATCH_USER_INPUT is the basic command matching function.
 *
 * The WHICH_COMMANDS parameter determines which list of commands
 * is matched against. Possibilities include:
 *
 * match_startup_commands - startup mode commands
 * match_resolve_commands - resolve mode commands
 * match_selectors        - selectors (e.g. "boys")
 * match_directions       - directions (e.g. "left")
 * <call menu index>      - normal commands, concepts, and calls
 *                          from specified call menu
 *
 * The USER_INPUT parameter is the user-specified string that is being
 * matched.
 *
 * MATCH_USER_INPUT returns the number of commands that are
 * compatible with the specified user input.  A command is compatible
 * if it is matches the user input as is or the user input could
 * be extended to match the command.
 *
 * If there is at least
 * one compatible command, then the maximal extension to the user
 * input that is compatible with all the compatible commands is returned
 * in EXTENSION; otherwise EXTENSION is cleared.
 * If there is at least one compatible command, then
 * a description of one such command is returned in MR.  EXTENSION
 * and MR can be NULL, if the corresponding information is not needed.
 *
 * MR->VALID is true if and only if there is at least one matching command.
 * MR->EXACT is true if and only if one or more commands exactly matched the
 * input (no extension required).  The remainder of MR is the
 * data to be returned to sd from one or more invocations of
 * user interface functions.
 *
 * The SF parameter is used to display a list of matching
 * commands. If SF is non-zero, then it must be a function pointer
 * that accepts two char * parameters and a match_result * parameter.
 * That function will be invoked
 * for each matching command.  The first parameter is the user input,
 * the second parameter is the extension corresponding to the command.
 * When showing matching commands,
 * wildcards are not expanded unless they are needed to match the
 * user input.  The extension may contain wildcards.  The third
 * parameter is the (volatile) match_result that describes the
 * matching command.  If SHOW_VERIFY is nonzero, then an attempt
 * will be made to verify that a call is possible in the current
 * setup before showing it.
 *
 * When searching for calls and concepts, it also searches for the special
 * commands provided by the "command_list" and "num_commands" arguments.
 * When such a command is found, we return "ui_command_select" as the kind,
 * and the index into that array as the index.
 *
 */

extern int match_user_input(
    int which_commands,
    long_boolean show,
    long_boolean show_verify)
{
#ifdef TIMING
    clock_t timer = clock();
    char time_buf[20];
    uims_debug_print("");
#endif

   /* Reclaim all old modifier blocks. */

   while (modifier_active_list) {
      modifier_block *item = modifier_active_list;
      modifier_active_list = item->gc_ptr;
      item->gc_ptr = modifier_inactive_list;
      modifier_inactive_list = item;
   }

   static_ss.extended_input[0] = 0;   /* needed if no matches or user input is empty */
   static_ss.match_count = 0;
   static_ss.exact_count = 0;
   static_ss.yielding_matches = 0;
   static_ss.exact_match = FALSE;
   static_ss.showing = show;
   static_ss.result.valid = FALSE;
   static_ss.result.exact = FALSE;
   static_ss.space_ok = FALSE;
   static_ss.verify = show_verify;
   static_call_menu = which_commands;

   if (static_call_menu >= 0) {
      search_menu(ui_call_select);
      search_menu(ui_concept_select);
      search_menu(ui_command_select);
   }  
   else {
      search_menu(ui_special_concept);
      /* During a resolve, we allow various other commands. */
      if (static_call_menu == match_resolve_commands) {
         static_call_menu = match_resolve_extra_commands;
         search_menu(ui_command_select);
      }
   }

#ifdef TIMING
    timer = clock() - timer;
    sprintf(time_buf, "%.2f", (double)timer / CLOCKS_PER_SEC);
    uims_debug_print(time_buf);
#endif

   return static_ss.match_count;
}
