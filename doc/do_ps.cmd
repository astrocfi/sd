mingw32-make ps.all
del psdoc.zip psdoc.tar psdoc.tar.gz
"C:\program files\winzip\wzzip" psdoc sd_doc.ps demo.ps sessions.ps relnotes.ps relnotes98.ps relnotes03.ps appnote1.ps appnote2.ps appnote3.ps appnote4.ps
crunfix sd_doc.ps %TEMP%\sd_doc.ps
crunfix demo.ps %TEMP%\demo.ps
crunfix sessions.ps %TEMP%\sessions.ps
crunfix relnotes.ps %TEMP%\relnotes.ps
crunfix relnotes98.ps %TEMP%\relnotes98.ps
crunfix relnotes03.ps %TEMP%\relnotes03.ps
crunfix appnote1.ps %TEMP%\appnote1.ps
crunfix appnote2.ps %TEMP%\appnote2.ps
crunfix appnote3.ps %TEMP%\appnote3.ps
crunfix appnote4.ps %TEMP%\appnote4.ps
pushd %TEMP%
tar cvf psdoc.tar sd_doc.ps demo.ps sessions.ps relnotes.ps relnotes98.ps relnotes03.ps appnote1.ps appnote2.ps appnote3.ps appnote4.ps
popd
copy %TEMP%\psdoc.tar .
gzip psdoc.tar -c > psdoc.tar.gz
del psdoc.tar
