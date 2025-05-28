del install.exe
del install.zip
"C:\program files\winzip\wzzip" install @zipfiles.lst
REM This is the old winzipse from Windows XP.  It works fine.
"C:\program files (x86)\winzip self-extractor\wzipse32" install.zip @zipsecmds.lst
