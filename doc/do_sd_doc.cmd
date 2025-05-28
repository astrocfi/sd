make sd_doc.all
del sdd.msg sdd.txt sdd.zip
echo cd sd> sdd.msg
call make_text sd_doc.ps sdd.msg
call make_text sd_doc.txt sdd.msg
echo cd ../public_html/sd/manual>> sdd.msg
echo uufile>> sdd.msg
uuencode sd_doc.pdf|uufix>> sdd.msg
call make_text sd_doc.html sdd.msg
call make_text sd_doc_toc.html sdd.msg
call wild_html sd_doc sdd.msg
pgp -sta +clearsig=on +armor=on sdd.msg -u wba -o sdd.txt
zip sdd sdd.txt
