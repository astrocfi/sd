REM        This creates the self-extracting archive with
REM           all ps files, for uploading to the web and
REM           for distribution on disk.
REM
make ps.all
del psdoc.lzh psdoc.exe psdoc.tar
lha a psdoc sd_doc.ps demo.ps sessions.ps relnotes.ps
lha s psdoc
crunfix sd_doc.ps \temp\sd_doc.ps
crunfix demo.ps \temp\demo.ps
crunfix sessions.ps \temp\sessions.ps
crunfix relnotes.ps \temp\relnotes.ps
pushd \temp
tar cvf psdoc.tar sd_doc.ps demo.ps sessions.ps relnotes.ps
popd
copy \temp\psdoc.tar .
gzip psdoc.tar -c > psdoc.gz
del ps.msg ps.txt ps.zip
echo cd sd> ps.msg
echo uufile>> ps.msg
uuencode psdoc.exe|uufix>> ps.msg
echo uufile>> ps.msg
uuencode psdoc.gz|uufix>> ps.msg
echo mv psdoc.gz psdoc.tar.gz>> ps.msg
pgp -sta +clearsig=on +armor=on ps.msg -u wba -o ps.txt
zip ps ps.txt
REM
REM         The file "ps.zip" may now be unzipped to "ps.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/psdoc.exe
REM                  sd/psdoc.tar.gz
REM
REM         The file "psdoc.exe" may also be copied to diskettes.