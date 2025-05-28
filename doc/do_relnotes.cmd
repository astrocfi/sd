make relnotes.all
del rel.msg rel.txt rel.zip
echo cd sd> rel.msg
call make_text relnotes.ps rel.msg
call make_text relnotes.txt rel.msg
echo uufile>> rel.msg
uuencode relnotes.pdf|uufix>> rel.msg
echo cd ../public_html/sd>> rel.msg
call make_text relnotes.html rel.msg
REM pgp -s +clearsig=off +armor=on +armorlines=0 rel.msg -u wba
pgp -sta +clearsig=on +armor=on rel.msg -u wba -o rel.txt
zip rel rel.txt
