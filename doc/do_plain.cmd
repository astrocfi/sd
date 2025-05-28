mingw32-make text.all
del textdoc.zip textdoc.tar textdoc.tar.gz
"C:\program files\winzip\wzzip" textdoc sd_doc.txt demo.txt sessions.txt relnotes.txt relnotes98.txt relnotes03.txt appnote1.txt appnote2.txt appnote3.txt appnote4.txt oldnotes
crunfix sd_doc.txt \temp\sd_doc.txt
crunfix demo.txt \temp\demo.txt
crunfix sessions.txt \temp\sessions.txt
crunfix relnotes.txt \temp\relnotes.txt
crunfix relnotes98.txt \temp\relnotes98.txt
crunfix relnotes03.txt \temp\relnotes03.txt
crunfix appnote1.txt \temp\appnote1.txt
crunfix appnote2.txt \temp\appnote2.txt
crunfix appnote3.txt \temp\appnote3.txt
crunfix appnote4.txt \temp\appnote4.txt
crunfix oldnotes \temp\oldnotes
pushd \temp
tar cvf textdoc.tar sd_doc.txt demo.txt sessions.txt relnotes.txt relnotes98.txt relnotes03.txt appnote1.txt appnote2.txt appnote3.txt appnote4.txt oldnotes
popd
copy \temp\textdoc.tar .
gzip textdoc.tar -c > textdoc.tar.gz
del textdoc.tar
