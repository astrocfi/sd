REM        This creates the self-extracting archive with
REM           all pdf files, for uploading to the web and
REM           for distribution on disk.
REM
make pdf.all
del pdfdoc.lzh pdfdoc.exe pdfdoc.tar
lha a pdfdoc sd_doc.pdf demo.pdf sessions.pdf relnotes.pdf
lha s pdfdoc
tar cvf pdfdoc.tar sd_doc.pdf demo.pdf sessions.pdf relnotes.pdf
gzip pdfdoc.tar -c > pdfdoc.gz
del pdf.msg pdf.txt pdf.zip
echo cd sd> pdf.msg
echo uufile>> pdf.msg
uuencode pdfdoc.exe|uufix>> pdf.msg
echo uufile>> pdf.msg
uuencode pdfdoc.gz|uufix>> pdf.msg
echo mv pdfdoc.gz pdfdoc.tar.gz>> pdf.msg
pgp -sta +clearsig=on +armor=on pdf.msg -u wba -o pdf.txt
zip pdf pdf.txt
REM
REM         The file "pdf.zip" may now be unzipped to "pdf.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/pdfdoc.exe
REM                  sd/pdfdoc.gz
REM
REM         The file "pdfdoc.exe" may also be copied to diskettes.