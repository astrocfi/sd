REM       ********* formatting sd_doc *********

make sd_doc.pdf
make sd_toc.ps
REM perl tocfront sd_toc.ps > sd_doc.ps

make sd_doc.txt
make sd_doc.info
make sd_doc.html
make sd_doc_toc.html

REM #       ********* packing file sd_doc.ps *********

REM cat > web_manual_msg - <<!
REM cd sd
REM textfile sd_doc.ps << EOFx464pqEOF
REM !

REM cat sd_doc.ps >> web_manual_msg

REM #       ********* packing file sd_doc.txt *********

REM cat >> web_manual_msg - <<!
REM EOFx464pqEOF
REM textfile sd_doc.txt << EOFx464pqEOF
REM !

REM cat sd_doc.txt >> web_manual_msg

REM #       ********* packing file sd_doc.html *********

REM cat >> web_manual_msg - <<!
REM EOFx464pqEOF
REM cd ../public_html/sd/manual
REM textfile sd_doc.html << EOFx464pqEOF
REM !

REM cat sd_doc.html >> web_manual_msg

REM #       ********* packing files sd_doc_NN.html *********

REM for index in sd_doc_*.html
REM do

REM cat >> web_manual_msg - <<!
REM EOFx464pqEOF
REM textfile $index << EOFx464pqEOF
REM !

REM cat $index >> web_manual_msg
REM done

REM cat >> web_manual_msg - <<!
REM EOFx464pqEOF
REM !

REM #       ********* now "web_manual_msg" should have stuff.

REM       ********* formatting sessions *********

make sessions.ps
make sessions.pdf
make sessions.txt
make sessions.info
make sessions.html
make sessions_toc.html

REM #       ********* packing file sessions.ps *********

REM cat > web_sessions_msg - <<!
REM cd sd
REM textfile sessions.ps << EOFx464pqEOF
REM !

REM cat sessions.ps >> web_sessions_msg

REM #       ********* packing file sessions.txt *********

REM cat >> web_sessions_msg - <<!
REM EOFx464pqEOF
REM textfile sessions.txt << EOFx464pqEOF
REM !

REM cat sessions.txt >> web_sessions_msg

REM #       ********* packing file sessions.html *********

REM cat >> web_sessions_msg - <<!
REM EOFx464pqEOF
REM cd ../public_html/sd/sessions
REM textfile sessions.html << EOFx464pqEOF
REM !

REM cat sessions.html >> web_sessions_msg

REM #       ********* packing files sessions_NN.html *********

REM for index in sessions_*.html
REM do

REM cat >> web_sessions_msg - <<!
REM EOFx464pqEOF
REM textfile $index << EOFx464pqEOF
REM !

REM cat $index >> web_sessions_msg
REM done

REM cat >> web_sessions_msg - <<!
REM EOFx464pqEOF
REM !

REM #       ********* now "web_sessions_msg" should have stuff.

REM       ********* formatting relnotes *********

make relnotes.ps
make relnotes.pdf
make relnotes.txt
make relnotes.html

REM       ********* packing relnotes into web_relnotes_msg *********

type piece_cdsd      > web_relnotes_msg

type piece_t_rel_ps >> web_relnotes_msg
type relnotes.ps    >> web_relnotes_msg
type piece_eof      >> web_relnotes_msg

type piece_t_rel_tx >> web_relnotes_msg
type relnotes.txt   >> web_relnotes_msg
type piece_eof      >> web_relnotes_msg

REM type piece_t_rel_pd >> web_relnotes_msg
REM type relnotes.pdf   >> web_relnotes_msg
REM type piece_eof      >> web_relnotes_msg

type piece_cdhtmlsd >> web_relnotes_msg

type piece_t_rel_ht >> web_relnotes_msg
type relnotes.html  >> web_relnotes_msg
type piece_eof      >> web_relnotes_msg

REM       ********* now "web_relnotes_msg" should have stuff.

REM       ********* formatting demo *********

make demo.ps
make demo.pdf
make demo.txt
make demo.html

REM       ********* formatting sample *********

make sample.html

REM       ********* formatting README and INSTALL *********

make README
make INSTALL
