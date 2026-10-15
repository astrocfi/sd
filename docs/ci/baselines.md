# Baselines

The manuals, the README, and the comments in the source are upstream text,
written long before these checks. Rather than fail on every problem in them, or skip them,
each documentation check records the findings it made when it was added in a
**baseline**, and fails only on a finding that is not there.

| Baseline | Check | Each entry is |
|---|---|---|
| `scripts/doc-warnings-baseline.txt` | `docs-build` | a `makeinfo` or TeX warning, without its line number |
| `scripts/doc-prose-baseline.txt` | `prose` | a check, a file, and the text it found (tab-separated) |
| `scripts/codespell-baseline.txt` | `codespell` | a whole line that codespell skips wherever it appears |

An entry names a finding by its text, never by its line number, so editing one
part of a manual does not disturb the entries for the rest.

None of `sd-regression`'s checks has a baseline.

The spelling baseline in `sd` holds lines from the comments in `src/`: a
parameter name in code that is commented out, and two accepted spellings that
codespell flags as variants. A comment line is recorded as the check reads it,
from a copy of the source with everything but the comments blanked. A line that
is all comment is recorded as it is in the source.

## Changing a Baseline

Each check regenerates its baseline from scratch:

```bash
scripts/build-docs.sh --update-baseline
scripts/check-doc-prose.sh --update-baseline
scripts/check-spelling.sh --update-baseline
```

Regenerate a baseline in the same change that fixes one of its findings, so
that the fixed problem cannot come back unnoticed. A fix that changes the text
of a line also removes that line's entry, which the regenerated baseline shows.

Do not add a new finding to a baseline to make a change pass: fix it. When a
finding cannot be fixed, say why in the pull request, and use the check's
own escape where it has one:

- A word that is right where it appears takes a trailing
  `codespell:ignore <word>` comment, or, in a line that cannot carry a comment,
  an entry in `ignore-words-list` in `.codespellrc` with a comment saying why.
- A `makeinfo` or TeX warning that cannot be avoided goes in the warnings
  baseline with `--update-baseline`, and the pull request says why.

TeX's overfull and underfull box messages are about line breaking only.
`scripts/build-docs.sh` counts them, but does not check them.

## The Developer Guide

This guide doesn't have a baseline. `scripts/build-dev-guide.sh` fails on every
Sphinx warning, and the guide's pages are checked by the text checks like the
rest of the developer documentation.
