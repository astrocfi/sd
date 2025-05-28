REM        This creates the program executable, both for
REM           uploading to the web and for distribution
REM           on disk.
REM
REM        You must first do an appropriate DJGPP "make"
REM           in directory \wba\sd .
REM
REM This uses the files sdtty.exe, mkcalls.exe,
REM         sd_calls.txt, and sd_calls.dat, readme
REM         cwsdpmi.exe, sdtty.lnk, sdtty.pif, sdtty.ico,
REM         sample1.ini, sample2.ini, sample3.ini
REM         and the shortcut setup command files
del installw.lzh
lha a installw ..\sd\sdtty.exe ..\sd\mkcalls.exe
lha a installw ..\sd\sd_calls.txt ..\sd\sd_calls.dat
lha a installw ..\sd\SDTTY.lnk ..\sd\SDTTY.ico
lha a installw ..\sd\SDTTY.pif ..\sd\cwsdpmi.exe
lha a installw Do_desk.cmd Do_prog.cmd Do_start.cmd
lha a installw sample1.ini sample2.ini sample3.ini readme
lha s installw
del binw.msg binw.txt binw.zip
echo cd sd> binw.msg
echo uufile>> binw.msg
uuencode installw.exe|uufix>> binw.msg
pgp -sta +clearsig=on +armor=on binw.msg -u wba -o binw.txt
zip binw binw.txt
REM
REM         The file "binw.zip" may now be unzipped to "binw.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/installw.exe
REM
REM         The file "installw.exe" may now be copied to diskettes.