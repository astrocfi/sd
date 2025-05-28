..\sdtty c4 -db ..\sd_calls.dat -diagnostic -no_console -no_graphics -no_color %2 < %1 > %1.tmp
fc /l /5 /n %1.tmp %1.ref
