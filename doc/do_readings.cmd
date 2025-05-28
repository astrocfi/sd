del rdg.msg rdg.txt rdg.zip
echo cd public_html/sd> rdg.msg
call make_text readings.html rdg.msg
pgp -sta +clearsig=on +armor=on rdg.msg -u wba -o rdg.txt
zip rdg rdg.txt
