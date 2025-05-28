REM        This updates all pdf files to the web, 3 times over:
REM           It creates the self-extracting archive
REM                   "sd/pdfdoc.exe"
REM           It creates the gzipped archive
REM                 "sd/pdfdoc.tar.gz"
REM           It creates the individual files for browsing
REM              "public_html/sd/relnotes.pdf"
REM              "public_html/sd/demo/demo.pdf"
REM            "public_html/sd/manual/sd_doc.pdf"
REM           "public_html/sd/sessions/sessions.pdf"
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
echo cd ../public_html/sd>> pdf.msg
echo uufile>> pdf.msg
uuencode relnotes.pdf|uufix>> pdf.msg
echo cd demo>> pdf.msg
echo uufile>> pdf.msg
uuencode demo.pdf|uufix>> pdf.msg
echo cd ../manual>> pdf.msg
echo uufile>> pdf.msg
uuencode sd_doc.pdf|uufix>> pdf.msg
echo cd ../sessions>> pdf.msg
echo uufile>> pdf.msg
uuencode sessions.pdf|uufix>> pdf.msg
pgp -sta +clearsig=on +armor=on pdf.msg -u wba -o pdf.txt
del pdf.msg
REM zip pdf pdf.txt
REM
REM         The file "pdf.txt" may now be mailed.
REM
REM         The file "pdfdoc.exe" may also be copied to diskettes.
