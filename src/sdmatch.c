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

#include "sd.h"
#include "sdmatch.h"
#include <string.h> /* for strcpy, strncmp */
#include <stdio.h>  /* for sprintf */
#include <ctype.h>  /* for tolower */

static char           **concept_menu_list;
static int            concept_menu_len;
static char           *empty_menu[] = {NULL};
static char           **call_menu_lists[NUM_CALL_LIST_KINDS];
static call_list_kind current_call_menu;
static long_boolean   commands_last_option;

typedef struct {
    char *full_input;      /* the current user input */
    char *extended_input;  /* the maximal common extension to the user input */
    char extension[200];   /* the extension for the current pattern */
    int match_count;       /* the number of matches so far */
    int exact_match;       /* true if an exact match has been found */
    int showing;           /* we are only showing the matching patterns */
    show_function sf;      /* function to call with matching command (if showing) */
    int verify;            /* nonzero => verify calls before showing */
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
static void search_menu(match_state *sp, char *menu[], int menu_length, uims_reply kind);
static void search_concept(match_state *sp);
static void match_pattern(match_state *sp, char pattern[], match_result *result);
static void match_suffix(match_state *sp, char *user, char *pat,
                          char *patxp, match_result *result);
static int  match_wildcard(match_state *sp, char *user, char *pat,
                          char *patxp, match_result *result);
static void record_a_match(match_state *sp, match_result *result);
static void strn_gcp(char *s1, char *s2);
static long_boolean verify_call(call_list_kind cl, int call_index, selector_kind who);
static long_boolean verify_call_with_selector(callspec_block *call, selector_kind sel);
static long_boolean try_call_with_selector(callspec_block *call, selector_kind sel);

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

/* startup_commands tracks the start_select_kind enumeration */

static char *startup_commands[] = {
    "Exit from the program",
    "Heads 1P2P",
    "Sides 1P2P",
    "Heads start",
    "Sides start",
    "Just as they are"
};

/* command_commands tracks the command_kind enumeration */

static char *command_commands[] = {
    "Exit the program",
    "Undo last call",
    IFMAC("End this sequence","Abort this sequence"),
    "Allow modifications",
    "Insert a comment ...",
    IFMAC("Save as ...","Change output file ..."),
    NOMAC "End this sequence ...",
    "Resolve ...",
    "Reconcile ...",
    IFMAC("Pick random call ...","Do anything ..."),
    IFMAC("Normalize setup ...","Nice setup ..."),
    NOMAC "Show neglected calls ...",
    IFMAC("Insert picture","Save picture")
};

/* resolve_commands tracks the resolve_command_kind enumeration */

static char *resolve_commands[] = {
    "abort the search",
    "find another",
    "go to next",
    "go to previous",
    "ACCEPT current choice",
    "raise reconcile point",
    "lower reconcile point"
};

/*
   Call MATCHER_INITIALIZE first to set up the concept menu.  The concepts are found
   in the external array concept_descriptor_table+general_concept_offset.
   The number of concepts in that list is general_concept_size.  For each
   i, the field concept_descriptor_table[i].name has the text that we
   should display for the user.  If SHOW_COMMANDS_LAST is true, then the
   commands will be displayed last when matching against a call menu;
   otherwise, they will be displayed first.
*/

extern void
matcher_initialize(long_boolean show_commands_last)
{
    int i;

    commands_last_option = show_commands_last;

    /* initialize our special empty call menu */
    call_menu_lists[call_list_empty] = empty_menu;
    number_of_calls[call_list_empty] = 0;

    /* fill in general concept menu */
    for (i=0; i<general_concept_size; i++)
        add_call_to_menu(&concept_menu_list, i, general_concept_size,
            concept_descriptor_table[i+general_concept_offset].name);

    concept_menu_len = general_concept_size;
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

    (*menu)[call_menu_index] = callname;
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
                 char *extension, show_function *sf, int show_verify)
{
    match_state ss;
    input_matcher *f;
    char *p;

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

    /* convert user input to lower case for easier comparison */
    p = user_input;
    while (*p) {*p = tolower(*p); p++;}

    if (which_commands == match_startup_commands)
        f = startup_matcher;
    else if (which_commands == match_resolve_commands)
        f = resolve_matcher;
    else if (which_commands == match_selectors)
        f = selector_matcher;
    else if ((which_commands >= 0) && (which_commands < NUM_CALL_LIST_KINDS)) {
        f = call_matcher;
        current_call_menu = which_commands;
    }
    else /* invalid parameter */
        return 0;

    (*f)(&ss);
    if (ss.match_count == 0 && extension)
        extension[0] = 0;
    if (mr) {
        *mr = ss.result;
        mr->space_ok = ss.space_ok;
    }
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
        search_menu(sp, command_commands, NUM_COMMAND_KINDS, ui_command_select);

    search_menu(sp,
        call_menu_lists[current_call_menu], number_of_calls[current_call_menu],
        ui_call_select);

    search_concept(sp);

    if (commands_last_option)
        search_menu(sp, command_commands, NUM_COMMAND_KINDS, ui_command_select);
}

static void
selector_matcher(match_state *sp)
{
    search_menu(sp, &selector_names[1], last_selector_kind, 0);
}

static void
search_menu(match_state *sp, char *menu[], int menu_length, uims_reply kind)
{
    int i;
    match_result result;

    result.valid = TRUE;
    result.exact = FALSE;
    result.kind = kind;
    result.who = -1;
    result.n = -1;
    for (i = 0; i < menu_length; i++) {
        result.index = i;
        match_pattern(sp, menu[i], &result);
    }
}

static void
search_concept(match_state *sp)
{
    int nrows;
    int rowoff;
    int kind;
    int col;
    int row;
    char *name;
    match_result result;

    result.valid = TRUE;
    result.exact = FALSE;
    result.kind = ui_special_concept;
    result.who = -1;
    result.n = -1;
    for (kind = 0; concept_offset_tables[kind]; kind++) {
        /* for each "submenu" */
        for (col = 0; ; col++) {
            /* for each "column" in the submenu */
            nrows = concept_size_tables[kind][col];
            if (nrows < 0)
                break;
            rowoff = concept_offset_tables[kind][col];
            for (row = 0; row < nrows; row++) {
                /* for each "row" in the column */
                name = concept_descriptor_table[rowoff+row].name;
                if (name[0] == '\0')
                    continue; /* empty slot in menu */
                result.index = (((col << 8) + row + 1) << 3) + kind;
                match_pattern(sp, name, &result);
            }
        }
    }
}

/*
 * Match_pattern tests the user input against a pattern (pattern)
 * that may contain wildcards (such as "<ANYONE>").  Pattern matching is
 * case-insensitive.  If the input is equivalent to a prefix of the pattern,
 * a match is recorded in the search state.  The user input should be in
 * lower case.
 */
 
static void
match_pattern(match_state *sp, char pattern[], match_result *result)
{
    if (pattern[0] != '*')
        match_suffix(sp, sp->full_input, pattern, sp->extension, result);
}

/*
 * Match_suffix continues the matching process for a suffix of the current
 * user input (User) and a suffix of the current pattern (Pat).  Patxp is
 * where the next character of the extension of the user input for the current
 * pattern is to be written.  The extension of
 * the current user input is in lower case to allow computation of the longest
 * common extension.
 */

static void
match_suffix(match_state *sp, char *user, char *pat, char *patxp, match_result *result)
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
            *patxp++ = tolower(*pat);
            match_suffix(sp, user, pat+1, patxp, result);
        }
        else {
            /* reached the end of the pattern */
            *patxp = '\0';
            record_a_match(sp, result);
        }
    }
    else if (*user == tolower(*pat)) {
        match_suffix(sp, user+1, pat+1, patxp, result);
    }
}

