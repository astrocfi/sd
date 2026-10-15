---
name: run-all-checks
description: Run the Sd checks -- the build and smoke test, the documentation build, and the text checks (spelling, prose, Markdown, shell scripts, and workflows) -- then fix what fails. Use when the user asks to run the checks, verify the build, run CI locally, or fix a lint, build, or documentation error.
---

# Run All Checks

Run every check the repository defines, and fix what fails. The checks are
defined by `scripts/run-all-checks.sh`.

## The script controls which checks run

`scripts/run-all-checks.sh` is the **single source of truth** for the checks.
CI runs it, and so does this skill. Run the checks the script runs, and only
those. To add or remove a check, change the script first, then bring
`.github/workflows/run-tests.yml` into step with it.

## Setup

The text checks use Python tools (`scripts/requirements.txt`). Install them once
into `.build/venv`, which the scripts then use:

```bash
scripts/run-all-checks.sh --setup
```

In `sd`, the documentation build needs `makeinfo` (Texinfo) and TeX
(`texlive-base` on Debian and Ubuntu); the build needs `make` and a C++
compiler.

## Using the Script

From the repository root:

```bash
scripts/run-all-checks.sh               # everything, in parallel
scripts/run-all-checks.sh --sequential  # everything, one check at a time
```

| Option | Checks |
|---|---|
| `-c`, `--code` | `build`: `scripts/build-sd.sh` |
| `-d`, `--docs` | `docs-build`: `scripts/build-docs.sh`; `dev-guide`: `scripts/build-dev-guide.sh`; and the text checks |
| `-m`, `--text` | `codespell`, `prose`, `pymarkdown`, `shellcheck`, and `actionlint` |
| `--build`, `--docs-build`, `--dev-guide`, `--codespell`, `--prose`, `--pymarkdown`, `--shellcheck`, `--actionlint` | that check only |

`sd-regression` has no source or manuals of its own, so its script runs only
`pymarkdown`, `shellcheck`, and `actionlint`, with `--lint`, and the regression
suite with `--regression`. `scripts/run-all-checks.sh --help` lists the checks a
repository has.

## What Each Check Does

| Check | Command | Fails on |
|---|---|---|
| `build` | `scripts/build-sd.sh` | a build error, a missing product, or a smoke test that cannot start a sequence |
| `docs-build` | `scripts/build-docs.sh` | a build error, or a warning not in `scripts/doc-warnings-baseline.txt` |
| `dev-guide` | `scripts/build-dev-guide.sh` | any Sphinx warning in the developer guide (`docs/`) |
| `codespell` | `scripts/check-spelling.sh` | a misspelling on a line not in `scripts/codespell-baseline.txt`; in `sd`, the comments in the C++ sources are checked too, and their code is not |
| `prose` | `scripts/check-doc-prose.sh` | an error not in `scripts/doc-prose-baseline.txt` |
| `pymarkdown` | `pymarkdown scan` | any finding in the Markdown listed in `scripts/checks.conf` |
| `shellcheck` | `shellcheck scripts/*.sh` | any finding |
| `actionlint` | `actionlint .github/workflows/*.yml` | any finding |

## Fixing Failures

| Check | Typical fix |
|---|---|
| `build` | Fix the code. Do not add warning suppressions to the makefiles. |
| `docs-build` | Fix the Texinfo (`doc_texinfo`). A baseline entry is only for a warning that cannot be fixed, added with `--update-baseline` and explained in the pull request. |
| `dev-guide` | Fix the page: a link that does not resolve, a page missing from every `toctree`, or a skipped heading level. |
| `codespell` | Fix the spelling. In a C++ source, change the comment and nothing else. If the word is right, end the line with `codespell:ignore <word>`, or add it to `ignore-words-list` in `.codespellrc` with a comment saying why. |
| `prose` | Rewrite the sentence (`doc_prose_style`). |
| `pymarkdown` | Fix the Markdown. |
| `shellcheck`, `actionlint` | Fix the script or the workflow. Disable a finding only with a comment that says why. |

The baselines hold findings in **upstream** text that predate the checks. Never
add a finding in new text to a baseline. When a fix removes a baseline finding,
regenerate the baseline with the script's `--update-baseline` option so the
entry is dropped.

## Execution Workflow

```text
Check Progress:
- [ ] build
- [ ] docs-build
- [ ] dev-guide
- [ ] codespell
- [ ] prose
- [ ] pymarkdown
- [ ] shellcheck
- [ ] actionlint
- [ ] All failures fixed
- [ ] The full script passes again
```

## Success Criteria

`scripts/run-all-checks.sh` ends with `run-all-checks: all checks passed` and
exits 0.
