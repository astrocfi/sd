---
description: Prose style for all Sd documentation -- audience separation, forbidden constructions, terminology, and the mechanical check that enforces them.
---

# Documentation Prose Style

This rule governs the words in the manuals (`doc/*.txi`), `README.md`, and the
developer documentation (`CONTRIBUTING.md`, the developer guide in `docs/`, and
the GitHub templates). The rules and skills in `.claude/` are for Claude Code,
and neither this rule nor its check governs their wording. It sits alongside
`doc_texinfo`, which governs the markup and the build. Where the two overlap,
this rule governs the sentence and that one governs the markup.

In `sd`, `scripts/check-doc-prose.sh` enforces the mechanically detectable
half. Run it before claiming a documentation change is done;
`scripts/run-all-checks.sh` runs it too. The rest of this rule is a review
checklist -- a human or a model has to read the prose to apply it.

The manuals are upstream text, written before this rule. The errors the check
finds in them today are recorded in `scripts/doc-prose-baseline.txt` and are not
reported again. Do not edit upstream text only to satisfy this rule, and do not
add new text that breaks it.

## 1. Audience Separation

The manuals and `README.md` are for **callers**. A caller types calls and
concepts, reads the setups Sd draws, and writes sequences. A caller has never
read the source.

**Never in the manuals or the README:**

- Source file names (`sdutil.cpp`, `database.h`).
- Internal functions, classes, and types (`toplevelmove`, `parse_block`,
  `setup_kind`).

**Legitimately user-facing, and therefore allowed:**

- What a caller types or sees: program names, command-line options, call and
  concept names, levels, menu commands, and the files Sd reads and writes
  (`sd.ini`, `sd_calls.dat`, sequence files).

The developer documentation is for contributors, and names files and functions
freely.

## 2. Confidentiality

The `sd` repository is public; `sd-regression` is private, because most of its
tests are transcriptions of choreography called at real dances. Documentation in
`sd` names a regression test (`t50t`) but NEVER quotes one, its reference
transcript, or its output. See the `security` rule.

## 3. Forbidden Constructions

| Do not write | Write instead | Why |
|---|---|---|
| "the call is refused, not guessed" | "the call is refused" | See the X-not-Y test below. |
| "this case is handled nowhere" | "this case is not handled" | You cannot handle something nowhere. |
| "Sdtty accepts no options here" | "Sdtty doesn't accept options here" | Put the negation on the verb. |
| "the resolve passes the gate" | "the resolve is accepted only when ..." | Name what is tested. |
| "every level accepts the same commands, so what you learn at one works at the next" | "every level accepts the same commands" | Delete clauses that restate the obvious. |

### The X-not-Y test

Keep "X, not Y" only when **Y is something a reasonable reader might genuinely
expect**, so that ruling it out does real work. If no reasonable reader would
assume Y, the contrast is noise: state X alone.

### The gate rule

Do not use **gate** to mean "meets a criterion". Replace it with wording that
names exactly what is tested.

### The negation rule

Put a negation on the verb, not on its object: write "doesn't accept
options" or "didn't change anything" in place of "accepts no options" or
"changed nothing". A negative subject is fine ("Nothing changes", "No test
ran"), and so are "there is no", "with no", and the like. So are the idioms in
which a common verb takes a negative object: "has no effect", "have no idea",
"do nothing", "knows nothing", "makes no sense", and "take no responsibility"
are good English. `scripts/wording.py` holds the verbs the check flags, and the
words it deliberately leaves alone.

## 4. Terminology

- The programs are **Sd** (the Windows program) and **Sdtty** (the console
  program), capitalized in prose. In Texinfo they are `@code{Sd}` and
  `@code{Sdtty}`. The command a user types is `sdtty`, in lower case, inside
  markup.
- Levels are written in capitals in prose: Mainstream, Plus, A1, A2, C1, C2,
  C3A, C3, C3X, C4A, C4, and C4X. They are lower case only where they are typed
  (`@kbd{sdtty c4}`).
- A **call** and a **concept** are different things, and the manuals keep them
  apart: a concept modifies a call.
- American spelling throughout. In `sd`, `codespell` checks it (see
  `.codespellrc`).
- **Define a term before using it**, or link to where it is defined.

## 5. Sentence Mechanics

- Short declarative sentences. A convoluted paragraph becomes two or three
  plain sentences.
- **Oxford comma, always.** Every list of three or more items separated by "and"
  or "or" takes a comma before the conjunction.
- In Texinfo, end a sentence with two spaces; in Markdown, with one (see
  `doc_texinfo`).
- Describe the program as it is. Outside the release notes, do not anchor prose
  to a moment ("recently", "currently", "no longer", "legacy").
- Do not describe a limitation as fundamental when it is a convention.
- Issue numbers appear only in comments, never in running prose.

## 6. Review Checklist

The script cannot judge these. Read for them:

1. Would a caller who has never seen the source understand every sentence of a
   manual?
2. Is every term defined before it is used, or linked to where it is defined?
3. For each "X, not Y": would a reasonable reader have expected Y?
4. Does each command-line option say what it does and what its default is?
5. Is any passage in a manual explaining how the code works rather than what
   the program does?
6. Is a convention described as a requirement?
7. Does anything in the public repository quote the private regression suite?

## 7. The Checks

| Check | Tier | What it finds |
|---|---|---|
| `nowhere` | error | "nowhere", except "nowhere near" |
| `texi-sentence-space` | error | one space after a sentence in Texinfo |
| `md-sentence-space` | error | two spaces after a sentence in Markdown |
| `internal-name` | error | a source file or internal name in a manual or the README |
| `regression-content` | error | a path into the regression suite in sd's developer documentation |
| `negative-object` | error | a verb whose object is a negative ("accepts no", "changed nothing"), with a suggested rewrite |
| `gate`, `oxford-comma`, `x-not-y`, `so-clause` | warning | the constructions in Section 3 and 5 |
| `texi-straight-quote` | warning | a straight double quote in Texinfo prose |
| `time-anchor` | warning | time-anchored words outside the release notes |
| `level-case` | warning | a lower-case level name in a manual's prose |
| `program-name` | warning | "SD" or "SDTTY" in a manual's prose |

Run `scripts/check-doc-prose.sh --warnings` to see the warning tier.
