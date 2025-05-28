srcdir = .

RM = del
MAKEINFO = makeinfo --no-split
MAKE_ASCII = makeinfo --no-split --no-headers
TXI2DVI = tex
TXI2PDF = pdftex
DVIPS = dvips
CRFIX = crfix
TXI2HTML = makeinfo --html

.SUFFIXES: .dvi .info .txt .txi .pdf .html .ps .pso .all

all: sessions.all sd_doc.all demo.all relnotes.all relnotes98.all sample.html install

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
	$(TXI2HTML) --no-split --no-header -o $@ $<

# Special ones:

install: install.txi
	$(MAKE_ASCII) --no-validate -o $@ $(srcdir)/install.txi



demo.all: demo.ps demo.pdf demo.txt demo.html

sessions.all: sessions.ps sessions.pdf sessions.txt sessions.html

sd_doc.all: sd_doc.ps sd_doc.pdf sd_doc.txt sd_doc.html

relnotes.all: relnotes.ps relnotes.pdf relnotes.txt relnotes.html
relnotes98.all: relnotes98.ps relnotes98.pdf relnotes98.txt relnotes98.html



ps.all: demo.ps sessions.ps sd_doc.ps relnotes.ps relnotes98.ps \
        appnote1.ps appnote2.ps appnote3.ps appnote4.ps

pdf.all: demo.pdf sessions.pdf sd_doc.pdf relnotes.pdf relnotes98.pdf \
         appnote1.pdf appnote2.pdf appnote3.pdf appnote4.pdf

info.all: demo.info sessions.info sd_doc.info relnotes.info relnotes98.info \
          appnote1.info appnote2.info appnote3.info appnote4.info

text.all: demo.txt sessions.txt sd_doc.txt relnotes.txt relnotes98.txt \
          appnote1.txt appnote2.txt appnote3.txt appnote4.txt

html.all: demo.html sessions.html sd_doc.html \
          relnotes.html relnotes98.html sample.html faq.html \
	  appnote1.html appnote2.html appnote3.html appnote4.html

clean::
	-$(RM) *.aux *.log *.toc *.dvi *.ps *.pso *.pdf *.info *.msg
	-$(RM) *.cp *.cps *.fn *.fns *.ky *.kys *.pg *.pgs
	-$(RM) *.tp *.tps *.vr *.vrs
	-$(RM) install
	-$(RM) faq.html appnote*.html
	-$(RM) sd_doc*.html sd_doc.txt
	-$(RM) sessions*.html sessions.txt
	-$(RM) relnotes*.html relnotes.txt
	-$(RM) relnotes98*.html relnotes98.txt
	-$(RM) demo*.html demo.txt

distclean: clean
	-$(RM) sequence.*
	-$(RM) *~ core
	-$(RM) *.tar *.tar.Z

realclean: distclean
	-$(RM) *.info *.PS sd_doc.txt sd_doc.ps

