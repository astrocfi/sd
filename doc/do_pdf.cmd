mingw32-make pdf.all
del pdfdoc.zip pdfdoc.tar pdfdoc.tar.gz
REM del pdfdoc.lzh pdfdoc.exe pdfdoc.zip pdfdoc.tar
"C:\program files\winzip\wzzip" pdfdoc sd_doc.pdf demo.pdf sessions.pdf relnotes.pdf relnotes98.pdf relnotes03.pdf appnote1.pdf appnote2.pdf appnote3.pdf appnote4.pdf
tar cvf pdfdoc.tar sd_doc.pdf demo.pdf sessions.pdf relnotes.pdf relnotes98.pdf relnotes03.pdf appnote1.pdf appnote2.pdf appnote3.pdf appnote4.pdf
gzip pdfdoc.tar -c > pdfdoc.tar.gz
del pdfdoc.tar
