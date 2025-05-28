REM        This updates all ps files to the web, 3 times over:
REM           It creates the self-extracting archive
REM                   "sd/psdoc.exe"
REM           It creates the gzipped archive
REM                  "sd/psdoc.tar.gz"
REM           It creates the individual files for downloading
REM                   "sd/sd_doc.ps"
REM                   "sd/demo.ps"
REM                   "sd/sessions.ps"
REM                   "sd/relnotes.ps"
REM                   "sd/appnote1.ps"
REM                   "sd/appnote2.ps"
REM                   "sd/appnote3.ps"
REM
make ps.all
del psdoc.lzh psdoc.exe psdoc.tar
lha a psdoc sd_doc.ps demo.ps sessions.ps relnotes.ps appnote1.ps appnote2.ps appnote3.ps
lha s psdoc
del psdoc.lzh
crunfix sd_doc.ps \temp\sd_doc.ps
crunfix demo.ps \temp\demo.ps
crunfix sessions.ps \temp\sessions.ps
crunfix relnotes.ps \temp\relnotes.ps
crunfix appnote1.ps \temp\appnote1.ps
crunfix appnote2.ps \temp\appnote2.ps
crunfix appnote3.ps \temp\appnote3.ps
pushd \temp
tar cvf psdoc.tar sd_doc.ps demo.ps sessions.ps relnotes.ps appnote1.ps appnote2.ps appnote3.ps
popd
copy \temp\psdoc.tar .
gzip psdoc.tar -c > psdoc.gz
del psdoc.tar ps.msg ps.txt ps.zip ps.asc
echo cd sd> ps.msg
echo uufile>> ps.msg
uuencode psdoc.exe|uufix>> ps.msg
echo uufile>> ps.msg
uuencode psdoc.gz|uufix>> ps.msg
echo mv psdoc.gz psdoc.tar.gz>> ps.msg
call make_text sd_doc.ps ps.msg
call make_text demo.ps ps.msg
call make_text sessions.ps ps.msg
call make_text relnotes.ps ps.msg
call make_text appnote1.ps ps.msg
call make_text appnote2.ps ps.msg
call make_text appnote3.ps ps.msg
pgp -sta +clearsig=on +armor=on ps.msg -u wba -o ps.asc
del ps.msg
ren ps.asc ps.txt
REM zip ps ps.txt
REM
REM         The file "ps.zip" may now be unzipped to "ps.txt",
REM            which can then be mailed.
REM
REM         The file "psdoc.exe" may also be copied to diskettes.