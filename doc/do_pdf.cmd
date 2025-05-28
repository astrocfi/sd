REM        This creates the self-extracting archive with
REM           all pdf files, for uploading to the web and
REM           for distribution on disk.
REM
del pdfdoc.lzh
lha a pdfdoc demo.pdf sessions.pdf
lha a pdfdoc sd_doc.pdf relnotes.pdf
lha s pdfdoc
del pdf.msg pdf.txt pdf.zip
echo cd sd> pdf.msg
echo uufile>> pdf.msg
uuencode pdfdoc.exe|uufix>> pdf.msg
pgp -sta +clearsig=on +armor=on pdf.msg -u wba -o pdf.txt
zip pdf pdf.txt
REM
REM         The file "pdf.zip" may now be unzipped to "pdf.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/pdfdoc.exe
REM
REM         The file "pdfdoc.exe" may now be copied to diskettes.