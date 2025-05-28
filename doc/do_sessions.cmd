make sessions.all
del ses.msg ses.txt ses.zip
echo cd sd> ses.msg
call make_text sessions.ps ses.msg
call make_text sessions.txt ses.msg
echo uufile>> ses.msg
uuencode sessions.pdf|uufix>> ses.msg
echo cd ../public_html/sd/sessions>> ses.msg
call make_text sessions.html ses.msg
call make_text sessions_toc.html ses.msg
REM call wild_html sessions ses.msg
REM pgp -s +clearsig=off +armor=on +armorlines=0 ses.msg -u wba
pgp -sta +clearsig=on +armor=on ses.msg -u wba -o ses.txt
zip ses ses.txt
