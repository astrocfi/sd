..\sdtty %1 -db ..\sd_calls.dat -diagnostic -no_console -no_graphics -no_color -resolve_test %2 < %3 > %3.tmp
fc /l /5 /n %3.tmp %3.ref
