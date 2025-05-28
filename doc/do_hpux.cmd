REM        This updates the hpux build (not including documentation) to the web
REM           It creates the gzipped archive
REM                  "sd/sd-hpux10.tar.gz"
REM
REM      Get the binary file hpux.gz from hpanee1 into sddoc before
REM              running this.
REM
del hpux.msg hpux.txt hpux.zip
echo cd sd> hpux.msg
echo uufile>> hpux.msg
uuencode hpux.gz|uufix>> hpux.msg
echo mv hpux.gz sd-hpux10.tar.gz>> hpux.msg
pgp -sta +clearsig=on +armor=on hpux.msg -u wba -o hpux.txt
zip hpux hpux.txt
REM
REM         The file "hpux.zip" may now be unzipped to "hpux.txt",
REM            which can then be mailed.
REM         It will upload sd/sd-hpux10.tar.gz