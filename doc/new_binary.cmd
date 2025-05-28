REM        This creates the program executable, both for
REM           uploading to the web and for distribution
REM           on disk.
REM
REM        You must first do an appropriate MSVC++ build
REM           in directory \wba\sd .
REM
REM        You must also build the html version of the manual,
REM           presumably by doing "do_html".
REM
REM This uses the files sd.exe, sdtty.exe, mkcalls.exe, sdlib.dll, deploy.exe,
REM         sd_calls.txt, and sd_calls.dat, readme,
REM         SD.lnk, SD plain.lnk, SDTTY.lnk, SDTTY plain.lnk, SD nocheckers.lnk,
REM         SD couple.lnk, edit sd.ini.lnk,
REM         sd manual.lnk, Release Notes.lnk, sd_doc.html,
REM         relnotes.html, relnotes98.html
REM         sample1.ini, sample2.ini, sample3.ini.
REM
del install.exe
del install.zip
"C:\program files\winzip\wzzip" install @zipfiles.lst
"C:\program files\winzip self-extractor\wzipse32" install.zip @zipsecmds.lst
del install.zip
REM del bin.txt bin.zip bin.asc
REM echo cd sd> bin.msg
REM echo uufile>> bin.msg
REM uuencode install.exe|uufix>> bin.msg
REM pgp -sta +clearsig=on +armor=on bin.msg -u wba -o bin.asc
REM del bin.msg
REM ren bin.asc bin.txt
REM REM zip bin bin.txt
REM REM
REM REM         The file "bin.zip" may now be unzipped to "bin.txt",
REM REM            which can then be mailed, creating these files:
REM REM
REM REM                  sd/install.exe
REM REM
REM REM         The file "install.exe" may now be copied to diskettes.
