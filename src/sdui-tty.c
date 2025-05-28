/* 
 * sdui-tty.c - SD TTY User Interface
 * Originally for Macintosh.  Unix version by gildea.
 * Time-stamp: <93/03/22 18:32:26 gildea>
 * Copyright (c) 1990,1991,1992,1993 Stephen Gildea, William B. Ackerman, and
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
 * Type Control-U to clear the line.
 *
 * Calls with a number or person designator must be typed with that
 * information.
 *
 * Uses the console interface defined in the Think-C 5.0 ANSI library.
 *
 * For use with version 30 of the Sd program.
 * Based on sdui-x11.c 1.10
 *
 *  The version of this file is as shown immediately below.  This
 *  string gets displayed at program startup, as the "ui" part of
 *  the complete version.
 */

static char *sdui_version = "1.6";

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
   uims_do_direction_popup
   uims_get_number_fields
   uims_do_modifier_popup
   uims_add_new_line
   uims_reduce_line_count
   uims_begin_search
   uims_begin_reconcile_history
   uims_end_reconcile_history
   uims_update_resolve_menu
   uims_terminate
   uims_database_tick_max
   uims_database_tick
   uims_database_tick_end
   uims_database_error
   uims_bad_argument
*/

#ifndef THINK_C
#define UNIX_STYLE
#endif

/* For "sprintf" and some IO stuff (fflush, printf, stdout) that we use
   during the "database tick" printing before the actual IO package is started.
   During normal operation, we don't do any IO at all in this file. */
#include <stdio.h>
/* For "strlen". */
#include <string.h>
/* For "isprint". */
#include <ctype.h>
/* For "atoi". */
#include <stdlib.h>

#include "sd.h"
#include "sdmatch.h"
#include "sdui-ttu.h"

#define DEL 0x7F

/*
 * The total version string looks something like
 * "1.4:db1.5:ui0.6tty"
 * We return the "0.6tty" part.
 */

Private char version_mem[12];

extern char *
uims_version_string(void)
{
    (void) sprintf(version_mem, "%stty", sdui_version);
    return version_mem;
}

/*
 * User Input functions
 */

/* not passed as args so refresh_input can be called from signal handler */
Private char user_input[200];
Private char *user_input_prompt;

void
refresh_input(void)
{
    user_input[0] = '\0';
    clear_line(); /* clear the current line */
    put_line(user_input_prompt);
}

Private void
get_string_input(char prompt[], char dest[])
{
    put_line(prompt);
    get_string(dest);
}


Private int
get_char_input(void)
{
    int c;
#ifdef THINK_C			/* Unix or DOS provides its own cursor */
    putchar('_'); /* a cursor */
#endif
    c = get_char();
#ifdef THINK_C
    putchar('\b'); putchar(' '); putchar('\b'); /* erase the cursor */
#endif
    return c;
}
  
/* This tells how many of the last lines currently on the screen contain
   the text that the main program thinks comprise the transcript.  That is,
   if we count this far up from the bottom of the screen (well, from the
   cursor, to be precise), we will get the line that the main program thinks
   is the first line of the transcript.  This variable will often be greater
   than the main program's belief of the transcript length, because we are
   fooling around with input lines (prompts, error messages, etc.) at the
   bottom of the screen.

   The main program typically updates the transcript in a "clean" way by
   calling "uims_reduce_line_count" to erase some number of lines at the
   end of the transcript, followed by a rewrite of new material.  We compare
   what "uims_reduce_line_count" tells us against the value of this variable
   to find out how many lines to actually erase.  That way, we erase the input
   lines and produce a truly clean transcript, on devices (like VT-100's)
   capable of doing so.  If the display device can't erase lines, the user
   will see the last few lines of the transcript overlapping from one
   command to the next.  This is the appropriate behavior for a printing
   terminal or computer emulation of same. */

Private int current_text_line;

/*
 * Throw away all but the first n lines of the text output.
 * n = 0 means to erase the entire buffer.
 */

Private void
text_output_trim(int n)
{
    if (current_text_line > n)
        erase_last_n(current_text_line-n);

    current_text_line = n;
}

