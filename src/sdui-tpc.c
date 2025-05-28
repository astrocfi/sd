/*
 * sdui-tpc.c - helper functions for Mac tty interface for port to DOS.
 * Time-stamp: <93/07/19 19:38:40 wba>
 * Copyright 1993 Stephen Gildea
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * By Stephen Gildea <gildea@lcs.mit.edu> February 1993
 *
 * Uses console I/O package with djgpp, a port of gcc to DOS on IBM PC.
 */

#include "sdui-ttu.h"
#include <stdio.h>
#include <gppconio.h>


extern void ttu_process_command_line(int *argcp, char ***argvp)
{
}

extern void ttu_initialize(void)
{
    gppconio_init();
}

extern void ttu_terminate(void)
{
}

extern int get_lines_for_more(void)
{
    /* PC's seem to have 25 lines on the screen.  That's so close to 24 that we
       might aas well use 24, so we won't lose in the event that some kind of
       VT-100 emulation is used. */
    return 24;
}

extern void clear_line(void)
{
    int yp = wherey();
    gotoxy(1, yp);
    clreol();
}

extern void rubout(void)
{
    printf("\b \b");
}

extern void erase_last_n(int n)
{
}

extern void put_line(char the_line[])
{
    printf(the_line);
}

extern void put_char(int c)
{
    (void) putchar(c);
}

extern int get_char(void)
{
    int n;

    do {
        fflush(stdout);
        n = getch();
        if (n > 127) {
    	if (n == 339) /* Delete */
    	    n = '\177';
    	else
    	    n = ' ';
        }
    } while (n == EOF);   /* busy wait (EOF means no character yet) */

    return n;
}

extern void get_string(char *dest)
{
    gets(dest);
}

extern void bell(void)
{
    (void) putchar('\007');
}

extern void initialize_signal_handlers(void)
{
}
