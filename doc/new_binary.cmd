pushd ..\sd
del install.exe
iexpress /N install.sed
del ..\sddoc\install.exe
move install.exe ..\sddoc
REM del install.zip
popd