/*
 * end of text output stuff
 */

Private char **concept_menu_list;

Private char *call_menu_prompts[NUM_CALL_LIST_KINDS];

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
#ifdef UNIX_STYLE
   ttu_process_command_line(argcp, argvp);
#else
   *argcp = ccommand(argvp); /* pop up window to get command line arguments */
#endif
}

/*
 * The main program calls this before any of the call menus are
 * created, that is, before any calls to uims_add_call_to_menu
 * or uims_finish_call_menu.
 */
 
extern void
uims_preinitialize(void)
{
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
    matcher_add_call_to_menu(cl, call_menu_index, name);
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
    call_menu_prompts[cl] = (char *) get_mem(50);  /* *** Too lazy to compute it. */

    if (cl == call_list_any)
        /* The menu name here is "(any setup)".  That may be a sensible
           name for a menu, but it isn't helpful as a prompt.  So we
           just use a vanilla prompt. */
        call_menu_prompts[cl] = "--> ";
    else
        (void) sprintf(call_menu_prompts[cl], "(%s)--> ", menu_name);
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
    call_menu_prompts[call_list_empty] = "--> ";   /* This prompt should never be used. */
    matcher_initialize(TRUE); /* want commands last in menu */
#if defined(UNIX_STYLE) && !defined(MSDOS)
    initialize_signal_handlers();
#endif
    resolver_is_unwieldy = TRUE;   /* Sorry about that. */
    ttu_initialize();
    current_text_line = 0;
}

Private void
pack_and_echo_character(int c)
{
    char *p = user_input;

    while (*p) p++;
    *p++ = c;
    *p = 0;
    put_char(c);
}

/* This tells how many more lines of matches (the stuff we print in response
   to a question mark) we can print before we have to say "--More--" to
   get permission from the user to continue.  If it goes negative, the
   user has given a negative reply to one of our queries, and so we don't
   print any more stuff. */
Private int match_counter;

/* This is what we reset the counter to whenever the user confirms.  That
   is, it is the number of lines we print per "screenful".  On a VT-100-like
   ("dumb") terminal, we will actually make it a screenful.  On a printing
   device or a workstation, we don't need to do the hold-screen stuff, because
   the device can handle output intelligently (on a printing device, it does
   this by letting us look at the paper that is spewing out on the floor;
   on a workstation we know that the window system provides real (infinite,
   of course) scrolling).  But on printing devices or workstations we still
   do the output in screenful-like blocks, because the user may not want
   to see an enormous amount of output. */
Private int match_lines;

Private void
start_matches(void)
{
   /*
    * Find the number of lines on the screen.
    * Used only for grouping "screenfuls" with "--more--" stuff
    * when showing long lists of choices after '?' is typed.
    * In other cases, we just assume the output mechanism can intelligently
    * handle what we send to it.
    */

    match_lines = get_lines_for_more();
    match_counter = match_lines-1; /* last line used for "--More--" prompt */
}

Private int
prompt_for_more_output(void)
{
    put_line("--More--");

    for (;;) {
        int c = get_char_input();
        clear_line();   /* Erase the "more" line; next item goes on that line. */

        switch (c) {
          case '\r':
          case '\n': match_counter = 1; /* show one more line */
                     return TRUE;  /* keep going */

          case '\b':
          case DEL:
          case 'q':
          case 'Q':  return FALSE; /* stop showing */

          case ' ':  return TRUE;  /* keep going */

          default:   put_line("Type Space to see more, Return for next line, Delete to stop:  --More--");
        }
    }
}

Private void
show_match(char *user_input, char *extension, Const match_result *mr)
{
    if (match_counter < 0) return;  /* Showing has been turned off. */

    if (match_counter <= 0) {
        match_counter = match_lines - 1;
        if (!prompt_for_more_output()) {
            match_counter = -1;   /* Turn it off. */
            return;
        }
    }
    match_counter--;
    put_line(user_input);
    put_line(extension);
    put_line("\n");
    current_text_line++;
}

