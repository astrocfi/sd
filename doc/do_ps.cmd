REM        This creates the self-extracting archive with
REM           all ps files, for uploading to the web and
REM           for distribution on disk.
REM
del psdoc.lzh
lha a psdoc sd_doc.ps demo.ps
lha a psdoc sessions.ps relnotes.ps
lha s psdoc
del ps.msg ps.txt ps.zip
echo cd sd> ps.msg
echo uufile>> ps.msg
uuencode psdoc.exe|uufix>> ps.msg
pgp -sta +clearsig=on +armor=on ps.msg -u wba -o ps.txt
zip ps ps.txt
REM
REM         The file "ps.zip" may now be unzipped to "ps.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/psdoc.exe
REM
REM         The file "psdoc.exe" may now be copied to diskettes.