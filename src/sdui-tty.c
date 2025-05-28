/* 
 * sdui-tty.c - SD TTY User Interface
 * Originally for Macintosh.  Unix version by gildea.
 * Time-stamp: <96/05/22 17:17:53 wba>
 * Copyright (c) 1990-1994 Stephen Gildea, William B. Ackerman, and
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
 * For use with version 31 of the Sd program.
 * Based on sdui-x11.c 1.10
 *
 * The version of this file is as shown immediately below.  This string
 * gets displayed at program startup, as the "ui" part of the complete
 * version.
 */

#define UI_VERSION_STRING "1.9"

/* See comments in sdmain.c regarding this string. */
static char *id="@(#)$He" "ader: Sd: sdui-tty.c "
   UI_VERSION_STRING
   "  wba@apollo.hp.com  16 Jun 96 $";

/* This file defines the following functions:
   uims_version_string
   uims_process_command_line
   uims_display_help
   uims_display_ui_intro_text
   uims_preinitialize
   uims_create_menu
   uims_postinitialize
   show_match
   uims_get_startup_command
   uims_get_call_command
   uims_get_resolve_command
   uims_do_comment_popup
   uims_do_outfile_popup
   uims_do_header_popup
   uims_do_getout_popup
   uims_do_abort_popup
   uims_do_neglect_popup
   uims_do_selector_popup
   uims_do_direction_popup
   uims_do_tagger_popup
   uims_do_circcer_popup
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

and the following data that are used by sdmatch.c :

   twice_concept_ptr
   two_calls_concept_ptr
   num_command_commands
   command_commands
   number_of_resolve_commands
   resolve_command_strings
   num_extra_resolve_commands
   extra_resolve_commands
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


static Const call_conc_option_state null_options = {
   selector_uninitialized,
   direction_uninitialized,
   0,
   0,
   0,
   0};

#include "sdui-ttu.h"

#define DEL 0x7F

/*
 * The total version string looks something like
 * "1.4:db1.5:ui0.6tty"
 * We return the "0.6tty" part.
 */

Private char version_mem[12];

extern char *uims_version_string(void)
{
    (void) sprintf(version_mem, "%stty", UI_VERSION_STRING);
    return version_mem;
}

/*
 * User Input functions
 */

/* This array is the same as static_ss.full_input, but has the original capitalization
   as typed by the user.  Static_ss.full_input is converted to all lower case for
   ease of searching. */
Private char user_input[INPUT_TEXTLINE_SIZE+1];
Private char *user_input_prompt;
Private char *function_key_expansion;

void
refresh_input(void)
{
   user_input[0] = '\0';
   static_ss.full_input[0] = '\0';
   static_ss.full_input_size = 0;
   function_key_expansion = (char *) 0;
   clear_line(); /* clear the current line */
   put_line(user_input_prompt);
}

Private void
get_string_input(char prompt[], char dest[])
{
   put_line(prompt);
   get_string(dest);
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

static int current_text_line;

/*
 * Throw away all but the first n lines of the text output.
 * n = 0 means to erase the entire buffer.
 */

Private void text_output_trim(int n)
{
    if (current_text_line > n)
        erase_last_n(current_text_line-n);

    current_text_line = n;
}

/*
 * end of text output stuff
 */

static char *call_menu_prompts[NUM_CALL_LIST_KINDS];

/*
 * The main program calls this before doing anything else, so we can
 * supply additional command line arguments.
 * Note: If we are writing a call list, the program will
 * exit before doing anything else with the user interface, but this
 * must be made anyway.
 */
 
extern void uims_process_command_line(int *argcp, char ***argvp)
{
#ifdef UNIX_STYLE
   ttu_process_command_line(argcp, *argvp);
#else
   *argcp = ccommand(argvp); /* pop up window to get command line arguments */
#endif
}

extern void uims_display_help(void)
{
#ifdef UNIX_STYLE
   ttu_display_help();
#else
   printf("\nIn addition, the usual X window system flags are supported.\n");
#endif
}

extern void uims_display_ui_intro_text(void)
{
   writestuff("At any time that you don't know what you can type,");
   newline();
   writestuff("type a question mark (?).  The program will show you all");
   newline();
   writestuff("legal choices.");
   newline();
   newline();
}

/*
 * The main program calls this before any of the call menus are
 * created, that is, before any calls to uims_create_menu.
 */
 
extern void uims_preinitialize(void)
{
    current_text_line = 0;
    ttu_initialize();
}

/*
 * Create a menu containing number_of_calls[cl] items.
 * Use the "menu_names" array to create a
 * title line for the menu.  The string is in static storage.
 * 
 * This will be called once for each value in the enumeration call_list_kind.
 */

extern void uims_create_menu(call_list_kind cl, callspec_block *call_name_list[])
{
   call_menu_prompts[cl] = (char *) get_mem(50);  /* *** Too lazy to compute it. */
   matcher_setup_call_menu(cl, call_name_list);

   if (cl == call_list_any)
      /* The menu name here is "(any setup)".  That may be a sensible
         name for a menu, but it isn't helpful as a prompt.  So we
         just use a vanilla prompt. */
      call_menu_prompts[cl] = "--> ";
   else
      (void) sprintf(call_menu_prompts[cl], "(%s)--> ", menu_names[cl]);
}


/* The main program calls this after all the call menus have been created,
   after all calls to uims_create_menu.
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
    matcher_initialize();
#if defined(UNIX_STYLE) && !defined(MSDOS)
    initialize_signal_handlers();
#endif
}

Private void
pack_and_echo_character(char c)
{
   /* Really should handle error better -- ring the bell,
      but this is called inside a loop. */

   if (static_ss.full_input_size < INPUT_TEXTLINE_SIZE) {
      user_input[static_ss.full_input_size] = c;
      static_ss.full_input[static_ss.full_input_size++] = tolower(c);
      user_input[static_ss.full_input_size] = '\0';
      static_ss.full_input[static_ss.full_input_size] = '\0';
      put_char(c);
   }
}

