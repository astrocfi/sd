/* 
 * sdui-tty.c - SD TTY User Interface
 * Originally for Macintosh.  Unix version by gildea.
 * Time-stamp: <96/05/22 17:17:53 wba>
 * Copyright (c) 1990-1994 Stephen Gildea, William B. Ackerman, and
 *   Alan Snyder
 *
 * Copyright (c) 1994-2000 William B. Ackerman
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
 * For use with version 34 of the Sd program.
 *
 * The version of this file is as shown immediately below.  This string
 * gets displayed at program startup, as the "ui" part of the complete
 * version.
 */

#define UI_VERSION_STRING "1.12"
#define UI_TIME_STAMP "wba@alum.mit.edu  14 Mar 2000 $"

/* This file defines the following functions:
   uims_version_string
   uims_process_command_line
   uims_display_help
   uims_display_ui_intro_text
   uims_open_session
   uims_create_menu
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
   uims_update_resolve_menu
   uims_terminate
   uims_database_tick_max
   uims_database_tick
   uims_database_tick_end
   uims_database_error
   uims_bad_argument

and the following other variables:

   screen_height
   no_cursor
   no_console
   no_line_delete
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

extern void exit(int code);

#include "sd.h"
#include "paths.h"

/* See comments in sdmain.c regarding this string. */
static Const char id[] = "@(#)$He" "ader: Sd: sdui-tty.c " UI_VERSION_STRING "  " UI_TIME_STAMP;


#define DEL 0x7F

/*
 * The total version string looks something like
 * "1.4:db1.5:ui0.6tty"
 * We return the "0.6tty" part.
 */

int screen_height = 25;
int no_cursor = 0;
int no_console = 0;
int no_line_delete = 0;

static char version_mem[12];

extern char *uims_version_string(void)
{
    (void) sprintf(version_mem, "%stty", UI_VERSION_STRING);
    return version_mem;
}

static char journal_name[MAX_TEXT_LINE_LENGTH];

static resolver_display_state resolver_happiness = resolver_display_failed;

int main(int argc, char *argv[])

{
   // In Sdtty, the defaults are reverse video (white-on-black) and pastel colors.
   ui_options.no_graphics = 0;
   ui_options.no_intensify = 0;
   ui_options.reverse_video = 1;
   ui_options.pastel_color = 1;
   ui_options.no_color = 0;
   ui_options.no_sound = 0;
   ui_options.sequence_num_override = -1;

   return sdmain(argc, argv);
}


/*
 * User Input functions
 */

/* This array is the same as GLOB_full_input, but has the original capitalization
   as typed by the user.  GLOB_full_input is converted to all lower case for
   ease of searching. */
static char user_input[INPUT_TEXTLINE_SIZE+1];
static char *user_input_prompt;
static char *function_key_expansion;

