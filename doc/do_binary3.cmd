REM        This creates the program executable, both for
REM           uploading to the web and for distribution
REM           on disk.
REM
REM        You must first do an appropriate DJGPP "make"
REM           in directory \wba\sd .
REM
REM This uses the files sdtty.exe, mkcalls.exe,
REM         sd_calls.txt, and sd_calls.dat, readme
REM         CWSDPMI.exe, SDTTY.lnk, SDTTY.pif, sdtty.ico,
REM         sample1.ini, sample2.ini, sample3.ini
REM         and the shortcut setup command files.
REM
del install3.lzh install3.exe
lha a install3 ..\sd\sdtty.exe ..\sd\mkcalls.exe
lha a install3 ..\sd\sd_calls.txt ..\sd\sd_calls.dat
lha a install3 ..\sd\SDTTY.lnk ..\sd\sdtty.ico
lha a install3 SDTTY.pif CWSDPMI.exe
lha a install3 sample1.ini sample2.ini sample3.ini readme
lha s install3
ren INSTALL3.EXE install3.exe
del install3.lzh
REM del bin3.txt bin3.zip bin3.asc
REM echo cd sd> bin3.msg
REM echo uufile>> bin3.msg
REM uuencode install3.exe|uufix>> bin3.msg
REM pgp -sta +clearsig=on +armor=on bin3.msg -u wba -o bin3.asc
REM del bin3.msg
REM ren bin3.asc bin3.txt
REM REM zip bin3 bin3.txt
REM REM
REM REM         The file "bin3.zip" may now be unzipped to "bin3.txt",
REM REM            which can then be mailed, creating these files:
REM REM
REM REM                  sd/install3.exe
REM REM
REM REM         The file "install3.exe" may now be copied to diskettes.
