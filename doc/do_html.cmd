REM        This creates the message with
REM           all html files, for uploading to the web.
REM
REM        The poster-child must have been previously installed
REM        with a "do_index <number>" command!!!
REM
make html.all
REM del html.msg html.txt html.zip html.asc
REM echo cd public_html/sd/manual>> html.msg
REM call make_text sd_doc.html html.msg
REM call make_text sd_doc_toc.html html.msg
REM call wild_html sd_doc html.msg
REM echo cd ..>> html.msg
REM call make_text index.html html.msg
REM call make_text intro.html html.msg
REM call make_text download-pc.html html.msg
REM call make_text readings.html html.msg
REM call make_text relnotes.html html.msg
REM call make_text relnotes98.html html.msg
REM call make_text sample.html html.msg
REM call make_text appnote1.html html.msg
REM call make_text appnote2.html html.msg
REM call make_text appnote3.html html.msg
REM call make_text appnote4.html html.msg
REM echo cd sessions>> html.msg
REM call make_text sessions.html html.msg
REM call make_text sessions_toc.html html.msg
REM echo cd ../demo>> html.msg
REM call make_text demo.html html.msg
REM pgp -sta +clearsig=on +armor=on html.msg -u wba -o html.asc
REM del html.msg
REM ren html.asc html.txt
REM REM zip html html.txt
REM REM
REM REM         The file "html.zip" may now be unzipped to "html.txt",
REM REM            which can then be mailed, creating these files:
REM REM
REM REM               public_html/sd/manual/sd_doc.html
REM REM               public_html/sd/manual/sd_doc_toc.html
REM REM               public_html/sd/manual/sd_doc*.html
REM REM               public_html/sd/relnotes.html
REM REM               public_html/sd/relnotes98.html
REM REM               public_html/sd/appnote1.html
REM REM               public_html/sd/appnote2.html
REM REM               public_html/sd/appnote3.html
REM REM               public_html/sd/appnote4.html
REM REM               public_html/sd/sessions/sessions.html
REM REM               public_html/sd/sessions/sessions_toc.html
