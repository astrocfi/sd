REM This uses intro.html as a source file.
echo cd public_html/sd> int.msg
call make_text intro.html int.msg
pgp -s +clearsig=off +armor=on +armorlines=0 int.msg -u wba
zip int int.asc
