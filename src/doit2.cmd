echo off
for %%f in (n*t) do call do1 %%f
for %%f in (r*t) do call dolvl c4 %%f -warn_on_overflow
for %%f in (g*t) do call do1 %%f -warn_on_overflow
echo on
