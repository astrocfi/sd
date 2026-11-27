---
name: bug-report
description: Standards for clear, reproducible Sd bug reports -- a wrong result from a call or concept, or any other bug, with the evidence each needs. Use when writing, filing, or reviewing a bug report or GitHub issue.
---

# Bug Report Standards

## 1. Core Components

`sd` has a template for each of two kinds of bug.

A **wrong result from a call or concept** ("Wrong Result from a Call or
Concept") MUST include:

- **Clear title** -- the symptom and where it appears (for example "`swing thru`
  from a 1/4 tag gives the wrong setup at C4").
- **The call or concept** -- as it was typed.
- **What it did and what it should do** -- side by side, citing the definition
  if you can.
- **Evidence** -- a transcript from the start of the sequence, or a screenshot.

It does not need the versions or the operating system.

**Any other bug** ("Other Bug in Sd") -- a crash, or a problem in the menus,
the output, or the database -- MUST include:

- **Clear title** -- the symptom and where it appears.
- **Versions** -- the program version (`sdtty -help` prints it on its first
  line) and the database version (written at the top of every sequence file).
- **Program and level** -- `Sd` (the Windows program) or `Sdtty` (the console
  program), and the level it was started at (`c4`, `p`, and so on).
- **Operating system** -- for example Windows 11 or Ubuntu 24.04.
- **Reproduction** -- the shortest sequence of calls, from the start of a
  sequence, that shows the problem. A sequence typed into `sdtty` with
  `-diagnostic` reproduces exactly, so give the command line too.
- **Expected vs. actual behavior** -- side by side.
- **Evidence** -- the transcript or the error message.

A report of either kind gives a **severity** from the scale below.

## 2. Severity Scale

| Level | Criteria |
|-------|----------|
| **Critical** | Crash, or a wrong setup reported as correct. |
| **High** | A call or concept that should work is refused, or a common workflow is blocked. |
| **Medium** | A rarely used call or concept is wrong, or a warning is wrong. |
| **Low** | A documentation error, or a cosmetic problem in a menu or transcript. |
| **Trivial** | Negligible effect on users. |

## 3. Report Templates

A wrong result from a call or concept:

```markdown
# Wrong Result: [Concise title]

## Call or Concept
[The call or concept, as typed.]

## What It Did
[The setup Sd gave, or the message it printed.]

## What It Should Do
[The setup expected, and why -- cite the definition if you can.]

## Severity
[Level] -- [Brief justification]

## Transcript or Screenshot
[The transcript from the start of the sequence, or a screenshot.]
```

Any other bug:

```markdown
# Bug Report: [Concise title]

## Description
[1-2 sentences: what is broken and its effect.]

## Environment
- **Program**: [Sd or Sdtty]
- **Program version**: [first line of `sdtty -help`, e.g. 39.85]
- **Database version**: [from the top of a sequence file]
- **Level**: [e.g. c4]
- **OS**: [e.g. Windows 11, Ubuntu 24.04]

## Severity
[Level] -- [Brief justification]

## Steps to Reproduce
1. Start: `sdtty c4 -db sd_calls.dat -diagnostic`
2. Enter: [the calls, one per line]
3. Observe: [what appears]

## Expected Behavior
[What should happen, and why -- cite the call definition if you can.]

## Actual Behavior
[What happens, with the transcript or error message.]

## Additional Notes
[Workarounds, related issues, a suspected cause.]
```

## 4. Writing Guidelines

1. Be objective and factual.
2. One problem per report.
3. For a bug other than a wrong result, give exact versions and the full error
   message.
4. Keep the reproduction as short as possible.
5. Reproduce the bug before filing it.

## 5. Confidentiality

A report in the public `sd` repository NEVER quotes a sequence from the private
regression suite. If a regression test shows the bug, name the test and write a
fresh, minimal sequence that reproduces it.
