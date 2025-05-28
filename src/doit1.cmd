echo off
..\sdtty m -db ..\sd_calls.dat -no_console -write_list mlist.tmp
fc /l /5 /n mlist.tmp mlist.ref
for %%f in (t*t) do call do1 %%f
for %%f in (o*t) do call do1 %%f -warn_on_overflow
for %%f in (c*t) do call dolvl c4 %%f -warn_on_overflow
for %%f in (k*t) do call do1 %%f -warn_on_overflow
echo on
