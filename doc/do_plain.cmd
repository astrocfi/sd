REM        This updates all plain files to the web, 3 times over:
REM           It creates the self-extracting archive
REM                   "sd/textdoc.exe"
REM           It creates the gzipped archive
REM                  "sd/textdoc.tar.gz"
REM           It creates the individual files for downloading
REM                   "sd/sd_doc.txt"
REM                   "sd/demo.txt"
REM                   "sd/sessions.txt"
REM                   "sd/relnotes.txt"
REM                   "sd/relnotes98.txt"
REM                   "sd/appnote1.txt"
REM                   "sd/appnote2.txt"
REM                   "sd/appnote3.txt"
REM
make text.all
del textdoc.lzh textdoc.exe textdoc.tar
lha a textdoc sd_doc.txt demo.txt sessions.txt relnotes.txt relnotes98.txt appnote1.txt appnote2.txt appnote3.txt oldnotes
lha s textdoc
del textdoc.lzh
crunfix sd_doc.txt \temp\sd_doc.txt
crunfix demo.txt \temp\demo.txt
crunfix sessions.txt \temp\sessions.txt
crunfix relnotes.txt \temp\relnotes.txt
crunfix relnotes98.txt \temp\relnotes98.txt
crunfix appnote1.txt \temp\appnote1.txt
crunfix appnote2.txt \temp\appnote2.txt
crunfix appnote3.txt \temp\appnote3.txt
crunfix oldnotes \temp\oldnotes
pushd \temp
tar cvf textdoc.tar sd_doc.txt demo.txt sessions.txt relnotes.txt relnotes98.txt appnote1.txt appnote2.txt appnote3.txt oldnotes
popd
copy \temp\textdoc.tar .
gzip textdoc.tar -c > textdoc.gz
del textdoc.tar plain.msg plain.txt plain.zip plain.asc
echo cd sd> plain.msg
echo uufile>> plain.msg
uuencode textdoc.exe|uufix>> plain.msg
echo uufile>> plain.msg
uuencode textdoc.gz|uufix>> plain.msg
echo mv textdoc.gz textdoc.tar.gz>> plain.msg
call make_text sd_doc.txt plain.msg
call make_text demo.txt plain.msg
call make_text sessions.txt plain.msg
call make_text relnotes.txt plain.msg
call make_text relnotes98.txt plain.msg
call make_text appnote1.txt plain.msg
call make_text appnote2.txt plain.msg
call make_text appnote3.txt plain.msg
pgp -sta +clearsig=on +armor=on plain.msg -u wba -o plain.asc
del plain.msg
ren plain.asc plain.txt
REM zip plain plain.txt
REM
REM         The file "plain.zip" may now be unzipped to "plain.txt",
REM            which can then be mailed.
REM
REM         The file "textdoc.exe" may also be copied to diskettes.
