/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

/*
 * sdui-tpc.c - helper functions for Mac tty interface for port to DOS.
 * Time-stamp: <93/07/19 19:38:40 wba>
 * Copyright (c) 1990-1995 Stephen Gildea and William B. Ackerman
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef DJGPP
#include "pc.h"
#include "keys.h"
#include "gppconio.h"
#else
#ifndef WIN32
#include <termios.h>   /* We use this stuff if "-no_cursor" was specified. */
#include <unistd.h>    /* This too. */
#endif
#endif
extern int diagnostic_mode;    /* We need this. */
#include "sdui-ttu.h"



static int screen_height = 25;
static int no_cursor = 0;
static char *text_ptr;           /* End of text buffer; where we are packing. */
static char text_buffer[10000];  /* This is *NOT* normally padded with a null.
                                    It only gets padded when we need to display it. */
static int lines_in_buffer;  /* Number of "newline" characters in the buffer. */

#if !defined(DJGPP) && !defined(WIN32)
static int current_tty_mode = 0;

static void csetmode(int mode)             /* 1 means raw, no echo, one character at a time;
                                                0 means normal. */
{
    static cc_t orig_eof = '\004';
    struct termios term;
    int fd;

    if (mode == current_tty_mode) return;

    fd = fileno(stdin);

    (void) tcgetattr(fd, &term);
    if (mode == 1) {
         orig_eof = term.c_cc[VEOF]; /* VMIN may clobber */
	term.c_cc[VMIN] = 1;	/* 1 char at a time */
	term.c_cc[VTIME] = 0;	/* no time limit on input */
	term.c_lflag &= ~(ICANON|ECHO);
    } else {
	term.c_cc[VEOF] = orig_eof;
	term.c_lflag |= ICANON|ECHO;
    }
    (void) tcsetattr(fd, TCSADRAIN, &term);
    current_tty_mode = mode;
}
#endif


