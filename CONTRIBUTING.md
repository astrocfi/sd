# Contributing to Sd

Thank you for your interest in contributing to Sd. This document explains how
the project is laid out, how to build and check it, and how its tests run. The
developer guide in the `sd` repository (`docs/`, built with
`scripts/build-dev-guide.sh`) covers the same ground in more depth, starting
with continuous integration.

## The Two Repositories

Sd is developed in two repositories that are used together:

- **`sd`** (public) holds the source (`src/`) and the manuals (`doc/`).
- **`sd-regression`** (private) holds the regression suite. Most of its tests
  are transcriptions of choreography called at real dances. The callers who own
  that material have allowed the Sd developers to use it for testing, but not to
  publish it, so only developers have access.

Never copy a regression test, a reference transcript, or any part of one into
the public `sd` repository, its issues, or its pull requests. Name a test
(`t50t`) and describe a failure in your own words instead.

## Getting Started

Clone both repositories into the same parent directory. The scripts in each
repository find the other one there:

```bash
git clone https://github.com/astrocfi/sd.git
git clone https://github.com/astrocfi/sd-regression.git   # developers only
```

```text
<parent>/
    sd/
    sd-regression/
```

The checks need:

- `make` and a C++ compiler, to build Sdtty;
- `makeinfo` (Texinfo) and TeX (`texlive-base` on Debian and Ubuntu), to build
  the manuals;
- Python 3, for the lint tools and Sphinx. Install them once:

  ```bash
  scripts/run-all-checks.sh --setup
  ```

  This creates a virtual environment in `.build/venv` from
  `scripts/requirements.txt` and `docs/requirements.txt`. Everything
  the scripts build goes under `.build/`, which ignores itself.

## Building

The makefiles are in `src/`, one per platform; there is no default makefile.
To build in place:

```bash
cd src
make -f makefile.linux64
```

`scripts/build-sd.sh` builds a copy of the sources in `.build/sd`, leaving
`src/` untouched, and checks that the result can start a sequence. Choose
another makefile with `--makefile` (for example `makefile.linux64debug`).

## Running the Checks

`scripts/run-all-checks.sh` runs every check; CI runs the same script.

```bash
scripts/run-all-checks.sh               # everything but the regression suite
scripts/run-all-checks.sh --regression  # the regression suite
scripts/run-all-checks.sh --help        # the scopes and single checks
```

| Check | What it does |
|---|---|
| `build` | Builds Sdtty, `mkcalls`, and `sd_calls.dat`, and smoke-tests them. |
| `docs-build` | Builds the manuals in every format, and fails on a new warning. |
| `dev-guide` | Builds the developer guide, and fails on any Sphinx warning. |
| `codespell` | Checks spelling in the documentation, the tooling, and the C++ comments, with American spelling enforced. |
| `prose` | Checks the documentation prose against `.claude/rules/doc_prose_style.md`. |
| `pymarkdown` | Lints the Markdown. |
| `shellcheck` | Lints the shell scripts in `scripts/`. |
| `actionlint` | Lints the GitHub Actions workflows. |
| `regression` | Runs the regression suite. |

The manuals and the README are upstream text, written before these checks. The
findings in them today are recorded in baselines (`scripts/*-baseline.txt`),
keyed by line text, and only a new finding fails a check. A change that fixes a
baseline finding regenerates the baseline with the check's `--update-baseline`
option. A new finding is fixed, not added to a baseline.

## The Regression Suite

Each test is a script of calls (`t50t`) and the transcript Sd should produce
for it (`t50t.ref`). `regression_tests/doit.sh` runs every test: it feeds each
script to `sdtty` on standard input, writes the transcript to a `.tmp` file,
and compares it with the reference. `-diagnostic` makes each run reproducible.

`scripts/run-regression-tests.sh` (which `run-all-checks.sh --regression` runs)
builds Sd, copies the suite into `.build/regression`, warms the menu cache for
every level, runs `doit.sh`, and reports each test by name. A passing run
ends:

