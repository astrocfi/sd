REM        This creates the self-extracting archive with
REM           all plain files, for uploading to the web and
REM           for distribution on disk.
REM
del plaindoc.lzh
lha a plaindoc sd_doc.txt demo.txt oldnotes
lha a plaindoc sessions.txt relnotes.txt
lha s plaindoc
del plain.msg plain.txt plain.zip
echo cd sd> plain.msg
echo uufile>> plain.msg
uuencode plaindoc.exe|uufix>> plain.msg
pgp -sta +clearsig=on +armor=on plain.msg -u wba -o plain.txt
zip plain plain.txt
REM
REM         The file "plain.zip" may now be unzipped to "plain.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/plaindoc.exe
REM
REM         The file "plaindoc.exe" may now be copied to diskettes.