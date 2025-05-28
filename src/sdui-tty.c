/* 
 * sdui-tty.c - SD TTY User Interface
 * Originally for Macintosh.  Unix version by gildea.
 * Time-stamp: <93/02/18 17:25:59 gildea>
 * Copyright (c) 1990,1991,1992 Stephen Gildea, William B. Ackerman, and
 *   Alan Snyder
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * By Alan Snyder, December 1992.
 *
 * This version of the Sd UI does completing reads instead of menus.
 * Type '?' to see possibilities.
 *    At the "More" prompt, type SPC to see the next page, DEL to stop.
 * Type SPC to complete the current word.
 * Type TAB to complete as much as possible.
 *
 * Calls with a number or person designator must be typed with that
 * information.
 *
 * Uses the console interface defined in the Think-C 5.0 ANSI library.
 *
 * For use with version 28 of the Sd program.
 * Based on sdui-x11.c 1.10
 *
 *  The version of this file is as shown immediately below.  This
 *  string gets displayed at program startup, as the "ui" part of
 *  the complete version.
 */

static char *sdui_version = "1.1";

/* This file defines the following functions:
   uims_process_command_line
   uims_version_string
   uims_preinitialize
   uims_add_call_to_menu
   uims_finish_call_menu
   uims_postinitialize
   uims_get_command
   uims_do_comment_popup
   uims_do_outfile_popup
   uims_do_getout_popup
   uims_do_abort_popup
   uims_do_neglect_popup
   uims_do_selector_popup
   uims_do_quantifier_popup
   uims_do_modifier_popup
   uims_do_concept_popup
   uims_add_new_line
   uims_reduce_line_count
   uims_update_resolve_menu
   uims_terminate
*/

/* This really wants to say "if NOT Macintosh" but I don't
   know how.  Alan?  */
#if defined(hpux) || defined (unix) || defined (__unix__) || defined(SYSV) || defined(SVR4) || defined(_AES_SOURCE) || defined(_SYS5_SOURCE) || defined(_XOPEN_SOURCE) || defined(sun)
#define UNIX_STYLE
#endif

#include "sd.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef UNIX_STYLE
#include <console.h>
#include "macstuff.h" /* support for Macintosh UI */
#else
#include "sdui-ttu.h"
#endif

typedef char *String;

#define DEL 0x7F

/*
 * The total version string looks something like
 * "1.4:db1.5:ui0.6tty"
 * We return the "0.6tty" part.
 */

static char version_mem[12];

extern char *
uims_version_string(void)
{
    (void) sprintf(version_mem, "%stty", sdui_version);
    return version_mem;
}

static int input_window_height = 32; /* number of lines */

/*
 * User Input functions
 *
 * These functions use the Think-C 5.0 console functions.
 */

static void
input_initialize()
{
    /*
     * Here we cause the default console window to be created. This console
     * window is used for echoing input and displaying input completions.
     */

    console_options.ncols = 44;                  /* number of columns */
    console_options.nrows = input_window_height; /* number of rows */
    console_options.left = 0;              /* position of left edge */
    console_options.top = 40;              /* position of top edge */
#ifndef UNIX_STYLE
    console_options.title = "\psd input";  /* \p creates a Pascal string */
#endif
    cshow(stdout);                         /* create the console window */
}

static void
clear_input()
{
    int x, y;
    cgetxy (&x, &y, stdout);
    cgotoxy (1, y, stdout);
    ccleol(stdout); /* clear the current line */
}

static void
refresh_input(String prompt, char user_input[])
{
    clear_input();
    printf("%s>%s", prompt, user_input);
}

static void
get_string_input(char prompt[], char dest[])
{
    csetmode(C_ECHO, stdin); /* normal input mode: echo, line editing */
    printf("%s", prompt);
    gets(dest);
    csetmode(C_RAW, stdin); /* raw input mode: no echo, does not block */
}

