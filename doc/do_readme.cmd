make readme
del rdm.msg rdm.txt rdm.zip
echo cd sd> rdm.msg
call make_text readme rdm.msg
pgp -sta +clearsig=on +armor=on rdm.msg -u wba -o rdm.txt
zip rdm rdm.txt
