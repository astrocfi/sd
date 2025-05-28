/*
 * sdmatch.c - command matching support
 *
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
#include "sdmatch.h"
#include <string.h> /* for strcpy */
#include <stdio.h>  /* for sprintf */
#include <ctype.h>  /* for tolower */

#ifdef TIMING
#include <time.h>
#endif

static callspec_block *empty_menu[] = {NULLCALLSPEC};
static callspec_block **call_menu_lists[NUM_CALL_LIST_KINDS];

static call_list_kind current_call_menu;
static long_boolean commands_last_option;

static int *concept_list; /* all concepts */
static int concept_list_length;

static int *level_concept_list; /* indices of concepts valid at current level */
static int level_concept_list_length;

typedef struct {
   char *full_input;      /* the current user input */
   char *extended_input;  /* the maximal common extension to the user input */
   char extension[200];   /* the extension for the current pattern */
   int match_count;       /* the number of matches so far */
   int yielding_matches;  /* the number of them that are marked "yield_if_ambiguous". */
   int exact_match;       /* true if an exact match has been found */
   int showing;           /* we are only showing the matching patterns */
   show_function sf;      /* function to call with matching command (if showing) */
   long_boolean verify;   /* true => verify calls before showing */
   int space_ok;          /* space is a legitimate next input character */
   match_result result;   /* value of the first or exact matching pattern */
} match_state;

typedef struct pat2_blockstruct {
   Cstring car;
   struct pat2_blockstruct *next;
} pat2_block;

static void match_wildcard(Cstring user, Cstring pat, pat2_block *pat2, char *patxp, Const match_result *result);


/* This is statically used by the match_wildcard and match_suffix_2 procedures. */

static match_state static_ss;


/* The following arrays must be coordinated with the Sd program */

/* BEWARE!!  This list is keyed to the definition of "start_select_kind" in sd.h . */
static Cstring startup_commands[] = {
    "exit from the program",
    "heads 1p2p",
    "sides 1p2p",
    "heads start",
    "sides start",
    "just as they are"
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

extern void
matcher_initialize(long_boolean show_commands_last)
{
   int i;
    int concept_number;
    concept_descriptor *p;
    int *item, *level_item;
    concept_kind end_marker = concept_diagnose;

    /* Decide whether we allow the "diagnose" concept, by deciding
        when we will stop the concept list scan. */
    if (diagnostic_mode) end_marker = marker_end_of_list;

    commands_last_option = show_commands_last;

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
        else if (p->kind == concept_comment || p->dup) {
            continue;
        }

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
        if (p->kind == concept_comment || p->dup) {
            continue;
        }

        *item = concept_number;
        item++;

        if (p->level <= calling_level) {
            *level_item = concept_number;
            level_item++;
        }
    }
}


extern void
matcher_setup_call_menu(call_list_kind cl, callspec_block *call_name_list[])
{
   int i;

   call_menu_lists[cl] = (callspec_block **) get_mem((number_of_calls[cl]) * sizeof(callspec_block *));

   for (i=0; i<number_of_calls[cl]; i++)
      call_menu_lists[cl][i] = call_name_list[i];
}

/*
 * Call Verification
 */

static long_boolean try_call_with_selector(callspec_block *call, selector_kind sel)
{
    real_jmp_buf my_longjmp_buffer;
    
    /* Create a temporary error handler. */

    longjmp_ptr = &my_longjmp_buffer;          /* point the global pointer at it. */
    if (setjmp(my_longjmp_buffer.the_buf)) {
        return FALSE;
    }

    interactivity = interactivity_database_init;   /* so deposit_call doesn't ask user for info */
    selector_for_initialize = sel; /* if selector needed, use this one */
    (void) deposit_call(call);
    if (parse_state.parse_stack_index != 0) {
        /* subsidiary calls are wanted: just assume it works */
        return TRUE;
    }
    parse_state.concept_write_ptr = &((*parse_state.concept_write_ptr)->next);
    toplevelmove(); /* does longjmp if error */
    return TRUE;
}


