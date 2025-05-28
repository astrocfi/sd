echo off
for %%f in (l*t) do call do1 %%f -warn_on_overflow
for %%f in (z*t) do call dolvl c4 %%f -warn_on_overflow
for %%f in (u*t) do call dolvl c4 %%f -warn_on_overflow
for %%f in (v*t) do call do1 %%f -warn_on_overflow
for %%f in (p*t) do call do1 %%f
for %%f in (b*t) do call dolvl c4 %%f -warn_on_overflow
for %%f in (y*t) do call dolvl c4 %%f -warn_on_overflow
for %%f in (s*t) do call do1 %%f -warn_on_overflow
for %%f in (h*t) do call dolvl c4 %%f -warn_on_overflow
for %%f in (xm*t) do call dolvl m %%f
for %%f in (xa1*t) do call dolvl a1 %%f
for %%f in (xa2*t) do call dolvl a2 %%f
for %%f in (xd*t) do call dolvl c2 %%f
for %%f in (xc3*t) do call dolvl c3 %%f
for %%f in (xr*t) do call dolvlr m -5 %%f
echo on
