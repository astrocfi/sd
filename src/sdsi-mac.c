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

    This is for version 30. */

/* This defines the following functions:
   general_initialize
   generate_random_number
   generate_random_concept_p
   get_mem
   get_mem_gracefully
   get_more_mem
   get_more_mem_gracefully
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
   read_8_from_database
   read_16_from_database
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

/* ***  This next test used to be
    ifdef _POSIX_SOURCE
   and, before that, a test for NO_UNISTD, which doesn't work for Think C.
   We have taken it out and replaced with what you see below.  If this breaks
   anything, let us know. */
#if defined(_POSIX_SOURCE) || defined(sun)
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

#include "paths.h"
#include "macguts.h"

static char *mac_fgets(char *s, int n, short file, OSErr *errcodep);
static void db_flush(void);

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

   buf = get_mem_gracefully(siz);
   if (!buf && siz != 0) {
      memory_allocation_failure(siz);
   }
   return buf;
}

/* This will not fail catastrophically, but may return nil pointer
   on nonzero request.  Client must check and react accordingly. */
extern void *get_mem_gracefully(unsigned int siz)
{
   return malloc(siz);
}

extern void *get_more_mem(void *oldp, unsigned int siz)
{
   void *buf;

   buf = get_more_mem_gracefully(oldp, siz);
   if (!buf && siz != 0) {
      memory_allocation_failure(siz);
   }
   return buf;
}

/* This will not fail catastrophically, but may return nil pointer
   on nonzero request.  Client must check and react accordingly. */
