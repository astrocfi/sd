---
description: Security rules for Sd and its private regression tests -- secrets, the confidentiality of the regression suite, GitHub Actions, and defensive C++.
---

# Security Best Practices

The Sd project is two repositories that are used together:

- `sd` is **public**. Its source, documentation, CI logs, CI caches, and artifacts
  can be read by anyone.
- `sd-regression` is **private**. Most of its test scripts are transcriptions of
  choreography called at real dances, and the callers who own that material have
  not agreed to its publication.

Most of the rules below exist to keep the second repository's contents out of the
first.

## 1. Regression-Test Confidentiality

- NEVER copy a regression test script (`*t`), a reference transcript (`*.ref`), a
  run transcript (`*.tmp`), or any excerpt of one into the `sd` repository, an `sd`
  issue, an `sd` pull request, a commit message, or a code comment.
- NEVER print regression-test content in a CI job that runs in the `sd`
  repository. Its logs are public. Report a failing test by **name** and by the
  number of differing lines only. `scripts/run-regression-tests.sh` does this
  unless it is given `--show-diffs`, and the `sd` workflow never passes that flag.
- NEVER upload regression-test content as a workflow artifact, and NEVER store it
  in the Actions cache of the `sd` repository. A pull request from a fork can
  restore caches written by the base branch.
- Discuss a failing regression test in the private `sd-regression` repository, or
  describe the failure in your own words without quoting the dance material.

## 2. Secrets

- NEVER commit secrets, tokens, passwords, or private keys.
- The `sd` workflow reads `sd-regression` through a read-only deploy key whose
  private half is the `SD_REGRESSION_SSH_KEY` Actions secret. Give the key read
  access only, and use it for nothing else.
- If a secret is committed or printed, rotate it at once. Deleting the commit or
  the log does not revoke it.

## 3. GitHub Actions

- NEVER trigger a workflow that holds a secret with `pull_request_target`, and
  NEVER check out a pull request's head in such a workflow. Either one lets a fork
  run its own code with the secret.
- Workflows that read `sd-regression` skip that job for pull requests from forks,
  which receive no secrets. A maintainer runs the suite for such a pull request
  by pushing its branch to the `sd` repository.
- Give each workflow the least `permissions:` it needs. The test workflows need
  `contents: read` only.
- Pin each action to a major version tag (for example `actions/checkout@v5`), and
  let Dependabot propose updates.

## 4. Defensive C++

- Treat everything Sd reads as untrusted: the call database, `sd.ini`, sequence
  files, and command-line arguments.
- Check the bounds of every fixed-size buffer and array before writing to it. Sd
  holds a setup in fixed-size arrays (`MAX_PEOPLE`), and many of its strings are
  fixed-size `char` buffers.
- Check the result of every file operation and report a failure to the user
  instead of continuing with a partly read file.

## 5. Code Review Checklist

When reviewing a pull request, verify:

- [ ] No secrets or credentials in code, configuration, or comments.
- [ ] No regression-test content in the diff, the description, or the CI logs.
- [ ] Every workflow that holds a secret runs only for trusted branches.
- [ ] New buffer writes are bounds-checked.
