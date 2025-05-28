REM This uses download-pc.html as a source file.
del dnp.msg dnp.txt dnp.zip dnp.asc
echo cd public_html/sd> dnp.msg
call make_text download-pc.html dnp.msg
pgp -sta +clearsig=on +armor=on dnp.msg -u wba -o dnp.asc
ren dnp.asc dnp.txt
del dnp.msg
REM zip dnp dnp.txt
