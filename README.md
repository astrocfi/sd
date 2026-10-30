# Introduction

Sd is a square dance caller's helper.  The program assists you in
writing sequences
for Western square dancing by doing the checker pushing.  You tell the
program what call you want to call next, and it computes the resulting
setup and shows it to you.

## Building on macOS

Install the Xcode command-line tools with `xcode-select --install`, then run
from the repository root:

```sh
cd src
make -f makefile.macos
./sdtty
```

The build produces the terminal application `sdtty`, the database compiler
`mkcalls`, and the required calls database `sd_calls.dat`. Run `sdtty` from
the directory containing `sd_calls.dat`.

If you previously built with different compiler flags, run
`make -f makefile.macos clean` before rebuilding.