/*
 * Match_wildcard tests for and handles pattern suffixes that being with
 * a wildcard such as "<ANYONE>".  A wildcard is handled only if there is
 * room in the Result struct to store the associated value.
 */

static int
match_wildcard(match_state *sp, char *user, char *pat, char *patxp, match_result *result)
{
    char temp[200];
    char *suffix;
    int i;
    match_result new_result;

    if (sp->showing && (user==0)) {
        /* if we are just listing the matching commands, there
           is no point in expanding wildcards that are past the
           part that matches the user input */
        return FALSE;
    }

    if ((strncmp(pat, "<ANYONE>", 8)==0) && (result->who == -1)) {
        suffix = pat+8;
        new_result = *result;
        for (i=1; i<=last_selector_kind; ++i) {
            sprintf(temp, "%s%s", selector_names[i], suffix);
            new_result.who = i;
            match_suffix(sp, user, temp, patxp, &new_result);
        }
        return FALSE;
    }
    else if ((strncmp(pat, "<N>", 3)==0) && (result->n == -1)) {
        suffix = pat+3;
        new_result = *result;
        for (i=1; i<=5; ++i) {
            sprintf(temp, "%d%s", i, suffix);
            new_result.n = i;
            match_suffix(sp, user, temp, patxp, &new_result);
        }
        return FALSE;
    }
    else if ((strncmp(pat, "<N/4>", 5)==0) && (result->n == -1)) {
        suffix = pat+5;
        new_result = *result;
        for (i=1; i<=5; ++i) {
            sprintf(temp, "%d/4%s", i, suffix);
            new_result.n = i;
            match_suffix(sp, user, temp, patxp, &new_result);
        }
        return FALSE;
    }
    else return FALSE;
}

/*
 * Record a match.  Extension is how the current input would be extended to
 * match the current pattern, in lower case.  Result is the value of the
 * current pattern.
 */
    
static void
record_a_match(match_state *sp, match_result *result)
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
    if ((call->callflags & cflag__requires_selector) &&
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
        
        for (sel=1; sel<last_selector_kind; ++sel) {
            if (verify_call_with_selector(call, sel))
                return (TRUE);
        }
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

    bits0 = history[history_ptr+1].warnings.bits[0];
    bits1 = history[history_ptr+1].warnings.bits[1];
    old_history_ptr = history_ptr;
    save_parse_state();

    result = try_call_with_selector(call, sel);
    
    history_ptr = old_history_ptr;
    restore_parse_state();
    history[history_ptr+1].warnings.bits[0] = bits0;
    history[history_ptr+1].warnings.bits[1] = bits1;
    longjmp_ptr = &longjmp_buffer;    /* restore the global error handler */
    initializing_database = FALSE;
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
