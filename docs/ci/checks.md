# The Check Runner

`scripts/run-all-checks.sh` runs the checks, locally and in CI. With no
options it runs every check except the regression suite, in parallel, and
prints a summary:

```text
Summary
  passed  build
  passed  docs-build
  passed  dev-guide
  passed  codespell
  passed  prose
  passed  pymarkdown
  passed  shellcheck
  passed  actionlint

run-all-checks: all checks passed
```

It exits with 0 when every check passed, 1 when any failed, and 2 for a usage
error.

## The Checks

| Check | Script or tool | What it does |
|---|---|---|
| `build` | `scripts/build-sd.sh` | Builds Sdtty, `mkcalls`, and `sd_calls.dat` in `src/`, and starts a sequence with them |
| `docs-build` | `scripts/build-docs.sh` | Builds the manuals in every format, and fails on a warning that is not in the baseline |
| `dev-guide` | `scripts/build-dev-guide.sh` | Builds this guide, and fails on any Sphinx warning |
| `codespell` | `scripts/check-spelling.sh` | Checks the spelling of the documentation and tooling, and of the comments in the C++ sources, with American spelling enforced |
| `prose` | `scripts/check-doc-prose.sh` | Checks the documentation prose against `.claude/rules/doc_prose_style.md` |
| `pymarkdown` | PyMarkdown | Lints the Markdown |
| `shellcheck` | ShellCheck | Lints `scripts/*.sh` |
| `actionlint` | actionlint | Lints the workflows, and the shell in their `run:` steps |
| `regression` | `scripts/run-regression-tests.sh` | Runs the regression suite; see [The Regression Suite in CI](regression.md) |

## Choosing What Runs

| Option | Checks |
|---|---|
| none | everything except `regression` |
| `-c`, `--code` | `build` |
| `-d`, `--docs` | `docs-build`, `dev-guide`, and the text checks |
| `-m`, `--text` | the text checks: `codespell`, `prose`, `pymarkdown`, `shellcheck`, and `actionlint` |
| `-r`, `--regression` | `regression` |
| `--<check>` | that check alone (`--prose`, `--dev-guide`, and so on) |

Scopes and single checks can be combined. `-s` (`--sequential`) runs the
checks one after another and prints each one's output as it runs, which is
easier to read when one fails. CI always runs sequentially. In the default
parallel mode each check's output is written to `.build/checks/<check>.log`
and printed when all of them have finished. The regression suite, which builds
in `src/` as the `build` check does, waits until the others have finished.

## Tools

The lint tools are Python packages pinned in `scripts/requirements.txt`, and
Sphinx and its extensions are pinned in `docs/requirements.txt`. The project
itself is not Python, so they live in a virtual environment of their own:

```bash
scripts/run-all-checks.sh --setup
```

This creates `.build/venv` and installs both files into it; every script then
uses it. Set `VENV` to use another environment, or `VENV=none` to use the tools
already on `PATH`, as CI does after installing them with `pip`.

The other tools come from the system: `make` and a C++ compiler for `build`,
`makeinfo` and TeX for `docs-build` (`texinfo` and `texlive-base` on Debian and
Ubuntu), and bash 4 or later for every script. macOS ships bash 3.2, so CI's
Mac job installs a newer one with Homebrew.

## Per-Repository Settings

`scripts/checks.conf` says what each check reads in this repository. It is
sourced by bash and holds plain assignments:

| Setting | What it lists |
|---|---|
| `REPO_KIND` | `sd` or `sd-regression` |
| `TEXINFO_FILES` | the manuals |
| `DEV_GUIDE_DIR` | this guide's Sphinx project |
| `MARKDOWN_FILES` | the Markdown that PyMarkdown checks |
| `PROSE_MARKDOWN_FILES` | the Markdown that the prose check reads |
| `SPELL_FILES` | the files codespell checks |
| `COMMENT_SPELL_FILES` | the source files whose comments, and nothing else, codespell checks |
| `SHELL_FILES` | the shell scripts ShellCheck checks |
| `WORKFLOW_FILES` | the workflows actionlint checks |

The file lists are git pathspecs. The scripts expand them with `git ls-files`,
so they match tracked files and new files that are not ignored, and never build
output.

## Build Output

Sd itself builds in place in `src/`, where git ignores the products, and the
regression suite runs in place in `sd-regression/regression_tests`. Everything
else the scripts build goes under `.build/` at the repository root. The first
script to write there also writes `.build/.gitignore`, which ignores the whole
directory, so nothing in it can be committed.

| Directory | Contents |
|---|---|
| `.build/venv` | the lint tools and Sphinx |
| `.build/docs` | the manuals; the finished files are in `.build/docs/output` |
| `.build/dev-guide` | this guide; open `.build/dev-guide/html/index.html` |
| `.build/checks` | the logs of a parallel run |
