REM This uses download-unix.html as a source file.
echo cd public_html/sd> dnldunix.msg
call make_text download-unix.html dnldunix.msg
pgp -s +clearsig=off +armor=on +armorlines=0 dnldunix.msg -u wba
zip dnldunix dnldunix.asc
