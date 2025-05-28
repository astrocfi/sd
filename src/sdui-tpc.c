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

static int screen_height = 25;
static int no_cursor = 0;
static char *text_ptr;           /* End of text buffer; where we are packing. */
static char text_buffer[10000];  /* This is *NOT* normally padded with a null.  It only gets padded
                                             when we need to display it. */
static int lines_in_buffer;  /* Number of "newline" characters in the buffer. */

extern void ttu_process_command_line(int *argcp, char ***argvp)
{
   int i;
   int argno = 1;
   char **argv = *argvp;

   while (argno < (*argcp)) {
      if (strcmp(argv[argno], "-no_cursor") == 0) {
         no_cursor = 1;
      }
      else if (strcmp(argv[argno], "-lines") == 0 && argno+1 < (*argcp)) {
         screen_height = atoi(argv[argno+1]);
         (*argcp) -= 2;      /* Remove two arguments from the list. */
         for (i=argno+1; i<=(*argcp); i++) argv[i-1] = argv[i+1];
         continue;
      }
      else {
         argno++;
         continue;
      }

      (*argcp)--;      /* Remove this argument from the list. */
      for (i=argno+1; i<=(*argcp); i++) argv[i-1] = argv[i];
   }
}

extern void ttu_initialize(void)
{
   gppconio_init();
   text_ptr = text_buffer;
   lines_in_buffer = 0;
}

extern void ttu_terminate(void)
{
}

extern int get_lines_for_more(void)
{
   return screen_height;
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
      while (c = *p++)
         pack_in_buffer(c);

      (void) fputs(the_line, stdout);
   }
   else {
      (void) fputs(the_line, stdout);
   }
}

extern void put_char(int c)
{
   if (!no_cursor)
      pack_in_buffer(c);

   (void) putchar(c);
}

extern int get_char(void)
{
   int n;

   do {
      fflush(stdout);
      n = getch();
   } while (n == EOF);   /* busy wait (EOF means no character yet) */

   if (n == 339) /* Delete */
      n = '\177';
   else if (n >= 315 && n <= 324)
      n += 128+1-315;     /* f1..f10 */
   else if (n >= 389 && n <= 390)
      n += 128+11-389;     /* f11..f12 */
   else if (n >= 340 && n <= 349)
      n += 160+1-340;     /* shift f1..f10 */
   else if (n >= 391 && n <= 392)
      n += 160+11-391;     /* shift f11..f12 */
   else if (n >= 350 && n <= 359)
      n += 192+1-350;     /* control f1..f10 */
   else if (n >= 393 && n <= 394)
      n += 192+11-393;     /* control f11..f12 */
   else if (n >= 360 && n <= 369)
      n += 224+1-360;     /* alt f1..f10 */
   else if (n >= 395 && n <= 396)
      n += 224+11-395;     /* alt f11..f12 */
   else if (n >= 128)
      n = ' ';

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
