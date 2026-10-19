---
description: Git, CI (GitHub Actions), the local check runner, and editor settings for the Sd repositories.
---

# Environment Best Practices

## 1. Source Control

- ALWAYS use **git** for all source code, documentation, and tests.
- The project is two repositories used together: `sd` (public: source and
  documentation) and `sd-regression` (private: the regression suite). Clone them
  side by side, under the same parent directory:

  ```text
  <parent>/
      sd/                 # public source and documentation
      sd-regression/      # private regression suite
  ```

  The scripts in both repositories find each other by that layout.
- A change that needs both repositories uses a branch **with the same name in
  each** (see the `git-workflow` skill). CI pairs the two by branch name.
- The Claude Code rules and skills that both repositories have are
  **identical**, and a change to one is made in both, in the paired pull
  requests. The rules and skills about the manuals are in `sd` only, since
  `sd-regression` has no manuals. Everything else, `scripts/` included,
  belongs to its own repository: each has the scripts it uses, and no others.

## 2. CI (GitHub Actions)

- ALWAYS use **GitHub Actions** for continuous integration. The workflow is
  `.github/workflows/run-tests.yml`.
- The local check runner (`scripts/run-all-checks.sh`, Section 3) is the
  **single source of truth** for which checks the repository runs. CI runs the
  same checks the script enables -- no more, no less.
- When the set of checks changes, change it in the script first, then bring the
  workflow into step with it in the same change.
- Every pull request MUST pass the checks before merge.
- Pin each action to a major version tag (for example `actions/checkout@v5`).

## 3. Local Check Runner

`scripts/run-all-checks.sh` runs every check with one command, so a contributor
can reproduce the CI result before pushing.

- It groups the checks into scopes (code, documentation, and text) so that one
  scope can be run while iterating. With no scope it runs them all.
- It offers a parallel mode (the default) and a sequential mode, which is easier
  to read when a check fails.
- The regression suite needs the private `sd-regression` repository, so the
  runner includes it only when asked (`--regression`).
- It exits non-zero when any check fails and prints a summary that names the
  failed checks.
- The Python lint tools it uses are pinned in `scripts/requirements.txt`:
  `actionlint-py`, `codespell`, `pymarkdownlnt`, and `shellcheck-py` in `sd`,
  and `actionlint-py`, `pymarkdownlnt`, and `shellcheck-py` in `sd-regression`.
  `scripts/run-all-checks.sh --setup` installs them in a virtual environment in
  `.build/venv`; never install them into the system Python.
- The lint tools' virtual environment, the runner's logs, and, in `sd`, the
  manuals' build go under `.build/`, which ignores itself. Sd itself builds in
  place, and the regression suite runs in place (Section 4).

## 4. Builds

- The source, in `sd`, builds with the makefiles in `src/`, each named for its
  platform. There is no default makefile, so every invocation needs `-f` (for
  example `make -f makefile.linux64`). `scripts/build-sd.sh` builds `sd/src`
  in place, from either repository, and git ignores what the build writes.
- The regression suite runs in place too: its scripts run `../../sd/src/sdtty`
  from `sd-regression/regression_tests`, so `sd` must be built before it runs.
- The documentation, in `sd`, builds with `doc/makefile` (Texinfo). Build it
  through `scripts/build-docs.sh`, which builds out of the source tree and
  checks the warnings against a recorded baseline.
- Build outputs are never committed.

## 5. Editor Settings (VS Code / Cursor)

`.vscode/settings.json` and `.cursor/settings.json` match the existing sources:

- **Indent**: 3 spaces in C++, no tabs (the Emacs `c-basic-offset:3` line at the
  top of each source file).
- **Line length**: a ruler at 88 characters (the Emacs `fill-column:88`).
- **Whitespace**: trailing whitespace is **not** trimmed on save. The sources are
  upstream releases, and an editor that rewrites lines it was not asked to touch
  makes the next release harder to compare.

## 6. Secrets and Configuration

- NEVER commit secrets, tokens, or credentials. Use GitHub Actions secrets (see
  the `security` rule).
