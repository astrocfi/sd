REM        This updates all pdf files to the web, 3 times over:
REM           It creates the self-extracting archive
REM                   "sd/pdfdoc.exe"
REM           It creates the gzipped archive
REM                 "sd/pdfdoc.tar.gz"
REM           It creates the individual files for browsing
REM              "public_html/sd/relnotes.pdf"
REM              "public_html/sd/relnotes98.pdf"
REM              "public_html/sd/demo/demo.pdf"
REM            "public_html/sd/manual/sd_doc.pdf"
REM           "public_html/sd/sessions/sessions.pdf"
REM               "public_html/sd/appnote1.pdf"
REM               "public_html/sd/appnote2.pdf"
REM               "public_html/sd/appnote3.pdf"
REM               "public_html/sd/appnote4.pdf"
REM
make pdf.all
del pdfdoc.zip pdfdoc.tar pdfdoc.tar.gz
REM del pdfdoc.lzh pdfdoc.exe pdfdoc.zip pdfdoc.tar
"C:\program files\winzip\wzzip" pdfdoc sd_doc.pdf demo.pdf sessions.pdf relnotes.pdf relnotes98.pdf appnote1.pdf appnote2.pdf appnote3.pdf appnote4.pdf
tar cvf pdfdoc.tar sd_doc.pdf demo.pdf sessions.pdf relnotes.pdf relnotes98.pdf appnote1.pdf appnote2.pdf appnote3.pdf appnote4.pdf
gzip pdfdoc.tar -c > pdfdoc.tar.gz
del pdfdoc.tar
REM del pdfdoc.tar pdf.msg pdf.txt pdf.zip pdf.asc
REM echo cd sd> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode pdfdoc.exe|uufix>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode pdfdoc.gz|uufix>> pdf.msg
REM echo mv pdfdoc.gz pdfdoc.tar.gz>> pdf.msg
REM echo cd ../public_html/sd>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode relnotes.pdf|uufix>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode relnotes98.pdf|uufix>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode appnote1.pdf|uufix>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode appnote2.pdf|uufix>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode appnote3.pdf|uufix>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode appnote4.pdf|uufix>> pdf.msg
REM echo cd demo>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode demo.pdf|uufix>> pdf.msg
REM echo cd ../manual>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode sd_doc.pdf|uufix>> pdf.msg
REM echo cd ../sessions>> pdf.msg
REM echo uufile>> pdf.msg
REM uuencode sessions.pdf|uufix>> pdf.msg
REM pgp -sta +clearsig=on +armor=on pdf.msg -u wba -o pdf.asc
REM del pdf.msg
REM ren pdf.asc pdf.txt
REM REM zip pdf pdf.txt
REM REM
REM REM         The file "pdf.txt" may now be mailed.
REM REM
REM REM         The file "pdfdoc.exe" may also be copied to diskettes.
