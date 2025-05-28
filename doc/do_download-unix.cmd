REM This uses download-unix.html as a source file.
del dnu.msg dnu.txt dnu.zip
echo cd public_html/sd> dnu.msg
call make_text download-unix.html dnu.msg
pgp -sta +clearsig=on +armor=on dnu.msg -u wba -o dnu.txt
zip dnu dnu.txt
