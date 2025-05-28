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
   matcher_add_call_to_menu
   match_user_input
*/

/* #define TIMING */ /* uncomment to display timing information */

#include "sd.h"
#include "sdmatch.h"
#include <string.h> /* for strcpy, strncmp */
#include <stdio.h>  /* for sprintf */
#include <ctype.h>  /* for tolower */

#ifdef TIMING
#include <time.h>
#endif

static char           *empty_menu[] = {NULL};
static char           **call_menu_lists[NUM_CALL_LIST_KINDS];
static call_list_kind current_call_menu;
static long_boolean   commands_last_option;

typedef struct {
    int number;            /* index of concept in concept_descriptor_table */
    char *name;            /* name of concept in all lower case */
} concept_item;

static concept_item *concept_list; /* all concepts */
static int concept_list_length;

static concept_item *level_concept_list; /* only concepts valid at current level */
static int level_concept_list_length;

typedef struct {
    char *full_input;      /* the current user input */
    char *extended_input;  /* the maximal common extension to the user input */
    char extension[200];   /* the extension for the current pattern */
    int match_count;       /* the number of matches so far */
    int exact_match;       /* true if an exact match has been found */
    int showing;           /* we are only showing the matching patterns */
    show_function sf;      /* function to call with matching command (if showing) */
    long_boolean verify;   /* true => verify calls before showing */
    int space_ok;          /* space is a legitimate next input character */
    match_result result;   /* value of the first or exact matching pattern */
} match_state;

typedef void input_matcher(match_state *sp);

static void add_call_to_menu(char ***menu, int call_menu_index,
                             int menu_size, char callname[]);
static void startup_matcher(match_state *sp);
static void resolve_matcher(match_state *sp);
static void call_matcher(match_state *sp);
static void selector_matcher(match_state *sp);
static void direction_matcher(match_state *sp);
static void search_menu(match_state *sp, char *menu[], int menu_length, uims_reply kind);
static void search_concept(match_state *sp);
static void match_pattern(match_state *sp, char pattern[], Const match_result *result);
static void match_suffix(match_state *sp, char *user, char *pat,
                          char *patxp, Const match_result *result);
static void match_suffix_2(match_state *sp, char *user, char *pat1, char *pat2,
                          char *patxp, Const match_result *result);
static int  match_wildcard(match_state *sp, char *user, char *pat,
                          char *patxp, Const match_result *result);
static void record_a_match(match_state *sp, Const match_result *result);
static void strn_gcp(char *s1, char *s2);
static long_boolean verify_call(call_list_kind cl, int call_index, selector_kind who);
static long_boolean verify_call_with_selector(callspec_block *call, selector_kind sel);
static long_boolean try_call_with_selector(callspec_block *call, selector_kind sel);

static char *strdup_lower(char *source);
static void strcpy_lower(char *dest, char *source);

/* the following arrays must be coordinated with the sd program */

#ifdef THINK_C
#define NOMAC "*"
#else
#define NOMAC
#endif

#ifdef THINK_C
#define IFMAC(x,y) x
#else
#define IFMAC(x,y) y
#endif

/*
 * We inhibit certain commands from matching by prefixing them with
 * an asterisk.
 */

/* the following arrays must be coordinated with the sd program */

/* BEWARE!!  This list is keyed to the definition of "start_select_kind" in sd.h . */
static char *startup_commands[] = {
    "exit from the program",
    "heads 1p2p",
    "sides 1p2p",
    "heads start",
    "sides start",
    "just as they are"
};

/* The first part of this array tracks the "command_kind" enumeration.
   There are NUM_COMMAND_KINDS items in that part.  The rest of it
   corresponds to the special commands defined in sdmatch.h .  There
   are NUM_SPECIAL_COMMANDS of those items. */

