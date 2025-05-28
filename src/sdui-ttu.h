/*
 * sdui-ttu.h - header file for sdui-ttu.c
 * Time-stamp: <93/02/10 14:56:32 gildea>
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
 */

#include <stdio.h>

#define C_ECHO 0		/* normal input mode: echo, line editing */
#define C_RAW 1			/* raw input mode: no echo, does not block */

/* we don't use this struct */
struct {
    int ncols;
    int nrows;
    int left;
    int top;
    char *title;
} console_options;


extern void cgetxy(int *xp, int *yp, FILE *filep);
extern void cgotoxy(int x, int y,  FILE *filep);
extern void ccleol(FILE *filep);
extern void ccleos(FILE *filep);
extern void csetmode(int mode, FILE *fp);
extern FILE *fopenc(void);
extern void cshow(FILE *filep);
