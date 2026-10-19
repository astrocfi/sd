# Purpose

<!-- Why is this change needed? What problem does it solve? -->

Closes #<!-- issue number -->

## Changes/Implementation Details

<!-- What changed and how? Note any non-obvious design decisions. Bullet list is fine. -->

-

## Type of Change

- [ ] Bug fix (non-breaking)
- [ ] New feature, call, or concept (non-breaking)
- [ ] Breaking change (alters existing behavior, the call database format, or the transcript format)
- [ ] Refactor (no change in behavior)
- [ ] Documentation
- [ ] Regression tests only
- [ ] CI / Build

## Paired Change

<!-- If the regression suite changes too, link the pull request in the other
     repository; it uses the same branch name. Otherwise write "None". -->

## Testing

- [ ] `sdtty`, `mkcalls`, and `sd_calls.dat` build (`scripts/run-all-checks.sh --code`)
- [ ] The regression suite passes (`scripts/run-all-checks.sh --regression`)
- [ ] The documentation builds with no new warnings (`scripts/run-all-checks.sh --docs`)
- [ ] Tested by hand (describe below)

<!-- Manual verification steps, if any. When regression results changed, name the
     tests and say why the new output is right -- without quoting them. -->

## Potential Impacts

<!-- Effects on callers' sequences, the call database format, the menu cache, or
     performance. Write "None" if straightforward. -->

## Checklist

- [ ] No secrets or credentials committed
- [ ] No regression-test content in the diff, this description, or the CI logs
- [ ] No warnings introduced (build, documentation, linters), or justified in Notes
- [ ] Documentation updated (if applicable)
- [ ] `DATABASE_FORMAT_VERSION` bumped if the format of `sd_calls.dat` changed
- [ ] No temporary or debug code left in

## Notes

<!-- Anything reviewers should know: tricky areas, unresolved questions, follow-up
     work. Say "None" if nothing relevant. -->
