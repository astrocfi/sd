REM This uses sample.html as a source file.
echo cd public_html/sd/sample> sample.msg
call make_text sample.html sample.msg
pgp -sta sample.msg -u wba
zip sample sample.asc
