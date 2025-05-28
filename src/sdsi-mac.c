/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992, 1993  William B. Ackerman and Alan Snyder.

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

    This is for version 28. */

/* This defines the following functions:
   general_initialize
   generate_random_number
   generate_random_concept_p
   get_mem
   get_more_mem
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
   database_filename
*/

/* You should compile this file (and might as well compile all the others
   too) with some indicator symbol defined that tells what language system
   semantics are to be provided.  This program requires at least POSIX.
   A better random number generator mechanism (48 bits) is provided under
   System 5 (_SYS5_SOURCE), OSF (_AES_SOURCE), XOPEN (_XOPEN_SOURCE), or
   some proprietary systems, so you should define such a symbol if your
   system provides those semantics.  If not, you should just turn on
   _POSIX_SOURCE.  Normally, this is done on the compiler invocation line
   (in the Makefile, or whatever) with some incantation like "-D_AES_SOURCE".

   We recommend _AES_SOURCE over _XOPEN_SOURCE, since the latter doesn't
   seem to recognize that 20th century programmers use include files with
   prototypes for library functions in them.  Under OSF/AES, the file
   stdlib.h has the necessary stuff.  Under XOPEN, you are expected to
   copy the prototypes by hand from the printed manual into your program.
   For those who absolutely must use XOPEN, we have done the necessary
   copying below.

   On HP-UX, we recommend turning on _AES_SOURCE for the reason given
   above.  You could also turn on _HPUX_SOURCE to get the routines,
   if you have no aversion to proprietary system semantics.

   On SUN-OS, giving no symbol will get SUN-OS semantics, which gets the
   48-bit routines, if you have no aversion to proprietary system semantics.
   This is what the "defined(sun)" is for.
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

/* We take pity on those poor souls who are compelled to XOPEN,
   an otherwise fine standard, that doesn't put prototypes
   into stdlib.h. */

#if defined(_XOPEN_SOURCE)
   /* We have taken the liberty of translating the prototypes
      appearing in the XOPEN manual into ANSI C.  We can't
      imagine why anyone would ever use the prototypes
      in the prehistoric nomenclature. */
extern void srand48(long int);
extern long int lrand48(void);
#endif

#if defined(_SYS5_SOURCE) || defined(_XOPEN_SOURCE) || defined(_AES_SOURCE) || defined(sun)
#define HAVE_RAND48
#endif

/* We also take pity on those poor souls who are compelled to use
   compilation systems that claim to be POSIX compliant, but
   aren't really, and do not have unistd.h. */

#ifndef NO_UNISTD
#include <unistd.h>
#endif

/* Or those who are otherwise compelled to use
    troglodyte development environments.
    gcc -traditional doesn't define __STDC__ */

#if defined(__STDC__) || defined(sun)
#include <stdlib.h>
#else
extern void free(void *ptr);
extern char *malloc(unsigned int siz);
extern char *realloc(char *oldp, unsigned int siz);
extern void exit(int code);
extern void srand48(long int);
extern long int lrand48(void);
#endif

/* I think we have everything now.  Isn't portability fun?  Before people
   started taking these standards as semi-seriously as they do now, it was
   MUCH HARDER to make a program portable than what you just saw. */

#include "sd.h"
#include "paths.h"

#include "sdmatch.h"
#include "macstuff.h"
#include "macguts.h" /* pstrcpy */

/*
 * The following functions are called directly by the UI.
 */

extern void set_call_list_file(int f);
extern void set_output_file(FSSpec fs);
extern int set_database_file(char *fn, short f);

/* These variables are external. */

int random_number;
int hashed_randoms;


static int last_hashed_randoms;

extern void general_initialize(void)
{
/* Sorry, plain POSIX doesn't have the nice rand48 stuff. */
#ifdef HAVE_RAND48
   srand48((long int) time((time_t *)0));
#else
   srand((unsigned int) time((time_t *)0));
#endif
}


extern int generate_random_number(int modulus)
{
   int j;

/* Sorry, plain POSIX doesn't have the nice rand48 stuff. */
#ifdef HAVE_RAND48
   random_number = (int) lrand48();
#else
   random_number = (int) rand();
#endif

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
   if (!buf && siz != 0) {
      memory_allocation_failure(siz);
   }
   return buf;
}

extern void *get_more_mem(void *oldp, unsigned int siz)
{
   void *buf;

   buf = realloc(oldp, siz);
   if (!buf && siz != 0) {
      memory_allocation_failure(siz);
   }
   return buf;
}

extern void free_mem(void *ptr)
{
   free(ptr);
}