static long_boolean verify_call_with_selector(callspec_block *call, selector_kind sel)
{
    int bits0, bits1;
    int old_history_ptr;
    long_boolean result;
    parse_block *marker;

    bits0 = history[history_ptr+1].warnings.bits[0];
    bits1 = history[history_ptr+1].warnings.bits[1];
    old_history_ptr = history_ptr;
    marker = mark_parse_blocks();
    
    result = try_call_with_selector(call, sel);
    
    history_ptr = old_history_ptr;
    release_parse_blocks_to_mark(marker);
    history[history_ptr+1].warnings.bits[0] = bits0;
    history[history_ptr+1].warnings.bits[1] = bits1;
    longjmp_ptr = &longjmp_buffer;    /* restore the global error handler */
    interactivity = interactivity_normal;
    return result;
}

/*
 * Return TRUE if the specified call appears to be legal in the
 * current context.
 */

static long_boolean verify_call(call_list_kind cl, int call_index, selector_kind who)
{
    callspec_block *call;

    if (who < 0)
        who = selector_uninitialized;
    
    call = main_call_lists[cl][call_index];
    if ((call->callflagsh & CFLAGH__REQUIRES_SELECTOR) &&
            (who == selector_uninitialized)) {

        /*
         * The call requires a selector, but no selector has been
         * specified.  Try each possible selector (except "no one")
         * to see if one works.
         *
         * The problem with this approach is that often any call
         * will succeed for some selector that matches zero people.
         * For example, in the starting state "HEADS...", any call
         * directed at the sides will succeed (doing nothing).
         * One could argue this is a bug!
         */
        
        /* sue: new plan: try (beaus, ends, all), just like initialization */
        
        if (verify_call_with_selector(call, selector_beaus))
           return(TRUE);
        if (verify_call_with_selector(call, selector_ends))
           return(TRUE);
        if (verify_call_with_selector(call, selector_all))
           return(TRUE);
/* this was the old code...        
        selector_kind sel;
        for (sel=1; sel<last_selector_kind; ++sel) {
            if (verify_call_with_selector(call, sel))
                return (TRUE);
        }
*/
    }
    else {
        return verify_call_with_selector(call, who);
    }
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
    
static void record_a_match(Const match_result *result)
{
   if (static_ss.extended_input) {
      if (static_ss.match_count == 0)
         /* this is the first match */
         strcpy(static_ss.extended_input, static_ss.extension);
      else
         strn_gcp(static_ss.extended_input, static_ss.extension);
   }

   if (static_ss.match_count == 0)
      static_ss.result = *result;      /* always copy the first match */
   else if (*static_ss.extension == '\0' && !(result->callflags1 & CFLAG1_YIELD_IF_AMBIGUOUS))
      static_ss.result = *result;      /* also copy any exact match, as long as it isn't yielding */

   if (*static_ss.extension == '\0')
      static_ss.result.exact = TRUE;

   static_ss.match_count++;

   if (result->callflags1 & CFLAG1_YIELD_IF_AMBIGUOUS)
      static_ss.yielding_matches++;

   if (static_ss.showing) {
      if (!static_ss.verify ||
            (result->kind != ui_call_select) ||
            verify_call(current_call_menu, result->index, result->who))
         (*static_ss.sf)(static_ss.full_input, static_ss.extension, result);
   }
}

/*
 * match_suffix_2 is like *****, except that the pattern is
 * supplied in two strings PAT1 and PAT2 that are treated as if they
 * were logically concatenated.  Also, PAT1 is assumed not to contain
 * wildcards.
 *
 */
/* Patxp is where the next character of the extension of the user input for the current pattern is to be written. */

static void match_suffix_2(Cstring user, Cstring pat1, pat2_block *pat2, char *patxp, Const match_result *result)
{
   Cstring cur_user = user;
   Cstring cur_pat1 = pat1;

   for (;;) {
      if (*cur_pat1 == 0 && pat2) {
         /* PAT1 has run out, get a string from PAT2 */
         cur_pat1 = pat2->car;
         pat2 = pat2->next;
      }
      else if (cur_user && (*cur_user == '\0')) {
         /* we have just reached the end of the user input */

         if (*cur_pat1 == ' ')
            static_ss.space_ok = TRUE;

         if (!pat2 && *cur_pat1 == '\0') {
            /* exact match */
            *patxp = '\0';
            record_a_match(result);
            break;
         }

         /* we need to look at the rest of the pattern because
            if it contains wildcards, then there are multiple matches */
         cur_user = (Cstring) 0;
      }
      else {
         char p = *cur_pat1++;

         /* Check for expansion of wildcards.  But if we are just listing
            the matching commands, there is no point in expanding wildcards
            that are past the part that matches the user input */

         if (p == '@') {
            match_wildcard(cur_user, cur_pat1, pat2, patxp, result);
   
            if (cur_user==0) {
               /* user input has run out, just looking for more wildcards */
   
               Cstring ep = get_escape_string(*cur_pat1++);

               if (ep && *ep) {
                  (void) strcpy(patxp, ep);
                  patxp += strlen(patxp);
               }
               else {
                  if (ep) {
                     while (*cur_pat1++ != '@');
                     cur_pat1++;
                  }

                  /* Don't write duplicate blanks. */
                  /* ***** It would be nice to make this not look back before the beginning!!!!! */
                  if (*cur_pat1 == ' ' && patxp[-1] == ' ') cur_pat1++;
               }
            }
            else {
               char u = *cur_user++;
               Cstring ep = get_escape_string(*cur_pat1++);

               if (u == '<') {
                  if (ep && *ep) {
                     int i;

                     for (i=1; ep[i]; i++) {
                        if (!cur_user[i-1]) {
                           while (ep[i]) { *patxp++ = ep[i]; i++; }
                           cur_user = 0;
                           goto cont;
                        }
   
                        if (cur_user[i-1] != tolower(ep[i])) return;
                     }
   
                     cur_user += strlen((char *) ep)-1;
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
                        while (*cur_pat1++ != '@');
                        cur_pat1++;
                     }

                     cur_user--;    /* Back up! */

                     /* Check for something that would cause the pattern effectively to have
                        two consecutive blanks, and compress them to one.  This can happen
                        if the pattern string has lots of confusing '@' escapes. */
                     if (*cur_pat1 == ' ' && cur_user[-1] == ' ') cur_pat1++;
                  }
               }
            }
         }
         else {
            if (cur_user==0) {
               /* user input has run out, just looking for more wildcards */
   
               if (p) {
                  /* there is more pattern */
                  *patxp++ = tolower(p);
               }
               else {
                  /* reached the end of the pattern */
   
                  *patxp = '\0';
   
                  if (!pat2) {
                     record_a_match(result);
                     break;
                  }
               }
            }
            else {
               char u = *cur_user++;
   
               if (u != p && (p > 'Z' || p < 'A' || u != p+'a'-'A'))
                  break;
            }
         }
      }
   }
}

