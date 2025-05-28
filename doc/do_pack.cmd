REM This packs the files to be uploaded to the web, from which users can manually
REM examine or download various documentation files, and manually initiate a
REM download and execution of the actual program, which is install.exe.
REM That file automatically extracts the files the the user needs.
REM See the script "new_binary.cmd" and the file list "zipfiles.lst" for that list.
REM
REM The rest of these files don't get downloaded to the user's computer unless
REM manually requested by clicking various things on the web pages.
REM
del pack.zip
"C:\program files\winzip\wzzip" pack *.html *.pdf *.txt *.tar.gz getacro.gif pdfdoc.zip textdoc.zip install.exe install3.exe install32.exe sd_source.zip
