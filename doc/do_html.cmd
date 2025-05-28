REM        This creates the message with
REM           all html files, for uploading to the web.
REM
REM        The poster-child must have been previously installed
REM        with a "do_index <number>" command!!!
REM
make html.all
del html.msg html.txt html.zip html.asc
echo cd public_html/sd/manual>> html.msg
call make_text sd_doc.html html.msg
call make_text sd_doc_toc.html html.msg
call wild_html sd_doc html.msg
echo cd ..>> html.msg
call make_text download-pc.html html.msg
call make_text index.html html.msg
call make_text readings.html html.msg
call make_text relnotes.html html.msg
call make_text relnotes98.html html.msg
call make_text appnote1.html html.msg
call make_text appnote2.html html.msg
call make_text appnote3.html html.msg
echo cd sessions>> html.msg
call make_text sessions.html html.msg
call make_text sessions_toc.html html.msg
echo cd ../demo>> html.msg
call make_text demo.html html.msg
pgp -sta +clearsig=on +armor=on html.msg -u wba -o html.asc
del html.msg
ren html.asc html.txt
REM zip html html.txt
REM
REM         The file "html.zip" may now be unzipped to "html.txt",
REM            which can then be mailed, creating these files:
REM
REM               public_html/sd/manual/sd_doc.html
REM               public_html/sd/manual/sd_doc_toc.html
REM               public_html/sd/manual/sd_doc*.html
REM               public_html/sd/relnotes.html
REM               public_html/sd/relnotes98.html
REM               public_html/sd/appnote1.html
REM               public_html/sd/appnote2.html
REM               public_html/sd/appnote3.html
REM               public_html/sd/sessions/sessions.html
REM               public_html/sd/sessions/sessions_toc.html
