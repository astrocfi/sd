/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

/*
 * sdui-tty.h - header file for interface between sdui-tty.c
 * and the low level character subsystem.
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

/* Process any command line arguments that we need.
   Returns nonzero if it failed -- error message has already been printed.
   Third argument tells where it can deposit TRUE if it wants to turn on
   escape sequences for drawing people.  Not all terminal interfaces want
   escape sequences.  The other arguments provide the "pn1" and "pn2" arrays
   showing how we want people described. */
extern int ttu_process_command_line(int *argcp,
                                    char **argv,
                                    int *use_escapes_for_drawing_people_p,
                                    char *pn1,
                                    char *pn2,
                                    char **direc_p);

/* Print the help message appropriate for the switches that we support. */
extern void ttu_display_help(void);

/* Change the title bar (or whatever it's called) on the window. */
extern void ttu_set_window_title(char s[]);

/* Initialize this package. */
extern void ttu_initialize(void);

/* Terminate this package. */
extern void ttu_terminate(void);

/* Get number of lines to use for "more" processing.  This number is
   not used for any other purpose -- the rest of the program is not concerned
   with the "screen" size. */

extern int get_lines_for_more(void);

extern FILE *journal_file;
extern int no_graphics;
extern int screen_height;
extern int no_cursor;
extern int no_console;
extern int no_intensify;
extern int no_color;
extern int no_sound;
extern int no_line_delete;


/* Clear the current line, leave cursor at left edge. */
extern void clear_line(void);

/* Backspace the cursor and clear the rest of the line, presumably
   erasing the last character. */
extern void rubout(void);

/* Move cursor up "n" lines and then clear rest of screen. */
extern void erase_last_n(int n);

/* Write a line.  The text may or may not have a newline at the end. */
/* This may or may not be after a prompt and/or echoed user input. */
extern void put_line(const char the_line[]);

/* Write a single character on the current output line. */
extern void put_char(int c);

/* Codes for special accelerator keystrokes. */

/* Function keys can be plain, shifted, control, alt, or control-alt. */

#define FKEY 128
#define SFKEY 144
#define CFKEY 160
#define AFKEY 176
#define CAFKEY 192

/* "Enhanced" keys can be plain, shifted, control, alt, or control-alt. */
/* e1 = page up
   e2 = page down
   e3 = end
   e4 = home
   e5 = left arrow
   e6 = up arrow
   e7 = right arrow
   e8 = down arrow
   e13 = insert
   e14 = delete */

#define EKEY 208
#define SEKEY 224
#define CEKEY 240
#define AEKEY 256
#define CAEKEY 272

/* Digits can be control, alt, or control-alt. */

#define CTLDIG 288
#define ALTDIG 298
#define CTLALTDIG 308

/* Numeric keypad can be control, alt, or control-alt. */

#define CTLNKP 318
#define ALTNKP 328
#define CTLALTNKP 338

/* Letters can be control, alt, or control-alt. */

#define CTLLET (348-'A')
#define ALTLET (374-'A')
#define CTLALTLET (400-'A')

#define FCN_KEY_TAB_LOW (FKEY+1)
#define FCN_KEY_TAB_LAST (CTLALTLET+'Z')

/* Get one character from input, no echo, no waiting for <newline>.
   Return large number for function keys and alt alphabetics:
      128+N for plain function key (F1 = 129)
      144+N for shifted
      160+N for control
      176+N for alt function key
      192..217 for ctl letter (ctl-A = 192)
      218..243 for alt letter (alt-A = 218)
      244..269 for ctl-alt letter (ctl-alt-A = 244) */

extern int get_char(void);

/* Get string from input, up to <newline>, with echoing and editing.
   Return it without the final <newline>. */
extern void get_string(char *dest);

/* Ring the bell, or whatever. */
extern void ttu_bell(void);

extern void initialize_signal_handlers(void);
extern void refresh_input(void);
