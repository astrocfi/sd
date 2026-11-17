---
description: Standards for the Sd manuals -- the Texinfo documentation system, prose conventions, markup, cross-references, and the documentation build.
---

# Documentation Foundation

This rule defines the documentation system, the markup conventions, and the
build requirements for the documentation in this repository. The words
themselves are governed by `doc_prose_style`; where the two overlap, that rule
governs the sentence and this one governs the markup and the build.

## 1. Documentation System

- The manuals are **Texinfo** (`.txi`) files in `doc/`, one per manual:
  `sd_doc.txi` (the main manual), `sessions.txi`, `demo.txi`, `faq.txi`,
  `readme.txi`, `install.txi`, `sample.txi`, the application notes
  (`appnote1.txi` to `appnote4.txi`), the release notes (`relnotes.txi`, and
  the archived `relnotes98.txi`, `relnotes03.txi`, and `relnotes09.txi`), the
  license (`gpl.txi`), and a template for an application note (`template.txi`).
- `doc/makefile` builds every format -- HTML, plain text, Info, PDF, and
  PostScript -- with `makeinfo` and TeX. The hand-written web pages in `doc/`
  (`*.html` and the `*.htmli` fragments) are not built.
- Developer documentation is Markdown at the repository root
  (`CONTRIBUTING.md`) and in `.claude/`. It is not part of the manuals.
- Build outputs are never committed.

## 2. Markup Conventions

Follow the markup the manuals already use:

| Text | Markup | Example |
|---|---|---|
| The programs | `@code` | `@code{Sd}`, `@code{Sdtty}` |
| A call or concept as the user types it | `@samp` | `@samp{swing thru}` |
| Other user input | `@kbd` | `@kbd{replace}` |
| A named key | `@key` | `@key{ENTER}` |
| A file | `@file` | `@file{sd.ini}` |
| A placeholder | `@var` | `@var{level}` |
| Quoted words | TeX quotes | ` ``this'' ` |
| An em dash | `---` | `call---and` |
| Examples and transcripts | `@example` | |

- End a sentence with a period and **two spaces**, as the manuals do. TeX and
  `makeinfo` use the two spaces to find the end of a sentence. They take a
  period after a capital letter for an abbreviation, so a sentence whose last
  word ends in a capital -- even inside markup -- ends with `@.`
  (`@samp{12 MATRIX}@.`).
- Do not use straight double quotes in prose. They print as straight quotes in
  every format.
- Keep source lines short enough to read in a terminal; most lines in the
  manuals are under 80 characters.

## 3. Nodes, Menus, and Cross-References

- Every `@node` line names its next, previous, and up nodes explicitly
  (`@node Calling, The Completing Reader, Getting Started, Top`). When a node is
  added, removed, or renamed, update the pointers of its neighbors and its
  parent's `@menu` in the same change.
- Refer to another part of a manual with `@xref` (at the start of a sentence),
  `@pxref` (inside parentheses), or `@ref` (elsewhere) -- never by a bare section
  title.
- `@xref` and `@ref` are followed by a period or a comma; `@pxref` is followed by
  a closing parenthesis. `makeinfo` warns otherwise.

## 4. Version and Date

The manuals carry the program version and a date (`@set sd-version`,
`@set Time-stamp:`), and their copyright notices carry a year. Change them only
as part of a release, following the release checklist in `doc/howtobuild.txt`.

## 5. Build Discipline

The manuals are correct only when they build, and build without new warnings:

```bash
scripts/build-docs.sh          # all formats, out of the source tree
scripts/read-docs.sh           # build the HTML and open it
```

- `scripts/build-docs.sh` copies `doc/` into `.build/docs`, runs `doc/makefile`
  there, and compares the warnings from `makeinfo` and TeX with
  `scripts/doc-warnings-baseline.txt`. A warning that is not in the baseline
  fails the build.
- The baseline lists the warnings that the upstream manuals already produce.
  Never add a warning to it to make a change pass. When a warning cannot be
  fixed, add it with `--update-baseline` and say why in the pull request.
- TeX's overfull and underfull box messages are about line breaking only; the
  build counts them but does not check them.

## 6. Change Discipline

- A change to the program that a user can see -- a call, a concept, a command,
  a command-line option, a menu, or a warning -- updates the manuals in the same
  change.
- NEVER leave stale or contradictory documentation. If a feature is removed,
  remove its documentation; if it is renamed, rename every reference.
- The archived release notes (`relnotes98.txi`, `relnotes03.txi`, and
  `relnotes09.txi`) record history and are not edited.