/*
 * Match_wildcard tests for and handles pattern suffixes that begin with
 * a wildcard such as "<anyone>".  A wildcard is handled only if there is
 * room in the Result struct to store the associated value.
 */

static void match_wildcard(Cstring user, Cstring pat, pat2_block *pat2, char *patxp, Const match_result *result)
{
   Cstring prefix;
   int i;
   match_result new_result;
   pat2_block p2b;
   char key = *pat++;
   p2b.car = pat;
   p2b.next = pat2;

   /* if we are just listing the matching commands, there
      is no point in expanding wildcards that are past the
      part that matches the user input.  That is why we test
      "static_ss.showing" and "(user == 0)". */

   if ((key == '6' || key == 'k') && (result->who == selector_uninitialized)) {
      if (user == 0) return;
      new_result = *result;
      for (i=1; i<=last_selector_kind; ++i) {
         new_result.who = (selector_kind) i;
         match_suffix_2(user, ((key == '6') ? selector_names : selector_singular)[i], &p2b, patxp, &new_result);
      }
   }
   else if (key == 'h' && (result->where == direction_uninitialized)) {
      if (user == 0) return;
      new_result = *result;
      for (i=1; i<=last_direction_kind; ++i) {
         new_result.where = (direction_kind) i;
         match_suffix_2(user, direction_names[i], &p2b, patxp, &new_result);
      }
   }
   else if (key == 'v' && (result->tagger & 0xFF000000UL) == 0) {
      /* We allow recursion 4 levels deep.  In fact, we consider it
         inappropriate to stack revert/reflect things.  There are special
         calls "revert, then reflect", etc. for this purpose. */
      if (user == 0) return;
      new_result = *result;
      new_result.tagger <<= 8;
      for (i=0; i<number_of_taggers; ++i) {
         new_result.tagger++;
         match_suffix_2(user, tagger_calls[i]->name, &p2b, patxp, &new_result);
      }
   }
   else if (key == 'j' && (!result->cross)) {
      char crossname[80];
      char *crossptr = crossname;

      new_result = *result;
      while ((*crossptr++ = *pat++) != '@');
      pat++;
      crossptr--;
      *crossptr = 0;
      new_result.cross = TRUE;
      p2b.car = pat;
      match_suffix_2(user, crossname, &p2b, patxp, &new_result);
   }
   else if (key == 'C' && (!result->cross)) {
      new_result = *result;
      new_result.cross = TRUE;
      p2b.car = pat;
      match_suffix_2(user, "cross ", &p2b, patxp, &new_result);
   }
   else if (key == 'J' && (!result->magic)) {
      char magicname[80];
      char *magicptr = magicname;

      new_result = *result;
      while ((*magicptr++ = *pat++) != '@');
      pat++;
      magicptr--;
      *magicptr = 0;
      new_result.magic = TRUE;
      p2b.car = pat;
      match_suffix_2(user, magicname, &p2b, patxp, &new_result);
   }
   else if (key == 'M' && (!result->magic)) {
      new_result = *result;
      new_result.magic = TRUE;
      p2b.car = pat;
      match_suffix_2(user, "magic ", &p2b, patxp, &new_result);
   }
   else if (key == 'E' && (!result->interlocked)) {
      char interlockedname[80];
      char *interlockedptr = interlockedname;

      new_result = *result;
      while ((*interlockedptr++ = *pat++) != '@');
      pat++;
      interlockedptr--;
      *interlockedptr = 0;
      new_result.interlocked = TRUE;
      p2b.car = pat;
      match_suffix_2(user, interlockedname, &p2b, patxp, &new_result);
   }
   else if (key == 'I' && (!result->interlocked)) {
      new_result = *result;
      new_result.interlocked = TRUE;
      p2b.car = pat;
      match_suffix_2(user, "interlocked ", &p2b, patxp, &new_result);
   }
   else if (key == 'e' && (!result->left)) {
      new_result = *result;
      while (*pat++ != '@');
      pat++;
      new_result.left = TRUE;
      p2b.car = pat;
      match_suffix_2(user, "left", &p2b, patxp, &new_result);
   }
   else if (key == '9') {
      if (user == 0) return;
      new_result = *result;
      new_result.howmanynumbers++;

      for (i=1;; i++) {
          prefix = cardinals[i-1];
          if (prefix == NULL) {
              break;
          }
          new_result.number_fields += 1 << ((new_result.howmanynumbers-1)*4);
          match_suffix_2(user, prefix, &p2b, patxp, &new_result);
      }
   }
   else if (key == 'u') {
      if (user == 0) return;
      new_result = *result;
      new_result.howmanynumbers++;

      for (i=1;; i++) {
          prefix = ordinals[i-1];
          if (prefix == NULL) {
              break;
          }
          new_result.number_fields += 1 << ((new_result.howmanynumbers-1)*4);
          match_suffix_2(user, prefix, &p2b, patxp, &new_result);
      }
   }
   else if (key == 'a' || key == 'b' || key == 'B') {
      if (user == 0) return;
      new_result = *result;

      new_result.howmanynumbers++;

      for (i=1;; i++) {
          prefix = n_4_patterns[i-1];
          if (prefix == NULL) {
              break;
          }
          new_result.number_fields += 1 << ((new_result.howmanynumbers-1)*4);
          match_suffix_2(user, prefix, &p2b, patxp, &new_result);
      }

      /* special case: allow "quarter" for 1/4 */
      new_result.number_fields = result->number_fields + (1 << ((new_result.howmanynumbers-1)*4));
      match_suffix_2(user, "quarter", &p2b, patxp, &new_result);

      /* special case: allow "half" or "1/2" for 2/4 */
      new_result.number_fields = result->number_fields + (2 << ((new_result.howmanynumbers-1)*4));
      match_suffix_2(user, "half", &p2b, patxp, &new_result);
      match_suffix_2(user, "1/2", &p2b, patxp, &new_result);

      /* special case: allow "three quarter" for 3/4 */
      new_result.number_fields = result->number_fields + (3 << ((new_result.howmanynumbers-1)*4));
      match_suffix_2(user, "three quarter", &p2b, patxp, &new_result);

      /* special case: allow "full" for 4/4 */
      new_result.number_fields = result->number_fields + (4 << ((new_result.howmanynumbers-1)*4));
      match_suffix_2(user, "full", &p2b, patxp, &new_result);
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
 
static void match_pattern(Cstring pattern, Const match_result *result)
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
          
         if (pch == '*') {
              return;
         }
         /* special case: static_ss.extension not set */
         static_ss.match_count++;
         return;
      }
   }
   else if (uch != tolower(pch) && pch != '@') {
      /* fails to match */
      return;
   }

   normal:

   if (pch == '*') {
      /* a commented out pattern */
      return;
   }

   p2b.car = pattern;
   p2b.next = (pat2_block *) 0;

   match_suffix_2(static_ss.full_input, "", &p2b, static_ss.extension, result);
}

