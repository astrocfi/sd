---
name: pull-request
description: Pull request structure -- purpose, implementation details, testing evidence, and review checklist -- for the Sd repositories. Use when writing a pull request title or description, or when reviewing a pull request.
---

# Pull Request Standards

## Scope of review

Treat the pull request as a **single unit of change**. The diff to review is the
set of all commits on the branch back to its merge-base with the target branch.
Review the final state of the branch against the base, and do not comment on
differences that exist only between commits within the pull request. Do not wrap
lines by hand; let GitHub flow them.

## Principles

1. **Descriptive title** -- an imperative sentence ("Fix the comment block
   memory leak").
2. **Purpose first** -- why the change is needed, then how it was done.
3. **Scope** -- one logical change. Split unrelated changes.
4. **Testing evidence** -- the build, the regression suite, and any manual
   checks.
5. **Paired change** -- when the regression suite changes too, link the pull
   request in the other repository, and say that the two merge together (see the
   `git-workflow` skill).
6. **Linked issues** -- `Closes #NNN` for each issue, listed separately:
   `Closes #NNN. Closes #MMM.`

## Template

`.github/pull_request_template.md` is applied when a pull request is opened.
Fill out every section:

- **Purpose** -- why the change is needed, and the issues it closes.
- **Changes / Implementation Details** -- what changed and how, including
  non-obvious design decisions.
- **Testing** -- which checks ran and their results. When regression results
  changed, say which tests changed and why the new output is right.
- **Potential Impacts** -- effects on users' sequences, the call database
  format, the menu cache, or performance. Write "None" if there are none.
- **Notes** -- tricky areas and follow-up work. Delete the section if it is not
  needed.

## Confidentiality

A pull request in the public `sd` repository NEVER quotes the content of a
regression test or of its output. Name the test (`t50t`) and describe the change
in your own words. Details belong in the private `sd-regression` pull request.

## Guidance

- **Release-visible changes** -- call out changes to call names, concept names,
  menus, warnings, and the transcript format, since callers see them.
- **Database format** -- a change to `database.h` that alters the format of
  `sd_calls.dat` bumps `DATABASE_FORMAT_VERSION`; say so in Potential Impacts.
- **Brevity vs. completeness** -- short enough that authors fill it out, detailed
  enough for a reviewer who has not followed the work.
