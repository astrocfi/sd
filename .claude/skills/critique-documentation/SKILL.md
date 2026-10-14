---
name: critique-documentation
description: Analyze the Sd documentation (the Texinfo manuals, README, and developer documentation) against the project's documentation rules and produce a report, without editing anything. Use when the user asks to critique, review, or audit the documentation, or to generate a report for fixing the docs.
---

# Critique Documentation

Analyze the project's documentation and produce a **report only** -- do not
modify any documentation file. The report is meant to be used later as a prompt
for an AI agent (or a developer) to fix the documentation.

## Scope

- **The manuals**: the Texinfo files in `doc/` (see `doc_texinfo` for the list).
- **The README** and the developer documentation (`CONTRIBUTING.md`).
- **The build**: `doc/makefile`, driven by `scripts/build-docs.sh`.

The manuals are upstream text. A finding in them is reported for the upstream
author to consider; it is not a defect in this repository's tooling.

## Project rules

Treat these documentation standards as authoritative and cite them by name in
findings:

- `doc_texinfo` (`.claude/rules/doc_texinfo.md`) -- the documentation system,
  the markup conventions, nodes and cross-references, version and date, and the
  build discipline.
- `doc_prose_style` (`.claude/rules/doc_prose_style.md`) -- audience separation,
  confidentiality, forbidden constructions, terminology, and sentence mechanics.
- `doc-user-guide` (`.claude/skills/doc-user-guide/SKILL.md`) -- what the main
  manual covers, including the reference for every command-line option.

If a rule file does not exist, skip the corresponding part of the critique
instead of inventing a standard, and do not report the rule's absence as a
finding.

## Checklist for Analysis

### 1. Build health (`doc_texinfo`)

- Does `scripts/build-docs.sh` pass? List any warning that is not in
  `scripts/doc-warnings-baseline.txt`, and summarize the baseline itself: which
  warnings could be fixed upstream.
- Are the overfull and underfull box messages concentrated in a few places that
  a rewording would fix?

### 2. Markup and structure (`doc_texinfo`)

- The markup follows the conventions table: programs in `@code`, calls and
  concepts as typed in `@samp`, input in `@kbd`, keys in `@key`, files in
  `@file`.
- Every `@node` has correct next, previous, and up pointers, and every child
  node is in its parent's `@menu`.
- Cross-references use `@xref`, `@pxref`, and `@ref`, never a bare section title.

### 3. Prose (`doc_prose_style`)

- Run `scripts/check-doc-prose.sh --warnings`. Report new errors, and the
  warnings that a reader would agree with.
- Apply the review checklist in Section 6 of the rule: what a script cannot
  judge.

### 4. Spelling

- Run `scripts/check-spelling.sh`. Summarize `scripts/codespell-baseline.txt`:
  the upstream misspellings that could be fixed.

### 5. Completeness and accuracy (`doc-user-guide`)

- Every command-line option that `sdtty -help` prints is documented, with the
  same name and meaning.
- Every level that `sdtty` accepts is described.
- The installation chapter describes the build that the makefiles in `src/`
  actually perform.
- Statements of version, date, and copyright year agree across the manuals (see
  the release checklist in `doc/howtobuild.txt`).

### 6. Confidentiality (`doc_prose_style`, `security`)

- Nothing in the public `sd` repository quotes the private regression suite.

## Output: Report Format

Produce a single Markdown report with this structure. Do **not** edit any
documentation file; only write the report. Omit a section whose rule file is
absent, and say so under "Rules applied".

```markdown
# Documentation Critique Report

**Generated:** [date]
**Scope:** doc/*.txi, README.md, CONTRIBUTING.md
**Rules applied:** [the rule files found; any absent and therefore skipped]

## Executive summary
- Overall assessment (strengths, main gaps).
- Build health: does scripts/build-docs.sh pass, and what is in its baseline?
- High-priority fixes vs. nice-to-have.

## 1. Build health
## 2. Markup and structure
## 3. Prose
## 4. Spelling
## 5. Completeness and accuracy
## 6. Confidentiality

## Recommended priorities
1. [Highest impact, feasible first step]
2. [Next]

## Prompt for an AI agent to fix the documentation

[A self-contained prompt that includes the findings above as context, requires
scripts/build-docs.sh, scripts/check-doc-prose.sh, and scripts/check-spelling.sh
to pass, asks for each fixed baseline entry to be removed with
--update-baseline, and forbids changes to program behavior.]
```

## Execution steps

1. **Inventory rules:** list which of the rule files above exist.
2. **Build:** run `scripts/build-docs.sh`, `scripts/check-doc-prose.sh
   --warnings`, and `scripts/check-spelling.sh`, and capture their output.
3. **Read:** sample each manual's top node and menus, the command-line option
   reference in `sd_doc.txi`, the installation chapter, and the FAQ.
4. **Compare:** check the option reference against `sdtty -help` from a fresh
   build (`make -f makefile.linux64` in a copy of `src/`).
5. **Classify:** for each checklist area, note specific files, nodes, and line
   numbers, and cite the supporting rule.
6. **Write:** produce the report in the format above.
7. **Do not** change, add, or remove any line in any documentation, source, or
   configuration file.
