/*
 * sdui-tpc.c - helper functions for Mac tty interface for port to DOS.
 * Time-stamp: <93/04/02 20:09:40 gildea>
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
#include <gppconio.h>


/*
 * cursor control
 */

static int initialized;

static void console_init(void)
{
    gppconio_init();
    initialized = 1;
}

/* ARGSUSED */
void cgotoxy(int x, int y,  FILE *filep)
{
    
    if (!initialized)
	console_init();
    gotoxy(x, y);
}

/* ARGSUSED */
void ccleol(FILE *filep)
{
    if (!initialized)
	console_init();
    clreol();
}

/* ARGSUSED */
void ccleos(FILE *filep)
{
    if (!initialized)
	console_init();
    clrscr();
}

/* ARGSUSED */
void cgetxy(int *xp, int *yp, FILE *filep)
{
    if (!initialized)
	console_init();
    *xp = wherex();
    *yp = wherey();
}

/*
 * terminal I/O mode control
 */

/* ARGSUSED */
void csetmode(int mode, FILE *fp)
{
    /* nothing needs to be done? */
}

FILE *fopenc()
{
    return stdout;
}

/* ARGSUSED */
void cshow(FILE *filep)
{
}
