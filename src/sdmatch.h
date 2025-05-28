/*
 * sdmatch.h - header file for command matcher
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



typedef struct {
   char full_input    [INPUT_TEXTLINE_SIZE+1]; /* the current user input */
   char extension     [INPUT_TEXTLINE_SIZE+1]; /* the extension for the current pattern */
   char extended_input[INPUT_TEXTLINE_SIZE+1]; /* the maximal common extension to the user input */
   int full_input_size;        /* Number of characters in full_input, not counting null. */
   int space_ok;               /* space is a legitimate next input character */
   int yielding_matches;       /* how many matches are yielding */
   match_result result;        /* value of the first or exact matching pattern */
} match_state;

/* These are provided by sdmatch.c */
extern match_state static_ss;

/* This is provided by sdui-XXX.c */
extern long_boolean verify_has_stopped;

extern void show_match(void);

extern void do_accelerator_spec(Cstring qq);

extern void matcher_initialize(void);

extern void matcher_setup_call_menu(call_list_kind cl);
    
extern int match_user_input(
   int which_commands,
   long_boolean show,
   long_boolean show_verify);