#ifdef MSDOS
#undef getchar
static int getchar(void)
{
    int n;

    fflush(stdout);
    n = getch();
    if (n > 127) {
	if (n == 339) /* Delete */
	    n = '\177';
	else
	    n = ' ';
    }
    return n;
}
#endif /* MSDOS */

static int
get_char_input(void)
{
    int c;
    csetmode(C_RAW, stdin); /* raw input mode, no echo, does not block */
    putchar('_'); /* a cursor */
    c = getchar();
    while (c == EOF) c = getchar(); /* busy wait (EOF means no character yet) */
    putchar('\b'); putchar(' '); putchar('\b'); /* erase the cursor */
    return c;
}

/*
 * Restore the normal input mode, if necessary.
 */

static void
restore_input_mode(void)
{
    csetmode(C_ECHO, stdin); /* normal input mode: echo, line editing */
}

/*
 * end of user input stuff
 */

/*
 * Text Output Support
 */
  
static FILE *output = 0;
static String current_text[300];
static int current_text_line;
static char text_output_buffer[10000];
static String text_output_pointer;
static int text_output_height;

/*
 * add a line to the text output area.
 * the_line does not have the trailing Newline in it and
 * is volitile, so we must copy it if we need it to stay around.
 */

static void
text_output_add_line(char the_line[])
{
    int len;

    if (output == 0) {
        /* create a second console window for text output */
        text_output_height = input_window_height;
        console_options.ncols = 59;                  /* number of columns */
        console_options.nrows = text_output_height;  /* number of rows */
        console_options.left = 274;                  /* position of left edge */
        console_options.top = 40;                    /* position of top edge */
#ifndef UNIX_STYLE
        console_options.title = "\psd output";  /* \p creates a Pascal string */
#endif
        output = fopenc();                      /* open the console window */
        current_text_line = 0;
        text_output_pointer = text_output_buffer;
        current_text[current_text_line] = text_output_pointer;
        *text_output_pointer = '\0';
    }
    len = strlen(the_line);
    strncpy(text_output_pointer, the_line, len);
    text_output_pointer += len;
    *text_output_pointer++ = '\n';
    ++current_text_line;
    current_text[current_text_line] = text_output_pointer;
    *text_output_pointer = '\0';
    fprintf(output, "%s\n", the_line);
}

/*
 * Throw away all but the first n lines of the text output.
 * n = 0 means to erase the entire buffer.
 */

static void
text_output_trim(int n)
{
    if (output) {
        current_text_line = n;
        text_output_pointer = current_text[current_text_line];
        *text_output_pointer = '\0';
        cgotoxy (1, 1, output);
        ccleos(output);
        if (current_text_line > text_output_height) {
            fprintf(output, "%s",
                current_text[current_text_line - text_output_height]);
        }
        else {
            fprintf(output, "%s", text_output_buffer);
        }
    }
}

static void
text_output_close(void)
{
    if (output) {
        fclose(output);
        output = 0;
    }
}

/*
 * end of text output stuff
 */

static String *concept_menu_list;
static int concept_menu_len;

static String empty_menu[] = {NULL};

static String *call_menu_lists[NUM_CALL_LIST_KINDS];
static String call_menu_names[NUM_CALL_LIST_KINDS];

/* the following arrays must be coordinated with the sd program */

/* startup_commands tracks the start_select_kind enumeration */
static String startup_commands[] = {
    "Exit from the program",
    "Heads 1P2P",
    "Sides 1P2P",
    "Heads start",
    "Sides start",
    "Just as they are"
};

/* command_commands tracks the command_kind enumeration */
static String command_commands[] = {
    "Exit the program",
    "Undo last call",
    "Abort this sequence",
    "Allow modifications",
    "Insert a comment ...",
    "Change output file ...",
    "End this sequence ...",
    "Resolve ...",
    "Reconcile ...",
    "Do anything ...",
    "Nice setup ...",
    "Show neglected calls ...",
    "Save picture"
};

