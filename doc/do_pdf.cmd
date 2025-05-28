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
REM               "public_html/sd/appnote1.pdf"
REM               "public_html/sd/appnote2.pdf"
REM
make pdf.all
del pdfdoc.lzh pdfdoc.exe pdfdoc.tar
lha a pdfdoc sd_doc.pdf demo.pdf sessions.pdf relnotes.pdf appnote1.pdf appnote2.pdf
lha s pdfdoc
del pdfdoc.lzh
tar cvf pdfdoc.tar sd_doc.pdf demo.pdf sessions.pdf relnotes.pdf appnote1.pdf appnote2.pdf
gzip pdfdoc.tar -c > pdfdoc.gz
del pdfdoc.tar pdf.msg pdf.txt pdf.zip pdf.asc
echo cd sd> pdf.msg
echo uufile>> pdf.msg
uuencode pdfdoc.exe|uufix>> pdf.msg
echo uufile>> pdf.msg
uuencode pdfdoc.gz|uufix>> pdf.msg
echo mv pdfdoc.gz pdfdoc.tar.gz>> pdf.msg
echo cd ../public_html/sd>> pdf.msg
echo uufile>> pdf.msg
uuencode relnotes.pdf|uufix>> pdf.msg
echo uufile>> pdf.msg
uuencode appnote1.pdf|uufix>> pdf.msg
echo uufile>> pdf.msg
uuencode appnote2.pdf|uufix>> pdf.msg
echo cd demo>> pdf.msg
echo uufile>> pdf.msg
uuencode demo.pdf|uufix>> pdf.msg
echo cd ../manual>> pdf.msg
echo uufile>> pdf.msg
uuencode sd_doc.pdf|uufix>> pdf.msg
echo cd ../sessions>> pdf.msg
echo uufile>> pdf.msg
uuencode sessions.pdf|uufix>> pdf.msg
pgp -sta +clearsig=on +armor=on pdf.msg -u wba -o pdf.asc
del pdf.msg
ren pdf.asc pdf.txt
REM zip pdf pdf.txt
REM
REM         The file "pdf.txt" may now be mailed.
REM
REM         The file "pdfdoc.exe" may also be copied to diskettes.
