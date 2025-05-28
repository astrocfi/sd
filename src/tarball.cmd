del /q tar\*
for %%f in (*.c) do crunfix %%f tar\%%f
for %%f in (*.h) do crunfix %%f tar\%%f
for %%f in (*.cmd) do crunfix %%f tar\%%f
for %%f in (makefile.*) do crunfix %%f tar\%%f
for %%f in (sd_calls.txt) do crunfix %%f tar\%%f
for %%f in (db_doc.txt) do crunfix %%f tar\%%f
for %%f in (pcfiles.lst) do crunfix %%f tar\%%f
pushd tar
del *~
tar cvf ..\tr%1 *
popd
gzip tr%1
