/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

/* 
 * sdui-tty.c - SD TTY User Interface
 * Originally for Macintosh.  Unix version by gildea.
 * Time-stamp: <96/05/22 17:17:53 wba>
 * Copyright (c) 1990-1994 Stephen Gildea, William B. Ackerman, and
 *   Alan Snyder
 *
 * Copyright (c) 1994-1998 William B. Ackerman
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
 * For use with version 32 of the Sd program.
 * Based on sdui-x11.c 1.10
 *
 * The version of this file is as shown immediately below.  This string
 * gets displayed at program startup, as the "ui" part of the complete
 * version.
 */

#define UI_VERSION_STRING "1.10"
#define UI_TIME_STAMP "wba@an.hp.com  29 Sept 98 $"

/* This file defines the following functions:
   uims_version_string
   uims_process_command_line
   uims_display_help
   uims_display_ui_intro_text
   uims_preinitialize
   uims_create_menu
   uims_postinitialize
   uims_set_window_title
   show_match
   uims_get_startup_command
   uims_get_call_command
   uims_get_resolve_command
   uims_do_comment_popup
   uims_do_outfile_popup
   uims_do_header_popup
   uims_do_getout_popup
   uims_do_write_anyway_popup
   uims_do_delete_clipboard_popup
   uims_do_abort_popup
   uims_do_session_init_popup
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

   num_command_commands
   command_commands
   number_of_resolve_commands
   resolve_command_strings
   startup_commands

and the following other variables:

   journal_file
   no_graphics
   screen_height
   no_cursor
   no_console
   no_intensify
   no_color
   no_sound
   no_line_delete
   concept_key_table
*/


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

/* See comments in sdmain.c regarding this string. */
static Const char id[] = "@(#)$He" "ader: Sd: sdui-tty.c " UI_VERSION_STRING "  " UI_TIME_STAMP;


#include "sdui-tty.h"

#define DEL 0x7F

/*
 * The total version string looks something like
 * "1.4:db1.5:ui0.6tty"
 * We return the "0.6tty" part.
 */

FILE *journal_file = (FILE *) 0;
int no_graphics = 0;
int screen_height = 25;
int no_cursor = 0;
int no_console = 0;
int no_intensify = 0;
int no_color = 0;
int no_sound = 0;
int no_line_delete = 0;

static char version_mem[12];

extern char *uims_version_string(void)
{
    (void) sprintf(version_mem, "%stty", UI_VERSION_STRING);
    return version_mem;
}

static match_result user_match;

static resolver_display_state resolver_happiness = resolver_display_failed;


typedef struct {
   modifier_block value;
} fcn_key_thing;