static char *command_commands[] = {
/* sue: changed "exit" to "quit" to match standard mac usage */
    IFMAC("quit the program","exit the program"),
    "undo last call",
    IFMAC("end this sequence","abort this sequence"),
    "insert a comment ...",
    IFMAC("save as ...","change output file ..."),
    NOMAC "end this sequence ...",
    "resolve ...",
    "reconcile ...",
    IFMAC("pick random call ...","do anything ..."),
/* sue: I prefer "nice setup", even on the mac */
    "nice setup ...",
    NOMAC "show neglected calls ...",
/* sue: "insert picture" isn't great because you must type 2 words to distinguish this
   from the command "insert a comment".  Changed it to "keep picture" */
    IFMAC("keep picture","save picture"),
    "refresh display",
/* The following items are the special ones. */
    IFMAC("modify next call","allow modifications"),
    "toggle concept levels"
};

/* resolve_commands tracks the resolve_command_kind enumeration */

static char *resolve_commands[] = {
    "abort the search",
    "find another",
    "next",
    "previous",
    "accept current choice",
    "raise reconcile point",
    "lower reconcile point"
};

static char *n_patterns[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    /* sue: added for consistency with stuff in direction popups */
    "8",
    0
};

static char *n_4_patterns[] = {
    "1/4",
    "2/4",
    "3/4",
    "4/4",
    "5/4",
    0
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
    int concept_number, count;
    concept_descriptor *p;
    concept_item *item, *level_item;

    commands_last_option = show_commands_last;

    /* initialize our special empty call menu */

    call_menu_lists[call_list_empty] = empty_menu;
    number_of_calls[call_list_empty] = 0;

    /* count the number of concepts to put in the lists */
    
    concept_list_length = 0;
    level_concept_list_length = 0;
    for (concept_number=0;;concept_number++) {
        p = &concept_descriptor_table[concept_number];
        if (p->kind == concept_comment) {
            continue;
        }
        if (p->kind == marker_end_of_list) {
            break;
        }
        concept_list_length++;
        if (p->level <= calling_level) {
            level_concept_list_length++;
        }
    }

    /* create the concept lists */

    concept_list = (concept_item *)get_mem(
        sizeof(concept_item) * concept_list_length);
    level_concept_list = (concept_item *)get_mem(
        sizeof(concept_item) * level_concept_list_length);

    item = concept_list;
    level_item = level_concept_list;
    for (concept_number=0;;concept_number++) {
        p = &concept_descriptor_table[concept_number];
        if (p->kind == concept_comment) {
            continue;
        }
        if (p->kind == marker_end_of_list) {
            break;
        }
        item->number = concept_number;
        item->name = strdup_lower(p->name);
        if (p->level <= calling_level) {
            level_item->number = item->number;
            level_item->name = item->name;
            level_item++;
        }
        item++;
    }
}

/*
 * Call MATCHER_ADD_CALL_TO_MENU to add a call to a call menu.
 * We have been given the name of one call (call number
 * call_menu_index, from 0 to number_of_calls[cl]) to be added to the
 * call menu cl (enumerated over the type call_list_kind.)
 * The string is guaranteed to be in stable storage.
 */
 
extern void
matcher_add_call_to_menu(call_list_kind cl, int call_menu_index, char name[])
{
    int menu_num = (int) cl;

    add_call_to_menu(&call_menu_lists[menu_num], call_menu_index,
             number_of_calls[menu_num], name);
}

