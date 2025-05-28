REM        This creates the program executable, both for
REM           uploading to the web and for distribution
REM           on disk.
REM
REM        You must first do an appropriate MSVC++ build
REM           in directory \wba\sd .
REM
REM This uses the files sd.exe, mkcalls.exe,
REM         sd_calls.txt, and sd_calls.dat, readme,
REM         sdtty.lnk, sdtty.pif,
REM         Do_desk.cmd, Do_prog.cmd, Do_start.cmd,
REM         sample1.ini, sample2.ini, sample3.ini.
del installs.lzh
lha a installs ..\sd\sd.exe ..\sd\mkcalls.exe
lha a installs ..\sd\sd_calls.txt ..\sd\sd_calls.dat
lha a installs ..\sd\SDTTY.lnk ..\sd\SDTTY.pif
lha a installs Do_desk.cmd Do_prog.cmd Do_start.cmd
lha a installs sample1.ini sample2.ini sample3.ini readme
lha s installs
del bins.msg bins.txt bins.zip
echo cd sd> bins.msg
echo uufile>> bins.msg
uuencode installs.exe|uufix>> bins.msg
pgp -sta +clearsig=on +armor=on bins.msg -u wba -o bins.txt
zip bins bins.txt
REM
REM         The file "bins.zip" may now be unzipped to "bins.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/installs.exe
REM
REM         The file "installs.exe" may now be copied to diskettes.