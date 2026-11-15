# Completion Stress Tests

These tests check that a change to the parser doesn't change what Sdtty does
when the user presses Tab, space, `?`, or Enter partway through a line. They
build Sdtty from two git refs, usually the base branch and a pull request, type
the same input into both, and report every session whose output differs. Given
a single build, they type the same input into it and report only crashes.

The regular regression suite only presses Enter on finished lines. These tests
press the keys on unfinished ones, over and over: after every prefix of a line,
inside nested brackets, and on random input. That is where a parser shortcut is
most likely to change the match count, refuse a line that should be accepted,
or crash.

They are slow, and they are not part of CI or `run-all-checks.sh`. Run them by
hand when changing `sdmatch.cpp` or the completion code in `sdui-tty.cpp`.

## Requirements

- Linux, `make`, and a C++ compiler. The builds use `makefile.linux64` by
  default; pass `--makefile` to `build` for another platform.
- Python 3.8 or later. Nothing outside the standard library.
- For the `suite-*` tests, a checkout of the private `sd-regression`
  repository. By default the tests look for it next to this repository, as
  described in `CONTRIBUTING.md`; pass `--regression-dir` to point elsewhere.
  The tests read the regression files in place and never copy them.

## Quick Start

```bash
cd tests/completion_stress

# Build the base branch and the branch under test.
./completion_stress.py build base origin/39.86
./completion_stress.py build pr my-parser-branch

# Run the tests, fastest first.
./completion_stress.py lines-keys base pr
./completion_stress.py lines-enter base pr
./completion_stress.py yield base pr
./completion_stress.py suite-keys base pr
./completion_stress.py suite-enter base pr
./completion_stress.py fuzz base pr
./completion_stress.py speed base pr
```

To check one version for crashes only, build it and give each test just that
build:

```bash
/seti/newnav/capped-run.sh ./completion_stress.py build mine            # the working tree
/seti/newnav/capped-run.sh ./completion_stress.py lines-keys mine
/seti/newnav/capped-run.sh ./completion_stress.py suite-enter mine
/seti/newnav/capped-run.sh ./completion_stress.py fuzz mine
```

A sanitizer build (`build --asan`) also catches memory errors and undefined
behavior that don't crash.

## Builds

`build NAME REF` exports `src/` at `REF` into `builds/NAME/` and builds Sdtty
there. Leave out `REF` to build the working tree, uncommitted changes
included. `--asan` builds with AddressSanitizer and UBSan. `--force` replaces
an existing build, and `--jobs N` runs `make` in parallel.

Each build carries a small test-only patch. In `-diagnostic` mode, Sdtty exits
when Tab, space, or Enter can't complete the line, which would end a session at
its first unfinished line. With the patch, when `SD_PROBE` is set in the
environment, Sdtty rings the bell instead, as it does outside `-diagnostic`.
The tests set `SD_PROBE`. Without it, a patched build behaves exactly like an
unpatched one.

Anywhere a test takes a build name, it also takes a path to any directory that
has `src/sdtty` in it. The build is then named after the directory.

## The Tests

Every test except `speed` takes one build or two.

| Test | Input | Sessions |
|---|---|---|
| `lines-keys` | 28 built-in lines with nested brackets, in 4 formations. Tab and space after every prefix, `?` after every `]`, then Enter. | 112 |
| `lines-enter` | The same lines and formations. Enter after every prefix, each in a fresh session. | about 4,300 |
| `yield` | An inner call, such as `boys run`, in `[[...] and circle 1/4` (or `1/2` or `3/4`), after 16 outer calls and concepts, in 5 formations. Tab, space, and `?` at each `]`, then Enter. Two of the inner calls, `tag` and `rotary circulate`, are marked `yield_if_ambiguous`, which Enter's accept test depends on. | 1,680 |
| `suite-keys` | Every regression test with a bracket in it, replayed at its own level. Before each bracketed line, Tab and space after every prefix and `?` after every `]`. | one per test file |
| `suite-enter` | The same tests. Enter at and just after each `]` of each bracketed line, in a fresh session that replays the test up to that line. | about 5,000 |
| `fuzz` | Random lines built from the call, concept, and selector names in the source, then mutated, with Tab, space, hyphen, `?`, and Enter at random points. It reports crashes, not differences, even with two builds. | 1,000 by default |
| `speed` | Three calls that are slow to type on a slow parser, with Tab and space after each `]`. Prints how long each build takes. | 3 per build |

The built-in lines, formations, and calls are in `cases.py`; the random input
generator is in `fuzzgen.py`.

All of the tests take `--workers`, which is half the CPUs by default, and
`--timeout`, in seconds per session. All but `fuzz` and `speed` take
`--limit N` to run only the first N sessions, which is handy for a quick check. `--memory-gb`
caps each Sdtty's address space at 3 GB by default, so a runaway `?` listing
fails on its own instead of taking the machine down. Sanitizer builds skip the
cap, since AddressSanitizer needs a large address space.

## Reading the Results

A session fails if Sdtty crashes, exits with an error, or loops forever once
its input runs out, or if a sanitizer build prints a report. Reports that a
sanitizer build prints at every start-up, before any input, are ignored.

### One Build

Each test prints a line for each failure and a summary at the end:

```text
FAIL ('waves', 'dodge [?'): mine: rc=-11 -> 00000_waves~dodge~.*
done in 1 s: 2 sessions, 1 failed, 0 timed out
```

It exits with status 1 if any session failed, and 0 otherwise. A session that
runs past its timeout is listed but doesn't count as a failure.

### Two Builds

Each test prints a line for each difference and a summary at the end:

```text
DIFFER ('lines', '[[swing thru] and rally but [hinge]] and roll', 29): base rc=0, pr rc=0 -> 00318_lines~swing~thru~and~rally~but~hinge~and~roll~29.*
done in 105 s: 330 sessions, 324 same, 6 differ, 0 fail on pr only, 0 timed out on base and 0 on pr with matching output up to that point
```

It also prints `FAIL` for each failure on either build, and `NEW` when a
session fails on the second build but not the first. It exits with status 1 if
any session differs or is `NEW`, and 0 otherwise. A failure that happens on
both builds, with the same output, doesn't count.

A session that runs past its timeout is cut off. If what it printed by then is
the start of what the other build printed, it counts as matching, and the
summary lists it under "timed out". A slow base build often times out on the
inputs that the parser optimizations speed up.

### Output Files

For each failure or difference, `out/<test>/` gets files with the same stem:
`.input` is what was typed, and there's one file for each build's output, with
its exit status on the first line and its standard error at the end. A
sanitizer build's logs stay in a directory under `out/<test>/`, named in the
`FAIL` line. To rerun a session by hand:

```bash
cd builds/pr/src
SD_PROBE=1 ./sdtty c4x -db sd_calls.dat -diagnostic -no_console -no_graphics \
    -no_color -old_style_filename < ../../../out/lines-enter/00318_lines~swing~thru~and~rally~but~hinge~and~roll~29.input
```

The `suite-*` tests use the level and flags that `doit.sh` uses for each test
file; the others use `c4x` unless given `--level`.

### `fuzz` and `speed`

`fuzz` prints `FAIL` for each failing session and saves its input as
`out/fuzz/seed-N.input`. With one build, it exits with status 1 if any session
failed. With two, it also prints `NEW` for each session that fails on the
second build but not the first, and only those count toward its exit status.
A session is reproducible from its seed as long as `sd_calls.txt`,
`sdctable.cpp`, and `sdtables.cpp` don't change.

`speed` prints each build's time for each call, notes when the builds printed
different output, and always exits with status 0.

`builds/` and `out/` are ignored by git.
