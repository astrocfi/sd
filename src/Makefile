# sample Makefile for Sd (square dance caller's helper)
# Time-stamp: <93/02/25 20:40:41 gildea>

# If your X system was compiled with an ANSI C compiler,
# you should run "xmkmf", which will use the Imakefile and your
# local X configuration information to generate a Makefile.
# This sample Makefile will be moved to Makefile.bak.

# If you must use this file directly, expect to have to edit it.

CC=gcc
CFLAGS=$(CDEBUGFLAGS) -traditional
# Note: the "-traditional" above is needed only if "fixincludes" has
# not been run on your machine.
# With gcc 1.40 on our Ultrix 4.2 cannot specify -traditional.

# CDEBUGFLAGS is used by both cc and ld.
# If you want a smaller executable and are willing to lose the ability
# to debug, use "-O" instead of "-g".
CDEBUGFLAGS = -O

# these flags are passed only to ld
LDFLAGS = $(CDEBUGFLAGS)

# additional flags for gildea's code
UICFLAGS = -Wswitch

#PEDCFLAGS = -pedantic -O -Wimplicit -Wreturn-type -Wunused -Wcomment
#PEDANTICW = -Wformat -Wchar-subscripts -Wshadow -Wpointer-arith
#PEDCFLAGS2 = $(PEDCFLAGS) $(PEDANTICW) -Wmissing-prototypes -Wnested-externs
# for HP Snake: +w1 -z

# Libraries the UI needs to be linked with.  If your X11 libraries aren't
# installed, you may want to add some -L<dir> flags to SDX11_LIBS below
# to specify the library directories explicitly.
SDX11_LIBS = -lXaw -lXmu -lXt -lXext -lX11

# SunOS 4 puts the correct curses library in a funny directory.
SDTTY_LIBS = -L/usr/5lib -lcurses
# on PC, link gppconio
#SDTTY_LIBS = -lpc

# override unnecessary SunOS 4.0 stuff:
#TARGET_ARCH=

SD_SRCS = sdmain.c sdinit.c sdutil.c sdbasic.c \
          sdtables.c sdctable.c sdtop.c sd.c sdpreds.c sd12.c sd16.c \
          sdgetout.c sdmoves.c sdtand.c sdconc.c sdistort.c \
          sdsi.c

SD_OBJS = sdmain.o sdinit.o sdutil.o sdbasic.o \
          sdtables.o sdctable.o sdtop.o sd.o sdpreds.o sd12.o sd16.o \
          sdgetout.o sdmoves.o sdtand.o sdconc.o sdistort.o \
          sdsi.o

SDX11_SRC = sdui-x11.c
SDX11_OBJ = sdui-x11.o

# for PC use sdui-tpc.c, for Unix use sdui-ttu.c
SDTTY_SRC = sdui-tty.c sdui-ttu.c
SDTTY_OBJ = sdui-tty.o sdui-ttu.o

all: sd sd_calls.dat
alltty: sdtty sd_calls.dat

sd: $(SD_OBJS) $(SDX11_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(SD_OBJS) $(SDX11_OBJ) $(SDX11_LIBS)

sdtty: $(SD_OBJS) $(SDTTY_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(SD_OBJS) $(SDTTY_OBJ) $(SDTTY_LIBS)

# PC version
#sdtty: $(SD_OBJS) $(SDTTY_OBJ)
#	$(CC) $(LDFLAGS) -o $@ sd*.o $(SDTTY_LIBS)
#	strip $*
#	aout2exe $*
 
mkcalls: mkcalls.o
	$(CC) $(LDFLAGS) -o $@ mkcalls.o

sd_calls.dat: sd_calls.txt mkcalls
	./mkcalls

# this rule and the aout2exe program are for qmake on the PC
.c.o:
	$(CC) $(CFLAGS) -c $*.c

sdui-x11.o: sdui-x11.c
	$(CC) $(CFLAGS) $(UICFLAGS) -c sdui-x11.c

mkcalls.o sdmain.o sdsi.o sdui-x11.o: paths.h

mkcalls.o: database.h

$(SD_OBJS) $(SDX11_OBJ): sd.h database.h


.SUFFIXES: .dvi .info .txinfo

.txinfo.dvi:
	tex $?

.txinfo.info:
	makeinfo $?

# TeX outputs front, body, toc; we want front, toc, body

sd_doc-sorted.dvi: sd_doc.dvi
	dviselect -i sd_doc.dvi =1:2 > tempa.dvi
	dviselect -i sd_doc.dvi :_1 > tempb.dvi
	(dviselect -i sd_doc.dvi -s 1: | dviselect =3:) > tempc.dvi
	dviconcat -o sd_doc-sorted.dvi tempa.dvi tempb.dvi tempc.dvi
	rm -f tempa.dvi tempb.dvi tempc.dvi

sd_doc.PS: sd_doc-sorted.dvi
	dvips -U -o sd_doc.PS sd_doc-sorted.dvi


clean::
	rm -f *~ core *.o sd mkcalls sd_calls.dat sd.tar sd.tar.Z

lint:
	/usr/lang/alint -ux $(SD_SRCS) $(SDX11_SRC)

DISTTXT = README Relnotes relnotes.archive Imakefile Makefile Sd.res \
            sd_calls.txt COPYING database.doc sd_doc.txinfo
DISTSRCS = sd.h database.h paths.h sdui-ttu.h mkcalls.c
DISTFILES = $(DISTTXT) $(SD_SRCS) $(SDX11_SRC) $(SDTTY_SRC) $(DISTSRCS)

distrib:
	tar cvf sd.tar $(DISTFILES)

ls:
	@echo $(DISTFILES)
