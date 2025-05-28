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
REM                   "sd/relnotes98.ps"
REM                   "sd/appnote1.ps"
REM                   "sd/appnote2.ps"
REM                   "sd/appnote3.ps"
REM
make ps.all
del psdoc.zip psdoc.tar psdoc.tar.gz
REM del psdoc.lzh psdoc.exe psdoc.tar
"C:\program files\winzip\wzzip" psdoc sd_doc.ps demo.ps sessions.ps relnotes.ps relnotes98.ps appnote1.ps appnote2.ps appnote3.ps
crunfix sd_doc.ps \temp\sd_doc.ps
crunfix demo.ps \temp\demo.ps
crunfix sessions.ps \temp\sessions.ps
crunfix relnotes.ps \temp\relnotes.ps
crunfix relnotes98.ps \temp\relnotes98.ps
crunfix appnote1.ps \temp\appnote1.ps
crunfix appnote2.ps \temp\appnote2.ps
crunfix appnote3.ps \temp\appnote3.ps
pushd \temp
tar cvf psdoc.tar sd_doc.ps demo.ps sessions.ps relnotes.ps relnotes98.ps appnote1.ps appnote2.ps appnote3.ps
popd
copy \temp\psdoc.tar .
gzip psdoc.tar -c > psdoc.tar.gz
del psdoc.tar
REM del psdoc.tar ps.msg ps.txt ps.zip ps.asc
REM echo cd sd> ps.msg
REM echo uufile>> ps.msg
REM uuencode psdoc.exe|uufix>> ps.msg
REM echo uufile>> ps.msg
REM uuencode psdoc.gz|uufix>> ps.msg
REM echo mv psdoc.gz psdoc.tar.gz>> ps.msg
REM call make_text sd_doc.ps ps.msg
REM call make_text demo.ps ps.msg
REM call make_text sessions.ps ps.msg
REM call make_text relnotes.ps ps.msg
REM call make_text relnotes98.ps ps.msg
REM call make_text appnote1.ps ps.msg
REM call make_text appnote2.ps ps.msg
REM call make_text appnote3.ps ps.msg
REM pgp -sta +clearsig=on +armor=on ps.msg -u wba -o ps.asc
REM del ps.msg
REM ren ps.asc ps.txt
REM REM zip ps ps.txt
REM REM
REM REM         The file "ps.zip" may now be unzipped to "ps.txt",
REM REM            which can then be mailed.
REM REM
REM REM         The file "psdoc.exe" may also be copied to diskettes.