/*
 * The searching functions indicate their results by setting the variables
 * last_match and exact_match.  Last_match indicates the last matching item
 * (if there is one or more matching item).  Exact_match is set if
 * an item matches the input string exactly.  Hopefully, if there is more than
 * one item that matches exactly, they are all equivalent.
 */

Private match_result user_match;

/* result is indicated in user_match */

Private void
get_user_input(char *prompt, int which)
{
    char extended_input[200];
    char *p;
    int c;
    int matches;
    int n;
    
    user_match.valid = FALSE;
    user_input_prompt = prompt;
    user_input[0] = '\0';
    put_line(user_input_prompt);

    for (;;) {
        /* At this point we always have the concatenation of "user_input_prompt"
           and "user_input" displayed on the current line. */

        c = get_char_input();

        if ((c == '\b') || (c == DEL)) {
            n = strlen(user_input);
            if (n > 0) {
                user_input[n-1] = '\0';
                rubout(); /* Update the display. */
            }
            continue;
        }
        else if (c == '?') {
            put_line ("\n");
            current_text_line++;
            start_matches();
            match_user_input(user_input, which, 0, 0, show_match, FALSE);
            put_line ("\n");     /* Write a blank line. */
            current_text_line++;
            put_line(user_input_prompt);   /* Redisplay the current line. */
            put_line(user_input);
            continue;
        }

        matches = match_user_input(user_input, which, &user_match, extended_input, (show_function) 0, FALSE);

        if (c == ' ') {
            /* extend only to one space, inclusive */
            p = extended_input;
            if (*p) {
                while (*p) {
                    if (*p != ' ')
                        pack_and_echo_character(*p++);
                    else
                        goto foobar;
                }
                continue;   /* Do *not* pack the character. */

                foobar: ;
                pack_and_echo_character(c);
            }
            else if (user_match.space_ok && matches > 1)
                pack_and_echo_character(c);
            else
                bell();
        }
        else if ((c == '\n') || (c == '\r')) {
            if ((matches == 1) || user_match.exact) {
                p = extended_input;
                while (*p)
                    pack_and_echo_character(*p++);

                put_line("\n");
                /* Include the input line in our count, so we will erase it
                   if we are trying to make the VT-100 screen look nice. */
                current_text_line++;
                return;
            }
            /* Tell how bad it is, then redisplay current line. */
	    if (matches > 0) {
                char tempstuff[200];

                (void) sprintf(tempstuff, "  (%d matches, type ? for list)\n", matches);
                put_line(tempstuff);
            }
            else {
                put_line("  (no matches)\n");
            }

            put_line(user_input_prompt);
            put_line(user_input);
            current_text_line++;   /* Count that line for erasure. */
        }
        else if (c == '\t' || c == '\033') {
            p = extended_input;
            if (*p) {
                while (*p)
                    pack_and_echo_character(*p++);
            }
            else
                bell();
        }
        else if (c == ('U'&'\037')) { /* C-u: kill line */
            user_input[0] = '\0';
            clear_line();           /* Clear the current line */
            put_line(user_input_prompt);    /* Redisplay the prompt. */
        }
        else if (isprint(c)) {
            pack_and_echo_character(c);
        }
        else {
            bell();
        }
    }
}

Private call_list_kind current_call_menu;

static char *modifications_prompts[] = {
    (char *) 0,
    "[simple modifications] ",
    "[all modifications] "};

