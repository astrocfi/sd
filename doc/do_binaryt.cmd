REM        This creates the program executable, both for
REM           uploading to the web and for distribution
REM           on disk.
REM
REM        You must first do an appropriate MSVC++ build
REM           in directory \wba\sd .
REM
REM This uses the files sdtty.exe, mkcalls.exe,
REM         sd_calls.txt, and sd_calls.dat, readme,
REM         SDTTY.lnk, sdtty.ico, edit.sd.ini.lnk,
REM         Do_deskt.cmd, Do_progt.cmd, Do_startt.cmd,
REM         sample1.ini, sample2.ini, sample3.ini
REM         and the shortcut setup command files.
REM
del install.lzh
lha a install ..\sd\sdtty.exe ..\sd\mkcalls.exe
lha a install ..\sd\sd_calls.txt ..\sd\sd_calls.dat
lha a install ..\sd\SDTTY.lnk ..\sd\sdtty.ico ..\sd\edit.sd.ini.lnk
lha a install Do_deskt.cmd Do_progt.cmd Do_startt.cmd
lha a install sample1.ini sample2.ini sample3.ini readme
lha s install
del bin.txt bin.zip
echo cd sd> bin.msg
echo uufile>> bin.msg
uuencode install.exe|uufix>> bin.msg
pgp -sta +clearsig=on +armor=on bin.msg -u wba -o bin.txt
del bin.msg
REM zip bin bin.txt
REM
REM         The file "bin.zip" may now be unzipped to "bin.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/install.exe
REM
REM         The file "install.exe" may now be copied to diskettes.