/* resolve_commands tracks the resolve_command_kind enumeration */
static String resolve_commands[] = {
    "abort the search",
    "find another",
    "go to next",
    "go to previous",
    "ACCEPT current choice",
    "raise reconcile point",
    "lower reconcile point"
};

/*
 * The main program calls this before doing anything else, so we can
 * supply additional command line arguments.
 * Note: If we are writing a call list, the program will
 * exit before doing anything else with the user interface, but this
 * must be made anyway.
 */
 
extern void
uims_process_command_line(int *argcp, char ***argvp)
{
   input_initialize();
#ifndef UNIX_STYLE
   *argcp = getcommand(argvp); /* pop up window to get command line arguments */
#endif
   printf("Reading database...\n");
}

/*
 * The main program calls this before any of the call menus are
 * created, that is, before any calls to uims_add_call_to_menu
 * or uims_finish_call_menu.
 */
 
extern void
uims_preinitialize(void)
{
    ;
}

static void
add_call_to_menu(String **menu, int call_menu_index, int menu_size,
    char callname[])
{
    if (call_menu_index == 0) {
        /* first item in this menu; set it up */
        *menu = (String *)get_mem((unsigned)(menu_size+1) * sizeof(String *));
    }

    (*menu)[call_menu_index] = callname;
}

/*
 * We have been given the name of one call (call number
 * call_menu_index, from 0 to number_of_calls[cl]) to be added to the
 * call menu cl (enumerated over the type call_list_kind.)
 * The string is guaranteed to be in stable storage.
 */
 
extern void
uims_add_call_to_menu(call_list_kind cl, int call_menu_index, char name[])
{
    int menu_num = (int) cl;

    add_call_to_menu(&call_menu_lists[menu_num], call_menu_index,
             number_of_calls[menu_num], name);
}


/*
 * Create a menu containing number_of_calls[cl] items, which are the
 * items whose text strings were previously transmitted by the calls
 * to uims_add_call_to_menu.  Use the "menu_name" argument to create a
 * title line for the menu.  The string is in static storage.
 * 
 * This will be called once for each value in the enumeration call_list_kind.
 */

extern void
uims_finish_call_menu(call_list_kind cl, char menu_name[])
{
   call_menu_names[cl] = menu_name;
 
}

/* The main program calls this after all the call menus have been created,
   after all calls to uims_add_call_to_menu and uims_finish_call_menu.
   This performs any final initialization required by the interface package.

   It must also perform any required setup of the concept menu.  The
   concepts are not presented procedurally.  Instead, they can be found
   in the external array concept_descriptor_table+general_concept_offset.
   The number of concepts in that list is general_concept_size.  For each
   i, the field concept_descriptor_table[i].name has the text that we
   should display for the user.
*/

extern void
uims_postinitialize(void)
{
    int i;

    /* initialize our special empty call menu */
    call_menu_lists[call_list_empty] = empty_menu;
    number_of_calls[call_list_empty] = 0;
    call_menu_names[call_list_empty] = "";

    /* fill in general concept menu */
    for (i=0; i<general_concept_size; i++)
        add_call_to_menu(&concept_menu_list, i, general_concept_size,
            concept_descriptor_table[i+general_concept_offset].name);

    concept_menu_len = general_concept_size;
}

static void
add_character(char user_input[], int c)
{
    char *p;
    p = user_input;
    while (*p) p++;
    *p++ = c;
    *p = 0;
}

