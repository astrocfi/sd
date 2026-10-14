# The Workflows

Each repository has one workflow, `.github/workflows/run-tests.yml`, named
"Run Tests".

## When They Run

| Event | `sd` | `sd-regression` |
|---|---|---|
| A pull request is opened or updated | yes | yes |
| A push to `main` or to a version branch (`39.99`) | yes | yes |
| A push of a tag (`v39.85`) | yes | yes |
| A manual run from the Actions tab | yes | yes |
| Weekly, on Monday at 06:00 UTC | yes | no |

A push to a work branch doesn't run anything by itself; the branch is tested
through its pull request. The weekly run of `sd` catches a change in GitHub's
runner images between releases.

A newer run for the same pull request, or the same branch, cancels the one in
progress.

## The `sd` Workflow

### Lint

Installs the tools in `scripts/requirements.txt` with Python 3.12 and runs
`scripts/run-all-checks.sh --text`.

### Build

Builds and smoke-tests Sdtty with `scripts/run-all-checks.sh --code`, once for
each makefile in the matrix:

| Runner | Makefile |
|---|---|
| `ubuntu-latest` | `makefile.linux64` |
| `ubuntu-latest` | `makefile.linux64debug` |
| `ubuntu-latest` | `makefile.linuxnostatic64` |
| `macos-latest` | `makefile.mac` |

The Windows builds (MinGW and Visual Studio) are not in the matrix.

### Docs

Installs Texinfo, TeX, and the packages in `docs/requirements.txt`, and runs
`scripts/run-all-checks.sh --docs-build --dev-guide`. It uploads two
artifacts, kept for 14 days:

- `manuals`: the manuals in every format, from `.build/docs/output`.
- `dev-guide`: this guide as HTML, from `.build/dev-guide/html`.

### Regression Tests

Checks out `sd-regression` with the deploy key, chooses the branch to test
against, and runs `scripts/run-all-checks.sh --regression`. It takes about
fifteen minutes. See [The Regression Suite in CI](regression.md).

## The `sd-regression` Workflow

It has two jobs:

- **Lint** runs PyMarkdown over this repository's Markdown, ShellCheck over
  its scripts, and actionlint over its workflow.
- **Regression tests** checks out the public `sd` repository (no key is
  needed), chooses the `sd` branch to test, and runs the suite. This repository
  is private, so the job sets `REGRESSION_SHOW_DIFFS` and its log shows the
  differences for each failing test.

## Running a Workflow by Hand

On the Actions tab, choose "Run Tests" and "Run workflow", and pick the branch.
Each workflow takes one optional input, the ref of the other repository to test
against:

| Repository | Input | Default |
|---|---|---|
| `sd` | `regression_ref` | the `sd-regression` branch with the same name, else `main` |
| `sd-regression` | `sd_ref` | the `sd` branch with the same name, else `main` |

The same run from the command line:

```bash
gh workflow run run-tests.yml --repo astrocfi/sd --ref 39.99 -f regression_ref=39.99-menu-fix
```

## Action Versions

Each action is pinned to a major version (`actions/checkout@v5`). Dependabot
(`.github/dependabot.yml`) opens a pull request each week when a new major
version is released. Its pull requests don't receive Actions secrets, so the
regression job skips them.
