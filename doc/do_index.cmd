REM This uses index.html as a source file.
del idx.msg idx.txt idx.zip
echo cd public_html/sd> idx.msg
call make_text index.html idx.msg
pgp -sta +clearsig=on +armor=on idx.msg -u wba -o idx.txt
zip idx idx.txt