/* This tells how many more lines of matches (the stuff we print in response
   to a question mark) we can print before we have to say "--More--" to
   get permission from the user to continue.  If verify_has_stopped goes on, the
   user has given a negative reply to one of our queries, and so we don't
   print any more stuff. */
Private int match_counter;
long_boolean verify_has_stopped;

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
   verify_has_stopped = FALSE;
}

Private int
prompt_for_more_output(void)
{
    put_line("--More--");

    for (;;) {
        char c = get_char();
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

extern void show_match(void)
{
   if (verify_has_stopped) return;  /* Showing has been turned off. */

   if (match_counter <= 0) {
      match_counter = match_lines - 1;
      if (!prompt_for_more_output()) {
         match_counter = -1;   /* Turn it off. */
         verify_has_stopped = TRUE;
         return;
      }
   }
   match_counter--;
   put_line(static_ss.full_input);
   put_line(static_ss.extension);
   put_line("\n");
   current_text_line++;
}


concept_descriptor *twice_concept_ptr = (concept_descriptor *) 0;
concept_descriptor *two_calls_concept_ptr = (concept_descriptor *) 0;


/* BEWARE!!  These two lists must stay in step. */

#ifdef SINGERS
int num_command_commands = 51;          /* The number of items in the tables, independent of NUM_COMMAND_KINDS. */
#else
int num_command_commands = 49;
#endif


Cstring command_commands[] = {
   "exit the program",
   "quit the program",
   "simple modifications",
   "allow modifications",
   "toggle concept levels",
   "toggle active phantoms",
   "toggle ignoreblanks",
   "toggle retain after error",
#ifdef SINGERS
   "toggle singing call",
   "toggle singing call with backward progression",
#endif
   "undo last call",
   "discard entered concepts",
   "abort this sequence",
   "insert a comment",
   "change output file",
   "change title",
   "write this sequence",
   "end this sequence",
   "keep picture",
   "refresh display",
   "resolve",
   "normalize",
   "standardize",
   "reconcile",
   "pick random call",
   "pick simple call",
   "pick concept call",
   "pick level call",
   "pick 8 person level call",
   "create any lines",
   "create waves",
   "create 2fl",
   "create lines in",
   "create lines out",
   "create inverted lines",
   "create 3x1 lines",
   "create any columns",
   "create columns",
   "create magic columns",
   "create dpt",
   "create cdpt",
   "create trade by",
   "create 8 chain",
   "create any 1/4 tag",
   "create 1/4 tag",
   "create 3/4 tag",
   "create 1/4 line",
   "create 3/4 line",
   "create diamonds",
   "create any tidal setup",
   "create tidal wave"
};

static command_kind command_command_values[] = {
   command_quit,
   command_quit,
   command_simple_mods,
   command_all_mods,
   command_toggle_conc_levels,
   command_toggle_act_phan,
   command_toggle_ignoreblanks,
   command_toggle_retain_after_error,
#ifdef SINGERS
   command_toggle_singer,
   command_toggle_singer_backward,
#endif
   command_undo,
   command_erase,
   command_abort,
   command_create_comment,
   command_change_outfile,
   command_change_header,
   command_getout,
   command_getout,
   command_save_pic,
   command_refresh,
   command_resolve,
   command_normalize,
   command_standardize,
   command_reconcile,
   command_random_call,
   command_simple_call,
   command_concept_call,
   command_level_call,
   command_8person_level_call,
   command_create_any_lines,
   command_create_waves,
   command_create_2fl,
   command_create_li,
   command_create_lo,
   command_create_inv_lines,
   command_create_3and1_lines,
   command_create_any_col,
   command_create_col,
   command_create_magic_col,
   command_create_dpt,
   command_create_cdpt,
   command_create_tby,
   command_create_8ch,
   command_create_any_qtag,
   command_create_qtag,
   command_create_3qtag,
   command_create_qline,
   command_create_3qline,
   command_create_dmd,
   command_create_any_tidal,
   command_create_tidal_wave
};


/* BEWARE!!  These two lists must stay in step. */

int number_of_resolve_commands = 10;    /* The number of items in the tables, independent of NUM_RESOLVE_COMMAND_KINDS. */

Cstring resolve_command_strings[] = {
    "abort the search",
    "exit the search",
    "quit the search",
    "undo the search",
    "find another",
    "next",
    "previous",
    "accept current choice",
    "raise reconcile point",
    "lower reconcile point"
};

static resolve_command_kind resolve_command_values[] = {
   resolve_command_abort,
   resolve_command_abort,
   resolve_command_abort,
   resolve_command_abort,
   resolve_command_find_another,
   resolve_command_goto_next,
   resolve_command_goto_previous,
   resolve_command_accept,
   resolve_command_raise_rec_point,
   resolve_command_lower_rec_point
};


/* BEWARE!!  These two lists must stay in step. */

int num_extra_resolve_commands = 1;     /* The short list of extra things we present during a resolve. */

Cstring extra_resolve_commands[] = {
   "write this sequence"
};

static command_kind extra_resolve_command_values[] = {
   command_getout
};



typedef struct {
   int which_test;      /* 0 to test for >= 0, negative to test for that value exactly. */
   char *line_to_put;
   uims_reply match_kind;
   long int match_index;
} fcn_key_thing;


#define FCN_KEY_TAB_LOW 129
#define FCN_KEY_TAB_LAST 172

static fcn_key_thing fcn_key_table[] = {
   {match_startup_commands,  "heads start\n",              ui_start_select,   start_select_heads_start},         /* F1 = 129 = heads start */
   {0,                       "two calls in succession\n",  ui_concept_select, (long int) &two_calls_concept_ptr},/* F2 = 130 = two calls in succession */
   {0,                       "pick random call\n",         ui_command_select, -1-command_random_call},           /* F3 = 131 = pick random call */
   {0,                       "resolve\n",                  ui_command_select, -1-command_resolve},               /* F4 = 132 = resolve */
   {0,                       "refresh display\n",          ui_command_select, -1-command_refresh},               /* F5 = 133 = refresh display */
   {0,                       "simple modifications\n",     ui_command_select, -1-command_simple_mods},           /* F6 = 134 = simple modifications */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 135 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 136 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 137 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 138 */
   {0,                       "pick level call\n",          ui_command_select, -1-command_level_call},           /* F11 = 139 = pick level call */
   {match_resolve_commands,  "find another\n",             ui_resolve_select, -1-resolve_command_find_another}, /* F12 = 140 = find another */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 141 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 142 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 143 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 144 */
   {match_startup_commands,  "sides start\n",              ui_start_select,   start_select_sides_start},        /* sF1 = 145 = sides start */
   {0,                       "twice\n",                    ui_concept_select, (long int) &twice_concept_ptr}, /* sF2 = 146 = twice */
   {0,                       "pick concept call\n",        ui_command_select, -1-command_concept_call},         /* sF3 = 147 = pick concept call */
   {0,                       "reconcile\n",                ui_command_select, -1-command_reconcile},            /* sF4 = 148 = reconcile */
   {0,                       "keep picture\n",             ui_command_select, -1-command_save_pic},             /* sF5 = 149 = keep picture */
   {0,                       "allow modifications\n",      ui_command_select, -1-command_all_mods},             /* sF6 = 150 = allow modifications */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 151 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 152 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 153 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 154 */
   {0,                       "pick 8 person level call\n", ui_command_select, -1-command_8person_level_call},  /* sF11 = 155 = pick 8 person level call */
   {match_resolve_commands,  "accept current choice\n",    ui_resolve_select, -1-resolve_command_accept},      /* sF12 = 156 = accept current choice */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 157 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 158 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 159 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 160 */
   {match_startup_commands,  "just as they are\n",         ui_start_select, start_select_as_they_are},          /* cF1 = 161 = just as they are */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 162 */
   {0,                       "pick simple call\n",         ui_command_select, -1-command_simple_call},          /* cF3 = 163 = pick simple call */
   {0,                       "normalize\n",                ui_command_select, -1-command_normalize},            /* cF4 = 164 = normalize */
   {0,                       "insert a comment\n",         ui_command_select, -1-command_create_comment},       /* cF5 = 165 = insert a comment */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 166 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 167 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 168 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 169 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 170 */
   {99,                      (char *) 0,                   ui_command_select, 0},                                     /* 171 */
   {match_resolve_commands,  "previous\n",                 ui_resolve_select, -1-resolve_command_goto_previous}/* cF12 = 172 = previous */
};


static match_result user_match;



Private void get_user_input(char *prompt, int which)
{
   char *p;
   char c;
   int nc;
   int matches;

   user_match.valid = FALSE;
   user_input_prompt = prompt;
   user_input[0] = '\0';
   static_ss.full_input[0] = '\0';
   static_ss.full_input_size = 0;
   function_key_expansion = (char *) 0;
   put_line(user_input_prompt);

   for (;;) {
      /* At this point we always have the concatenation of "user_input_prompt"
         and "user_input" displayed on the current line. */

      start_expand:

      if (function_key_expansion) {
         c = *function_key_expansion++;
         if (c) goto got_char;
         else function_key_expansion = (char *) 0;
      }

      nc = get_char();

      if (nc >= 128) {
         if (nc == 135) {                                       /* F7 = 135 = toggle concept levels */
            if (which >= 0) {
               put_line("toggle concept levels\n");
               current_text_line++;
               user_match.match.kind = ui_command_select;
               user_match.match.index = -1-command_toggle_conc_levels;
               user_match.valid = TRUE;
               return;
            }
            else if (which == match_startup_commands) {
               put_line("toggle concept levels\n");
               current_text_line++;
               user_match.match.kind = ui_start_select;
               user_match.match.index = (int) start_select_toggle_conc;
               user_match.valid = TRUE;
               return;
            }
         }
         else if (nc == 136)
            function_key_expansion = "<anything>";                   /* F8 */
         else if (nc == 137 || nc == 153) {                          /* F9 or sF9 = undo or abort the search, as appropriate. */
            if (which >= 0) {
               put_line("undo last call\n");
               current_text_line++;
               user_match.match.kind = ui_command_select;
               user_match.match.index = -1-command_undo;
               user_match.valid = TRUE;
               return;
            }
            else if (which == match_resolve_commands) {
               put_line("abort the search\n");
               current_text_line++;
               user_match.match.kind = ui_resolve_select;
               user_match.match.index = -1-resolve_command_abort;
               user_match.valid = TRUE;
               return;
            }
            else if (which == match_startup_commands) {
               put_line("exit from the program\n");
               current_text_line++;
               user_match.match.kind = ui_start_select;
               user_match.match.index = (int) start_select_exit;
               user_match.valid = TRUE;
               return;
            }
         }
         else if (nc == 138)
            function_key_expansion = "write this sequence\n";        /* F10 */
         else if (nc == 151) {                                       /* sF7 = toggle active phantoms */
            if (which >= 0) {
               put_line("toggle active phantoms\n");
               current_text_line++;
               user_match.match.kind = ui_command_select;
               user_match.match.index = -1-command_toggle_act_phan;
               user_match.valid = TRUE;
               return;
            }
            else if (which == match_startup_commands) {
               put_line("toggle active phantoms\n");
               current_text_line++;
               user_match.match.kind = ui_start_select;
               user_match.match.index = (int) start_select_toggle_act;
               user_match.valid = TRUE;
               return;
            }
         }
         else if (nc == 152)
            function_key_expansion = "<concept>";                    /* sF8 */
                                                                     /* See above for sF9. */
         else if (nc == 154) {                                       /* sF10 = change output file */
            if (which >= 0) {
               put_line("change output file\n");
               current_text_line++;
               user_match.match.kind = ui_command_select;
               user_match.match.index = -1-command_change_outfile;
               user_match.valid = TRUE;
               return;
            }
            else if (which == match_startup_commands) {
               put_line("change output file\n");
               current_text_line++;
               user_match.match.kind = ui_start_select;
               user_match.match.index = (int) start_select_change_outfile;
               user_match.valid = TRUE;
               return;
            }
         }
         else if (nc >= FCN_KEY_TAB_LOW && nc <= FCN_KEY_TAB_LAST &&
                     ((fcn_key_table[nc-FCN_KEY_TAB_LOW].which_test == 0 && which >= 0) ||
                     (fcn_key_table[nc-FCN_KEY_TAB_LOW].which_test <= 0 && which == fcn_key_table[nc-FCN_KEY_TAB_LOW].which_test))) {
            put_line(fcn_key_table[nc-FCN_KEY_TAB_LOW].line_to_put);
            current_text_line++;
            user_match.match.kind = fcn_key_table[nc-FCN_KEY_TAB_LOW].match_kind;
            user_match.match.index = fcn_key_table[nc-FCN_KEY_TAB_LOW].match_index;

            if (user_match.match.kind == ui_concept_select) {
               user_match.match.concept_ptr = *((concept_descriptor **) fcn_key_table[nc-FCN_KEY_TAB_LOW].match_index);
               if (!user_match.match.concept_ptr) continue;
               user_match.match.next = (modifier_block *) 0;
               user_match.match.call_conc_options = null_options;
               user_match.next = (match_result *) 0;
            }
            else
               user_match.match.index = fcn_key_table[nc-FCN_KEY_TAB_LOW].match_index;

            user_match.valid = TRUE;
            return;
         }
         else continue;      /* Ignore the function key. */
      }

      c = nc;

      if (function_key_expansion)
         goto start_expand;

      got_char:

      if ((c == '\b') || (c == DEL)) {
         if (static_ss.full_input_size > 0) {
            static_ss.full_input_size--;
            user_input[static_ss.full_input_size] = '\0';
            static_ss.full_input[static_ss.full_input_size] = '\0';
            function_key_expansion = (char *) 0;
            rubout(); /* Update the display. */
         }
         continue;
      }
      else if (c == '?' || c == '!') {
         put_line ("\n");
         current_text_line++;
         start_matches();
         (void) match_user_input(which, TRUE, c == '?');
         put_line ("\n");     /* Write a blank line. */
         current_text_line++;
         put_line(user_input_prompt);   /* Redisplay the current line. */
         put_line(user_input);
         continue;
      }

      if (c == ' ') {
         /* extend only to one space, inclusive */
         matches = match_user_input(which, FALSE, FALSE);
         user_match = static_ss.result;
         p = static_ss.extended_input;

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
         else if (static_ss.space_ok && matches > 1)
            pack_and_echo_character(c);
         else if (diagnostic_mode)
            goto diagnostic_error;
         else
            bell();
      }
      else if ((c == '\n') || (c == '\r')) {
         matches = match_user_input(which, FALSE, FALSE);
         user_match = static_ss.result;

         /* We forbid a match consisting of two or more "direct parse" concepts, such as "grand cross".
            Direct parse concepts may only be stacked if they are followed by a call.
            The "match.next" field indicates that direct parse concepts were stacked. */

         if (  (matches == 1 || matches - static_ss.yielding_matches == 1 || user_match.exact)
                              &&
              (!user_match.match.next || user_match.match.kind == ui_call_select || user_match.match.kind == ui_concept_select)) {

            p = static_ss.extended_input;
            while (*p)
               pack_and_echo_character(*p++);

            put_line("\n");
            /* Include the input line in our count, so we will erase it
               if we are trying to make the VT-100 screen look nice. */
            current_text_line++;
            return;
         }

         if (diagnostic_mode)
            goto diagnostic_error;

         /* Tell how bad it is, then redisplay current line. */
         if (matches > 0) {
            char tempstuff[200];

            (void) sprintf(tempstuff, "  (%d matches, type ! or ? for list)\n", matches);
            put_line(tempstuff);
         }
         else
            put_line("  (no matches)\n");

         put_line(user_input_prompt);
         put_line(user_input);
         current_text_line++;   /* Count that line for erasure. */
      }
      else if (c == '\t' || c == '\033') {
         (void) match_user_input(which, FALSE, FALSE);
         user_match = static_ss.result;
         p = static_ss.extended_input;

         if (*p) {
            while (*p)
               pack_and_echo_character(*p++);
         }
         else if (diagnostic_mode)
            goto diagnostic_error;
         else
            bell();
      }
      else if (c == ('U'&'\037')) { /* C-u: kill line */
         user_input[0] = '\0';
         static_ss.full_input[0] = '\0';
         static_ss.full_input_size = 0;
         function_key_expansion = (char *) 0;
         clear_line();           /* Clear the current line */
         put_line(user_input_prompt);    /* Redisplay the prompt. */
      }
      else if (isprint(c))
         pack_and_echo_character(c);
      else if (diagnostic_mode)
         goto diagnostic_error;
      else
         bell();
   }

   diagnostic_error:

   uims_terminate();
   (void) fputs("\nParsing error during diagnostic.\n", stdout);
   (void) fputs("\nParsing error during diagnostic.\n", stderr);
   final_exit(1);
}




static char *banner_prompts0[] = {
    "",
    "simple modifications",
    "all modifications",
    "??"};

static char *banner_prompts1[] = {
    "",
    "all concepts",
    "??",
    "??"};

static char *banner_prompts2[] = {
    "",
    "AP",
    "??",
    "??"};

static char *banner_prompts3[] = {
    "",
    "singer",
    "reverse singer",
    "??"};




extern uims_reply uims_get_startup_command(void)
{
   get_user_input("Enter startup command> ", (int) match_startup_commands);
   uims_menu_index = user_match.match.index;
   return user_match.match.kind;
}


/* This returns TRUE if it fails, e.g. the user waves the mouse away. */
extern long_boolean uims_get_call_command(uims_reply *reply_p)
{
   char prompt_buffer[200];
   char *prompt_ptr;
   int banner_mode;

   if (allowing_modifications)
      parse_state.call_list_to_use = call_list_any;

   prompt_ptr = prompt_buffer;
   prompt_buffer[0] = '\0';

   /* Put any necessary special things into the prompt. */

   banner_mode = (singing_call_mode << 6) |
                 (using_active_phantoms << 4) |
                 (allowing_all_concepts << 2) |
                 (allowing_modifications);

   if (banner_mode != 0) {
      int i;
      int comma = 0;

      (void) strcat(prompt_buffer, "[");

      for (i=0 ; i<4 ; i++,banner_mode>>=2) {


         if (banner_mode&3) {

            if (comma) (void) strcat(prompt_buffer, ",");

            if (i==0) {
               (void) strcat(prompt_buffer, banner_prompts0[banner_mode&3]);
            }

            if (i==1) {
               (void) strcat(prompt_buffer, banner_prompts1[banner_mode&3]);
            }

            if (i==2) {
               (void) strcat(prompt_buffer, banner_prompts2[banner_mode&3]);
            }

            if (i==3) {
               (void) strcat(prompt_buffer, banner_prompts3[banner_mode&3]);
            }

            comma |= banner_mode&3;
         }
      }

      (void) strcat(prompt_buffer, "] ");
      (void) strcat(prompt_buffer, call_menu_prompts[parse_state.call_list_to_use]);
   }
   else
      prompt_ptr = call_menu_prompts[parse_state.call_list_to_use];

   get_user_input(prompt_ptr, (int) parse_state.call_list_to_use);

   *reply_p = user_match.match.kind;

   uims_menu_index = user_match.match.index;

   if (user_match.match.kind == ui_command_select) {
      /* Translate the command. */

      if (user_match.match.index < 0)
         uims_menu_index = -1-user_match.match.index;   /* Special encoding from a function key. */
      else
         uims_menu_index = (int) command_command_values[user_match.match.index];
   }
   else {
      parse_block *save1 = (parse_block *) 0;
      modifier_block *anythings = &user_match.match;

      /* This stuff is duplicated in verify_call in sdmatch.c . */

      while (anythings) {
         call_conc_option_state save_stuff = user_match.match.call_conc_options;

         /* First, if we have already deposited a call, and we see more stuff, it must be
            concepts or calls for an "anything" subcall. */

         if (save1) {
            parse_block *tt = get_parse_block();
            save1->concept = &marker_concept_mod;
            save1->next = tt;
            tt->concept = &marker_concept_mod;
            tt->call = base_calls[1];   /* "nothing" */
            tt->options.number_fields = 2*DFM1_CALL_MOD_BIT;  /* "mandatory_anycall" */
            parse_state.concept_write_ptr = &tt->subsidiary_root;
            save1 = (parse_block *) 0;
         }

         user_match.match.call_conc_options = anythings->call_conc_options;

         if (anythings->kind == ui_call_select) {
            if (deposit_call(anythings->call_ptr)) return TRUE;
            save1 = *parse_state.concept_write_ptr;
            *reply_p = ui_call_select;
         }
         else if (anythings->kind == ui_concept_select) {
            if (deposit_concept(anythings->concept_ptr)) return TRUE;
         }
         else break;   /* Huh? */

         user_match.match.call_conc_options = save_stuff;
         anythings = anythings->next;
      }
   }

   return FALSE;
}


extern uims_reply uims_get_resolve_command(void)
{
   get_user_input("Enter search command> ", (int) match_resolve_commands);

   if (user_match.match.kind == ui_command_select) {
      uims_menu_index = extra_resolve_command_values[user_match.match.index];
   }
   else {
      if (user_match.match.index < 0)
         uims_menu_index = -1-user_match.match.index;   /* Special encoding from a function key. */
      else
         uims_menu_index = (int) resolve_command_values[user_match.match.index];
   }

   return user_match.match.kind;
}


Private int get_popup_string(char prompt[], char dest[])
{
    char buffer[200];

    (void) sprintf(buffer, "%s: ", prompt);
    get_string_input(buffer, dest);
    return POPUP_ACCEPT_WITH_STRING;
}

extern int uims_do_comment_popup(char dest[])
{
    return get_popup_string("Enter comment", dest);
}

extern int uims_do_outfile_popup(char dest[])
{
    char buffer[MAX_TEXT_LINE_LENGTH];
    (void) sprintf(buffer, "Sequence output file is \"%s\".\n", outfile_string);

    put_line(buffer);
    current_text_line++;
    return get_popup_string("Enter new file name", dest);
}

extern int uims_do_header_popup(char dest[])
{
   if (header_comment[0]) {
      char buffer[MAX_TEXT_LINE_LENGTH];
      (void) sprintf(buffer, "Current title is \"%s\".\n", header_comment);
      put_line(buffer);
      current_text_line++;
   }
   return get_popup_string("Enter new title", dest);
}

extern int uims_do_getout_popup(char dest[])
{
    put_line("Type comment for this sequence, if desired.\n");
    current_text_line++;
    return get_popup_string("Enter comment", dest);
}

#ifdef NEGLECT
extern int uims_do_neglect_popup(char dest[])
{
    put_line("Specify integer percentage of neglected calls.\n");
    current_text_line++;
    return get_popup_string("Enter percentage", dest);
}
#endif

Private int confirm(char *question)
{
    char c;

    for (;;) {
        put_line(question);
        c = get_char();
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

        if (diagnostic_mode) {
            uims_terminate();
            (void) fputs("\nParsing error during diagnostic.\n", stdout);
            (void) fputs("\nParsing error during diagnostic.\n", stderr);
            final_exit(1);
        }

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

extern int uims_do_modifier_popup(Cstring callname, modify_popup_kind kind)
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
        case modify_popup_only_circ:
            line_format = "The \"%s\" can be replaced with a modified circulate-like call.\n";
            break;
    }

    sprintf(tempstuff, line_format, callname);
    put_line(tempstuff);
    current_text_line++;
    return confirm("Do you want to replace it? ");
}

/*
 * UIMS_BEGIN_SEARCH is called at the beginning of each search mode
 * command (resolve, reconcile, nice setup, pick random call).
 */

extern void uims_begin_search(command_kind goal)
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

extern void uims_update_resolve_menu(command_kind goal, int cur, int max, resolver_display_state state)
{
    char title[MAX_TEXT_LINE_LENGTH];

    create_resolve_menu_title(goal, cur, max, state, title);
    put_line(title);
    put_line("\n");
    current_text_line++;
}

volatile uint32 popup_retval;     /* Just love that HP-UX C compiler!!!!! */

extern int uims_do_selector_popup(void)
{
   if (!user_match.valid || (user_match.match.call_conc_options.who == selector_uninitialized)) {
      match_result saved_match = user_match;
      get_user_input("Enter who> ", (int) match_selectors);
      popup_retval = user_match.match.index+1;      /* We skip the zeroth selector, which is selector_uninitialized. */
      user_match = saved_match;
   }
   else {
      popup_retval = (int) user_match.match.call_conc_options.who;
      user_match.match.call_conc_options.who = selector_uninitialized;
   }
   return popup_retval;
}

extern int uims_do_direction_popup(void)
{
   int retval;

   if (!user_match.valid || (user_match.match.call_conc_options.where == direction_uninitialized)) {
      match_result saved_match = user_match;
      get_user_input("Enter direction> ", (int) match_directions);
      retval = user_match.match.index+1;      /* We skip the zeroth direction, which is direction_uninitialized. */
      user_match = saved_match;
   }
   else {
      retval = (int) user_match.match.call_conc_options.where;
      user_match.match.call_conc_options.where = direction_uninitialized;
   }
   return retval;
}

extern int uims_do_tagger_popup(int tagger_class)
{
   uint32 retval;
   int j = 0;

   if (interactivity == interactivity_verify) {
      user_match.match.call_conc_options.tagger = verify_options.tagger;
      if (user_match.match.call_conc_options.tagger == 0) user_match.match.call_conc_options.tagger = (tagger_class << 5) + 1;
   }
   else if (!user_match.valid || (user_match.match.call_conc_options.tagger <= 0)) {
      match_result saved_match = user_match;
      get_user_input("Enter tagging call> ", ((int) match_taggers) + tagger_class);
      saved_match.match.call_conc_options.tagger = user_match.match.call_conc_options.tagger;
      user_match = saved_match;
   }

   while ((user_match.match.call_conc_options.tagger & 0xFF000000UL) == 0) {
      user_match.match.call_conc_options.tagger <<= 8;
      j++;
   }

   retval = user_match.match.call_conc_options.tagger >> 24;
   user_match.match.call_conc_options.tagger &= 0x00FFFFFF;
   while (j-- != 0) user_match.match.call_conc_options.tagger >>= 8;   /* Shift it back. */

   if (interactivity == interactivity_verify)
      verify_options.tagger = user_match.match.call_conc_options.tagger;

   return retval;
}


extern int uims_do_circcer_popup(void)
{
   uint32 retval;

   if (interactivity == interactivity_verify) {
      retval = verify_options.circcer;
      if (retval == 0) retval = 1;
   }
   else if (!user_match.valid || (user_match.match.call_conc_options.circcer == 0)) {
      match_result saved_match = user_match;
      get_user_input("Enter circulate replacement> ", (int) match_circcer);
      retval = user_match.match.call_conc_options.circcer;
      user_match = saved_match;
   }
   else {
      retval = user_match.match.call_conc_options.circcer;
      user_match.match.call_conc_options.circcer = 0;
   }

   return retval;
}


extern uint32 uims_get_number_fields(int nnumbers)
{
   int i;
   uint32 number_fields = user_match.match.call_conc_options.number_fields;
   int howmanynumbers = user_match.match.call_conc_options.howmanynumbers;
   uint32 number_list = 0;

   for (i=0 ; i<nnumbers ; i++) {
      uint32 this_num;

      if (!user_match.valid || (howmanynumbers <= 0)) {
         for (;;) {
            char buffer[200];
            get_string_input("How many? ", buffer);
            if (buffer[0] == '!' || buffer[0] == '?') {
               put_line("Type a number between 1 and 8\n");
               current_text_line++;
            }
            else {
               this_num = atoi(buffer);
               break;
            }
         }
      }
      else {
         this_num = number_fields & 0xF;
         number_fields >>= 4;
         howmanynumbers--;
      }

      if (this_num <= 0 || this_num > 8) return 0;    /* User gave bad answer. */
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
 * The following three functions allow the UI to put up a progress
 * indicator while the call database is being read (and processed).
 *
 * uims_database_tick_max is called before reading the database
 * with the number of ticks that will be sent.
 * uims_database_tick is called repeatedly with the number of new
 * ticks to add.
 */

Private int db_tick_max;
Private int db_tick_cur;   /* goes from 0 to db_tick_max */

#define TICK_STEPS 52
Private int tick_displayed;   /* goes from 0 to TICK_STEPS */

extern void uims_database_tick_max(int n)
{
   db_tick_max = n;
   db_tick_cur = 0;
   printf("Sd: reading database...");
   fflush(stdout);
   tick_displayed = 0;
}

extern void uims_database_tick(int n)
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

extern void uims_database_tick_end(void)
{
    printf("done\n");
}

extern void uims_database_error(Cstring message, Cstring call_name)
{
   print_line(message);
   if (call_name) {
      print_line("  While reading this call from the database:");
      print_line(call_name);
   }
}

extern void uims_bad_argument(Cstring s1, Cstring s2, Cstring s3)
{
   if (s1) print_line(s1);
   if (s2) print_line(s2);
   if (s3) print_line(s3);
   print_line("Use the -help flag for help.");
   exit_program(1);
}
