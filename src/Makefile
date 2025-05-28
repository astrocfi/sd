# sample Makefile for Sd (square dance caller's helper)

# If your X system was compiled with an ANSI C compiler,
# you should run "xmkmf", which will use the Imakefile and your
# local X configuration information to generate a Makefile.
# This sample Makefile will be moved to Makefile.bak.

# If you must use this file directly, expect to have to edit it.

CC=gcc
CFLAGS=$(FLAGS) -traditional
# Note: the "-traditional" above is needed only if "fixincludes" has
# not been run on your machine.

# FLAGS is used by both cc and ld.
# If you want a smaller executable and are willing to lose the ability
# to debug, use "-O" instead of "-g".
FLAGS = -O

# these flags are passed only to ld
LDFLAGS = $(FLAGS)

# additional flags for gildea's code
UICFLAGS = -Wswitch

#PEDCFLAGS = -pedantic -O -Wimplicit -Wreturn-type -Wunused -Wcomment
#PEDANTICW = -Wformat -Wchar-subscripts -Wshadow -Wpointer-arith
#PEDCFLAGS2 = $(PEDCFLAGS) $(PEDANTICW) -Wmissing-prototypes -Wnested-externs
# for HP Snake: +w1 -z

SDUI_OBJ = sdui-x11.o
SDUI_SRC = sdui-x11.c

# Libraries the UI needs to be linked with.  If your X11 libraries aren't
# installed, you may want to add some -L<dir> flags to LDFLAGS below
# to specify the library directories explicitly.
SDUI_LIBS = -lXaw -lXmu -lXt -lXext -lX11

# override unnecessary SunOS 4.0 stuff:
#TARGET_ARCH=

SD_OBJS = sdmain.o sdinit.o sdutil.o sdbasic.o sdtables.o \
          sdtop.o sd.o sdpreds.o sd12.o sd16.o \
          sdgetout.o sdmoves.o sdtand.o sdconc.o sdistort.o \
          sdsi.o

SD_SRCS = sdmain.c sdinit.c sdutil.c sdbasic.c sdtables.c \
          sdtop.c sd.c sdpreds.c sd12.c sd16.c \
          sdgetout.c sdmoves.c sdtand.c sdconc.c sdistort.c \
          sdsi.c

all: sd mkcalls sd_calls.dat

sd: $(SD_OBJS) $(SDUI_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(SD_OBJS) $(SDUI_OBJ) $(SDUI_LIBS)

mkcalls: mkcalls.o
	$(CC) $(LDFLAGS) -o $@ mkcalls.o

sd_calls.dat: sd_calls.txt mkcalls
	./mkcalls

# TeX outputs front, body, toc; we want front, toc, body

sd_doc-sorted.dvi: sd_doc.dvi
	dviselect -i sd_doc.dvi =1:2 > tempa.dvi
	dviselect -i sd_doc.dvi :_1 > tempb.dvi
	(dviselect -i sd_doc.dvi -s 1: | dviselect =3:) > tempc.dvi
	dviconcat -o sd_doc-sorted.dvi tempa.dvi tempb.dvi tempc.dvi
	rm -f tempa.dvi tempb.dvi tempc.dvi

sd_doc.PS: sd_doc-sorted.dvi
	dvips -U -o sd_doc.PS sd_doc-sorted.dvi


sdui-x11.o: sdui-x11.c
	$(CC) $(CFLAGS) $(UICFLAGS) -c sdui-x11.c

mkcalls.o sdsi.o: paths.h

mkcalls.o sdmove.o sdbasic.o sd16.o: database.h

$(SD_OBJS): sd.h database.h


clean::
	rm -f *~ core *.o sd mkcalls sd_calls.dat sd.tar

lint:
	/usr/lang/alint -ux $(SD_SRCS) $(SDUI_SRC)

DISTFILES = README Relnotes relnotes.archive Imakefile Makefile Sd.ad \
            sd_calls.txt COPYING database.doc sd_doc.txi
DISTSRCS = sd.h database.h paths.h mkcalls.c

distrib:
	tar cvf sd.tar $(DISTFILES) $(SD_SRCS) $(SDUI_SRC) $(DISTSRCS)