extern int ttu_process_command_line(int *argcp, char **argv)
{
   int i;
   int argno = 1;
   int triangles = 1;

   while (argno < (*argcp)) {
      if (strcmp(argv[argno], "-no_line_delete") == 0) ;   /* ignore this */
      else if (strcmp(argv[argno], "-no_cursor") == 0)
         no_cursor = 1;
      else if (strcmp(argv[argno], "-no_graphics") == 0)
         triangles = 0;
      else if (strcmp(argv[argno], "-lines") == 0 && argno+1 < (*argcp)) {
         screen_height = atoi(argv[argno+1]);
         goto remove_two;
      }
      else if (strcmp(argv[argno], "-journal") == 0 && argno+1 < (*argcp)) {
         journal_file = fopen(argv[argno+1], "w");

         if (!journal_file) {
            printf("Can't open journal file\n");
            perror(argv[argno+1]);
            return 1;
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

   /* If no "-no_graphics" switch was not given, and our run-time
      system supports it, switch over to the "pointy triangles"
      for drawing pictures. */
#if defined(WIN32) || defined(DJGPP)
   if (triangles)
      ui_directions = "?\020?\021????\036?\037?????";
#endif

   return 0;
}

extern void ttu_display_help(void)
{
   printf("-lines <N>                  assume this many lines on the screen\n");
   printf("-no_cursor                  do not use screen management functions\n");
   printf("-no_graphics                do not use special characters for showing facing directions\n");
   printf("-journal <filename>         echo input commands to journal file\n");
}

extern void ttu_initialize(void)
{
#ifdef DJGPP
   gppconio_init();
#endif

   text_ptr = text_buffer;
   lines_in_buffer = 0;
}

extern void ttu_terminate(void)
{
   if (journal_file) (void) fclose(journal_file);
#if !defined(DJGPP) && !defined(WIN32)
   csetmode(0);   /* Restore normal input mode. */
#endif
}

extern int get_lines_for_more(void)
{
   return screen_height;
}

extern void clear_line(void)
{
#ifdef DJGPP
   int yp = wherey();
   gotoxy(1, yp);
   clreol();
#else
   printf(" XXX\n");
#endif
}

extern void rubout(void)
{
   printf("\b \b");
}

extern void erase_last_n(int n)
{
#ifdef DJGPP
   char *p = text_buffer;
   int c = 0;

   if (!no_cursor) {
      if (lines_in_buffer > n) {
         lines_in_buffer = lines_in_buffer-n;

         /* Now point p just after the "lines_in_buffer"th newline. */
         while (p != text_ptr) {   /* Just being careful. */
            if (*p++ == '\n') {
               /* Found a "newline", count same. */
               c++;
               if (c == lines_in_buffer)
                  break;      /* We found the new ending spot. */
            }
         }
      }
      else {
         lines_in_buffer = 0;
      }

      /* Now "p" has the new end. */

      text_ptr = p;
      *p = '\0';

      gotoxy(1, 1);
      clrscr();
      printf("%s", text_buffer);
   }
   /* Otherwise, we take no action at all. */
#endif
}

static void pack_in_buffer(char c)
{
   *text_ptr++ = c;
   if (c == '\n') {
      if (lines_in_buffer >= screen_height) {
         /* We need to throw away a line at the start of the buffer. */
         char *p = text_buffer;
         char *sp = text_buffer;

         while (p != text_ptr) {   /* Just being careful. */
            if (*p++ == '\n') {
               /* Found the new text, copy it down. */
               while (p != text_ptr) *sp++ = *p++;
               text_ptr = sp;
               break;
            }
         }
      }
      else
         lines_in_buffer++;
   }
}

extern void put_line(char the_line[])
{
   if (!no_cursor) {
      char *p = the_line;
      char c;
      while ((c = *p++))
         pack_in_buffer(c);
   }

   (void) fputs(the_line, stdout);
}

extern void put_char(int c)
{
   if (!no_cursor)
      pack_in_buffer((char) c);

   (void) putchar(c);
}


extern int get_char(void)
#if defined(WIN32)
{
   return getchar();
}
#elif defined(DJGPP)
{
   int n;

   if (diagnostic_mode) return getchar();

   do {
      fflush(stdout);
      n = getxkey();
   } while (n == EOF);   /* busy wait (EOF means no character yet) */

   if (n == 0) {     /* Escape for function keys on some keyboards. */
      do {           /* Get the indicator character. */
         fflush(stdout);
         n = getxkey();
      } while (n == EOF);

      if (n == 83)
         n = '\177';        /* delete */
      else if (n >= 59 && n <= 68)
         n += 128+1-59;     /* f1..f10 */
      else if (n >= 133 && n <= 134)
         n += 128+11-133;     /* f11..f12 */
      else if (n >= 84 && n <= 93)
         n += 144+1-84;     /* shift f1..f10 */
      else if (n >= 135 && n <= 136)
         n += 144+11-135;     /* shift f11..f12 */
      else if (n >= 94 && n <= 103)
         n += 160+1-94;     /* control f1..f10 */
      else if (n >= 137 && n <= 138)
         n += 160+11-137;     /* control f11..f12 */
      else if (n >= 104 && n <= 113)
         n += 176+1-104;     /* alt f1..f10 */
      else if (n >= 139 && n <= 140)
         n += 176+11-139;     /* alt f11..f12 */
      else
         n = ' ';
   }
   else if (n == K_Delete)           /* Delete */
      n = '\177';
   else if (n == K_EDelete)          /* The real delete key */
      n = '\177';
   else if (n >= K_F1 && n <= K_F10)
      n += 128+1-K_F1;               /* F1..F10 */
   else if (n >= K_F11 && n <= K_F12)
      n += 128+11-K_F11;             /* F11..F12 */
   else if (n >= K_Shift_F1 && n <= K_Shift_F10)
      n += 144+1-K_Shift_F1;         /* shift F1..F10 */
   else if (n >= K_Shift_F11 && n <= K_Shift_F12)
      n += 144+11-K_Shift_F11;       /* shift F11..F12 */
   else if (n >= K_Control_F1 && n <= K_Control_F10)
      n += 160+1-K_Control_F1;       /* control F1..F10 */
   else if (n >= K_Control_F11 && n <= K_Control_F12)
      n += 160+11-K_Control_F11;     /* control F11..F12 */
   else if (n >= K_Alt_F1 && n <= K_Alt_F10)
      n += 176+1-K_Alt_F1;           /* alt F1..F10 */
   else if (n >= K_Alt_F11 && n <= K_Alt_F12)
      n += 176+11-K_Alt_F11;         /* alt F11..F12 */
   else if (n >= 128)
      n = ' ';

   return n;
}
#else
{
   int c;

   csetmode(1);         /* Raw, no echo, single-character mode. */

   for ( ;; ) {
      c = getchar();
      if (c != '\r') return c;
   }
}
#endif

extern void get_string(char *dest)
{
   int size;
#if !defined(DJGPP) && !defined(WIN32)
   csetmode(0);         /* Regular full-line mode with system echo. */
#endif
   (void) gets(dest);
   size = strlen(dest);
   if (size > 0 && dest[size-1] == '\n') {
      size--;
      dest[size] = '\000';
   }

   /* Some libraries (Cygnus GCC) have been observed putting a
      control-M at the ends (although it does remove the control-J.) */
   if (size > 0 && dest[size-1] == '\r') {
      size--;
      dest[size] = '\000';
   }
}

extern void bell(void)
{
   (void) putchar('\007');
}

extern void initialize_signal_handlers(void)
{
}
