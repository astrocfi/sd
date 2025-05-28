REM        This updates all plain pdf files to the web:
REM        It creates the individual files for browsing
REM              "public_html/sd/relnotes.pdf"
REM              "public_html/sd/relnotes98.pdf"
REM              "public_html/sd/demo/demo.pdf"
REM            "public_html/sd/manual/sd_doc.pdf"
REM           "public_html/sd/sessions/sessions.pdf"
REM               "public_html/sd/appnote1.pdf"
REM               "public_html/sd/appnote2.pdf"
REM               "public_html/sd/appnote3.pdf"
REM
del pdf1.msg pdf1.txt pdf1.asc
echo cd public_html/sd>> pdf1.msg
echo uufile>> pdf1.msg
uuencode relnotes.pdf|uufix>> pdf1.msg
echo uufile>> pdf1.msg
uuencode relnotes98.pdf|uufix>> pdf1.msg
echo uufile>> pdf1.msg
uuencode appnote1.pdf|uufix>> pdf1.msg
echo uufile>> pdf1.msg
uuencode appnote2.pdf|uufix>> pdf1.msg
echo uufile>> pdf1.msg
uuencode appnote3.pdf|uufix>> pdf1.msg
echo cd demo>> pdf1.msg
echo uufile>> pdf1.msg
uuencode demo.pdf|uufix>> pdf1.msg
echo cd ../manual>> pdf1.msg
echo uufile>> pdf1.msg
uuencode sd_doc.pdf|uufix>> pdf1.msg
echo cd ../sessions>> pdf1.msg
echo uufile>> pdf1.msg
uuencode sessions.pdf|uufix>> pdf1.msg
pgp -sta +clearsig=on +armor=on pdf1.msg -u wba -o pdf1.asc
REM del pdf1.msg
ren pdf1.asc pdf1.txt
REM zip pdf1 pdf1.txt
REM
REM         The file "pdf1.txt" may now be mailed.
