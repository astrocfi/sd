REM This uses index1.htmli, index2.htmli, and the poster as source files.
del index.msg index.txt index.zip index.asc
copy index1.htmli index.html
type poster%1.htmli >> index.html
type index2.htmli >> index.html
REM echo cd public_html/sd> index.msg
REM call make_text index.html index.msg
REM pgp -sta +clearsig=on +armor=on index.msg -u wba -o index.asc
REM ren index.asc index.txt
REM REM zip index index.txt
