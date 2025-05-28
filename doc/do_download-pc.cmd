REM This uses download-pc.html as a source file.
echo cd public_html/sd> dnldpc.msg
call make_text download-pc.html dnldpc.msg
pgp -s +clearsig=off +armor=on +armorlines=0 dnldpc.msg -u wba
zip dnldpc dnldpc.asc
