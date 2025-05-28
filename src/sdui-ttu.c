/*
 * sdui-ttu.c - helper functions for Mac tty interface to make it run on Unix.
 * Time-stamp: <93/02/11 16:10:04 gildea>
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
 *
 * Uses terminfo database if TERMINFO defined.
 * Uses POSIX termios unless NO_TERMIOS defined.
 */

#include "sdui-ttu.h"

/*
 * cursor control
 */

#define TERMINFO

#ifdef TERMINFO
extern char *tgetstr();
#endif

static char *top_of_screen;
static char *clear_to_end_of_line;
static char *clear_to_end_of_screen;

static void terminfo_init(FILE *filep)
{
#ifndef TERMINFO
    /* Provide standard defaults in case terminfo fails */
    top_of_screen = "\033[H";
    clear_to_end_of_line = "\033[K";
    clear_to_end_of_screen = "\033[J";
#else

    setupterm(NULL, fileno(filep), NULL); /* exits on failure, too harsh? */
    top_of_screen = tgetstr("ho", NULL);
    clear_to_end_of_line = tgetstr("ce", NULL);
    clear_to_end_of_screen = tgetstr("cd", NULL);
#endif
}

/* ARGSUSED */
void cgotoxy(int x, int y,  FILE *filep)
{
    if (!top_of_screen)
	terminfo_init(filep);
    /* we know this is only used to go to the start of the
       line or the top of the screen */
    if (y == 1)
#ifdef TERMINFO
	putp(top_of_screen);
#else
	fputs(top_of_screen, filep);
#endif
    else
	fputs("\r", filep);	/* beginning of line */
}

void ccleol(FILE *filep)
{
    if (!clear_to_end_of_line)
	terminfo_init(filep);
#ifdef TERMINFO
    putp(clear_to_end_of_line);
#else
    fputs(clear_to_end_of_line, filep);
#endif
}

void ccleos(FILE *filep)
{
    if (!clear_to_end_of_screen)
	terminfo_init(filep);
#ifdef TERMINFO
    putp(clear_to_end_of_screen);
#else
    fputs(clear_to_end_of_screen, filep);
#endif
}

/* ARGSUSED */
void cgetxy(int *xp, int *yp, FILE *filep)
{
    /* dummy */
}

/*
 * terminal I/O mode control
 */

#ifdef NO_TERMIOS

#include <sgtty.h>

void csetmode(int mode, FILE *fp)
{
    struct sgttyb term;
    int fd = fileno(fp);

    ioctl(fd, TIOCGETP, &term);
    if (mode == C_RAW) {
	term.sg_flags &= ~ECHO;
	term.sg_flags |= CBREAK;
    } else {
	term.sg_flags |= ECHO;
	term.sg_flags &= ~CBREAK;
    }
    ioctl(fd, TIOCSETN, &term);
}

#else /* NO_TERMIOS */

#include <termios.h>
#include <unistd.h>

void csetmode(int mode, FILE *fp)
{
    static cc_t orig_eof = '\004';
    struct termios term;
    int fd = fileno(fp);

    tcgetattr(fd, &term);
    if (mode == C_RAW) {
	orig_eof = term.c_cc[VEOF]; /* VMIN may clobber */
	term.c_cc[VMIN] = 1;	/* 1 char at a time */
	term.c_cc[VTIME] = 0;	/* no time limit on input */
	term.c_lflag &= ~(ICANON|ECHO);
    } else {
	term.c_cc[VEOF] = orig_eof;
	term.c_lflag |= ICANON|ECHO;
    }
    tcsetattr(fd, TCSADRAIN, &term);
}

#endif /* NO_TERMIOS */

FILE *fopenc()
{
    return stdout;
}

/* ARGSUSED */
void cshow(FILE *filep)
{
}