static void
strn_gcp(char *s1, char *s2)
{
    /* set S1 to be the greatest common prefix of S1 and S2 */
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

static int match_counter;
static int match_lines;

typedef struct {
    int valid;       /* set to TRUE if a match was found */
    int exact;       /* set to TRUE if an exact match was found */
    uims_reply kind;
    int index;
    int who;         /* matches <anyone> */
    int n;           /* matches <n> */
} match_result;

typedef struct {
    String full_input;     /* the current user input */
    String extended_input; /* the maximal common extension to the user input */
    int match_count;       /* the number of matches so far */
    int exact_match;       /* true if an exact match has been found */
    int showing;           /* we are only showing the matching patterns */
    match_result result;   /* value of the first or exact matching pattern */
} search_state;

static void
start_matches()
{
    match_lines = input_window_height;
    match_counter = match_lines-1; /* last line used for "--More--" prompt */
}

static int
prompt_for_more_output(int *pcounter)
{
    int c;
    for (;;) {
        printf("--More--");
        c = get_char_input();
        clear_input();
        switch (c) {
          case '\r':
          case '\n': *pcounter = 1; /* show one more line */
                     return TRUE; /* keep going */

          case '\b':
          case DEL:
          case 'q':
          case 'Q':  return FALSE; /* stop showing */

          case ' ':  return TRUE; /* keep going */

          default:   puts("Type Space to see more, Delete to stop.");
        }
    }
}

static void
show_match(search_state *sp, String s)
{
    if (sp->showing) {
        if (match_counter <= 0) {
            match_counter = match_lines - 1;
            sp->showing = prompt_for_more_output(&match_counter);
        }
        match_counter--;
        if (sp->showing) printf("%s%s\n", sp->full_input, s);
    }
}

/*
 * The searching functions indicate their results by setting the variables
 * last_match and exact_match.  Last_match indicates the last matching item
 * (if there is one or more matching item).  Exact_match is set if
 * an item matches the input string exactly.  Hopefully, if there is more than
 * one item that matches exactly, they are all equivalent.
 */

static int save_special_index;
    /* save concept menu index for uims_do_concept_popup */

/*
 * Record a match.  Extension is how the current input would be extended to
 * match the current pattern, in lower case.  Result is the value of the
 * current pattern.
 */
    
static void
record_a_match(search_state *sp, char extension[], match_result result)
{
    if (sp->match_count == 0)
        /* this is the first match */
        strcpy(sp->extended_input, extension);
    else
        strn_gcp(sp->extended_input, extension);
    if ((sp->match_count == 0) || (*extension == '\0'))
        /* first match or an exact match */
        sp->result = result;
    if (*extension == '\0')
        sp->result.exact = TRUE;
    ++sp->match_count;
    show_match(sp, extension);
}

static void
match_suffix(search_state *sp, String user, String pat,
    String patx, String patxp, match_result result);

/*
 * Match_wildcard tests for and handles pattern suffixes that being with
 * a wildcard such as "<ANYONE>".  A wildcard is handled only if there is
 * room in the Result struct to store the associated value.
 */

static int
match_wildcard(search_state *sp, String user, String pat,
    String patx, String patxp, match_result result)
{
    char temp[200];
    String suffix;
    int i;

    if (sp->showing && (user==0)) {
        /* if we are just listing the matching commands, there
           is no point in expanding wildcards that are past the
           part that matches the user input */
        return FALSE;
    }

    if ((strncmp(pat, "<ANYONE>", 8)==0) && (result.who == -1)) {
        suffix = pat+8;
        for (i=1; i<=last_selector_kind; ++i) {
            sprintf(temp, "%s%s", selector_names[i], suffix);
            result.who = i;
            match_suffix(sp, user, temp, patx, patxp, result);
        }
        return TRUE;
    }
    else if ((strncmp(pat, "<N>", 3)==0) && (result.n == -1)) {
        suffix = pat+3;
        for (i=1; i<=5; ++i) {
            sprintf(temp, "%d%s", i, suffix);
            result.n = i;
            match_suffix(sp, user, temp, patx, patxp, result);
        }
        return TRUE;
    }
    else if ((strncmp(pat, "<N/4>", 5)==0) && (result.n == -1)) {
        suffix = pat+5;
        for (i=1; i<=5; ++i) {
            sprintf(temp, "%d/4%s", i, suffix);
            result.n = i;
            match_suffix(sp, user, temp, patx, patxp, result);
        }
        return TRUE;
    }
    else return FALSE;
}

/*
 * Match_suffix continues the matching process for a suffix of the current
 * user input (User) and a suffix of the current pattern (Pat).  Patx is where
 * the extension of the user input for this pattern is being written.  Patxp is
 * where the next character of the extension is to be written.  The extension of
 * the current user input is in lower case to allow computation of the longest
 * common extension.
 */

static void
match_suffix(search_state *sp, String user, String pat,
    String patx, String patxp, match_result result)
{
    if (user && (*user == '\0')) {
        /* we have just reached the end of the user input */
        if (*pat == '\0') {
            /* exact match */
            *patxp = '\0';
            record_a_match(sp, patx, result);
        }
        else {
            /* we need to look at the rest of the pattern because
               if it contains wildcards, then there are multiple matches */
            match_suffix(sp, 0, pat, patx, patxp, result);
        }
    }
    else if ((*pat == '<') &&
             match_wildcard(sp, user, pat, patx, patxp, result)) {
        ; /* everything done by match_wildcard */
    }    
    else if (user==0) {
        /* user input has run out, just looking for more wildcards */
        if (*pat) {
            /* there is more pattern */
            *patxp++ = tolower(*pat);
            match_suffix(sp, user, pat+1, patx, patxp, result);
        }
        else {
            /* reached the end of the pattern */
            *patxp = '\0';
            record_a_match(sp, patx, result);
        }
    }
    else if (tolower(*user) == tolower(*pat)) {
        match_suffix(sp, user+1, pat+1, patx, patxp, result);
    }
}

/*
 * Match_pattern tests the current user input against a pattern (pattern)
 * that may contain wildcards (such as "<ANYONE>").  Pattern matching is
 * case-insensitive.  If the input is equivalent to a prefix of the pattern,
 * a match is recorded in the search state.
 */
 
static void
match_pattern(search_state *sp, char pattern[], match_result result)
{
    char extension[200];
    match_suffix(sp, sp->full_input, pattern, extension, extension, result);
}

static void
search_menu(search_state *sp, String menu[], int menu_length, uims_reply kind)
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
        match_pattern(sp, menu[i], result);
    }
}

