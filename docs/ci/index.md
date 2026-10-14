# Continuous Integration

Every pull request in either repository is built and checked by GitHub Actions
before it is merged. The checks themselves live in `scripts/`, and CI runs them
through the same script a developer runs locally,
`scripts/run-all-checks.sh`. A result in CI can therefore be reproduced on a
developer's machine with one command.

## What Runs Where

| Repository | Job | What it checks |
|---|---|---|
| `sd` | Lint | Spelling, documentation prose, Markdown, shell scripts, and workflows |
| `sd` | Build | Sdtty builds and starts, with each Linux makefile and the Mac makefile |
| `sd` | Docs | The manuals build in every format, and the developer guide builds |
| `sd` | Regression tests | The private regression suite passes against this build |
| `sd-regression` | Lint | Markdown, shell scripts, and the workflow |
| `sd-regression` | Regression tests | The suite passes against the matching `sd` branch |

The two repositories test each other. A change that needs both is made on a
branch with the same name in each, and CI in either repository checks out the
other at that branch. See
[Pairing the two repositories](regression.md#pairing-the-two-repositories).

## Principles

- **One source of truth.** `scripts/run-all-checks.sh` decides which checks
  exist. A workflow job installs what its checks need and runs the script; it
  never runs a check of its own. When the set of checks changes, the script
  changes first, and the workflow follows in the same change.
- **Scripts per repository.** Each repository's `scripts/` holds the checks
  that repository runs, and no others. `sd-regression` doesn't have source or
  manuals of its own, so it lints its Markdown, its scripts, and its workflow,
  and runs the suite.
- **Shared Claude Code files.** The rules and skills for Claude Code in
  `.claude/` that both repositories have are identical, so that a skill
  behaves the same in either. Change them in both, in the paired pull
  requests. The rules and skills about the manuals are in `sd` only.
- **Public logs.** The `sd` repository is public, and so are its Actions logs,
  artifacts, and caches. Nothing from `sd-regression` is printed, uploaded, or
  cached there.
- **Least privilege.** The workflows have `contents: read` only. The `sd`
  workflow reads the private suite with a read-only deploy key.
- **New problems only.** The manuals predate the documentation checks. The
  findings in them when the checks were added are recorded in baselines, and
  only a new finding fails a check. See [Baselines](baselines.md).

```{toctree}
:maxdepth: 2

checks
workflows
regression
baselines
maintenance
```
