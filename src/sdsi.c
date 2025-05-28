/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992  William B. Ackerman.

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

    This is for version 25. */

/* This defines the following functions:
   general_initialize
   generate_random_number
   generate_random_concept_p
   get_mem
   free_mem
   get_date
   open_file
   probe_file
   write_file
   close_file
   print_line
   print_id_error
   init_error
   add_resolve_indices
   final_exit
   open_database
   read_from_database
   close_database
   fill_in_neglect_percentage
   parse_number
   open_call_list_file
   read_from_call_list_file
   write_to_call_list_file
   close_call_list_file

and the following external variables:
   random_number
   hashed_randoms
*/


#define _POSIX_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* We take pity on those poor souls who are compelled to use
    troglodyte development environments. */

#ifdef __STDC__
#include <stdlib.h>
#else
extern void free(void *ptr);
extern char *malloc(unsigned int siz);
extern void exit(int code);
extern void srand48(long int);
extern long int lrand48(void);
#endif

#include "sd.h"
#include "paths.h"


/* These variables are external. */

int random_number;
int hashed_randoms;


static int last_hashed_randoms;
static FILE *fp;
static FILE *fildes;
static long_boolean file_error;
static char fail_message[MAX_ERR_LENGTH];
static char fail_errstring[MAX_ERR_LENGTH];


static char *get_errstring(void)
{
#ifdef sun
   extern int sys_nerr;
   extern char *sys_errlist[];

   if (errno < sys_nerr) return sys_errlist[errno];
   else return "?unknown error?";
#else
   return strerror(errno);
#endif
}


extern void general_initialize(void)
{
   srand48(time((long int *)0));
}


extern int generate_random_number(int modulus)
{
   int j;

   random_number = lrand48();
   j = random_number % modulus;
   last_hashed_randoms = hashed_randoms;       /* save in case we need to undo it */
   hashed_randoms = hashed_randoms*37+j;
   return(j);
}


extern long_boolean generate_random_concept_p(void)
{
   int i = generate_random_number(8);

   /* Since we are not going to use the random number in a one-to-one way, we run the risk
      of not having hashed_randoms uniquely represent what is happening.  To remedy
      the problem, we undo the transformation that was just made to hashed_randoms,
      and redo it with just the information we intend to use. */

   hashed_randoms = last_hashed_randoms*37;       /* Now just need to add something. */

   if (i < CONCEPT_PROBABILITY) {
      hashed_randoms++;          /* This should do the trick. */
      return (TRUE);
   }
   else {
      return (FALSE);
   }
}


extern void *get_mem(unsigned int siz)
{
   void *buf;

   buf = malloc(siz);
   if (!buf) {
      fprintf(stderr, "Can't allocate %d bytes of memory.\n", siz);
      perror("malloc");
      exit_program(2);
   }
   return(buf);
}


extern void free_mem(void *ptr)
{
   free(ptr);
}


extern void get_date(char dest[])
{
   long int clock;
   char *junk;
   char *dstptr;

   time(&clock);
   junk = ctime(&clock);
   dstptr = dest;
   string_copy(&dstptr, junk);
   if (dstptr[-1] == '\n') dstptr[-1] = '\0';         /* Stupid UNIX! */
}



extern void open_file(void)
{
   int this_file_position;

   file_error = FALSE;

   if (!(fildes = fopen(outfile_string, "a"))) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
      file_error = TRUE;
      return;
   }

   this_file_position = ftell(fildes);

   if ((last_file_position != -1) && (last_file_position != this_file_position)) {
      writestuff("Warning -- file has been modified since last sequence.");
      newline();
      newline();
   }

   if (this_file_position == 0) {
      writestuff("File does not exist, creating it.");
      newline();
      newline();
   }
   else {
      char junk[1];

      if (last_file_position == -1) {
         writestuff("Appending to existing file.");
         newline();
         newline();
      }

      junk[0] = '\f';      /* Write a formfeed (end-of-page indicator). */
      if ((fwrite(junk, 1, 1, fildes) != 1) || ferror(fildes)) {
         (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
         (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
         file_error = TRUE;
      }
   }
}


extern long_boolean probe_file(char filename[])
{
   if (!access(filename, W_OK))
      return (TRUE);
   else
      return (FALSE);
}


extern void write_file(char line[])
{
   int size;
   char nl = '\n';

   if (file_error) return;    /* Don't keep trying after a failure. */

   size = strlen(line);
   if ((fwrite(line, 1, size, fildes) != size) || ferror(fildes)) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
      file_error = TRUE;      /* Indicate that sequence will not get written. */
      return;
   }

   if ((fwrite(&nl, 1, 1, fildes) != 1) || ferror(fildes)) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
      file_error = TRUE;      /* Indicate that sequence will not get written. */
   }
}


