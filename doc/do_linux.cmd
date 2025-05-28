REM        This creates the program executable, for
REM           uploading to the web.
REM
REM        You must first do an appropriate Linux "make",
REM           and bring the files "sdtty" and "mkcalls" back
REM           to \wba\sd.
REM
REM This uses the files sdtty, mkcalls,
REM         sd_calls.txt, and sd_calls.dat, readme
REM         sample1.ini, sample2.ini, sample3.ini.
REM
del sdtty.tar sdtty.gz
copy ..\sd\sdtty \temp\sdtty
copy ..\sd\mkcalls \temp\mkcalls
crunfix ..\sd\sd_calls.txt \temp\sd_calls.txt
copy ..\sd\sd_calls.dat \temp\sd_calls.dat
crunfix readme \temp\readme
crunfix sample1.ini \temp\sample1.ini
crunfix sample2.ini \temp\sample2.ini
crunfix sample3.ini \temp\sample3.ini
pushd \temp
tar cvf sdtty.tar sdtty mkcalls sd_calls.txt sd_calls.dat sample1.ini sample2.ini sample3.ini readme
popd
copy \temp\sdtty.tar .
gzip sdtty.tar -c > sdtty.gz
del sdtty.tar
del binu.txt binu.zip binu.asc
echo cd sd> binu.msg
echo uufile>> binu.msg
uuencode sdtty.gz|uufix>> binu.msg
echo mv sdtty.gz sdtty.tar.gz>> binu.msg
pgp -sta +clearsig=on +armor=on binu.msg -u wba -o binu.asc
del binu.msg
ren binu.asc binu.txt
REM zip binu binu.txt
REM
REM         The file "binu.zip" may now be unzipped to "binu.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/sdtty.tar.gz
