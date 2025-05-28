/*
 * sdui-ttu.h - header file for sdui-ttu.c
 * Time-stamp: <93/06/25 19:39:06 gildea>
 * Copyright 1993 Stephen Gildea
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * By Stephen Gildea <gildea@lcs.mit.edu> January 1993
 */


/* Process any command line arguments that we need. */
extern void ttu_process_command_line(int *argcp, char ***argvp);

/* Initialize this package. */
extern void ttu_initialize(void);

/* Terminate this package. */
extern void ttu_terminate(void);

/* Get number of lines to use for "more" processing.  This number is
   not used for any other purpose -- the rest of the program is not concerned
   with the "screen" size. */

extern int get_lines_for_more(void);

/* Clear the current line, leave cursor at left edge. */
extern void clear_line(void);

/* Backspace the cursor and clear the rest of the line, presumably
   erasing the last character. */
extern void rubout(void);

/* Move cursor up "n" lines and then clear rest of screen. */
extern void erase_last_n(int n);

/* Write a line.  The text may or may not have a newline at the end. */
/* This may or may not be after a prompt and/or echoed user input. */
extern void put_line(char the_line[]);

/* Write a single character on the current output line. */
extern void put_char(int c);

/* Get one character from input, no echo, no waiting for <newline>. */
extern int get_char(void);

/* Get string from input, up to <newline>, with echoing and editing.
   Return it without the final <newline>. */
extern void get_string(char *dest);

/* Ring the bell, or whatever. */
extern void bell(void);

extern void initialize_signal_handlers(void);
extern void refresh_input(void);