extern void close_file(void)
{
   char foo[MAX_ERR_LENGTH*10];

   if (file_error) goto fail;
   last_file_position = ftell(fildes);

   if (!fclose(fildes)) return;

   (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
   (void) strncpy(fail_message, "close", MAX_ERR_LENGTH);

   fail:

   (void) strncpy(foo, "WARNING!!!  Sequence has not been written!  File ", MAX_ERR_LENGTH);
   (void) strncat(foo, fail_message, MAX_ERR_LENGTH);
   (void) strncat(foo, " failure on '", MAX_ERR_LENGTH);
   (void) strncat(foo, outfile_string, MAX_ERR_LENGTH);
   (void) strncat(foo, "': ", MAX_ERR_LENGTH);
   (void) strncat(foo, fail_errstring, MAX_ERR_LENGTH);
   specialfail(foo);
}

/* Used only for printing error messages, so writes to stderr */
extern void print_line(char s[])
{
   fprintf(stderr, "%s\n", s);
}


extern void print_id_error(int n)
{
   fprintf(stderr, "Call didn't identify self -- %d\n", n);
}


extern void init_error(char s[])
{
   fprintf(stderr, "Error initializing program: %s\n", s);
}


extern void add_resolve_indices(char junk[], int cur, int max)
{
   sprintf(junk, "%d out of %d", cur, max);
}



extern void final_exit(int code)
{
   exit(code);
}



extern void open_database(void)
{
   int format_version, n;

   if (!(fp = fopen(DATABASE_FILENAME, "r"))) {
      fprintf(stderr, "Can't open database file.\n");
      perror(DATABASE_FILENAME);
      exit_program(1);
   }

   if (read_from_database() != DATABASE_MAGIC_NUM) {
      fprintf(stderr,
         "Database file '%s' has improper format.\n", DATABASE_FILENAME);
      exit_program(1);
   }

   format_version = read_from_database();
   if (format_version != DATABASE_FORMAT_VERSION) {
      fprintf(stderr,
         "Database format version (%d) is not the required one (%d) -- you must recompile the database.\n",
         format_version, DATABASE_FORMAT_VERSION);
      exit_program(1);
   }

   abs_max_calls = read_from_database();
   max_base_calls = read_from_database();

   n = read_from_database();
   sprintf(major_database_version, "%d", n);
   n = read_from_database();
   sprintf(minor_database_version, "%d", n);
}


extern int read_from_database(void)
{
   int bar;

   bar = (fgetc(fp)&0xFF) << 8;
   bar |= fgetc(fp)&0xFF;
   return(bar);
}


extern void close_database(void)
{
   fclose(fp);
}


extern int parse_number(char junk[])
{
   int n;

   if (sscanf(junk, "%d", &n) != 1) return(0);
   return(n);
}


extern void fill_in_neglect_percentage(char junk[], int n)
{
   sprintf(junk, "LEAST RECENTLY USED %d%% OF THE CALLS ARE:", n);
}


static FILE *call_list_file;

extern long_boolean open_call_list_file(call_list_mode_t call_list_mode, char filename[])
{
   if (call_list_mode == call_list_mode_abridging) {
      call_list_file = fopen(filename, "r");
   }
   else {
      call_list_file = fopen(filename, "w");
   }

   if (!call_list_file) {
      printf("Can't open call list file\n");
      perror(filename);
      return TRUE;
   }
   else
      return FALSE;
}


extern char *read_from_call_list_file(char name[], int n)
{
   return (fgets(name, n, call_list_file));
}


extern void write_to_call_list_file(char name[])
{
   fputs(name, call_list_file);
   fputs("\n", call_list_file);
}


extern long_boolean close_call_list_file(void)
{
   if (fclose(call_list_file)) {
      printf("Can't close call list file\n");
      return TRUE;
   }
   else
      return FALSE;
}
