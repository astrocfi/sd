REM        This creates the message with
REM           just the index page, download-pc, and readings,
REM           for uploading to the web.
REM
del html.msg html.txt html.zip html.asc
echo cd public_html/sd>> html.msg
REM call make_text download-pc.html html.msg
REM call make_text readings.html html.msg
call make_text index.html html.msg
pgp -sta +clearsig=on +armor=on html.msg -u wba -o html.asc
del html.msg
ren html.asc html.txt
