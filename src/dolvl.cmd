..\sdtty %1 -db ..\sd_calls.dat -diagnostic -no_console -no_graphics -no_color %3 < %2 > %2.tmp
fc /l /5 /n %2.tmp %2.ref