extern void get_date(char dest[])
{
   time_t clock;
   char *junk;
   char *dstptr;

   time(&clock);
   junk = ctime(&clock);
   dstptr = dest;
   string_copy(&dstptr, junk);
   if (dstptr[-1] == '\n') dstptr[-1] = '\0';         /* Stupid UNIX! */
}

/*
 * Outfile I/O
 */

static FSSpec output_filespec;  /* the specification of the output file */
static short temp_file;         /* the temporary file used to write output */
static long temp_directory;     /* the directory id of the temp file */
static Str63 temp_file_name;    /* the name of the temp file */
static long_boolean file_error;
static char fail_message[MAX_ERR_LENGTH];
static char fail_errstring[MAX_ERR_LENGTH];

static char *errstring(int code)
{
    static char buf[100];
    sprintf(buf, "Error %d", code);
    return buf;
}

/* set_output_file is called directly by the UI */
extern void
set_output_file(FSSpec fs)
{
    output_filespec = fs;
    pstrcpy((StringPtr) outfile_string, output_filespec.name);
    PtoCstr((StringPtr) outfile_string);
}

extern void
open_file(void)
{
    long time;
    int result;

    file_error = FALSE;
    GetDateTime(&time);
    sprintf((char *) temp_file_name, "%d sd", time);
    CtoPstr(temp_file_name);
    temp_directory = output_filespec.parID;   
    result = HCreate(output_filespec.vRefNum, temp_directory, temp_file_name,
                     'ttxt', 'TEXT');
    if (result == 0)
        result = HOpenDF(output_filespec.vRefNum, temp_directory, temp_file_name,
                         fsRdWrPerm, &temp_file);
    if (result != 0) {
        (void) strncpy(fail_errstring, errstring(result), MAX_ERR_LENGTH);
        (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
        file_error = TRUE;
        return;
    }
}

extern long_boolean
probe_file(char filename[])
{
    return (TRUE);
}

extern void
write_file(char line[])
{
    int size, result;
    char c = '\r';

    if (file_error) return;    /* Don't keep trying after a failure. */
    size = strlen(line);
    result = FSWrite(temp_file, &size, (Ptr) line);
    if (result == 0) {
        size = 1;
        result = FSWrite(temp_file, &size, (Ptr) &c);
    }
    if (result != 0) {
        (void) strncpy(fail_errstring, errstring(result), MAX_ERR_LENGTH);
        (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
        file_error = TRUE;      /* Indicate that sequence will not get written. */
        return;
    }
}

extern void
close_file(void)
{
    char foo[MAX_ERR_LENGTH*10];
    int result;
    long pos;
    static FIDParam parm;

    if (!file_error) {
        result = GetFPos(temp_file, &pos);
        if (result == 0)
            result = SetEOF(temp_file, pos);
        if (result == 0)
            result = FlushVol(0L, output_filespec.vRefNum);
        if (result == 0) {
            result = FSClose(temp_file);
            temp_file = 0;
        }
        if (result == 0) {
            parm.ioCompletion = 0;
            parm.ioNamePtr = (StringPtr) &output_filespec.name;
            parm.ioVRefNum = output_filespec.vRefNum;
            parm.ioDestNamePtr = (StringPtr) &temp_file_name;
            parm.ioDestDirID = temp_directory;
            parm.ioSrcDirID = output_filespec.parID;
            result = PBExchangeFilesSync(&parm);
        }
        if (result == 0) {
            result = HDelete(output_filespec.vRefNum, temp_directory, temp_file_name);
            /* this fails after "Save As" claiming file is busy */
            if (result != 0) {
                sprintf(foo, "Unable to delete temporary file. Error %d.", result);
                specialfail(foo);
            }
        }
        if (result == 0)
            return;
        (void) strncpy(fail_errstring, errstring(result), MAX_ERR_LENGTH);
        (void) strncpy(fail_message, "close", MAX_ERR_LENGTH);
    }

   (void) strncpy(foo, "WARNING!!!  Sequence has not been written!  File ", MAX_ERR_LENGTH);
   (void) strncat(foo, fail_message, MAX_ERR_LENGTH);
   (void) strncat(foo, " failure on \"", MAX_ERR_LENGTH);
   (void) strncat(foo, outfile_string, MAX_ERR_LENGTH);
   (void) strncat(foo, "\": ", MAX_ERR_LENGTH);
   (void) strncat(foo, fail_errstring, MAX_ERR_LENGTH);
   specialfail(foo);
}

/*
 * Error printing.
 */

extern void print_line(char s[]) /* should never be called */
{
   miscellaneous_error(s);
}

extern void print_id_error(int n)
{
    char buf[200];
    sprintf(buf, "Call didn't identify self -- %d", n);
    uims_database_error(buf, 0);
}

extern void init_error(char s[])
{
    char buf[200];
    sprintf(buf, "Error initializing program: %s", s);
    miscellaneous_error(buf);
}

extern void add_resolve_indices(char junk[], int cur, int max)
{
   sprintf(junk, "%d out of %d", cur, max);
}

extern void final_exit(int code)
{
   exit(code);
}

static void io_error(int code, char *when)
{
    char buf[200];
    sprintf(buf, "I/O error %d while %s", code, when);
    miscellaneous_error(buf);
    exit_program(2);
}

/*
 * Call Database I/O
 */
 
char *database_filename = DATABASE_FILENAME;
static char database_filename_buf[200];
static short database_file;
static int database_bytes;
static unsigned char *database_next;
#define DATABASE_BUFSIZE 512
static unsigned char database_buffer[DATABASE_BUFSIZE];

extern int set_database_file(char *fn, short f) /* called directly by UI */
{
    /* return 0 if OK, otherwise avoid side effect */

    int format_version;
    short old_file;

    old_file = database_file;
    database_file = f; /* needed so read_from_database works */

    SetFPos(database_file, fsFromStart, 0);
    database_bytes = 0;

    if (read_from_database() != DATABASE_MAGIC_NUM) {
        FSClose(database_file);
        database_file = old_file;
        return database_bad_format(fn);
    }

    format_version = read_from_database();
    if (format_version != DATABASE_FORMAT_VERSION) {
        FSClose(database_file);
        database_file = old_file;
        return database_wrong_version(fn, format_version, DATABASE_FORMAT_VERSION);
    }

    if (old_file != 0)
        FSClose(old_file);
    strcpy(database_filename_buf, fn);
    database_filename = database_filename_buf;
    return 0;
}

extern void open_database(void)
{
    int n;
    long len;

    SetFPos(database_file, fsFromStart, 4);
    database_bytes = 0;

    GetEOF(database_file, &len);
    database_begin(database_filename);

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
    int result;
    long n;

    if (database_bytes <= 0) {
        n = DATABASE_BUFSIZE;
        database_next = database_buffer;
        result = FSRead(database_file, &n, database_next);
        if ((result != 0) && (result != eofErr))
            io_error(result, "reading database");
        database_bytes = n;
    }
    bar = (*database_next++) << 8;
    bar |= (*database_next++);
    database_bytes -= 2;
    return bar;
}

extern void close_database(void)
{
    int result;

    result = FSClose(database_file);
    if (result != 0)
        io_error(result, "closing database");
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


/*
 * File List I/O
 */
 
static int call_list_file;
static int writing;

extern void set_call_list_file(int f) /* called directly by UI */
{
    call_list_file = f;
}

extern long_boolean open_call_list_file(call_list_mode_t call_list_mode, char filename[])
{
    int result;
    if (!call_list_file)
        return TRUE;
    writing = (call_list_mode != call_list_mode_abridging);
    result = SetFPos(call_list_file, fsFromStart, 0);
    return FALSE;
}

extern char *read_from_call_list_file(char name[], int n)
{
   int result;
   static IOParam parm;

   parm.ioCompletion = 0;
   parm.ioRefNum = call_list_file;
   parm.ioVersNum = 0;
   parm.ioBuffer = (Ptr) name;
   parm.ioReqCount = n-1;
   parm.ioPosMode = ('\r' << 8) + 128; /* stop at Return character */
   parm.ioPosOffset = 0;
   result = PBReadSync(&parm);
   if (result == eofErr)
       return 0;
   if (result != 0)
       io_error(result, "reading call list");
   n = parm.ioActCount-1;
   if (name[n] == '\r')
       name[n] = '\n';
   name[n+1] = '\0';
   return name;
}

extern void write_to_call_list_file(char name[])
{
    int result;
    char c;
    int count;

    count = strlen(name);
    result = FSWrite(call_list_file, &count, (Ptr) name);
    if (result != 0)
        io_error(result, "writing call list");
    c = '\r';
    count = 1;
    result = FSWrite(call_list_file, &count, (Ptr) &c);
    if (result != 0)
        io_error(result, "writing call list");
}

extern long_boolean close_call_list_file(void)
{
    int result, f;
    long pos;
    short vol;

    f = call_list_file;
    call_list_file = 0; /* ensure no attempt to close twice! */
    result = GetFPos(f, &pos);
    if (result == 0 && writing)
        result = SetEOF(f, pos);
    if (result == 0 && writing)
        result = GetVRefNum(f, &vol);
    if (result == 0)
        result = FSClose(f);
    if (result == 0 && writing)
        result = FlushVol(0L, vol);
    if (result != 0) {
        io_error(result, "closing call list file");
        return TRUE;
    }
    return FALSE;
}