extern void *get_more_mem_gracefully(void *oldp, unsigned int siz)
{
   return realloc(oldp, siz);
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

long_boolean file_error;

static FSSpec output_filespec;     /* the specification of the output file */
static FSSpec temp_filespec;       /* the specification of the temp file */
static short temp_file;            /* the temporary file used to write output */
/* sue: made this an external variable for use in macmenu.c and macstuff.c */
long_boolean output_append = FALSE; /* append to output file */

/* set_output_file is called directly by the UI */
extern void
set_output_file(const FSSpec *fs)
{
    output_filespec = *fs;
    pstrcpy((StringPtr) outfile_string, output_filespec.name);
    PtoCstr((StringPtr) outfile_string);
    output_append = FALSE;
}

/* set_output_file_append is called directly by the UI */
extern void
set_output_file_append(const FSSpec *fs)
{
    output_filespec = *fs;
    pstrcpy((StringPtr) outfile_string, output_filespec.name);
    PtoCstr((StringPtr) outfile_string);
    output_append = TRUE;
}

extern void
open_file(void)
{
    long time;
    int result;

    file_error = FALSE;
    if (output_append) {
        /* append to specified output file (which exists) */
        result = mac_open_file(&output_filespec, fsRdWrPerm, &temp_file);
        if (result == 0) {
            /* position at end of file */
            result = SetFPos(temp_file, fsFromLEOF, 0);
            if (result != 0) {
                FSClose(temp_file);
            }
        }
    }
    else {
        /* create and open a new temporary file */
        GetDateTime(&time);
        sprintf((char *)temp_filespec.name, "%d sd", time);
        CtoPstr((char *)temp_filespec.name);
        temp_filespec.vRefNum = output_filespec.vRefNum;
        temp_filespec.parID = output_filespec.parID;
   
        result = mac_create_file(&temp_filespec, 'ttxt', 'TEXT');
        if (result == 0) {
            result = mac_open_file(&temp_filespec, fsRdWrPerm, &temp_file);
        }
    }
    if (result != 0) {
        file_error = TRUE;
    }
}

extern long_boolean
probe_file(char filename[])
{
    return TRUE;
}

extern void
write_file(char line[])
{
    int size, result;
    char c = '\r';

    if (file_error) {
        /* Don't keep trying after a failure. */
        return;
    }
    size = strlen(line);
    result = FSWrite(temp_file, &size, (Ptr) line);
    if (result == 0) {
        size = 1;
        result = FSWrite(temp_file, &size, (Ptr) &c);
    }
    if (result != 0) {
        file_error = TRUE;      /* Indicate that sequence will not get written. */
    }
}

extern void
close_file(void)
{
    OSErr result;
    long pos;
    static FIDParam parm;

    if (!file_error) {
        result = GetFPos(temp_file, &pos);
        if (result == 0) {
            result = SetEOF(temp_file, pos);
        }
        if (result == 0) {
            result = FlushVol(0L, output_filespec.vRefNum);
        }
        if (result == 0) {
            result = FSClose(temp_file);
            temp_file = 0;
        }
        if (!output_append) {
            /* exchange temporary file with real output file */
            mac_replace_file(&output_filespec, &temp_filespec);
        }
        if (result == 0) {
            return;
        }
        file_error = TRUE;
    }
    sequence_write_error(outfile_string);
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
 **********************************************************************
 *  CALL DATABASE INPUT
 **********************************************************************
 */

char *database_filename = DATABASE_FILENAME;
static char database_filename_buf[200];
static short database_file;
static int database_bytes;
static unsigned char *database_next;
#define DATABASE_BUFSIZE 512
static unsigned char database_buffer[DATABASE_BUFSIZE];
static int abs_max_calls_temp;
static int max_base_calls_temp;
static char database_version_temp[200];
static long database_filepos_temp;

long_boolean
set_database_file(const char *fn, short f) /* called directly by UI */
{
    /* return 0 if OK, otherwise avoid side effect */

    int format_version, n, j;
    short old_file;

    old_file = database_file;
    database_file = f; /* needed so read_from_database works */

    SetFPos(database_file, fsFromStart, 0);
    database_bytes = 0;

    if (read_16_from_database() != DATABASE_MAGIC_NUM) {
        FSClose(database_file);
        database_file = old_file;
        database_bad_format(fn);
        return TRUE;
    }

    format_version = read_16_from_database();
    if (format_version != DATABASE_FORMAT_VERSION) {
        FSClose(database_file);
        database_file = old_file;
        database_wrong_version(fn, format_version, DATABASE_FORMAT_VERSION);
        return TRUE;
    }

    abs_max_calls_temp = read_16_from_database();
    max_base_calls_temp = read_16_from_database();

    n = read_16_from_database();

    if (n > 80) {
        FSClose(database_file);
        database_file = old_file;
        database_bad_format(fn);
        return TRUE;
    }

    for (j=0; j<n; j++)
        database_version_temp[j] = read_8_from_database();

    database_version_temp[j] = '\0';

    if (old_file != 0) {
        FSClose(old_file);
    }
    strcpy(database_filename_buf, fn);
    database_filename = database_filename_buf;

    database_filepos_temp = database_next - database_buffer;

    return FALSE;
}

extern void
open_database(void)
{
    int n;
    long len;

    SetFPos(database_file, fsFromStart, database_filepos_temp);
    database_bytes = 0;

    GetEOF(database_file, &len);
    database_begin(database_filename);

    abs_max_calls = abs_max_calls_temp;
    max_base_calls = max_base_calls_temp;
    strcpy(database_version, database_version_temp);
}

extern unsigned int
read_8_from_database(void)
{
    if (database_bytes <= 0) {
        int result;
        long n;
        n = DATABASE_BUFSIZE;
        database_next = database_buffer;
        result = FSRead(database_file, &n, database_next);
        if ((result != 0) && (result != eofErr)) {
            stop_alert(DatabaseReadErrorAlert, "", "");
            exit_program(2);
        }
        database_bytes = n;
    }
    database_bytes--;
    return *database_next++;
}

extern unsigned int
read_16_from_database(void)
{
    unsigned int bar;

    bar = (read_8_from_database() & 0xFF) << 8;
    bar |= read_8_from_database() & 0xFF;
    return bar;
}

extern void
close_database(void)
{
    int result;

    result = FSClose(database_file);
    if (result != 0) {
        stop_alert(DatabaseReadErrorAlert, "", "");
        exit_program(2);
    }
}

/*
 **********************************************************************
 *  CALL LIST FILE I/O
 **********************************************************************
 */

static int call_list_file;
static int writing;

extern void
set_call_list_file(int f) /* called directly by UI */
{
    call_list_file = f;
}

extern long_boolean
open_call_list_file(call_list_mode_t call_list_mode, char filename[])
{
    int result;
    if (!call_list_file) {
        return TRUE;
    }
    writing = (call_list_mode != call_list_mode_abridging);
    result = SetFPos(call_list_file, fsFromStart, 0);
    return FALSE;
}

extern char *
read_from_call_list_file(char *name, int n)
{
    OSErr errcode;

    name = mac_fgets(name, n, call_list_file, &errcode);
    if (errcode != 0) {
        stop_alert(CallListReadErrorAlert, "", "");
        exit_program(2);
    }
    return name;
}

extern void
write_to_call_list_file(char name[])
{
    int result;
    char c;
    int count;

    count = strlen(name);
    result = FSWrite(call_list_file, &count, (Ptr) name);
    if (result != 0) {
        stop_alert(CallListWriteErrorAlert, "", "");
        exit_program(2);
    }
    c = '\r';
    count = 1;
    result = FSWrite(call_list_file, &count, (Ptr) &c);
    if (result != 0) {
        stop_alert(CallListWriteErrorAlert, "", "");
        exit_program(2);
    }
}

extern long_boolean
close_call_list_file(void)
{
    int result, f;
    long pos;
    short vol;

    f = call_list_file;
    call_list_file = 0; /* ensure no attempt to close twice! */
    result = GetFPos(f, &pos);
    if (result == 0 && writing) {
        result = SetEOF(f, pos);
    }
    if (result == 0 && writing) {
        result = GetVRefNum(f, &vol);
    }
    if (result == 0) {
        result = FSClose(f);
    }
    if (result == 0 && writing) {
        result = FlushVol(0L, vol);
    }
    if (result != 0) {
        if (writing) {
            stop_alert(CallListWriteErrorAlert, "", "");
        }
        else {
            stop_alert(CallListReadErrorAlert, "", "");
        }
        return TRUE;
    }
    return FALSE;
}

/*
 **********************************************************************
 *  SUPPORT FOR CALL DATABASE COMPILER
 **********************************************************************
 */

#define DB_OUTPUT_BUFSIZ 512
static unsigned char db_output_buffer[DB_OUTPUT_BUFSIZ];
static unsigned char *db_output_ptr;
static int db_output_count;

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
    OSErr errcode;

    s = mac_fgets(s, n, db_input, &errcode);
    if (errcode != 0) {
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
    if (db_output_ptr == NULL) {
        db_output_ptr = db_output_buffer;
        db_output_count = DB_OUTPUT_BUFSIZ;
    }
    *db_output_ptr++ = ch;
    --db_output_count;
    if (db_output_count == 0) {
        db_flush();
    }
}

/*
 *  db_flush
 *
 *  Flush database output buffer.
 *
 */

static void
db_flush(void)
{
    long n = db_output_ptr - db_output_buffer;
    if (n > 0) {
        if (FSWrite(db_output, &n, db_output_buffer) != 0) {
            db_output_error();
        }
        db_output_ptr = db_output_buffer;
        db_output_count = DB_OUTPUT_BUFSIZ;
    }
}

/*
 *  db_rewind_output
 *
 */

void
db_rewind_output(int pos)
{
    long eof_pos;
    OSErr errcode;

    db_flush();
    errcode = GetFPos(db_output, &eof_pos) ||
                    SetEOF(db_output, eof_pos) ||
                    SetFPos(db_output, fsFromStart, pos);
    if (errcode != 0) {
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
    OSErr errcode = FSClose(db_input);
    db_input = -1;
    if (errcode != 0) {
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
    long pos;
    short vol;
    OSErr errcode;

    db_flush();
    errcode = GetVRefNum(db_output, &vol) ||
              FSClose(db_output) ||
              FlushVol(0L, vol);
    db_output = -1;
    if (errcode != 0) {
        db_output_error();
    }
}

/*
 *  db_cleanup
 *
 */

void
db_cleanup(void)
{
    if (db_input >= 0) {
        FSClose(db_input);
        db_input = -1;
    }
    if (db_output >= 0) {
        FSClose(db_output);
        db_output = -1;
    }
}

/*
 *  db_input_error
 *
 */

void
db_input_error(void)
{
    stop_alert(DatabaseCompileReadAlert, db_input_filename, "");
    do_exit();
}

/*
 *  db_output_error
 *
 */

void
db_output_error(void)
{
    stop_alert(DatabaseCompileWriteAlert, db_output_filename, "");
    do_exit();
}

/*
 *  mac_fgets
 *
 */

static char *
mac_fgets(char *s, int n, short file, OSErr *errcodep)
{
    int result;
    static IOParam parm;

    *errcodep = 0;
    parm.ioCompletion = NULL;
    parm.ioRefNum = file;
    parm.ioVersNum = 0;
    parm.ioBuffer = (Ptr) s;
    parm.ioReqCount = n-1;
    parm.ioPosMode = ('\r' << 8) + 128; /* stop at Return character */
    parm.ioPosOffset = 0;
    result = PBReadSync(&parm);
    if (result == eofErr) {
        return NULL;
    }
    if (result != 0) {
        *errcodep = result;
        return NULL;
    }
    n = parm.ioActCount-1;
    if (s[n] == '\r') {
        s[n] = '\n';
    }
    s[n+1] = '\0';
    return s;
}
