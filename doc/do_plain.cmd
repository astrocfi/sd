mingw32-make text.all
del textdoc.zip textdoc.tar textdoc.tar.gz
"C:\program files\winzip\wzzip" textdoc sd_doc.txt demo.txt sessions.txt relnotes.txt relnotes98.txt relnotes03.txt relnotes09.txt appnote1.txt appnote2.txt appnote3.txt appnote4.txt
crunfix sd_doc.txt %TEMP%\sd_doc.txt
crunfix demo.txt %TEMP%\demo.txt
crunfix sessions.txt %TEMP%\sessions.txt
crunfix relnotes.txt %TEMP%\relnotes.txt
crunfix relnotes98.txt %TEMP%\relnotes98.txt
crunfix relnotes03.txt %TEMP%\relnotes03.txt
crunfix relnotes09.txt %TEMP%\relnotes09.txt
crunfix appnote1.txt %TEMP%\appnote1.txt
crunfix appnote2.txt %TEMP%\appnote2.txt
crunfix appnote3.txt %TEMP%\appnote3.txt
crunfix appnote4.txt %TEMP%\appnote4.txt
pushd %TEMP%
tar cvf textdoc.tar sd_doc.txt demo.txt sessions.txt relnotes.txt relnotes98.txt relnotes03.txt relnotes09.txt appnote1.txt appnote2.txt appnote3.txt appnote4.txt
popd
copy %TEMP%\textdoc.tar .
gzip textdoc.tar -c > textdoc.tar.gz
del textdoc.tar