static void
search_concept(search_state *sp)
{
    int maxcolumn;
    int nrows;
    int rowoff;
    int kind;
    int col;
    int row;
    String name;
    match_result result;

    result.valid = TRUE;
    result.exact = FALSE;
    result.kind = ui_special_concept;
    result.who = -1;
    result.n = -1;
    for (kind = 0; concept_offset_tables[kind]; kind++) {
        /* for each "submenu" */
        /* determine menu size */
        for (maxcolumn=0; concept_size_tables[kind][maxcolumn]>=0; maxcolumn++)
            ;
        for (col = 0; col < maxcolumn; col++) {
            /* for each "column" in the submenu */
            nrows = concept_size_tables[kind][col];
            rowoff = concept_offset_tables[kind][col];
            for (row = 0; row < nrows; row++) {
                /* for each "row" in the column */
                name = concept_descriptor_table[rowoff+row].name;
                if (name[0] == '\0') continue; /* empty slot in menu */
                result.index = (((col << 8) + row + 1) << 3) + kind;
                match_pattern(sp, name, result);
            }
        }
    }
}

typedef void input_matcher(search_state *sp);

static match_result user_match;

/* return number of matching commands */
/* extend string in USER_INPUT */
/* result is indicated in user_match */

static int
extend_user_input(char user_input[], input_matcher *f)
{
    search_state ss;
    char extended_input[200];

    ss.full_input = user_input;
    ss.extended_input = extended_input;
    ss.match_count = 0;
    ss.exact_match = FALSE;
    ss.showing = FALSE;
    ss.result.valid = FALSE;
    ss.result.exact = FALSE;

    (*f)(&ss);
    if (ss.match_count > 0)
        (void) strcat(user_input, extended_input);
    user_match = ss.result;
    return ss.match_count;
}

