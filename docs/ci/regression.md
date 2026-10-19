# The Regression Suite in CI

The regression suite is in the private `sd-regression` repository. Each test is
a script of calls (`t50t`) and the transcript Sd should produce for it
(`t50t.ref`). Most tests are transcriptions of choreography called at real
dances; the callers who own that material allow the Sd developers to use it
for testing, but not to publish it.

Both repositories run the suite in CI: `sd` against the `sd-regression` branch
that matches the change, and `sd-regression` against the matching `sd` branch.

## How a Run Works

`scripts/run-all-checks.sh --regression` runs `scripts/run-regression-tests.sh`,
which runs the suite's own `regression_tests/doit.sh`, exactly as a developer
does. The two repositories are cloned side by side, and the suite's scripts run
`../../sd/src/sdtty` from `sd-regression/regression_tests`, so the run uses
both checkouts where they are:

1. It builds Sd in place in `sd/src` with `scripts/build-sd.sh`.
2. It runs `doit.sh` in `sd-regression/regression_tests`, and writes its
   output, which includes the differences and therefore the tests' contents, to
   `regression_tests/doit.log`, which it does not print. `doit.sh` first starts
   Sdtty once at each level it uses, so that the menu cache exists before the
   first test at that level. Without the cache, that test reports a cache miss
   that its reference transcript does not contain.
3. It compares each reference transcript (`.ref`) with the transcript the run
   wrote (`.tmp`), and reports each test by name.

Git ignores everything the build and the run write, in both checkouts.

```text
regression: doit.sh ran in 628s
regression: 285 passed, 0 failed, 0 not run by doit.sh
regression: passed
```

A failing test is listed below the counts as `FAILED <test> (N line(s)
differ)`, and the run ends `regression: FAILED`.

A test is "not run" when it has a reference transcript but `doit.sh` did not
write a transcript for it. A run fails when any test fails, or when no test
ran.

The whole run takes about fifteen minutes on a GitHub runner, and about half
that on a fast workstation.

## Pairing the Two Repositories

A change that alters what the suite expects (a new test, or a reference
transcript that changes because Sd's output does) needs both repositories.
It is made on a branch with **the same name** in each, and the two pull
requests are merged together.

`scripts/companion-ref.sh` chooses which ref of the other repository to test
against. It takes the first of these that exists there:

1. The ref given to a manual run (`regression_ref` in `sd`, `sd_ref` in
   `sd-regression`). If it does not exist, the job fails.
2. For a tag, the same tag. Releases are tagged in both repositories
   (`v39.85`).
3. The branch being built; for a pull request, its head branch.
4. For a pull request, its base branch.
5. `main`.

So a pull request from `39.99-menu-fix` into `39.99` in `sd` is tested against
`39.99-menu-fix` in `sd-regression` if that branch exists, and against `39.99`
there if it does not. The job's summary and log say which ref was chosen and
why:

```text
companion-ref: 39.99 (3f2a9c1) -- the pull request's base branch ('39.99-menu-fix' has no partner)
```

To test a change against a different branch of the other repository, run the
workflow by hand with that ref (see
[Running a Workflow by Hand](workflows.md#running-a-workflow-by-hand)).

## The Deploy Key

`sd`'s regression job reads `sd-regression` with a **deploy key**: an SSH key
that is registered with `sd-regression` alone, with read access only. Its
private half is the Actions secret `SD_REGRESSION_SSH_KEY` in `sd`, which the
job passes to `actions/checkout`. The checkout doesn't keep credentials
(`persist-credentials: false`), so the steps that run the suite cannot use the
key.

When the secret is missing, the job fails at once with an error that says how
to set it up. To set it up, or to replace the key, see
[Rotating the deploy key](maintenance.md#rotating-the-deploy-key).

If the suite moves to another repository, set the Actions variable
`SD_REGRESSION_REPOSITORY` in `sd` to its `owner/name`. `sd-regression` has the
matching variable `SD_REPOSITORY`. `sd` is public, so `sd-regression`'s job
doesn't need a key.

## Keeping the Suite Private

`sd`'s Actions logs, artifacts, and caches can be read by anyone. Its
regression job therefore:

- prints test names and the number of differing lines, never a test's
  contents. `scripts/run-regression-tests.sh` refuses `--show-diffs` (and
  `REGRESSION_SHOW_DIFFS`) when it runs in `sd` on GitHub Actions;
- doesn't upload an artifact or write to the Actions cache. A pull request
  from a fork can restore a cache written by the base branch.

`sd-regression` is private, so its job sets `REGRESSION_SHOW_DIFFS` and its
log shows the differences.

The same rules apply to people: never copy a test, a reference transcript, or
any part of one into an `sd` issue, pull request, commit message, or code
comment. Name the test, and describe the failure in your own words. See
`.claude/rules/security.md`.

## Pull Requests From Forks and Dependabot

GitHub gives a pull request from a fork no Actions secrets, and gives
Dependabot's pull requests none either. The regression job is skipped for
both, rather than failing for want of the key. A maintainer runs the suite for
a fork's change by pushing its branch to `sd`, where the pull request from that
branch gets the secret.

The workflows are never triggered by `pull_request_target`, which would run a
fork's code with the secrets.

## Investigating a Failure

The job's log names each failing test, and only that. To see why, run the
suite locally with both repositories checked out side by side:

```bash
scripts/run-all-checks.sh --regression
```

Then compare `<test>.tmp` with `<test>.ref` in
`sd-regression/regression_tests`, or rerun with
`scripts/run-regression-tests.sh --show-diffs`. The failing run in
`sd-regression`'s CI, if there is one, also shows the differences.

When a change to Sd changes a transcript on purpose, update the reference on
the `sd-regression` branch with the same name as the `sd` branch.
