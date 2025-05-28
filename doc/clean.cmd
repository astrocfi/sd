echo on
rem        This command alone cleans temporary stuff and mailing
rem        files, but not actual derived objects.  With the argument
rem        "clean" (that is, "clean clean") it deletes all derived
rem        objects.
rem
rem
if /i "%1" == "clean" goto realclean
goto plainstuff
:realclean
rem
rem    This is the stuff we delete when we want to make things really clean.
rem       It deletes a lot of useful derived objects.
rem
del sd_doc.html
del sd_doc_*.html
del sd_doc.txt
del sessions.html
del sessions_toc.html
del sessions.txt
del relnotes.html
del relnotes98.html
del relnotes03.html
del relnotes.txt
del relnotes98.txt
del relnotes03.txt
del faq.html
del appnote*.html
del appnote*.txt
del demo.html
del demo.txt
del sample.html
del *.ps
del *.pdf
del psdoc.exe
del textdoc.exe
del pdfdoc.exe
del install.exe
del installw.exe
del install3.exe
del installs.exe
del *.asc
del *.gz
del *.dvi
:plainstuff
rem
rem      This is the stuff we want to clean all the time.  It's just temporary junk.
rem      Well, it's also the "txt" or "zip" files that we mail to upload.
rem
del html.txt
del ps.txt
del plain.txt
del pdf.txt
del dnu.txt
del dnp.txt
del html.zip
del ps.zip
del plain.zip
del pdf.zip
del dnu.zip
del dnp.zip
del sd_doc.info
del demo.info
del sessions.info
del relnotes.info
del relnotes98.info
del relnotes03.info
del appnote*.info
del textdoc.tar
del psdoc.tar
del pdfdoc.tar
del sd_toc.pso
del bin.msg
del binw.msg
del bins.msg
del bins.txt
del bins.zip
del html.msg
del ps.msg
del plain.msg
del pdf.msg
del dnu.msg
del dnp.msg
del *.lzh
del *.log
del *.aux
del *.toc
del *.cp
del *.fn
del *.vr
del *.tp
del *.ky
del *.pg

