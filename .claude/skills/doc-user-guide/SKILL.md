---
name: doc-user-guide
description: Content and completeness rules for the Sd manuals, especially the main manual (doc/sd_doc.txi) and its command-line option reference. Use when writing, editing, or reviewing a manual in doc/.
---

# User Guide

The manuals are for callers who **use** Sd and Sdtty to write sequences. Build on
`doc_texinfo` (markup, nodes, build) and `doc_prose_style` (the words). Write for
someone who knows square dancing and can run a program, but who has never read
the source; never require the reader to read source code to do a documented
task.

## 1. The Manuals

| File | Purpose |
|---|---|
| `sd_doc.txi` | The main manual: everything a caller needs. |
| `readme.txi` | The introduction, which is also the main manual's first chapter. |
| `install.txi` | Building and installing Sd and Sdtty. |
| `sessions.txi` | The Sdtty session feature. |
| `demo.txi` | A demonstration of Sdtty. |
| `faq.txi` | Frequently asked questions. |
| `sample.txi` | A sample sequence. |
| `appnote1.txi` to `appnote4.txi` | Application notes on specific topics. |
| `relnotes.txi` | The release notes for the current series. |
| `gpl.txi` | The GNU General Public License. |
| `template.txi` | A template for an application note. |

`relnotes98.txi`, `relnotes03.txi`, and `relnotes09.txi` are archives and are
not edited. `readme.txi` repeats the main manual's introduction, so a change to
one is made to the other in the same change. Other material that belongs in one
manual is not repeated in another; link to it.

## 2. Required Content of the Main Manual

`sd_doc.txi` MUST cover:

- **Introduction** -- what Sd is for, what it is not, and how it decides what is
  correct, before any mechanics.
- **Getting started** -- starting the program, choosing a level, and starting a
  sequence.
- **Calling** -- entering calls and concepts, the menus, and the commands that
  are not calls (undo, resolve, write the sequence, and so on).
- **Levels** -- every level the program accepts, with its name in prose and as
  typed.
- **Files** -- every file Sd reads or writes: the call database, the
  initialization file, sequence files, and the menu caches.
- **Command-line options** -- see Section 3.

## 3. Documenting Command-Line Options

For every option that `sdtty -help` prints:

- Give the exact option and its argument placeholder (`-print_length @var{n}`).
- Say what it does and what its default is.
- Say which program accepts it when only one does ("Sd only", "Sdtty only"), as
  `sdtty -help` does.
- Show at least one complete invocation in an `@example` block.

Keep the reference in exact agreement with `sdtty -help`. A change that adds,
removes, or renames an option updates the manual in the same change.

## 4. Style

- Lead each chapter with its purpose, then the mechanics.
- Show every command in an `@example` block; never make the reader reconstruct
  one from prose.
- State expected results: what the caller sees, and what files are written.
- Cross-reference other chapters with `@xref`, `@pxref`, or `@ref` instead of
  repeating them.
