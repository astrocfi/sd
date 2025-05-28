REM        This creates the program executable, both for
REM           uploading to the web and for distribution
REM           on disk.
REM
REM        You must first do an appropriate MSVC++ build
REM           in directory \wba\sd .
REM
REM This uses the files sd.exe, sdtty.exe, mkcalls.exe, sdlib.dll,
REM         sd_calls.txt, and sd_calls.dat, readme,
REM         SD.lnk, SDTTY.lnk, SD nocheckers.lnk, SD couple.lnk, edit sd.ini.lnk,
REM         sd.ico, sdtty.ico, sdry.ico, sdby.ico, sdcpl.ico,
REM         sample1.ini, sample2.ini, sample3.ini.
REM
del install.exe
del install.zip
"C:\program files\winzip\wzzip" install @zipfiles.lst
"C:\program files\winzip self-extractor\wzipse32" install.zip @zipsecmds.lst
del install.zip
del bin.txt bin.zip bin.asc
echo cd sd> bin.msg
echo uufile>> bin.msg
uuencode install.exe|uufix>> bin.msg
pgp -sta +clearsig=on +armor=on bin.msg -u wba -o bin.asc
del bin.msg
ren bin.asc bin.txt
REM zip bin bin.txt
REM
REM         The file "bin.zip" may now be unzipped to "bin.txt",
REM            which can then be mailed, creating these files:
REM
REM                  sd/install.exe
REM
REM         The file "install.exe" may now be copied to diskettes.
