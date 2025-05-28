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
REM del textdoc.lzh textdoc.exe textdoc.zip textdoc.tar
del textdoc.zip textdoc.tar textdoc.tar.gz
"C:\program files\winzip\wzzip" textdoc sd_doc.txt demo.txt sessions.txt relnotes.txt relnotes98.txt appnote1.txt appnote2.txt appnote3.txt oldnotes
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
gzip textdoc.tar -c > textdoc.tar.gz
del textdoc.tar
REM del textdoc.tar plain.msg plain.txt plain.zip plain.asc
REM echo cd sd> plain.msg
REM echo uufile>> plain.msg
REM uuencode textdoc.exe|uufix>> plain.msg
REM echo uufile>> plain.msg
REM uuencode textdoc.gz|uufix>> plain.msg
REM echo mv textdoc.gz textdoc.tar.gz>> plain.msg
REM call make_text sd_doc.txt plain.msg
REM call make_text demo.txt plain.msg
REM call make_text sessions.txt plain.msg
REM call make_text relnotes.txt plain.msg
REM call make_text relnotes98.txt plain.msg
REM call make_text appnote1.txt plain.msg
REM call make_text appnote2.txt plain.msg
REM call make_text appnote3.txt plain.msg
REM pgp -sta +clearsig=on +armor=on plain.msg -u wba -o plain.asc
REM del plain.msg
REM ren plain.asc plain.txt
REM REM zip plain plain.txt
REM REM
REM REM         The file "plain.zip" may now be unzipped to "plain.txt",
REM REM            which can then be mailed.
REM REM
REM REM         The file "textdoc.exe" may also be copied to diskettes.
