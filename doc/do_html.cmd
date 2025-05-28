REM        This creates the message with
REM           all html files, for uploading to the web.
REM
make html.all
del html.msg html.txt html.zip
echo cd public_html/sd/manual>> html.msg
call make_text sd_doc.html html.msg
call make_text sd_doc_toc.html html.msg
call wild_html sd_doc html.msg
echo cd ..>> html.msg
call make_text relnotes.html html.msg
echo cd sessions>> html.msg
call make_text sessions.html html.msg
call make_text sessions_toc.html html.msg
echo cd ../demo>> html.msg
call make_text demo.html html.msg
pgp -sta +clearsig=on +armor=on html.msg -u wba -o html.txt
del html.msg
REM zip html html.txt
REM
REM         The file "html.zip" may now be unzipped to "html.txt",
REM            which can then be mailed, creating these files:
REM
REM               public_html/sd/manual/sd_doc.html
REM               public_html/sd/manual/sd_doc_toc.html
REM               public_html/sd/manual/sd_doc*.html
REM               public_html/sd/relnotes.html
REM               public_html/sd/sessions/sessions.html
REM               public_html/sd/sessions/sessions_toc.html