extern uims_reply
uims_get_command(mode_kind mode, call_list_kind *call_menu)
{
    if (mode == mode_startup) {
         get_user_input("Enter startup command> ", (int) match_startup_commands);
         uims_menu_index = user_match.index;
    }
    else if (mode == mode_resolve) {
         get_user_input("Enter resolve command> ", (int) match_resolve_commands);
         uims_menu_index = user_match.index;
    }
    else {
        char prompt_buffer[200];
        char *prompt_ptr;

        check_menu:

        if (allowing_modifications)
            *call_menu = call_list_any;

        current_call_menu = *call_menu;
        prompt_ptr = prompt_buffer;

        /* Put any necessary special things into the prompt. */

        if (allowing_all_concepts) {
            if (allowing_modifications != 0)
                (void) sprintf(prompt_ptr, "[all concepts]%s%s", modifications_prompts[allowing_modifications], call_menu_prompts[current_call_menu]);
            else
                (void) sprintf(prompt_ptr, "[all concepts]%s", call_menu_prompts[current_call_menu]);
        }
        else {
            if (allowing_modifications != 0)
                (void) sprintf(prompt_ptr, "%s%s", modifications_prompts[allowing_modifications], call_menu_prompts[current_call_menu]);
            else
                prompt_ptr = call_menu_prompts[current_call_menu];
        }

        get_user_input(prompt_ptr, (int) current_call_menu);

        uims_menu_index = user_match.index;

        if (user_match.kind == ui_command_select && uims_menu_index >= NUM_COMMAND_KINDS) {
            if (uims_menu_index == NUM_COMMAND_KINDS+SPECIAL_COMMAND_ALLOW_MODS) {
                /* Increment "allowing_modifications" up to a maximum of 2. */
                if (allowing_modifications != 2) allowing_modifications++;
            }
            else {   /* Must be "allow all concepts". */
                allowing_all_concepts = !allowing_all_concepts;
            }
            goto check_menu;
        }
    }

    return user_match.kind;
}
                                                              

Private int
get_popup_string(char prompt[], char dest[])
{
    char buffer[200];

    (void) sprintf(buffer, "%s: ", prompt);
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
    char buffer[200];
    (void) sprintf(buffer, "Sequence output file is \"%s\".\n", outfile_string);

    put_line(buffer);
    current_text_line++;
    return get_popup_string("Enter new file name", dest);
}

extern int
uims_do_getout_popup(char dest[])
{
    put_line("Specify text label for sequence.\n");
    current_text_line++;
    return get_popup_string("Enter label", dest);
}

extern int
uims_do_neglect_popup(char dest[])
{
    put_line("Specify integer percentage of neglected calls.\n");
    current_text_line++;
    return get_popup_string("Enter percentage", dest);
}

Private int
confirm(char *question)
{
    int c;

    for (;;) {
        put_line(question);
        c = get_char_input();
        if ((c=='n') || (c=='N')) {
            put_line("no\n");
            current_text_line++;
            return POPUP_DECLINE;
        }
        if ((c=='y') || (c=='Y')) {
            put_line("yes\n");
            current_text_line++;
            return POPUP_ACCEPT;
        }
        put_char(c);
        put_line("\n");
        put_line("Answer y or n\n");
        current_text_line += 2;
        bell();
    }
}

extern int
uims_do_abort_popup(void)
{
    put_line("The current sequence will be aborted.\n");
    current_text_line++;
    return confirm("Do you really want to abort it? ");
}

extern int
uims_do_modifier_popup(char callname[], modify_popup_kind kind)
{
    char *line_format = "Internal error: unknown modifier kind.\n";
    char tempstuff[200];
    
    switch (kind) {
        case modify_popup_any:
            line_format = "The \"%s\" can be replaced.\n";
            break;
        case modify_popup_only_tag:
            line_format = "The \"%s\" can be replaced with a tagging call.\n";
            break;
        case modify_popup_only_scoot:
            line_format = "The \"%s\" can be replaced with a scoot/tag (chain thru) (and scatter).\n";
            break;
    }

    sprintf(tempstuff, line_format, callname);
    put_line(tempstuff);
    current_text_line++;
    return confirm("Do you want to replace it? ");
}

/*
 * UIMS_BEGIN_SEARCH is called at the beginning of each search mode
 * command (resolve, reconcile, nice setup, do anything).
 */

extern void
uims_begin_search(search_kind goal)
{
}

/*
 * UIMS_BEGIN_RECONCILE_HISTORY is called at the beginning of a reconcile,
 * after UIMS_BEGIN_SEARCH,
 * and whenever the current reconcile point changes.  CURRENTPOINT is the
 * current reconcile point and MAXPOINT is the maximum possible reconcile
 * point.  This call is followed by calls to UIMS_REDUCE_LINE_COUNT
 * and UIMS_ADD_NEW_LINE that
 * display the current sequence with the reconcile point indicated.  These
 * calls are followed by a call to UIMS_END_RECONCILE_HISTORY.
 * Return TRUE to cause sd to forget its cached history output; do this
 * if the reconcile history is written to a separate window.
 */

