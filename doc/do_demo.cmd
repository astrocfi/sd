make demo.all
del dem.msg dem.txt dem.zip
echo cd sd> dem.msg
call make_text demo.ps dem.msg
call make_text demo.txt dem.msg
echo cd ../public_html/sd/demo>> dem.msg
echo uufile>> dem.msg
uuencode demo.pdf|uufix>> dem.msg
call make_text demo.html dem.msg
pgp -sta +clearsig=on +armor=on dem.msg -u wba -o dem.txt
zip dem dem.txt
