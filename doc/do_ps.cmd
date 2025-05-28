mingw32-make ps.all
del psdoc.zip psdoc.tar psdoc.tar.gz
"C:\program files\winzip\wzzip" psdoc sd_doc.ps demo.ps sessions.ps relnotes.ps relnotes98.ps relnotes03.ps appnote1.ps appnote2.ps appnote3.ps appnote4.ps
crunfix sd_doc.ps \temp\sd_doc.ps
crunfix demo.ps \temp\demo.ps
crunfix sessions.ps \temp\sessions.ps
crunfix relnotes.ps \temp\relnotes.ps
crunfix relnotes98.ps \temp\relnotes98.ps
crunfix relnotes03.ps \temp\relnotes03.ps
crunfix appnote1.ps \temp\appnote1.ps
crunfix appnote2.ps \temp\appnote2.ps
crunfix appnote3.ps \temp\appnote3.ps
crunfix appnote4.ps \temp\appnote4.ps
pushd \temp
tar cvf psdoc.tar sd_doc.ps demo.ps sessions.ps relnotes.ps relnotes98.ps relnotes03.ps appnote1.ps appnote2.ps appnote3.ps appnote4.ps
popd
copy \temp\psdoc.tar .
gzip psdoc.tar -c > psdoc.tar.gz
del psdoc.tar