```text
regression: 285 passed, 0 failed, 0 not run by doit.sh
regression: passed
```

A failing test is listed as `FAILED <test> (N line(s) differ)`. To see why,
compare `.build/regression/sd-regression/regression_tests/<test>.tmp` with
`<test>.ref` there. `--show-diffs` prints the differences, but only outside
the public repository's CI.

When a change to Sd changes a transcript on purpose, update the reference in
`sd-regression`, on a branch with the same name as the `sd` branch (see below).

## Branches

- `main` is always releasable, and changes reach it through pull requests.
- A version branch (`39.99`) collects the work for one release, in both
  repositories.
- A work branch is named for the version and the change (`39.99-menu-fix`).

A change that alters what the regression suite expects is made on a branch
with **the same name** in both repositories, and the two pull requests are
merged together. CI in either repository tests against the other repository's
branch of the same name; if there is none, against the pull request's base
branch; and otherwise against `main`. A tag is tested against the same tag.
Releases are tagged in both repositories (`v39.85`).

## Regression Tests in CI

The `sd` workflow (`.github/workflows/run-tests.yml`) reads the private suite
with a **deploy key**: an SSH key that can read `sd-regression` and nothing
else. A maintainer with admin access to both repositories sets it up once:

```bash
ssh-keygen -t ed25519 -N '' -C 'sd CI: read sd-regression' -f sd-regression-key
gh repo deploy-key add sd-regression-key.pub --repo astrocfi/sd-regression \
    --title 'sd CI (read-only)'
gh secret set SD_REGRESSION_SSH_KEY --repo astrocfi/sd < sd-regression-key
rm sd-regression-key sd-regression-key.pub
```

`gh repo deploy-key add` creates a read-only key unless it is given
`--allow-write`. If the suite is moved to another repository, set the
`SD_REGRESSION_REPOSITORY` Actions variable in `sd` to its `owner/name`.

The `sd-regression` workflow reads the public `sd` repository and doesn't need
a key.

Because the `sd` repository is public:

- The regression job prints test names and counts, never test content, and
  `scripts/run-regression-tests.sh` refuses `--show-diffs` there.
- Nothing from `sd-regression` is uploaded as an artifact or stored in the
  Actions cache.
- A pull request from a fork doesn't get any secrets, so the regression job is
  skipped for it. A maintainer runs the suite for such a change by pushing its
  branch to `sd`. Dependabot's pull requests skip the job for the same reason.

## Documentation

The manuals are Texinfo files in `doc/`; see `.claude/rules/doc_texinfo.md` and
`.claude/rules/doc_prose_style.md` for how they are written.

```bash
scripts/build-docs.sh        # build every format into .build/docs/output
scripts/read-docs.sh         # build the HTML and open the main manual
scripts/check-doc-prose.sh   # check the prose (--warnings for the heuristics)
scripts/check-spelling.sh    # check the spelling
```

The developer guide is a Sphinx project in `docs/`, in Markdown (MyST). It
follows the prose rule like the rest of the developer documentation.

```bash
scripts/build-dev-guide.sh          # build it into .build/dev-guide/html
scripts/build-dev-guide.sh --open   # build it and open it in a browser
```

## Pull Requests

1. Create a branch (see Branches above).
2. Make the change, and update the manuals if a user can see it.
3. Run `scripts/run-all-checks.sh`, and `--regression` if you have the suite.
4. Open a pull request; the template lists what to fill in.

## Reporting Issues

Report a bug in the public `sd` repository. For a call or concept that gives a
wrong result, use the "Wrong Result from a Call or Concept" template: the call,
what it did, what it should do, and a transcript or screenshot. For any other
bug, use the "Other Bug in Sd" template: the program and its version, the level,
the operating system, and the shortest sequence that shows the problem. Do not
quote a regression test; write a fresh, minimal sequence.

Ask for a new call, concept, or application with the "New Call, Concept, or
Application" template, and for any other feature with the "Other New Feature"
template.
