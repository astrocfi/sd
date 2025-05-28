REM        This creates the program executable, for
REM           uploading to the web.
REM
REM        You must first do an appropriate Linux "make",
REM           and bring the files "sdtty" and "mkcalls" back
REM           to \wba\sd.
REM
REM This uses the files sdtty, mkcalls,
REM         sd_calls.txt, and sd_calls.dat.
REM
md sd
del linux.tar linux.tar.gz
copy ..\sd\sdtty .\sd\sdtty
copy ..\sd\mkcalls .\sd\mkcalls
crunfix ..\sd\sd_calls.txt .\sd\sd_calls.txt
copy ..\sd\sd_calls.dat .\sd\sd_calls.dat
chmod 755 .\sd\sdtty
chmod 755 .\sd\mkcalls
chmod 644 .\sd\sd_calls.txt
chmod 644 .\sd\sd_calls.dat
ls -l sd
tar cvf linux.tar sd
pushd sd
del /Q *
popd
rd sd
gzip linux.tar -c > linux.tar.gz
del linux.tar
