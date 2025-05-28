REM This uses intro.html as a source file.
del int.msg int.txt int.zip
echo cd public_html/sd> int.msg
call make_text intro.html int.msg
pgp -sta +clearsig=on +armor=on int.msg -u wba -o int.txt
del int.msg
REM zip int int.txt
