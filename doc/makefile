# Generated automatically from Makefile.in by configure.
# Makefile for Sd (square dance caller's helper)
# Time-stamp: <96/05/05 10:50:56 gildea>

srcdir = .

RM = del
MAKEINFO = makeinfo --no-split
MAKE_ASCII = makeinfo --no-split --no-headers
TXI2DVI = tex
TXI2PDF = pdftex
DVIPS = dvips
CRFIX = crfix
TXI2HTML = perl texi2html
TOCFRONT = perl tocfront

.SUFFIXES: .dvi .info .txt .txi .pdf .html .ps .pso .all

all: sessions.all sd_doc.all demo.all relnotes.all sample.html readme install

.txi.dvi:
	$(TXI2DVI) $<
	$(TXI2DVI) $<

.txi.pdf:
	$(TXI2PDF) $<
	$(TXI2PDF) $<

.dvi.pso:
	$(DVIPS) -t letter -o $@ $?

.pso.ps:
	$(CRFIX) $? $@

.txi.info:
	$(MAKEINFO) -I$(srcdir) $<

.txi.txt:
	$(MAKE_ASCII) -o $@ -I$(srcdir) $<

.txi.html:
	$(TXI2HTML) -monolithic $<

# Special ones:

sd_doc.pdf: readme.txi

sd_doc.dvi: readme.txi

sd_doc.html: readme.txi

sd_doc.info: readme.txi

sd_doc.txt: readme.txi

sd_doc_toc.html: sd_doc.txi
	$(TXI2HTML) -split_node -menu sd_doc.txi

sessions_toc.html: sessions.txi
	$(TXI2HTML) -split_node -menu sessions.txi

install: install.txi
	$(MAKE_ASCII) --no-validate -o $@ $(srcdir)/install.txi

readme: readme.txi
	$(MAKE_ASCII) --no-validate -o $@ $(srcdir)/readme.txi



demo.all: demo.ps demo.pdf demo.txt demo.html

sessions.all: sessions.ps sessions.pdf sessions.txt sessions.html sessions_toc.html

sd_doc.all: sd_doc.ps sd_doc.pdf sd_doc.txt sd_doc.html sd_doc_toc.html

relnotes.all: relnotes.ps relnotes.pdf relnotes.txt relnotes.html



ps.all: demo.ps sessions.ps sd_doc.ps relnotes.ps appnote1.ps appnote2.ps

pdf.all: demo.pdf sessions.pdf sd_doc.pdf relnotes.pdf appnote1.pdf appnote2.pdf

info.all: demo.info sessions.info sd_doc.info relnotes.info appnote1.info appnote2.info

text.all: demo.txt sessions.txt sd_doc.txt relnotes.txt appnote1.txt appnote2.txt

html.all: demo.html sessions.html sessions_toc.html sd_doc.html sd_doc_toc.html relnotes.html \
          appnote1.html appnote2.html

clean::
	-$(RM) *.aux *.log *.toc *.dvi *.ps *.pso *.pdf *.info *.msg
	-$(RM) *.cp *.cps *.fn *.fns *.ky *.kys *.pg *.pgs
	-$(RM) *.tp *.tps *.vr *.vrs
	-$(RM) readme install
	-$(RM) sd_doc*.html sd_doc.txt
	-$(RM) sessions*.html sessions.txt
	-$(RM) relnotes*.html relnotes.txt
	-$(RM) demo*.html demo.txt
	-$(RM) sample*.html

distclean: clean
	-$(RM) sequence.*
	-$(RM) *~ core
	-$(RM) *.tar *.tar.Z

realclean: distclean
	-$(RM) *.info *.PS sd_doc.txt sd_doc.ps

DOCSRCS = sd_doc.txi readme.txi install.txi gpl.txi
DISTTXT = readme install oldnotes COPYING sample1.ini sample2.ini demo.ps demo.txt sessions.ps sessions.txt \
            relnotes.ps relnotes.txt \
            Imakefile Makefile.in configure.in configure Sd.res sd_calls.txt \
            sd_doc.PS sd_doc.txt db_doc.txt
DISTSRCS = sd.h database.h paths.h sdui-ttu.h sdmatch.h
DISTFILES = $(DISTTXT) $(DOCSRCS) $(SRCS) $(DISTSRCS) $(PCFILES)

TAR = tar
TARFLAGS = 
DIST_TARFILE = sd-31.79.tar
COMPRESS = compress

dist: dist-files
	$(TAR) $(TARFLAGS) $(DIST_TARFILE) $(DISTFILES)
	$(RM) $(DIST_TARFILE).Z
	$(COMPRESS) $(DIST_TARFILE)

dist-files: configure Makefile install readme sd_doc.txt sd_doc.PS \
            demo.ps demo.txt sessions.ps sessions.txt relnotes.ps relnotes.txt pc-files

pcdist: sdtty.exe mkcalls.exe sd_calls.dat
	lha a $@ readme oldnotes
	lha a $@ sdtty.exe mkcalls.exe cwsdpmi.exe sdtty.pif
	lha a $@ sample1.ini sample2.ini demo.txt demo.ps sessions.txt sessions.ps relnotes.txt relnotes.ps
	lha a $@ sd_calls.txt sd_calls.dat
	lha a $@ sd_doc.txt sd_doc.ps
	lha s $@

BIN_TARFILE = sd-31.79-.tar



TARFILES1 = readme oldnotes
TARFILES2 = sd sdtty mkcalls \
	 sample1.ini sample2.ini demo.ps demo.txt sessions.ps sessions.txt \
          relnotes.ps relnotes.txt \
	 sd_calls.txt sd_calls.dat \
	 Sd.res sd_doc.txt sd_doc.PS

$(BIN_TARFILE).Z: Makefile $(TARFILES2) $(TARFILES1)
	$(TAR) $(TARFLAGS) $(BIN_TARFILE) $(TARFILES1) $(TARFILES2)
	$(RM) $@
	-$(COMPRESS) < $(BIN_TARFILE) > $@

pc-files: $(PCFILES)
