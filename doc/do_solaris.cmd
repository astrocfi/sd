REM        This creates the program executable, for
REM           uploading to the web.
REM
REM        You must first produce the files "sdtty" and "mkcalls"
REM           in \wba\sd.
REM
REM This uses the files sdtty, mkcalls,
REM         sd_calls.txt, and sd_calls.dat
REM         sample1.ini, sample2.ini, sample3.ini.
REM
md sd
del solaris.tar solaris.tar.gz
copy ..\sd\sdtty .\sd\sdtty
copy ..\sd\mkcalls .\sd\mkcalls
crunfix ..\sd\sd_calls.txt .\sd\sd_calls.txt
copy ..\sd\sd_calls.dat .\sd\sd_calls.dat
crunfix ..\sd\sample1.ini .\sd\sample1.ini
crunfix ..\sd\sample2.ini .\sd\sample2.ini
crunfix ..\sd\sample3.ini .\sd\sample3.ini
chmod 755 .\sd\sdtty
chmod 755 .\sd\mkcalls
chmod 644 .\sd\sd_calls.txt
chmod 644 .\sd\sd_calls.dat
chmod 644 .\sd\sample1.ini
chmod 644 .\sd\sample2.ini
chmod 644 .\sd\sample3.ini
ls -l sd
tar cvf solaris.tar sd
pushd sd
del /Q *
popd
rd sd
gzip solaris.tar -c > solaris.tar.gz
del solaris.tar
