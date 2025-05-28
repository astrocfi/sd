/*
 * sdui-ttu.c - helper functions for Mac tty interface to make it run on Unix.
 * Time-stamp: <93/03/17 16:23:04 gildea>
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
 * Uses terminfo database unless NO_TERMINFO defined.
 * Uses POSIX termios unless NO_TERMIOS defined.
 */

#include "sdui-ttu.h"

/*
 * cursor control
 */

#ifndef NO_TERMINFO
extern char *tgetstr();
#endif

static char *top_of_screen = (char *) 0;
static char *clear_to_end_of_line = (char *) 0;
static char *clear_to_end_of_screen = (char *) 0;

static void terminfo_init(FILE *filep)
{
#ifndef NO_TERMINFO
    int status;

    /* You won't believe this, but the procedure "setupterm" does not use
       the usual return-an-integer-which-is-zero-if-successful-and-nonzero-
       with-a-reason-for-failure-in-errno protocol that seems to have been
       good enough for most of the rest of UNIX.  Instead, you pass the
       address to which the success or failure status is to be stored
       (success=1, other values indicate failure, but the meaning isn't
       documented beyond that).  If the address passed is null and the
       call fails, setupterm PRINTS A MESSAGE (remember, we are trying to
       set up special terminal I/O, so the meaning of printing a message
       is murky), AND THEN ABORTS THE ENTIRE #$%*@#(!%$$#^%!$# PROGRAM!!!!!!
       I'm not making this up! */

    /* Find out what kind of "terminal" we are using.  What we would like is
       to communicate with the user through something like the SUPDUP
       protocol, or some other method at least as technologically advanced
       as that 20-year-old system.  After all, we might be on a WORKSTATION
       or other device that has powerful bit-mapped graphical I/O
       capabilities.  As such, we ought to be able to access those
       capabilities in a reasonable way.  Instead, we seem to have been
       provided with the "curses" interface, whose complexity-of-use to
       generality,-power,-and-winningness-of-design ratio is utterly
       astonishing.  Just using the stupid VT-100 escape sequences is
       probably the best way to get what we want, considering the lengths
       the OS seems to be trying to go through in order to prevent us from
       doing something reasonable.  But we will humor the system by doing
       the "setupterm" call, so it can look in its "termcap" database to
       find out whether we are connected, for example, to a Datapoint
       Beehive terminal which the OS after 20 years still hasn't figured
       out how to handle.  If we are lucky, the "setupterm" call will report
       failure (perhaps we are using a WORKSTATION!) in which case we will
       just use the VT-100 escape sequences and assume that that WORKSTATION
       will do the right thing with them.  If we are unlucky, the
       "setupterm" call will succeed, in which case we will do whatever it
       tells us to do, hoping that the "termcap" file has correctly
       described whatever abomination it, rightly or wrongly, thinks we
       are connected to. */

    setupterm(NULL, fileno(filep), &status);

    if (status == 1) {
       /* Setupterm succeeded.  Use the "curses" stuff and pray for the day
          when we don't need to do this any more. */
       top_of_screen = tgetstr("ho", NULL);
       clear_to_end_of_line = tgetstr("ce", NULL);
       clear_to_end_of_screen = tgetstr("cd", NULL);
    }
#endif

    /* Setupterm or some other part of the system may have failed.
       Maybe we aren't using "terminfo" at all, lucky us!
       Use VT-100 protocols and pray for the day when we don't need
       to do this any more. */
    if (!top_of_screen) top_of_screen = "\033[H";
    if (!clear_to_end_of_line) clear_to_end_of_line = "\033[K";
    if (!clear_to_end_of_screen) clear_to_end_of_screen = "\033[J";
}

/* ARGSUSED */
void cgotoxy(int x, int y,  FILE *filep)
{
    if (!top_of_screen)
	terminfo_init(filep);
    /* we know this is only used to go to the start of the
       line or the top of the screen */
    if (y == 1)
#ifndef NO_TERMINFO
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
#ifndef NO_TERMINFO
    putp(clear_to_end_of_line);
#else
    fputs(clear_to_end_of_line, filep);
#endif
}

void ccleos(FILE *filep)
{
    if (!clear_to_end_of_screen)
	terminfo_init(filep);
#ifndef NO_TERMINFO
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

static int current_tty_mode = C_ECHO;

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
    current_tty_mode = mode;
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
    current_tty_mode = mode;
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

/*
 * signal handling
 */

#include <signal.h>

/* ARGSUSED */
static void stop_handler(int n)
{
    if (current_tty_mode != C_ECHO) {
	csetmode(C_ECHO, stdin);
	current_tty_mode = C_RAW;
    }
    signal(SIGTSTP, SIG_DFL);
    kill(0, SIGTSTP);
}

/* ARGSUSED */
static void cont_handler(int n)
{
    refresh_input();
    if (current_tty_mode != C_ECHO) {
	csetmode(current_tty_mode, stdin);
    }
    initialize_signal_handlers();
}

void initialize_signal_handlers(void)
{
    signal(SIGTSTP, stop_handler);
    signal(SIGCONT, cont_handler);
}
