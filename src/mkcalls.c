/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992, 1993  William B. Ackerman.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    This is for version 30. */

/* mkcalls.c */

#include "paths.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

/* ***  This next test used to be
    ifdef _POSIX_SOURCE
   We have taken it out and replaced with what you see below.  If this breaks
   anything, let us know. */
#if defined(_POSIX_SOURCE) || defined(sun)
#include <unistd.h>
#endif

/* We take pity on those poor souls who are compelled to use
    troglodyte development environments. */

/* ***  This next test used to be
    if defined(__STDC__) && !defined(athena_rt) && !defined(athena_vax)
   We have taken it out and replaced with what you see below.  If this breaks
   anything, let us know. */
#if defined(__STDC__) || defined(sun)
#include <stdlib.h>
#else
extern void free(void *ptr);
extern char *malloc(unsigned int siz);
extern char *realloc(char *oldp, unsigned int siz);
extern void exit(int code);
#endif

#include <stdarg.h>

#ifndef SEEK_SET		/* stdlib.h may not define it */
#define SEEK_SET 0
#endif

/* This table is a copy of the one in sdtables.c . */

/* BEWARE!!  This list is keyed to the definition of "begin_kind" in sd.h . */
/*   It must also match the similar table in the sdtables.c. */
int begin_sizes[] = {
   0,          /* b_nothing */
   1,          /* b_1x1 */
   2,          /* b_1x2 */
   2,          /* b_2x1 */
   3,          /* b_1x3 */
   3,          /* b_3x1 */
   4,          /* b_2x2 */
   4,          /* b_dmd */
   4,          /* b_pmd */
   4,          /* b_star */
   6,          /* b_trngl */
   6,          /* b_ptrngl */
   8,          /* b_trngl4 */
   8,          /* b_ptrngl4 */
   6,          /* b_bone6 */
   6,          /* b_pbone6 */
   6,          /* b_short6 */
   6,          /* b_pshort6 */
   8,          /* b_qtag */
   8,          /* b_pqtag */
   8,          /* b_bone */
   8,          /* b_pbone */
   8,          /* b_rigger */
   8,          /* b_prigger */
   8,          /* b_spindle */
   8,          /* b_pspindle */
   8,          /* b_hrglass */
   8,          /* b_phrglass */
   8,          /* b_crosswave */
   8,          /* b_pcrosswave */
   4,          /* b_1x4 */
   4,          /* b_4x1 */
   8,          /* b_1x8 */
   8,          /* b_8x1 */
   8,          /* b_2x4 */
   8,          /* b_4x2 */
   6,          /* b_2x3 */
   6,          /* b_3x2 */
   6,          /* b_1x6 */
   6,          /* b_6x1 */
   12,         /* b_3x4 */
   12,         /* b_4x3 */
   12,         /* b_2x6 */
   12,         /* b_6x2 */
   16,         /* b_2x8 */
   16,         /* b_8x2 */
   16,         /* b_4x4 */
   10,         /* b_1x10 */
   10,         /* b_10x1 */
   12,         /* b_1x12 */
   12,         /* b_12x1 */
   14,         /* b_1x14 */
   14,         /* b_14x1 */
   16,         /* b_1x16 */
   16,         /* b_16x1 */
   16,         /* b_c1phan */
   8,          /* b_galaxy */
   24,         /* b_4x6 */
   24,         /* b_6x4 */
   8,          /* b_thar */
   8,          /* b_ptpd */
   8,          /* b_pptpd */
   8,          /* b_3x1dmd */
   8,          /* b_p3x1dmd */
   12,         /* b_3dmd */
   12,         /* b_p3dmd */
   16,         /* b_4dmd */
   16};        /* b_p4dmd */

extern void do_exit(void)
{
   /* close files here if desired */
   exit(1);
}


extern void dbcompile_signoff(int bytes, int calls)
{
   printf("%d bytes written, %d calls\n", bytes, calls);
}


extern int do_printf(char *fmt, ...)
{
   int n;
   va_list ap;

   va_start(ap, fmt);
   n = vprintf(fmt, ap);
   va_end(ap);
   return n;
}


extern void dbcompile(void);

static void db_input_error(void);
static void db_output_error(void);

static FILE *db_input = NULL;
static FILE *db_output = NULL;
#define FILENAME_LEN 200
static char db_input_filename[FILENAME_LEN];
static char db_output_filename[FILENAME_LEN];


void main(int argc, char *argv[])
{
   if (argc == 2)
       strncpy(db_input_filename, argv[1], FILENAME_LEN);
   else
       strncpy(db_input_filename, CALLS_FILENAME, FILENAME_LEN);

   strncpy(db_output_filename, DATABASE_FILENAME, FILENAME_LEN);

   db_input = fopen(db_input_filename, "r");
   if (!db_input) {
      fprintf(stderr, "Can't open input file ");
      perror(db_input_filename);
      exit(1);
   }

   if (remove(db_output_filename)) {
      if (errno != ENOENT) {
	 fprintf(stderr, "trouble deleting old output file ");
	 perror(db_output_filename);
         /* This one does NOT abort. */
      }
   }

   /* The "b" in the mode is meaningless and harmless in POSIX.  Some systems,
      however, require it for correct handling of binary data. */
   db_output = fopen(db_output_filename, "wb");
   if (!db_output) {
      fprintf(stderr, "Can't open output file ");
      perror(db_output_filename);
      exit(1);
   }

   dbcompile();
   exit(0);
}


/*
 * db_gets
 *
 *  Read one line (including a terminating newline) into S, stopping
 *  at a maximum of N-1 characters.  Return NULL on end of file.
 *
 */

char *
db_gets(char *s, int n)
{
    s = fgets(s, n, db_input);
    if ((s == NULL) && (!feof(db_input))) {
        db_input_error();
    }
    return s;
}


/*
 *  db_putc
 *
 *  Write one character to the new database file.
 *
 */

void
db_putc(char ch)
{
    if (fputc(ch, db_output) == EOF) {
        db_output_error();
    }
}


/*
 *  db_rewind_output
 *
 */

void
db_rewind_output(int pos)
{
    if (fseek(db_output, pos, SEEK_SET)) {
        db_output_error();
    }
}


/*
 *  db_close_input
 *
 */

void
db_close_input(void)
{
    int result = fclose(db_input);
    db_input = NULL;
    if (result != 0) {
        db_input_error();
    }
}


/*
 *  db_close_output
 *
 */

void
db_close_output(void)
{
    int result = fclose(db_output);
    db_output = NULL;
    if (result != 0) {
        db_output_error();
    }
}


/*
 *  db_input_error
 *
 */

static void
db_input_error(void)
{
    fprintf(stderr, "Error reading input file ");
    perror(db_input_filename);
    exit(1);
}


/*
 *  db_output_error
 *
 */

static void
db_output_error(void)
{
    fprintf(stderr, "Error writing output file ");
    perror(db_output_filename);
    exit(1);
}
