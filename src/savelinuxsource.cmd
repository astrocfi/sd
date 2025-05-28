REM        This saves the source files in a manner suitable
REM           for transmission to Linux.
REM
REM     Setcygnus must be used.
REM
md sd
for %%f in (*.cpp) do crunfix %%f .\sd\%%f
for %%f in (*.c) do crunfix %%f .\sd\%%f
for %%f in (*.h) do crunfix %%f .\sd\%%f
for %%f in (sample*.ini) do crunfix %%f .\sd\%%f
for %%f in (makefile*) do crunfix %%f .\sd\%%f
crunfix sd_calls.txt .\sd\sd_calls.txt
crunfix db_doc.txt .\sd\db_doc.txt
del linuxsource.tar linuxsource.tar.gz
tar cvf linuxsource.tar sd
del /Q sd\*
rd sd
gzip linuxsource.tar -c > linuxsource.tar.gz
del linuxsource.tar