static int
extend_user_input_to_space(char user_input[], input_matcher *f)
{
    search_state ss;
    char extended_input[200];
    String p;

    ss.full_input = user_input;
    ss.extended_input = extended_input;
    ss.match_count = 0;
    ss.exact_match = FALSE;
    ss.showing = FALSE;
    ss.result.valid = FALSE;
    ss.result.exact = FALSE;

    (*f)(&ss);
    if (ss.match_count > 0) {
        p = user_input + strlen(user_input);
        (void) strcat(user_input, extended_input);
        while (*p) {
            if (*p == ' ') {
                p[1] = '\0';
                break;
            }
            p++;
        }
    }
    user_match = ss.result;
    return ss.match_count;
}

static void
show_matching_commands(char user_input[], input_matcher *f)
{
    search_state ss;
    char extended_input[200];

    ss.full_input = user_input;
    ss.extended_input = extended_input;
    ss.match_count = 0;
    ss.exact_match = FALSE;
    ss.showing = TRUE;
    ss.result.valid = FALSE;
    ss.result.exact = FALSE;

    (*f)(&ss);
}
    
/* result is indicated in user_match */

static void
get_user_input(String prompt, input_matcher *f)
{
    char user_input[200];
    int c;
    int matches;
    int n;
    
    user_input[0] = 0;
    user_match.valid = FALSE;
    for (;;) {
        refresh_input(prompt, user_input);
        c = get_char_input();
        if ((c == '\b') || (c == DEL)) {
            n = strlen(user_input);
            if (n > 0) user_input[n-1] = 0;
        }
        else if ((c == '\n') || (c == '\r')) {
            matches = extend_user_input(user_input, f);
            if ((matches == 1) || user_match.exact) {
                refresh_input(prompt, user_input);
                printf("\n");
                return;
            }
            printf(" %d matches\n", matches);
        }
        else if (c == '\t') {
            n = strlen(user_input);
            extend_user_input(user_input, f);
            if (strlen(user_input) == n)
                putchar(7); /* ring the bell */
        }
        else if (c == ' ') {
            n = strlen(user_input);
            extend_user_input_to_space(user_input, f);
            if (strlen(user_input) == n) {
                add_character(user_input, ' ');
                if (extend_user_input(user_input, f) > 0)
                    user_input[n+1] = '\0'; /* append the space */
                else {
                    user_input[n] = '\0'; /* do not append the space */
                    putchar(7); /* ring the bell */
                }
            }
        }
        else if (c == '?') {
            printf ("\n");
            start_matches();
            show_matching_commands(user_input, f);
        }
        else if (isprint(c)) {
            add_character(user_input, c);
        }
        else {
            putchar(7); /* ring the bell */
        }
    }
}

static void
start_matcher(search_state *sp)
{
    search_menu(sp, startup_commands, NUM_START_SELECT_KINDS, ui_start_select);
}

static void
resolve_matcher(search_state *sp)
{
    search_menu(sp, resolve_commands,
        NUM_RESOLVE_COMMAND_KINDS, ui_resolve_select);
}

static call_list_kind current_call_menu;

static void
call_matcher(search_state *sp)
{
    search_menu(sp, concept_menu_list, concept_menu_len, ui_concept_select);

    search_menu(sp, command_commands, NUM_COMMAND_KINDS, ui_command_select);

    search_menu(sp,
        call_menu_lists[current_call_menu], number_of_calls[current_call_menu],
        ui_call_select);

    search_concept(sp);
}

static void
selector_matcher(search_state *sp)
{
    search_menu(sp, &selector_names[1], last_selector_kind, 0);
}

