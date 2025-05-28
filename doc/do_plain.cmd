REM        This creates the self-extracting archive with
REM           all plain files, for uploading to the web and
REM           for distribution on disk.
REM
make text.all
del textdoc.lzh textdoc.exe textdoc.tar
lha a textdoc sd_doc.txt demo.txt sessions.txt relnotes.txt oldnotes
lha s textdoc
crunfix sd_doc.txt \temp\sd_doc.txt
crunfix demo.txt \temp\demo.txt
crunfix sessions.txt \temp\sessions.txt
crunfix relnotes.txt \temp\relnotes.txt
crunfix oldnotes \temp\oldnotes
pushd \temp
tar cvf textdoc.tar sd_doc.txt demo.txt sessions.txt relnotes.txt oldnotes
popd
copy \temp\textdoc.tar .
gzip textdoc.tar -c > textdoc.gz
del plain.msg plain.txt plain.zip
echo cd sd> plain.msg
echo uufile>> plain.msg
uuencode textdoc.exe|uufix>> plain.msg
echo uufile>> plain.msg
uuencode textdoc.gz|uufix>> plain.msg
echo mv textdoc.gz textdoc.tar.gz>> plain.msg
pgp -sta +clearsig=on +armor=on plain.msg -u wba -o plain.txt
zip plain plain.txt
REM
REM         The file "plain.zip" may now be unzipped to "plain.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/textdoc.exe
REM                  sd/textdoc.tar.gz
REM
REM         The file "textdoc.exe" may also be copied to diskettes.