static fcn_key_thing *fcn_key_table_normal[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
static fcn_key_thing *fcn_key_table_start[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
static fcn_key_thing *fcn_key_table_resolve[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];


/* BEWARE!!  This list is keyed to the definition of "start_select_kind" in sd.h . */
Cstring startup_commands[] = {
   "exit from the program",
   "heads 1p2p",
   "sides 1p2p",
   "heads start",
   "sides start",
   "just as they are",
   "toggle concept levels",
   "toggle active phantoms",
   "toggle retain after error",
   "toggle nowarn mode",
   "toggle singing call",
   "toggle reverse singing call",
   "initialize session file",
   "change output file",
   "change title",
   (Cstring) 0
};


typedef struct {
   Cstring command_name;
   command_kind action;
} command_list_menu_item;

typedef struct {
   Cstring command_name;
   resolve_command_kind action;
} resolve_list_menu_item;


static command_list_menu_item command_menu[] = {
   {"exit the program",               command_quit},
   {"quit the program",               command_quit},
   {"simple modifications",           command_simple_mods},
   {"allow modifications",            command_all_mods},
   {"toggle concept levels",          command_toggle_conc_levels},
   {"toggle active phantoms",         command_toggle_act_phan},
   {"toggle retain after error",      command_toggle_retain_after_error},
   {"toggle nowarn mode",             command_toggle_nowarn_mode},
   {"undo last call",                 command_undo},
   {"discard entered concepts",       command_erase},
   {"abort this sequence",            command_abort},
   {"insert a comment",               command_create_comment},
   {"change output file",             command_change_outfile},
   {"change title",                   command_change_header},
   {"write this sequence",            command_getout},
   {"end this sequence",              command_getout},
   {"cut to clipboard",               command_cut_to_clipboard},
   {"clipboard cut",                  command_cut_to_clipboard},
   {"delete entire clipboard",        command_delete_entire_clipboard},
   {"clipboard delete all",           command_delete_entire_clipboard},
   {"delete one call from clipboard", command_delete_one_call_from_clipboard},
   {"clipboard delete one",           command_delete_one_call_from_clipboard},
   {"paste one call",                 command_paste_one_call},
   {"clipboard paste one",            command_paste_one_call},
   {"paste all calls",                command_paste_all_calls},
   {"clipboard paste all",            command_paste_all_calls},
   {"keep picture",                   command_save_pic},
   {"refresh display",                command_refresh},
   {"resolve",                        command_resolve},
   {"normalize",                      command_normalize},
   {"standardize",                    command_standardize},
   {"reconcile",                      command_reconcile},
   {"pick random call",               command_random_call},
   {"pick simple call",               command_simple_call},
   {"pick concept call",              command_concept_call},
   {"pick level call",                command_level_call},
   {"pick 8 person level call",       command_8person_level_call},
   {"create any lines",               command_create_any_lines},
   {"create waves",                   command_create_waves},
   {"create 2fl",                     command_create_2fl},
   {"create lines in",                command_create_li},
   {"create lines out",               command_create_lo},
   {"create inverted lines",          command_create_inv_lines},
   {"create 3x1 lines",               command_create_3and1_lines},
   {"create any columns",             command_create_any_col},
   {"create columns",                 command_create_col},
   {"create magic columns",           command_create_magic_col},
   {"create dpt",                     command_create_dpt},
   {"create cdpt",                    command_create_cdpt},
   {"create trade by",                command_create_tby},
   {"create 8 chain",                 command_create_8ch},
   {"create any 1/4 tag",             command_create_any_qtag},
   {"create 1/4 tag",                 command_create_qtag},
   {"create 3/4 tag",                 command_create_3qtag},
   {"create 1/4 line",                command_create_qline},
   {"create 3/4 line",                command_create_3qline},
   {"create diamonds",                command_create_dmd},
   {"create any tidal setup",         command_create_any_tidal},
   {"create tidal wave",              command_create_tidal_wave},
   {(Cstring) 0}};


int num_command_commands;   /* Size of the command menu. */
Cstring *command_commands;
command_kind *command_command_values;

int number_of_resolve_commands;
Cstring *resolve_command_strings;
static resolve_command_kind *resolve_command_values;


static resolve_list_menu_item resolve_menu[] = {
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


/*
 * User Input functions
 */

/* This array is the same as static_ss.full_input, but has the original capitalization
   as typed by the user.  Static_ss.full_input is converted to all lower case for
   ease of searching. */
static char user_input[INPUT_TEXTLINE_SIZE+1];
static char *user_input_prompt;
static char *function_key_expansion;

void refresh_input(void)
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

static char *call_menu_prompts[NUM_CALL_LIST_KINDS];

/* For the "alternate_glyphs_1" command-line switch. */
static char alt1_names1[] = "        ";
static char alt1_names2[] = "1P2R3O4C";



/*
 * The main program calls this before doing anything else, so we can
 * supply additional command line arguments.
 * Note: If we are writing a call list, the program will
 * exit before doing anything else with the user interface, but this
 * must be made anyway.
 */
 
extern void uims_process_command_line(int *argcp, char ***argvp)
{
   int argno = 1;
   char **argv = *argvp;

   while (argno < (*argcp)) {
      int i;

      if (strcmp(argv[argno], "-no_line_delete") == 0)
         no_line_delete = 1;
      else if (strcmp(argv[argno], "-no_cursor") == 0)
         no_cursor = 1;
      else if (strcmp(argv[argno], "-no_console") == 0)
         no_console = 1;
      else if (strcmp(argv[argno], "-no_intensify") == 0)
         no_intensify = 1;
      else if (strcmp(argv[argno], "-no_color") == 0)
         no_color = 1;
      else if (strcmp(argv[argno], "-no_sound") == 0)
         no_sound = 1;
      else if (strcmp(argv[argno], "-no_graphics") == 0)
         no_graphics = 1;
      else if (strcmp(argv[argno], "-alternate_glyphs_1") == 0) {
         pn1 = alt1_names1;
         pn2 = alt1_names2;
      }
      else if (strcmp(argv[argno], "-lines") == 0 && argno+1 < (*argcp)) {
         screen_height = atoi(argv[argno+1]);
         goto remove_two;
      }
      else if (strcmp(argv[argno], "-journal") == 0 && argno+1 < (*argcp)) {
         journal_file = fopen(argv[argno+1], "w");

         if (!journal_file) {
            printf("Can't open journal file\n");
            perror(argv[argno+1]);
            exit_program(1);
         }

         goto remove_two;
      }
      else {
         argno++;
         continue;
      }

      (*argcp)--;      /* Remove this argument from the list. */
      for (i=argno+1; i<=(*argcp); i++) argv[i-1] = argv[i];
      continue;

      remove_two:

      (*argcp) -= 2;      /* Remove two arguments from the list. */
      for (i=argno+1; i<=(*argcp); i++) argv[i-1] = argv[i+1];
      continue;
   }

   if (ttu_process_command_line(argcp, *argvp, &use_escapes_for_drawing_people,
                                pn1, pn2, &direc))
      exit_program(1);
}

extern void uims_display_help(void)
{
   ttu_display_help();
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
   int i;

   current_text_line = 0;
   ttu_initialize();

   (void) memset(fcn_key_table_normal, 0,
                 sizeof(fcn_key_thing *) * (FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1));
   (void) memset(fcn_key_table_start, 0,
                 sizeof(fcn_key_thing *) * (FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1));
   (void) memset(fcn_key_table_resolve, 0,
                 sizeof(fcn_key_thing *) * (FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1));

   /* Find out how big the command menu needs to be. */

   for (num_command_commands = 0 ;
        command_menu[num_command_commands].command_name ;
        num_command_commands++) ;

   command_commands = (Cstring *) get_mem(sizeof(Cstring) * num_command_commands);
   command_command_values =
      (command_kind *) get_mem(sizeof(command_kind) * num_command_commands);

   for (i = 0 ; i < num_command_commands; i++) {
      command_commands[i] = command_menu[i].command_name;
      command_command_values[i] = command_menu[i].action;
   }

   /* Find out how big the resolve menu needs to be. */

   for (number_of_resolve_commands = 0 ;
        resolve_menu[number_of_resolve_commands].command_name ;
        number_of_resolve_commands++) ;

   resolve_command_strings = (Cstring *) get_mem(sizeof(Cstring) * number_of_resolve_commands);
   resolve_command_values = (resolve_command_kind *)
      get_mem(sizeof(resolve_command_kind) * number_of_resolve_commands);

   for (i = 0 ; i < number_of_resolve_commands; i++) {
      resolve_command_strings[i] = resolve_menu[i].command_name;
      resolve_command_values[i] = resolve_menu[i].action;
   }
}

/*
 * Create a menu containing number_of_calls[cl] items.
 * Use the "menu_names" array to create a
 * title line for the menu.  The string is in static storage.
 * 
 * This will be called once for each value in the enumeration call_list_kind.
 */

extern void uims_create_menu(call_list_kind cl)
{
   call_menu_prompts[cl] = (char *) get_mem(50);  /* *** Too lazy to compute it. */
   matcher_setup_call_menu(cl);

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


Private int translate_keybind_spec(char key_name[])
{
   int key_length;
   int d1, d2, digits;

   key_length = strlen(key_name);

   if (key_length < 2) return -1;

   d2 = key_name[key_length-1] - '0';
   if (d2 >= 0 && d2 <= 9) {
      digits = d2;
      d1 = key_name[key_length-2] - '0';
      if (d1 >= 0 && d1 <= 9) {
         digits += d1*10;
         key_length--;
      }

      if (key_name[key_length-2] == 'f') {
         if (digits < 1 || digits > 12)
            return -1;

         if (key_length == 2) {
            return FKEY+digits;
         }
         else if (key_length == 3 && key_name[0] == 's') {
            return SFKEY+digits;
         }
         else if (key_length == 3 && key_name[0] == 'c') {
            return CFKEY+digits;
         }
         else if (key_length == 3 && key_name[0] == 'a') {
            return AFKEY+digits;
         }
         else if (key_length == 4 && key_name[0] == 'c' && key_name[1] == 'a') {
            return CAFKEY+digits;
         }
         else {
            return -1;
         }
      }
      if (key_name[key_length-2] == 'e') {
         if (digits > 15)
            return -1;

         if (key_length == 2) {
            return EKEY+digits;
         }
         else if (key_length == 3 && key_name[0] == 's') {
            return SEKEY+digits;
         }
         else if (key_length == 3 && key_name[0] == 'c') {
            return CEKEY+digits;
         }
         else if (key_length == 3 && key_name[0] == 'a') {
            return AEKEY+digits;
         }
         else if (key_length == 4 && key_name[0] == 'c' && key_name[1] == 'a') {
            return CAEKEY+digits;
         }
         else {
            return -1;
         }
      }
      else if (key_length == 2 && key_name[0] == 'c') {
         return CTLDIG+digits;
      }
      else if (key_length == 2 && key_name[0] == 'a') {
         return ALTDIG+digits;
      }
      else if (key_length == 3 && key_name[0] == 'c' && key_name[1] == 'a') {
         return CTLALTDIG+digits;
      }
      else { 
         return -1;
      }
   }
   else if (key_name[key_length-1] >= 'a' && key_name[key_length-1] <= 'z') {
      if (key_length == 2 && key_name[0] == 'c') {
         return CTLLET+key_name[key_length-1]+'A'-'a';
      }
      else if (key_length == 2 && key_name[0] == 'a') {
         return ALTLET+key_name[key_length-1]+'A'-'a';
      }
      else if (key_length == 3 && key_name[0] == 'c' && key_name[1] == 'a') {
         return CTLALTLET+key_name[key_length-1]+'A'-'a';
      }
      else {
         return -1;
      }
   }
   else
      return -1;
}


static void do_accelerator_spec(Cstring qq)
{
   char key_name[MAX_FILENAME_LENGTH];
   char junk_name[MAX_FILENAME_LENGTH];
   int ccount;
   int matches;
   int menu_type = call_list_any;
   int keybindcode = -1;

   if (!qq[0] || qq[0] == '#') return;   /* This is a blank line or a comment. */

   if (sscanf(qq, "%s %n%s", key_name, &ccount, junk_name) == 2) {
      if (key_name[0] == '+') {
         menu_type = match_startup_commands;
         keybindcode = translate_keybind_spec(&key_name[1]);
      }
      else if (key_name[0] == '*') {
         menu_type = match_resolve_commands;
         keybindcode = translate_keybind_spec(&key_name[1]);
      }
      else
         keybindcode = translate_keybind_spec(key_name);
   }

   if (keybindcode < 0) {
      printf("Bad format in key binding \"%s\".\n", qq);
      return;
   }

   strcpy(static_ss.full_input, &qq[ccount]);
   matches = match_user_input(menu_type, FALSE, FALSE);
   user_match = static_ss.result;

   if ((matches != 1 && matches - static_ss.yielding_matches != 1 && !user_match.exact)) {
      /* Didn't find the target of the key binding.  Below C4X, failure to find
         something could just mean that it was a call off the list.  At C4X, we
         take it seriously.  So the initialization file should always be tested at C4X. */
      if (calling_level >= l_c4x) {
         printf("Didn't find target of key binding \"%s\".\n", qq);
         return;
      }
   }
   else {
      fcn_key_thing **table_thing;
      fcn_key_thing *newthing;

      if (user_match.match.packed_next_conc_or_subcall ||
          user_match.match.packed_secondary_subcall) {
         printf("Target of key binding \"%s\" is too complicated.\n", qq);
         return;
      }

      newthing = (fcn_key_thing *) get_mem(sizeof(fcn_key_thing));
      newthing->value = user_match.match;

      if (user_match.match.kind == ui_concept_select ||
          user_match.match.kind == ui_call_select ||
          user_match.match.kind == ui_command_select) {
         table_thing = &fcn_key_table_normal[keybindcode-FCN_KEY_TAB_LOW];
      }
      else if (user_match.match.kind == ui_start_select) {
         table_thing = &fcn_key_table_start[keybindcode-FCN_KEY_TAB_LOW];
      }
      else if (user_match.match.kind == ui_resolve_select) {
         table_thing = &fcn_key_table_resolve[keybindcode-FCN_KEY_TAB_LOW];
      }
      else {
         printf("Anomalous key binding \"%s\".\n", qq);
         return;
      }

      if (*table_thing) {
         printf("Redundant key binding \"%s\".\n", qq);
         return;
      }

      *table_thing = newthing;
   }
}


extern void uims_postinitialize(void)
{
   long_boolean save_allow = allowing_all_concepts;
   allowing_all_concepts = TRUE;

   call_menu_prompts[call_list_empty] = "--> ";   /* This prompt should never be used. */
   matcher_initialize();

   /* Process the keybindings for user-definable calls, concepts, and commands. */

   if (open_accelerator_region()) {
      char q[INPUT_TEXTLINE_SIZE];
      while (get_accelerator_line(q))
         do_accelerator_spec(q);
   }
   else {
      Cstring *q;
      for (q = concept_key_table ; *q ; q++)
         do_accelerator_spec(*q);
   }

   allowing_all_concepts = save_allow;

#if !defined(MSDOS)
   initialize_signal_handlers();
#endif
}


extern void uims_set_window_title(char s[])
{
   ttu_set_window_title(s);   
}


extern void uims_bell(void)
{
   if (!no_sound) ttu_bell();   
}


Private void pack_and_echo_character(char c)
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

   if (static_ss.result.indent) put_line("   ");
   put_line(static_ss.full_input);
   put_line(static_ss.extension);
   put_line("\n");
   current_text_line++;
}


Private long_boolean get_user_input(char *prompt, int which)
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

      /* Control-U can come in either of two forms.  Either way, kill the line */

      if (nc == CTLLET+'U' || nc == ('U'&'\037')) {
         user_input[0] = '\0';
         static_ss.full_input[0] = '\0';
         static_ss.full_input_size = 0;
         function_key_expansion = (char *) 0;
         clear_line();           /* Clear the current line */
         put_line(user_input_prompt);    /* Redisplay the prompt. */
         continue;
      }

      if (nc >= 128) {
         char linebuff[MAX_TEXT_LINE_LENGTH];
         fcn_key_thing *keyptr;
         int which_target = which;

         if (which_target > 0) which_target = 0;

         if (nc < FCN_KEY_TAB_LOW || nc > FCN_KEY_TAB_LAST)
            continue;      /* Ignore this key. */

         /* This one is hard-wired. */

         if (nc == FKEY+8) {
            function_key_expansion = "<anything>";    /* F8 */
            goto do_character;
         }

         if (which_target == match_startup_commands)
            keyptr = fcn_key_table_start[nc-FCN_KEY_TAB_LOW];
         else if (which_target == match_resolve_commands)
            keyptr = fcn_key_table_resolve[nc-FCN_KEY_TAB_LOW];
         else if (which_target == 0)
            keyptr = fcn_key_table_normal[nc-FCN_KEY_TAB_LOW];
         else
            continue;

         if (!keyptr) continue;

         /* If we get here, we have a function key to process from the table. */

         user_match.match = keyptr->value;
         user_match.indent = FALSE;
         user_match.real_next_subcall = (match_result *) 0;
         user_match.real_secondary_subcall = (match_result *) 0;

         switch (user_match.match.kind) {
         case ui_command_select:
            put_line(command_commands[user_match.match.index]);
            user_match.match.index = -1-command_command_values[user_match.match.index];
            break;
         case ui_resolve_select:
            put_line(resolve_menu[user_match.match.index].command_name);
            user_match.match.index = -1-resolve_menu[user_match.match.index].action;
            break;
         case ui_start_select:
            put_line(startup_commands[user_match.match.index]);
            break;
         case ui_concept_select:
            unparse_call_name(user_match.match.concept_ptr->name,
                              linebuff,
                              &user_match.match.call_conc_options);
            put_line(linebuff);
            user_match.match.index = 0;
            break;
         case ui_call_select:
            unparse_call_name(user_match.match.call_ptr->name,
                              linebuff,
                              &user_match.match.call_conc_options);
            put_line(linebuff);
            user_match.match.index = 0;
            break;
         default:
            continue;
         }

         put_line("\n");
         current_text_line++;

         user_match.valid = TRUE;
         return FALSE;
      }

   do_character:

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
         put_char(c);
         put_line("\n");
         current_text_line++;
         start_matches();
         (void) match_user_input(which, TRUE, c == '?');
         put_line("\n");     /* Write a blank line. */
         current_text_line++;
         put_line(user_input_prompt);   /* Redisplay the current line. */
         put_line(user_input);
         continue;
      }
      else if (c == ' ' || c == '-') {
         /* extend only to one space or hyphen, inclusive */
         matches = match_user_input(which, FALSE, FALSE);
         user_match = static_ss.result;
         p = static_ss.extended_input;

         if (*p) {
            while (*p) {
               if (*p != ' ' && *p != '-')
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
            uims_bell();
      }
      else if ((c == '\n') || (c == '\r')) {
         matches = match_user_input(which, FALSE, FALSE);
         user_match = static_ss.result;

         if (!strcmp(static_ss.full_input, "help")) {
            put_line("\n");
            current_text_line++;
            return TRUE;
         }

         /* We forbid a match consisting of two or more "direct parse" concepts, such as
            "grand cross".  Direct parse concepts may only be stacked if they are followed
            by a call.  The "match.next" field indicates that direct parse concepts
            were stacked. */

         if ((matches == 1 || matches - static_ss.yielding_matches == 1 || user_match.exact) &&
             ((!user_match.match.packed_next_conc_or_subcall &&
               !user_match.match.packed_secondary_subcall) ||
              user_match.match.kind == ui_call_select ||
              user_match.match.kind == ui_concept_select)) {

            p = static_ss.extended_input;
            while (*p)
               pack_and_echo_character(*p++);

            put_line("\n");

            if (journal_file) {
               fputs(static_ss.full_input, journal_file);
               fputc('\n', journal_file);
            }

            /* Include the input line in our count, so we will erase it
               if we are trying to make the VT-100 screen look nice. */
            current_text_line++;
            return FALSE;
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
            uims_bell();
      }
      else if (isprint(c))
         pack_and_echo_character(c);
      else if (diagnostic_mode)
         goto diagnostic_error;
      else
         uims_bell();
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
   for (;;) {
      if (!get_user_input("Enter startup command> ", (int) match_startup_commands))
         break;

      /* ****** remember to remove "start_select_help" in sd.h */

      writestuff("The program wants you to start a sequence.  Type, for example, "
                 "'heads start', and press Enter.  Then type a call, such as "
                 "'pass the ocean', and press Enter again.");
      newline();
   }

   uims_menu_index = user_match.match.index;
   return user_match.match.kind;
}


static uint32 the_topcallflags;
static long_boolean there_is_a_call;



/* This stuff is duplicated in verify_call in sdmatch.c . */
static long_boolean deposit_call_tree(modifier_block *anythings, parse_block *save1, int key)
{
   /* First, if we have already deposited a call, and we see more stuff, it must be
      concepts or calls for an "anything" subcall. */

   if (save1) {
      parse_block *tt = get_parse_block();
      /* Run to the end of any already-deposited things.  This could happen if the
         call takes a tagger -- it could have a search chain before we even see it. */
      while (save1->next) save1 = save1->next;
      save1->next = tt;
      save1->concept = &marker_concept_mod;
      tt->concept = &marker_concept_mod;
      tt->call = base_calls[(key == 6) ? base_call_null_second: base_call_null];
      tt->replacement_key = key;
      parse_state.concept_write_ptr = &tt->subsidiary_root;
   }

   save1 = (parse_block *) 0;
   user_match.match.call_conc_options = anythings->call_conc_options;

   if (anythings->kind == ui_call_select) {
      if (deposit_call(anythings->call_ptr, &anythings->call_conc_options)) return TRUE;
      save1 = *parse_state.concept_write_ptr;
      if (!there_is_a_call) the_topcallflags = parse_state.topcallflags1;
      there_is_a_call = TRUE;
   }
   else if (anythings->kind == ui_concept_select) {
      if (deposit_concept(anythings->concept_ptr)) return TRUE;
   }
   else return TRUE;   /* Huh? */

   if (anythings->packed_next_conc_or_subcall) {
      /* key for "mandatory_anycall" */
      if (deposit_call_tree(anythings->packed_next_conc_or_subcall, save1, 2)) return TRUE;
   }

   if (anythings->packed_secondary_subcall) {
      /* key for "mandatory_secondary_call" */
      if (deposit_call_tree(anythings->packed_secondary_subcall, save1, 6)) return TRUE;
   }

   return FALSE;
}




/* This returns TRUE if it fails, e.g. the user waves the mouse away. */
extern long_boolean uims_get_call_command(uims_reply *reply_p)
{
   char prompt_buffer[200];
   char *prompt_ptr;
   int banner_mode;
   long_boolean retval = FALSE;

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

   if (get_user_input(prompt_ptr, (int) parse_state.call_list_to_use)) {
      /* User typed "help". */
      *reply_p = ui_command_select;
      uims_menu_index = command_help;
      return FALSE;
   }

   *reply_p = user_match.match.kind;

   uims_menu_index = user_match.match.index;

   if (user_match.match.index < 0)
      /* Special encoding from a function key. */
      uims_menu_index = -1-user_match.match.index;
   else if (user_match.match.kind == ui_command_select) {
      /* Translate the command. */
      uims_menu_index = (int) command_command_values[user_match.match.index];
   }
   else {
      call_conc_option_state save_stuff = user_match.match.call_conc_options;
      there_is_a_call = FALSE;
      retval = deposit_call_tree(&user_match.match, (parse_block *) 0, 2);
      user_match.match.call_conc_options = save_stuff;
      if (there_is_a_call) {
         parse_state.topcallflags1 = the_topcallflags;
         *reply_p = ui_call_select;
      }
   }

   return retval;
}


extern uims_reply uims_get_resolve_command(void)
{
   for (;;) {
      if (!get_user_input("Enter search command> ", (int) match_resolve_commands))
         break;

      if (resolver_happiness == resolver_display_failed)
         writestuff("The program is trying to resolve, but has failed to find anything."
                    "  You can type 'find another' to keep trying.");
      else
         writestuff("The program is searching for resolves.  If you like the currently "
                    "displayed resolve, you can type 'accept' and press Enter."
                    "  If not, you can type 'find another'.");

      newline();
   }

   if (user_match.match.index < 0)
      uims_menu_index = -1-user_match.match.index;   /* Special encoding from a function key. */
   else
      uims_menu_index = (int) resolve_command_values[user_match.match.index];

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
         if (journal_file) fputc('n', journal_file);
         return POPUP_DECLINE;
      }
      if ((c=='y') || (c=='Y')) {
         put_line("yes\n");
         current_text_line++;
         if (journal_file) fputc('y', journal_file);
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
      uims_bell();
   }
}

extern int uims_do_write_anyway_popup(void)
{
    put_line("This sequence is not resolved.\n");
    current_text_line++;
    return confirm("Do you want to write it anyway? ");
}

extern int uims_do_delete_clipboard_popup(void)
{
    put_line("There are calls in the clipboard.\n");
    current_text_line++;
    return confirm("Do you want to delete all of them? ");
}

extern int uims_do_abort_popup(void)
{
    put_line("The current sequence will be aborted.\n");
    current_text_line++;
    return confirm("Do you really want to abort it? ");
}

extern int uims_do_session_init_popup(void)
{
    put_line("You already have a session file.\n");
    current_text_line++;
    return confirm("Do you really want to delete it and start over? ");
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

    resolver_happiness = state;

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

      for (;;) {
         if (!get_user_input("Enter who> ", (int) match_selectors))
            break;

         writestuff("The program wants you to type a person designator.  "
                    "Try typing something like 'boys' and pressing Enter.");
         newline();
      }

      /* We skip the zeroth selector, which is selector_uninitialized. */
      popup_retval = user_match.match.index+1;
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

      for (;;) {
         if (!get_user_input("Enter direction> ", (int) match_directions))
            break;

         writestuff("The program wants you to type a direction.  "
                    "Try typing something like 'right' and pressing Enter.");
         newline();
      }

      /* We skip the zeroth direction, which is direction_uninitialized. */
      retval = user_match.match.index+1;
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
      if (user_match.match.call_conc_options.tagger == 0)
        user_match.match.call_conc_options.tagger = (tagger_class << 5) + 1;
   }
   else if (!user_match.valid || (user_match.match.call_conc_options.tagger == 0)) {
      match_result saved_match = user_match;

      for (;;) {
         if (!get_user_input("Enter tagging call> ", ((int) match_taggers) + tagger_class))
            break;

         writestuff("The program wants you to type an 'ATC' (tagging) call.  "
                    "Try typing something like 'vertical tag' and pressing Enter.");
         newline();
      }

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

      for (;;) {
         if (!get_user_input("Enter circulate replacement> ", (int) match_circcer))
            break;

         writestuff("The program wants you to type a circulating call as part of "
                    "a call like 'in roll motivate'.  "
                    "Try typing something like 'in roll circulate' and pressing Enter.");
         newline();
      }

      retval = user_match.match.call_conc_options.circcer;
      user_match = saved_match;
   }
   else {
      retval = user_match.match.call_conc_options.circcer;
      user_match.match.call_conc_options.circcer = 0;
   }

   return retval;
}


extern uint32 uims_get_number_fields(int nnumbers, long_boolean forbid_zero)
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
               put_line("Type a number between 0 and 15\n");
               current_text_line++;
            }
            else if (!buffer[0]) return ~0;
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

      if (forbid_zero && this_num == 0) return ~0;
      if (this_num > 15) return ~0;    /* User gave bad answer. */
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

/* Throw away all but the first n lines of the text output.
   n = 0 means to erase the entire buffer. */

extern void uims_reduce_line_count(int n)
{
   if (current_text_line > n)
      erase_last_n(current_text_line-n);

   current_text_line = n;
}

extern void uims_terminate(void)
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
