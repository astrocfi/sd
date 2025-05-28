/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

/* SD -- square dance caller's helper.

    Copyright (C) 1990-1998  William B. Ackerman.

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
   6,          /* b_1x2dmd */
   6,          /* b_p1x2dmd */
   6,          /* b_2x1dmd */
   6,          /* b_p2x1dmd */
   8,          /* b_qtag */
   8,          /* b_pqtag */
   8,          /* b_bone */
   8,          /* b_pbone */
   8,          /* b_rigger */
   8,          /* b_prigger */
   8,          /* b_2stars */
   8,          /* b_p2stars */
   8,          /* b_spindle */
   8,          /* b_pspindle */
   8,          /* b_hrglass */
   8,          /* b_phrglass */
   8,          /* b_dhrglass */
   8,          /* b_pdhrglass */
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
  10,          /* b_2x5 */
  10,          /* b_5x2 */
  10,          /* b_d2x5 */
  10,          /* b_d5x2 */
  10,          /* b_wqtag */
  10,          /* b_pwqtag */
  10,          /* b_deep2x1dmd */
  10,          /* b_pdeep2x1dmd */
  10,          /* b_whrglass */
  10,          /* b_pwhrglass */
   6,          /* b_1x6 */
   6,          /* b_6x1 */
   12,         /* b_3x4 */
   12,         /* b_4x3 */
   12,         /* b_2x6 */
   12,         /* b_6x2 */
   14,         /* b_2x7 */
   14,         /* b_7x2 */
   12,         /* b_d3x4 */
   12,         /* b_d4x3 */
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
   18,         /* b_3x6 */
   18,         /* b_6x3 */
   24,         /* b_3x8 */
   24,         /* b_8x3 */
   20,         /* b_4x5 */
   20,         /* b_5x4 */
   24,         /* b_4x6 */
   24,         /* b_6x4 */
   20,         /* b_2x10 */
   20,         /* b_10x2 */
   24,         /* b_2x12 */
   24,         /* b_12x2 */
   12,         /* b_deepqtg */
   12,         /* b_pdeepqtg */
   16,         /* b_deepbigqtg */
   16,         /* b_pdeepbigqtg */
   12,         /* b_deepxwv */
   12,         /* b_pdeepxwv */
   20,         /* b_3oqtg */
   20,         /* b_p3oqtg */
   8,          /* b_thar */
   8,          /* b_alamo */
   8,          /* b_ptpd */
   8,          /* b_pptpd */
   8,          /* b_1x3dmd */
   8,          /* b_p1x3dmd */
   8,          /* b_3x1dmd */
   8,          /* b_p3x1dmd */
   12,         /* b_3dmd */
   12,         /* b_p3dmd */
   16,         /* b_4dmd */
   16,         /* b_p4dmd */
   12,         /* b_3ptpd */
   12,         /* b_p3ptpd */
   16,         /* b_4ptpd */
   16,         /* b_p4ptpd */
   8,          /* b_wingedstar */
   8,          /* b_pwingedstar */
   8,          /* b_323 */
   8,          /* b_p323 */
   10,         /* b_343 */
   10,         /* b_p343 */
   12,         /* b_525 */
   12,         /* b_p525 */
   14,         /* b_545 */
   14,         /* b_p545 */
   14,         /* bh545 */
   14,         /* bhp545 */
   12,         /* b_3mdmd */
   12,         /* b_p3mdmd */
   12,         /* b_3mptpd */
   12,         /* b_p3mptpd */
   16,         /* b_4mdmd */
   16,         /* b_p4mdmd */
   16,         /* b_4mptpd */
   16,         /* b_p4mptpd */
   12,         /* b_bigh */
   12,         /* b_pbigh */
   12,         /* b_bigx */
   12,         /* b_pbigx */
   16,         /* b_bigbigh */
   16,         /* b_pbigbigh */
   16,         /* b_bigbigx */
   16,         /* b_pbigbigx */
   12,         /* b_bigrig */
   12,         /* b_pbigrig */
   12,         /* b_bighrgl */
   12,         /* b_pbighrgl */
   12,         /* b_bigdhrgl */
   12,         /* b_pbigdhrgl */
   12,         /* b_bigbone */
   12,         /* b_pbigbone */
   12,         /* b_bigdmd */
   12,         /* b_pbigdmd */
   12,         /* b_bigptpd */
   12,         /* b_pbigptpd */
   16,         /* b_dblxwave */
   16};        /* b_pdblxwave */

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


int main(int argc, char *argv[])
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
   return 0;
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
/* There is a gross bug in the DJGPP library.  When figuring the return value,
   fputc sign-extends the input datum and returns that as an int.  This means
   that, if we try to write 0xFF, we get back -1, which is the error condition.
   So we have to use errno as the sole means of telling whether an error occurred. */
#if defined(MSDOS)
   errno = 0;
   (void) fputc(ch, db_output);
   if (errno)
      db_output_error();
#else
   if (fputc(ch, db_output) == EOF) {
       db_output_error();
   }
#endif
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