extern uims_reply
uims_get_command(mode_kind mode, call_list_kind call_menu,
    int modifications_flag)
{
    if (mode == mode_startup) {
         get_user_input("Enter command", &start_matcher);
         uims_menu_index = user_match.index;
         return user_match.kind;
    }
    else if (mode == mode_resolve) {
         get_user_input("Enter resolve command", &resolve_matcher);
         uims_menu_index = user_match.index;
         return user_match.kind;
    }
    else {
        current_call_menu = call_menu;
        printf("%s\n", call_menu_names[current_call_menu]);
        get_user_input("Enter call", &call_matcher);
        if (user_match.kind == ui_special_concept) {
            /* right now, just indicate which submenu */
            /* uims_do_concept_popup will be called to get the rest */
            save_special_index = user_match.index >> 3;
            user_match.index &= 07;
        }
        uims_menu_index = user_match.index;
        return user_match.kind;
    }
}

static int
get_popup_string(char prompt[], char dest[])
{
    char buffer[200];

    sprintf(buffer, "%s: ", prompt);
    get_string_input(buffer, dest);
    return POPUP_ACCEPT_WITH_STRING;
}

extern int
uims_do_comment_popup(char dest[])
{
    return get_popup_string("Enter comment", dest);
}

extern int
uims_do_outfile_popup(char dest[])
{
    puts("Change name of sequence output file.");
    return get_popup_string("Enter file name", dest);
}

extern int
uims_do_getout_popup(char dest[])
{
    puts("Specify text label for sequence.");
    return get_popup_string("Enter label", dest);
}

extern int
uims_do_neglect_popup(char dest[])
{
    puts("Specify integer percentage of neglected calls.");
    return get_popup_string("Enter percentage", dest);
}

static int
confirm(String question)
{
    int c;
    
    for (;;) {
        printf("%s ", question);
        c = get_char_input();
        if ((c=='n') || (c=='N')) {
            puts("no");
            return POPUP_DECLINE;
        }
        if ((c=='y') || (c=='Y')) {
            puts("yes");
            return POPUP_ACCEPT;
        }
        printf("%c\nAnswer y or n\n", c);
        putchar(7); /* ring bell */
    }
}

extern int
uims_do_abort_popup(void)
{
    puts("The current sequence will be aborted.");
    return confirm("Do you really want to abort it?");
}

extern int
uims_do_modifier_popup(char callname[], modify_popup_kind kind)
{
   char *line_format = "Internal error: unknown modifier kind.";

   switch (kind) {
     case modify_popup_any:
       line_format = "The \"%s\" can be replaced.";
       break;
     case modify_popup_only_tag:
       line_format = "The \"%s\" can be replaced with a tagging call.";
       break;
     case modify_popup_only_scoot:
       line_format = "The \"%s\" can be replaced with a scoot/tag (chain thru) (and scatter).";
       break;
   }
   printf(line_format, callname);
   putchar('\n');
   return confirm("Do you want to replace it?");
}

extern void
uims_update_resolve_menu(char *title)
{
    printf("%s\n", title);   
}

extern int
uims_do_selector_popup(void)
{
    int n;

    if (user_match.valid && (user_match.who >= 0)) {
        n = user_match.who;
        user_match.who = -1;
        return n;
    }
    else {
        /* We skip the zeroth selector, which is selector_uninitialized. */
        get_user_input("Enter who", selector_matcher);
        return user_match.index+1;
    }
}    

extern int
uims_do_quantifier_popup(void)
{
    int n;
    char buffer[200];

    if (user_match.valid && (user_match.n >= 1)) {
        n = user_match.n;
        user_match.n = -1;
        return n;
    }
    else {
        get_string_input("How many? ", buffer);
        n = atoi(buffer);
        return n;
    }
}

/* ARGSUSED */
extern int
uims_do_concept_popup(int kind)
{
    return save_special_index;
}

/*
 * add a line to the text output area.
 * the_line does not have the trailing Newline in it and
 * is volitile, so we must copy it if we need it to stay around.
 */

extern void
uims_add_new_line(char the_line[])
{
    text_output_add_line(the_line);
}

/*
 * Throw away all but the first n lines of the text output.
 * n = 0 means to erase the entire buffer.
 */

extern void
uims_reduce_line_count(int n)
{
    text_output_trim(n);
}

extern void
uims_terminate(void)
{
    text_output_close();
    restore_input_mode();
}