extern int
uims_begin_reconcile_history(int currentpoint, int maxpoint)
{
    return FALSE;
}

/*
 * Return TRUE to cause sd to forget its cached history output.
 */

extern int
uims_end_reconcile_history(void)
{
    put_line("\n");
    return FALSE;
}

extern void
uims_update_resolve_menu(search_kind goal, int cur, int max, resolver_display_state state)
{
    char title[MAX_TEXT_LINE_LENGTH];

    create_resolve_menu_title(goal, cur, max, state, title);
    put_line(title);
    put_line("\n");
    current_text_line++;
}

extern int
uims_do_selector_popup(void)
{
    int n;

    if (user_match.valid && (user_match.who > selector_uninitialized)) {
        n = (int) user_match.who;
        user_match.who = selector_uninitialized;
        return n;
    }
    else {
        /* We skip the zeroth selector, which is selector_uninitialized. */
        get_user_input("Enter who> ", (int) match_selectors);
        return user_match.index+1;
    }
}    

extern int
uims_do_direction_popup(void)
{
    int n;

    if (user_match.valid && (user_match.where > direction_uninitialized)) {
        n = (int) user_match.where;
        user_match.where = direction_uninitialized;
        return n;
    }
    else {
        /* We skip the zeroth direction, which is direction_uninitialized. */
        get_user_input("Enter direction> ", (int) match_directions);
        return user_match.index+1;
    }
}    


extern unsigned int uims_get_number_fields(int howmany)
{
   int i;
   char buffer[200];
   unsigned int number_list = 0;

   for (i=0 ; i<howmany ; i++) {
      unsigned int this_num;

      if (user_match.valid && (user_match.howmanynumbers >= 1)) {
         this_num = user_match.number_fields & 0xF;
         user_match.number_fields >>= 4;
         user_match.howmanynumbers--;
      }
      else {
         get_string_input("How many? ", buffer);
         this_num = atoi(buffer);
      }

      if (this_num == 0 || this_num > 8) return 0;    /* User gave bad answer. */
      number_list |= (this_num << (i*4));
   }

   return number_list;
}


/*
 * add a line to the text output area.
 * the_line does not have the trailing Newline in it and
 * is volatile, so we must copy it if we need it to stay around.
 */

extern void
uims_add_new_line(char the_line[])
{
    current_text_line++;
    put_line(the_line);
    put_line("\n");
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
    ttu_terminate();
}

/*
 * The following two functions allow the UI to put up a progress
 * indicator while the call database is being read (and processed).
 *
 * uims_database_tick_max is called before reading the database
 * with the number of ticks that will be sent.
 * uims_database_tick is called repeatedly with the number of new
 * ticks to add.
 */

Private int db_tick_max;
Private int db_tick_cur;	/* goes from 0 to db_tick_max */

#define TICK_STEPS 52
Private int tick_displayed;	/* goes from 0 to TICK_STEPS */

extern void
uims_database_tick_max(int n)
{
   db_tick_max = n;
   db_tick_cur = 0;
   printf("Sd: reading database...");
   fflush(stdout);
   tick_displayed = 0;
}

extern void
uims_database_tick(int n)
{
    int tick_new;

    db_tick_cur += n;
    tick_new = TICK_STEPS*db_tick_cur/db_tick_max;
    while (tick_displayed < tick_new) {
	printf(".");
	tick_displayed++;
    }
    fflush(stdout);
}

extern void
uims_database_tick_end(void)
{
    printf("done\n");
}

extern void
uims_database_error(char *message, char *call_name)
{
   print_line(message);
   if (call_name) {
      print_line("  While reading this call from the database:");
      print_line(call_name);
   }
}

extern void
uims_bad_argument(char *s1, char *s2, char *s3)
{
   if (s1) print_line(s1);
   if (s2) print_line(s2);
   if (s3) print_line(s3);
   print_line("Use the -help flag for help.");
   exit_program(1);
}
