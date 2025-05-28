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
del install3.lzh
lha a install3 ..\sd\sdtty.exe ..\sd\mkcalls.exe
lha a install3 ..\sd\sd_calls.txt ..\sd\sd_calls.dat
lha a install3 ..\sd\SDTTY.lnk ..\sd\SDTTY.ico
lha a install3 ..\sd\SDTTY.pif ..\sd\cwsdpmi.exe
lha a install3 Do_desk.cmd Do_prog.cmd Do_start.cmd
lha a install3 sample1.ini sample2.ini sample3.ini readme
lha s install3
del bin3.txt bin3.zip
echo cd sd> bin3.msg
echo uufile>> bin3.msg
uuencode install3.exe|uufix>> bin3.msg
pgp -sta +clearsig=on +armor=on bin3.msg -u wba -o bin3.txt
del bin3.msg
zip bin3 bin3.txt
REM
REM         The file "bin3.zip" may now be unzipped to "bin3.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/install3.exe
REM
REM         The file "install3.exe" may now be copied to diskettes.