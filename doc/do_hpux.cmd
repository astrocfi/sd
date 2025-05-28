REM        This updates all hpux files (not including documentation) to the web
REM           It creates the gzipped archive
REM                  "sd/sdtty-hpux10.tar.gz"
REM
make readme
del hpux.tar
copy ..\sd\sdtty \temp\sdtty
copy ..\sd\mkcalls \temp\mkcalls
crunfix readme \temp\readme
crunfix ..\sd\sd_calls.txt \temp\sd_calls.txt
copy ..\sd\sd_calls.dat \temp\sd_calls.dat
crunfix sample1.ini \temp\sample1.ini
crunfix sample2.ini \temp\sample2.ini
pushd \temp
tar cvf hpux.tar sdtty mkcalls readme sd_calls.txt sd_calls.dat sample1.ini sample2.ini
popd
copy \temp\hpux.tar .
gzip hpux.tar -c > hpux.gz
del hpux.msg hpux.txt hpux.zip
echo cd sd> hpux.msg
echo uufile>> hpux.msg
uuencode hpux.gz|uufix>> hpux.msg
echo mv hpux.gz sdtty-hpux10.tar.gz>> hpux.msg
pgp -sta +clearsig=on +armor=on hpux.msg -u wba -o hpux.txt
zip hpux hpux.txt
REM
REM         The file "hpux.zip" may now be unzipped to "hpux.txt",
REM            which can then be mailed.
REM         It will upload sd/sdtty-hpux10.tar.gz