static void search_menu(uims_reply kind, int which_command)
{
   int i;
   Cstring *menu;
   int menu_length;
   match_result result;

   result.valid = TRUE;
   result.exact = FALSE;
   result.kind = kind;
   result.who = selector_uninitialized;
   result.where = direction_uninitialized;
   result.tagger = 0;
   result.left = FALSE;
   result.cross = FALSE;
   result.magic = FALSE;
   result.interlocked = FALSE;
   result.number_fields = 0;
   result.howmanynumbers = 0;
   result.callflags1 = 0;

   if (kind == ui_call_select) {
      current_call_menu = which_command;

      menu_length = number_of_calls[current_call_menu];

      for (i = 0; i < menu_length; i++) {
         result.index = i;
         result.callflags1 = call_menu_lists[current_call_menu][i]->callflags1;
         match_pattern(call_menu_lists[current_call_menu][i]->name, &result);
      }
   }
   else {
      if (kind == ui_concept_select) {
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
            result.index = *item;
            match_pattern(concept_descriptor_table[*item].name, &result);
            item++;
         }
      }
      else if (which_command == match_taggers) {
         for (i = 0; i < number_of_taggers; i++) {
            result.index = i;
            match_pattern(tagger_calls[i]->name, &result);
         }
      }
      else {
         if (kind == ui_command_select) {
            menu = command_commands;
            menu_length = num_command_commands;
         }
         else if (which_command == match_directions) {
            menu = &direction_names[1];
            menu_length = last_direction_kind;
         }
         else if (which_command == match_selectors) {
            menu = &selector_names[1];
            menu_length = last_selector_kind;
         }
         else if (which_command == match_startup_commands) {
            kind = ui_start_select;
            menu = startup_commands;
            menu_length = NUM_START_SELECT_KINDS;
         }
         else if (which_command == match_resolve_commands) {
            kind = ui_resolve_select;
            menu = resolve_command_strings;
            menu_length = number_of_resolve_commands;
         }
   
         result.kind = kind;

         for (i = 0; i < menu_length; i++) {
            result.index = i;
            match_pattern(menu[i], &result);
         }
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
    char *user_input,
    int which_commands,
    match_result *mr,
    char *extension,
    show_function sf,
    long_boolean show_verify)
{
    char *p;

#ifdef TIMING
    clock_t timer = clock();
    char time_buf[20];
    uims_debug_print("");
#endif

    static_ss.full_input = user_input;
    static_ss.extended_input = extension;
    static_ss.match_count = 0;
    static_ss.yielding_matches = 0;
    static_ss.exact_match = FALSE;
    static_ss.showing = (sf != 0);
    static_ss.sf = sf;
    static_ss.result.valid = FALSE;
    static_ss.result.exact = FALSE;
    static_ss.space_ok = FALSE;
    static_ss.verify = show_verify;

    if (extension) { /* needed if no matches or user input is empty */
        extension[0] = 0;
    }

    /* convert user input to lower case for easier comparison */
    p = user_input;
    while (*p) {*p = tolower(*p); p++;}

    if (which_commands >= 0) {
       if (!commands_last_option)
           search_menu(ui_command_select, which_commands);
   
       search_menu(ui_call_select, which_commands);
       search_menu(ui_concept_select, which_commands);
   
       if (commands_last_option)
           search_menu(ui_command_select, which_commands);
    }
    else
       search_menu((uims_reply) 0, which_commands);

    if (mr) {
        *mr = static_ss.result;
        mr->space_ok = static_ss.space_ok;
        mr->yielding_matches = static_ss.yielding_matches;
    }

#ifdef TIMING
    timer = clock() - timer;
    sprintf(time_buf, "%.2f", (double)timer / CLOCKS_PER_SEC);
    uims_debug_print(time_buf);
#endif

    return static_ss.match_count;
}