void refresh_input(void)
{
   erase_matcher_input();
   user_input[0] = '\0';
   function_key_expansion = (char *) 0;
   clear_line(); /* clear the current line */
   put_line(user_input_prompt);
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


static void get_string_input(char prompt[], char dest[], int max)
{
   put_line(prompt);
   get_string(dest, max);
   current_text_line++;
}



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
   journal_name[0] = '\0';

   while (argno < (*argcp)) {
      int i;

      if (strcmp(argv[argno], "-no_line_delete") == 0)
         no_line_delete = 1;
      else if (strcmp(argv[argno], "-no_cursor") == 0)
         no_cursor = 1;
      else if (strcmp(argv[argno], "-no_console") == 0)
         no_console = 1;
      else if (strcmp(argv[argno], "-alternate_glyphs_1") == 0) {
         ui_options.pn1 = alt1_names1;
         ui_options.pn2 = alt1_names2;
      }
      else if (strcmp(argv[argno], "-lines") == 0 && argno+1 < (*argcp)) {
         screen_height = atoi(argv[argno+1]);
         goto remove_two;
      }
      else if (strcmp(argv[argno], "-journal") == 0 && argno+1 < (*argcp)) {
         strcpy(journal_name, argv[argno+1]);
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
}

extern void uims_display_help(void)
{
   ttu_display_help();
}


extern long_boolean uims_open_session(int argc, char **argv)
{
   int session_outcome;
   Cstring session_error_msg;
   char session_error_msg1[200], session_error_msg2[200];

   char line[MAX_FILENAME_LENGTH];

   if (glob_call_list_mode != call_list_mode_none) {
      if (open_call_list_file(call_list_string))
         exit_program(1);
   }

   /* Put up the session list. */

   if (get_first_session_line()) goto no_session;

   printf("Do you want to use one of the following sessions?\n\n");

   while (get_next_session_line(line))
      printf("%s\n", line);

   printf("Enter the number of the desired session:  ");

   if (!fgets(line, MAX_FILENAME_LENGTH, stdin) ||
       !line[0] ||
       line[0] == '\r' ||
       line[0] == '\n')
      goto no_session;

   if (!sscanf(line, "%d", &session_index)) {
      session_index = 0;         /* User typed garbage -- exit the program immediately. */
      close_init_file();
      return TRUE;
   }

   if (session_index < 0) {
      close_init_file();
      return TRUE;    /* Exit the program immediately. */
   }

   session_outcome = process_session_info(&session_error_msg);

   if (session_outcome & 2)
      printf("%s\n", session_error_msg);

   if (session_outcome & 1) {
      /* We are not using a session, either because the user selected
         "no session", or because of some error in processing the
         selected session. */
      goto no_session;
   }

   goto really_do_it;

   no_session:

   session_index = 0;
   sequence_number = -1;

   really_do_it:

   if (ui_options.sequence_num_override > 0)
      sequence_number = ui_options.sequence_num_override;

   if (calling_level == l_nonexistent_concept) {

      /* The level never got specified, either from a command line argument
         or from the session file.  Perhaps the program was invoked under
         a window-ish OS in which one clicks on icons rather than typing
         a command line.  In that case, we need to query the user for the level. */

      calling_level = l_mainstream;   /* Default in case we fail. */
      printf("Enter the level: ");

      if (fgets(line, MAX_FILENAME_LENGTH, stdin)) {
         int size = strlen(line);

         while (size > 0 && (line[size-1] == '\n' || line[size-1] == '\r'))
            line[--size] = '\000';

         (void) parse_level(line, &calling_level);
      }

      (void) strncat(outfile_string, filename_strings[calling_level], MAX_FILENAME_LENGTH);
   }

   if (new_outfile_string)
      (void) install_outfile_string(new_outfile_string);

   starting_sequence_number = sequence_number;

   if (glob_call_list_mode == call_list_mode_none ||
       glob_call_list_mode == call_list_mode_abridging) {
      current_text_line = 0;
      ttu_initialize();
   }

   initialize_misc_lists();
   prepare_to_read_menus();

   /* Opening the database sets up the values of
      abs_max_calls and max_base_calls.
      Must do before telling the uims so any open failure messages
      come out first. */

   if (open_database(session_error_msg1, session_error_msg2)) {
      uims_fatal_error(session_error_msg1, session_error_msg2);
      exit_program(1);
   }

   build_database(glob_call_list_mode);

   /* This is the thing that takes all the time! */

   initialize_menus(glob_call_list_mode);

   /* If we wrote a call list file, that's all we do. */
   if (glob_call_list_mode == call_list_mode_writing || glob_call_list_mode == call_list_mode_writing_full) {
      close_init_file();
      return TRUE;
   }

   call_menu_prompts[call_list_empty] = "--> ";   /* This prompt should never be used. */

   matcher_initialize();

   {
      long_boolean save_allow = allowing_all_concepts;
      allowing_all_concepts = TRUE;

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
   }

   ttu_final_option_setup();

#if !defined(MSDOS)
   initialize_signal_handlers();
#endif

   close_init_file();

   return FALSE;
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




extern void uims_set_window_title(char s[])
{
   char full_text[MAX_TEXT_LINE_LENGTH];

   if (journal_name[0]) {
      (void) sprintf(full_text, "Sdtty %s {%s}", s, journal_name);
   }
   else {
      (void) sprintf(full_text, "Sdtty %s", s);
   }

   ttu_set_window_title(full_text);
}


extern void uims_bell(void)
{
   if (!ui_options.no_sound) ttu_bell();
}


static void pack_and_echo_character(char c)
{
   /* Really should handle error better -- ring the bell,
      but this is called inside a loop. */

   if (GLOB_user_input_size < INPUT_TEXTLINE_SIZE) {
      user_input[GLOB_user_input_size] = c;
      GLOB_user_input[GLOB_user_input_size++] = tolower(c);
      user_input[GLOB_user_input_size] = '\0';
      GLOB_user_input[GLOB_user_input_size] = '\0';
      put_char(c);
   }
}

/* This tells how many more lines of matches (the stuff we print in response
   to a question mark) we can print before we have to say "--More--" to
   get permission from the user to continue.  If showing_has_stopped goes on, the
   user has given a negative reply to one of our queries, and so we don't
   print any more stuff. */
static int match_counter;

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
static int match_lines;


static int prompt_for_more_output(void)
{
    put_line("--More--");

    for (;;) {
        int c = get_char();
        clear_line();   /* Erase the "more" line; next item goes on that line. */

        switch (c) {
        case '\r':
        case '\n':
           match_counter = 1; /* show one more line */
           return TRUE;       /* but otherwise keep going */
        case '\b':
        case DEL:
        case EKEY+14:    /* The "delete" key on a PC. */
        case 'q':
        case 'Q':
           return FALSE; /* stop showing */
        case ' ':
           return TRUE;  /* keep going */
        default:   put_line("Type Space to see more, Return for next line, Delete to stop:  --More--");
        }
    }
}

extern void show_match(void)
{
   if (showing_has_stopped) return;  /* Showing has been turned off. */

   if (match_counter <= 0) {
      match_counter = match_lines - 1;
      if (!prompt_for_more_output()) {
         match_counter = -1;   /* Turn it off. */
         showing_has_stopped = TRUE;
         return;
      }
   }
   match_counter--;

   if (GLOB_match.indent) put_line("   ");
   put_line(GLOB_user_input);
   put_line(GLOB_full_extension);
   put_line("\n");
   current_text_line++;
}


static long_boolean get_user_input(char *prompt, int which)
{
   char *p;
   char c;
   int nc;
   int matches;

   user_match.valid = FALSE;
   user_input_prompt = prompt;
   erase_matcher_input();
   user_input[0] = '\0';
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
         char linebuff [INPUT_TEXTLINE_SIZE+1];
         modifier_block *keyptr;
         int chars_deleted;
         int which_target = which;

         if (which_target > 0) which_target = 0;

         if (nc < FCN_KEY_TAB_LOW || nc > FCN_KEY_TAB_LAST)
            continue;      /* Ignore this key. */

         keyptr = fcn_key_table_normal[nc-FCN_KEY_TAB_LOW];

         /* Check for special bindings.
            These always come from the main binding table, even if
            we are doing something else, like a resolve. */

         if (keyptr && keyptr->index < 0) {
            switch (keyptr->index) {
            case special_index_deleteline:
               erase_matcher_input();
               strcpy(user_input, GLOB_user_input);
               function_key_expansion = (char *) 0;
               clear_line();           /* Clear the current line */
               put_line(user_input_prompt);    /* Redisplay the prompt. */
               continue;
            case special_index_deleteword:
               chars_deleted = delete_matcher_word();
               while (chars_deleted-- > 0) rubout();
               strcpy(user_input, GLOB_user_input);
               function_key_expansion = (char *) 0;
               continue;
            case special_index_quote_anything:
               function_key_expansion = "<anything>";
               goto do_character;
            default:
               continue;    /* Ignore all others. */
            }
         }

         if (which_target == match_startup_commands)
            keyptr = fcn_key_table_start[nc-FCN_KEY_TAB_LOW];
         else if (which_target == match_resolve_commands)
            keyptr = fcn_key_table_resolve[nc-FCN_KEY_TAB_LOW];
         else if (which_target == 0)
            keyptr = fcn_key_table_normal[nc-FCN_KEY_TAB_LOW];
         else
            continue;

         if (!keyptr) {
            /* If user hits alt-F4 and there is no binding for it, we handle it in
               the usual way anyway.  This makes the behavior similar to Sd, where
               the system automatically provides that action. */
            if (nc == AFKEY+4) {
               if (which_target == match_startup_commands ||
                   uims_do_abort_popup() == POPUP_ACCEPT)
                  exit_program(0);
            }

            continue;   /* No binding for this key; ignore it. */
         }

         /* If we get here, we have a function key to process from the table. */

         user_match.match = *keyptr;
         user_match.indent = FALSE;
         user_match.real_next_subcall = (match_result *) 0;
         user_match.real_secondary_subcall = (match_result *) 0;

         switch (user_match.match.kind) {
         case ui_command_select:
            strcpy(linebuff, command_commands[user_match.match.index]);
            user_match.match.index = -1-command_command_values[user_match.match.index];
            break;
         case ui_resolve_select:
            strcpy(linebuff, resolve_menu[user_match.match.index].command_name);
            user_match.match.index = -1-resolve_menu[user_match.match.index].action;
            break;
         case ui_start_select:
            strcpy(linebuff, startup_commands[user_match.match.index]);
            break;
         case ui_concept_select:
            unparse_call_name(user_match.match.concept_ptr->name,
                              linebuff,
                              &user_match.match.call_conc_options);
            // Reject off-level concept accelerator key presses.
            if (!allowing_all_concepts &&
                user_match.match.concept_ptr->level > higher_acceptable_level[calling_level])
               continue;
            user_match.match.index = 0;
            break;
         case ui_call_select:
            unparse_call_name(user_match.match.call_ptr->name,
                              linebuff,
                              &user_match.match.call_conc_options);
            user_match.match.index = 0;
            break;
         default:
            continue;
         }

         user_match.valid = TRUE;

         put_line(linebuff);
         put_line("\n");

         if (journal_file) {
            fputs(linebuff, journal_file);
            fputc('\n', journal_file);
         }

         current_text_line++;
         return FALSE;
      }

   do_character:

      c = nc;

      if (function_key_expansion)
         goto start_expand;

   got_char:

      if ((c == '\b') || (c == DEL)) {
         if (GLOB_user_input_size > 0) {
            GLOB_user_input_size--;
            user_input[GLOB_user_input_size] = '\0';
            GLOB_user_input[GLOB_user_input_size] = '\0';
            function_key_expansion = (char *) 0;
            rubout(); /* Update the display. */
         }
         continue;
      }
      else if (c == '?' || c == '!') {
         put_char(c);
         put_line("\n");
         current_text_line++;
         match_lines = get_lines_for_more();
         match_counter = match_lines-1; /* last line used for "--More--" prompt */
         showing_has_stopped = FALSE;
         (void) match_user_input(which, TRUE, c == '?', FALSE);
         put_line("\n");     /* Write a blank line. */
         current_text_line++;
         put_line(user_input_prompt);   /* Redisplay the current line. */
         put_line(user_input);
         continue;
      }
      else if (c == ' ' || c == '-') {
         /* extend only to one space or hyphen, inclusive */
         matches = match_user_input(which, FALSE, FALSE, TRUE);
         //         user_match = GLOB_match;
         p = GLOB_echo_stuff;

         if (*p) {
            while (*p) {
               if (*p != ' ' && *p != '-')
                  pack_and_echo_character(*p++);
               else {
                  pack_and_echo_character(c);
                  goto foobar;
               }
            }
            continue;   /* Do *not* pack the character. */

            foobar: ;
         }
         else if (GLOB_space_ok && matches > 1)
            pack_and_echo_character(c);
         else if (diagnostic_mode)
            goto diagnostic_error;
         else
            uims_bell();
      }
      else if ((c == '\n') || (c == '\r')) {
         matches = match_user_input(which, FALSE, FALSE, TRUE);
         user_match = GLOB_match;

         if (!strcmp(GLOB_user_input, "help")) {
            put_line("\n");
            user_match.match.kind = ui_help_simple;
            current_text_line++;
            return TRUE;
         }

         /* We forbid a match consisting of two or more "direct parse" concepts, such as
            "grand cross".  Direct parse concepts may only be stacked if they are followed
            by a call.  The "match.next" field indicates that direct parse concepts
            were stacked. */

         if ((matches == 1 || matches - GLOB_yielding_matches == 1 || user_match.exact) &&
             ((!user_match.match.packed_next_conc_or_subcall &&
               !user_match.match.packed_secondary_subcall) ||
              user_match.match.kind == ui_call_select ||
              user_match.match.kind == ui_concept_select)) {

            p = GLOB_echo_stuff;
            while (*p)
               pack_and_echo_character(*p++);

            put_line("\n");

            if (journal_file) {
               fputs(GLOB_user_input, journal_file);
               fputc('\n', journal_file);
            }

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
         (void) match_user_input(which, FALSE, FALSE, TRUE);
         user_match = GLOB_match;
         p = GLOB_echo_stuff;

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
   return FALSE;
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
    "SC",
    "RSC",
    "??"};

static char *banner_prompts4[] = {
    "",
    "MG",
    "??",
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

   if (user_match.match.kind == ui_start_select) {
      /* Translate the command. */
      uims_menu_index = (int) startup_command_values[user_match.match.index];
   }

   return user_match.match.kind;
}



/* This returns TRUE if it fails, e.g. the user waves the mouse away. */
extern long_boolean uims_get_call_command(uims_reply *reply_p)
{
   char prompt_buffer[200];
   char *prompt_ptr;
   long_boolean retval = FALSE;

   if (allowing_modifications)
      parse_state.call_list_to_use = call_list_any;

   prompt_ptr = prompt_buffer;
   prompt_buffer[0] = '\0';

   /* Put any necessary special things into the prompt. */

   int banner_mode = (allowing_minigrand << 8) |
      (singing_call_mode << 6) |
      (using_active_phantoms << 4) |
      (allowing_all_concepts << 2) |
      (allowing_modifications);

   if (banner_mode != 0) {
      int i;
      int comma = 0;

      (void) strcat(prompt_buffer, "[");

      for (i=0 ; i<5 ; i++,banner_mode>>=2) {
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

            if (i==4) {
               (void) strcat(prompt_buffer, banner_prompts4[banner_mode&3]);
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
      // User typed "help".
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


static int get_popup_string(char prompt[], char dest[])
{
    char buffer[200];

    (void) sprintf(buffer, "%s: ", prompt);
    get_string_input(buffer, dest, 200);
    return POPUP_ACCEPT_WITH_STRING;
}

extern int uims_do_comment_popup(char dest[])
{
   int retval = get_popup_string("Enter comment", dest);

   if (retval) {
      if (journal_file) {
         fputs(dest, journal_file);
         fputc('\n', journal_file);
      }
   }

   return retval;
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

static int confirm(char *question)
{
   int c;

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

      if (c < 128) put_char(c);

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

extern void uims_update_resolve_menu(command_kind goal, int cur, int max,
                                     resolver_display_state state)
{
   char title[MAX_TEXT_LINE_LENGTH];

   resolver_happiness = state;

   create_resolve_menu_title(goal, cur, max, state, title);
   uims_add_new_line(title, 0);
}

extern int uims_do_selector_popup(void)
{
   int retval;

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
      retval = user_match.match.index+1;
      user_match = saved_match;
   }
   else {
      retval = (int) user_match.match.call_conc_options.who;
      user_match.match.call_conc_options.who = selector_uninitialized;
   }
   return retval;
}

extern int uims_do_direction_popup(void)
{
   int retval;

   if (!user_match.valid ||
       (user_match.match.call_conc_options.where == direction_uninitialized)) {
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
   int retval;

   if (!user_match.valid ||
       (user_match.match.call_conc_options.tagger == 0)) {
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

   retval = user_match.match.call_conc_options.tagger;
   user_match.match.call_conc_options.tagger = 0;
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
            get_string_input("How many? ", buffer, 200);
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

extern void uims_add_new_line(char the_line[], uint32 drawing_picture)
{
    put_line(the_line);
    put_line("\n");
    current_text_line++;
}

/* Throw away all but the first n lines of the text output.
   n = 0 means to erase the entire buffer. */

extern void uims_reduce_line_count(int n)
{
   if (current_text_line > n)
      erase_last_n(current_text_line-n);

   current_text_line = n;
}


extern bool uims_choose_font()
{ return FALSE; }


extern bool uims_print_this()
{ return FALSE; }


extern bool uims_print_any()
{ return FALSE; }


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

static int db_tick_max;
static int db_tick_cur;   /* goes from 0 to db_tick_max */

#define TICK_STEPS 52
static int tick_displayed;   /* goes from 0 to TICK_STEPS */

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
   print_line("");
   print_line(message);
   if (call_name) {
      print_line("while reading this call from the database:");
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


extern void uims_fatal_error(Cstring pszLine1, Cstring pszLine2)
{
   session_index = 0;    /* We don't write back the session file in this case. */
   fprintf(stderr, "%s\n", pszLine1);
   if (pszLine2)
      fprintf(stderr, "%s\n", pszLine2);
}


extern void uims_final_exit(int code)
{
   exit(code);
}