static void
add_call_to_menu(char ***menu, int call_menu_index, int menu_size,
                 char callname[])
{
    if (call_menu_index == 0) {
        /* first item in this menu; set it up */
        *menu = (char **)get_mem((unsigned)(menu_size+1) * sizeof(char **));
    }

    (*menu)[call_menu_index] = strdup_lower(callname);
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
 */

extern int
match_user_input(char *user_input, int which_commands, match_result *mr,
                 char *extension, show_function sf, long_boolean show_verify)
{
    match_state ss;
    input_matcher *f;
    char *p;

#ifdef TIMING
    clock_t timer = clock();
    char time_buf[20];
    uims_debug_print("");
#endif

    ss.full_input = user_input;
    ss.extended_input = extension;
    ss.match_count = 0;
    ss.exact_match = FALSE;
    ss.showing = (sf != 0);
    ss.sf = sf;
    ss.result.valid = FALSE;
    ss.result.exact = FALSE;
    ss.space_ok = FALSE;
    ss.verify = show_verify;

    if (extension) { /* needed if no matches or user input is empty */
        extension[0] = 0;
    }

    /* convert user input to lower case for easier comparison */
    p = user_input;
    while (*p) {*p = tolower(*p); p++;}

    if (which_commands == match_startup_commands)
        f = startup_matcher;
    else if (which_commands == match_resolve_commands)
        f = resolve_matcher;
    else if (which_commands == match_selectors)
        f = selector_matcher;
    else if (which_commands == match_directions)
        f = direction_matcher;
    else if ((which_commands >= 0) && (which_commands < NUM_CALL_LIST_KINDS)) {
        f = call_matcher;
        current_call_menu = which_commands;
    }
    else /* invalid parameter */
        return 0;

    (*f)(&ss);
    if (mr) {
        *mr = ss.result;
        mr->space_ok = ss.space_ok;
    }

#ifdef TIMING
    timer = clock() - timer;
    sprintf(time_buf, "%.2f", (double)timer / CLOCKS_PER_SEC);
    uims_debug_print(time_buf);
#endif

    return ss.match_count;
}
    
static void
startup_matcher(match_state *sp)
{
    search_menu(sp, startup_commands, NUM_START_SELECT_KINDS, ui_start_select);
}

static void
resolve_matcher(match_state *sp)
{
    search_menu(sp, resolve_commands,
        NUM_RESOLVE_COMMAND_KINDS, ui_resolve_select);
}

static void
call_matcher(match_state *sp)
{
    /* search_menu(sp, concept_menu_list, concept_menu_len, ui_concept_select); */

    if (!commands_last_option)
        search_menu(sp, command_commands, NUM_COMMAND_KINDS+NUM_SPECIAL_COMMANDS, ui_command_select);

    search_menu(sp,
        call_menu_lists[current_call_menu], number_of_calls[current_call_menu],
        ui_call_select);

    search_concept(sp);

    if (commands_last_option)
        search_menu(sp, command_commands, NUM_COMMAND_KINDS+NUM_SPECIAL_COMMANDS, ui_command_select);
}

static void
selector_matcher(match_state *sp)
{
    search_menu(sp, &selector_names[1], last_selector_kind, (uims_reply) 0);
}

static void
direction_matcher(match_state *sp)
{
    search_menu(sp, &direction_names[1], last_direction_kind, (uims_reply) 0);
}

static void
search_menu(match_state *sp, char *menu[], int menu_length, uims_reply kind)
{
    int i;
    match_result result;

    result.valid = TRUE;
    result.exact = FALSE;
    result.kind = kind;
    result.who = selector_uninitialized;
    result.where = direction_uninitialized;
    result.number_fields = 0;
    result.howmanynumbers = 0;
    for (i = 0; i < menu_length; i++) {
        result.index = i;
        match_pattern(sp, menu[i], &result);
    }
}

static void
search_concept(match_state *sp)
{
    int count;
    concept_item *item;
    match_result result;

    result.valid = TRUE;
    result.exact = FALSE;
    result.kind = ui_concept_select;
    result.who = selector_uninitialized;
    result.where = direction_uninitialized;
    result.number_fields = 0;
    result.howmanynumbers = 0;

    if (allowing_all_concepts) {
        item = concept_list;
        count = concept_list_length;
    }
    else {
        item = level_concept_list;
        count = level_concept_list_length;
    }

    while (--count >= 0) {
        result.index = item->number;
        match_pattern(sp, item->name, &result);
        ++item;
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
 
static void
match_pattern(match_state *sp, char pattern[], Const match_result *result)
{
    int pch, uch;

    /* the following special cases are purely to improve performance */

    pch = pattern[0];
    uch = sp->full_input[0];

    if (uch == '\0') {
        /* null user input matches everything (except a comment) */
        /* special case: pattern never begins with SPACE */
        /* special case: ignore wildcards, we know there are multiple matches */
        
        if (pch == '*') {
            return;
        }
        /* special case: sp->extension not set */
        ++sp->match_count;
	    if (sp->showing) {
	        if (!sp->verify
	                 || (result->kind != ui_call_select)
	                 || verify_call(current_call_menu, result->index, result->who)) {
                (*sp->sf)("", pattern, result);
            }
        }
        return;
    }

    if ((uch != pch) && (pch != '<')) {
        /* fails to match */
        return;
    }

    if (pch == '*') {
        /* a commented out pattern */
        return;
    }

    match_suffix(sp, sp->full_input, pattern, sp->extension, result);
}

/*
 * Match_suffix continues the matching process for a suffix of the current
 * user input (User) and a suffix of the current pattern (Pat).  Patxp is
 * where the next character of the extension of the user input for the current
 * pattern is to be written.
 *
 */

static void
match_suffix(match_state *sp, char *user, char *pat, char *patxp, Const match_result *result)
{
    int exact;

    if (user && (*user == '\0')) {
        /* we have just reached the end of the user input */
        if (*pat == '\0') {
            /* exact match */
            *patxp = '\0';
            record_a_match(sp, result);
        }
        else {
            if (*pat == ' ')
                sp->space_ok = TRUE;
            /* we need to look at the rest of the pattern because
               if it contains wildcards, then there are multiple matches */
            match_suffix(sp, 0, pat, patxp, result);
        }
    }
    else if ((*pat == '<') &&
             match_wildcard(sp, user, pat, patxp, result)) {
        ; /* everything done by match_wildcard */
    }    
    else if (user==0) {
        /* user input has run out, just looking for more wildcards */
        if (*pat) {
            /* there is more pattern */
            *patxp++ = *pat;
            match_suffix(sp, user, pat+1, patxp, result);
        }
        else {
            /* reached the end of the pattern */
            *patxp = '\0';
            record_a_match(sp, result);
        }
    }
    else if (*user == *pat) {
        match_suffix(sp, user+1, pat+1, patxp, result);
    }
}


/*
 * Match_suffix_2 is like match_suffix, except that the pattern is
 * supplied in two strings PAT1 and PAT2 that are treated as if they
 * were logically concatenated.  Also, PAT1 is assumed not to contain
 * wildcards.
 *
 */

static void
match_suffix_2(match_state *sp, char *user, char *pat1, char *pat2,
               char *patxp, Const match_result *result)
{
    if (*pat1 == 0) {
        /* PAT1 has run out, switch to PAT2 */
        match_suffix(sp, user, pat2, patxp, result);
    }
    else if (user && (*user == '\0')) {
        /* we have just reached the end of the user input */
        if (*pat1 == ' ')
            sp->space_ok = TRUE;
        /* we need to look at the rest of the pattern because
           if it contains wildcards, then there are multiple matches */
        match_suffix_2(sp, NULL, pat1, pat2, patxp, result);
    }
    else if (user==0) {
        /* user input has run out, just looking for more wildcards */
        *patxp++ = *pat1;
        match_suffix_2(sp, user, pat1+1, pat2, patxp, result);
    }
    else if (*user == *pat1) {
        match_suffix_2(sp, user+1, pat1+1, pat2, patxp, result);
    }
}

/*
 * Match_wildcard tests for and handles pattern suffixes that being with
 * a wildcard such as "<anyone>".  A wildcard is handled only if there is
 * room in the Result struct to store the associated value.
 */

static int
match_wildcard(match_state *sp, char *user, char *pat, char *patxp, Const match_result *result)
{
    char temp[200];
    char *prefix, *suffix;
    int i;
    match_result new_result;

    if (sp->showing && (user==0)) {
        /* if we are just listing the matching commands, there
           is no point in expanding wildcards that are past the
           part that matches the user input */
        return FALSE;
    }

    if ((strncmp(pat, "<anyone>", 8)==0) && (result->who == selector_uninitialized)) {
        suffix = pat+8;
        new_result = *result;
        for (i=1; i<=last_selector_kind; ++i) {
            new_result.who = (selector_kind) i;
            prefix = selector_names[i];
            match_suffix_2(sp, user, prefix, suffix, patxp, &new_result);
        }
        return FALSE;
    }
    else if ((strncmp(pat, "<direction>", 11)==0) && (result->where == direction_uninitialized)) {
        suffix = pat+11;
        new_result = *result;
        for (i=1; i<=last_direction_kind; ++i) {
            new_result.where = (direction_kind) i;
            prefix = direction_names[i];
            match_suffix_2(sp, user, prefix, suffix, patxp, &new_result);
        }
        return FALSE;
    }
    else if (strncmp(pat, "<n>", 3)==0) {
        suffix = pat+3;
        new_result = *result;
        new_result.howmanynumbers++;

        for (i=1;; ++i) {
            new_result.number_fields += 1 << ((new_result.howmanynumbers-1)*4);
            prefix = n_patterns[i-1];
            if (prefix == NULL) {
                break;
            }
            match_suffix_2(sp, user, prefix, suffix, patxp, &new_result);
        }
        return FALSE;
    }
    else if (strncmp(pat, "<n/4>", 5)==0) {
        suffix = pat+5;
        new_result = *result;

        new_result.howmanynumbers++;

        for (i=1;; ++i) {
            new_result.number_fields += 1 << ((new_result.howmanynumbers-1)*4);
            prefix = n_4_patterns[i-1];
            if (prefix == NULL) {
                break;
            }
            match_suffix_2(sp, user, prefix, suffix, patxp, &new_result);
        }

        /* special case: allow 1/2 for 2/4 */
        new_result.number_fields = result->number_fields + (2 << ((new_result.howmanynumbers-1)*4));
        match_suffix_2(sp, user, "1/2", suffix, patxp, &new_result);
        return FALSE;
    }
    else return FALSE;
}

/*
 * Record a match.  Extension is how the current input would be extended to
 * match the current pattern.  Result is the value of the
 * current pattern.  Special case: if user input is empty, extension is
 * not set.
 */
    
static void
record_a_match(match_state *sp, Const match_result *result)
{
    if (sp->extended_input) {
        if (sp->match_count == 0)
            /* this is the first match */
            strcpy(sp->extended_input, sp->extension);
        else
            strn_gcp(sp->extended_input, sp->extension);
    }
    if ((sp->match_count == 0) || (*sp->extension == '\0'))
        /* first match or an exact match */
        sp->result = *result;
    if (*sp->extension == '\0')
        sp->result.exact = TRUE;
    ++sp->match_count;
	if (sp->showing) {
	    if (!sp->verify
	             || (result->kind != ui_call_select)
	             || verify_call(current_call_menu, result->index, result->who)) {
            (*sp->sf)(sp->full_input, sp->extension, result);
        }
    }
}

/*
 * STRN_GCP sets S1 to be the greatest common prefix of S1 and S2.
 */

static void
strn_gcp(char *s1, char *s2)
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
 * Call Verification
 */

/*
 * Return TRUE if the specified call appears to be legal in the
 * current context.
 */

static long_boolean
verify_call(call_list_kind cl, int call_index, selector_kind who)
{
    callspec_block *call;
    selector_kind sel;

    if (who < 0)
        who = selector_uninitialized;
    
    call = main_call_lists[cl][call_index];
    if ((call->callflags1 & CFLAG1_REQUIRES_SELECTOR) &&
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
        
        if (verify_call_with_selector(call, selector_beaux))
           return(TRUE);
        if (verify_call_with_selector(call, selector_ends))
           return(TRUE);
        if (verify_call_with_selector(call, selector_all))
           return(TRUE);
/* this was the old code...        
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

static long_boolean
verify_call_with_selector(callspec_block *call, selector_kind sel)
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
    initializing_database = FALSE;
    not_interactive = FALSE;
    return result;
}

static long_boolean
try_call_with_selector(callspec_block *call, selector_kind sel)
{
    real_jmp_buf my_longjmp_buffer;
    
    /* Create a temporary error handler. */

    longjmp_ptr = &my_longjmp_buffer;          /* point the global pointer at it. */
    if (setjmp(my_longjmp_buffer.the_buf)) {
        return FALSE;
    }

    initializing_database = TRUE; /* so deposit_call doesn't ask user for info */
    not_interactive = TRUE;
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

/* Utility */

static char *
strdup_lower(char *source)
{
    char *dest;
    dest = get_mem(strlen(source) + 1);
    strcpy_lower(dest, source);
    return dest;
}

static void
strcpy_lower(char *dest, char *source)
{
    while (*dest++ = tolower(*source++));
}
