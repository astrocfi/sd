---
name: git-workflow
description: Commits, branch names, paired branches across sd and sd-regression, pull requests, and release tags. Use when writing a commit message, creating or naming a branch, opening or merging a pull request, or tagging a release.
---

# Git Workflow

## 1. Commit Messages

Use the **Conventional Commits** format for tooling and infrastructure changes:

```text
<type>[(<scope>)]: <imperative summary>   (50 chars max for subject)

[Optional body -- wrap at 72 chars. Explain *what* and *why*, not *how*.]

[Optional footer -- e.g., Closes #123]
```

| Type | When to use |
|------|-------------|
| `feat` | New user-facing feature. |
| `fix` | Bug fix. |
| `docs` | Documentation-only change. |
| `test` | Adding or updating regression tests only. |
| `build` | Makefile or build-system change. |
| `ci` | CI configuration change. |
| `chore` | Maintenance that fits none of the above. |

- The subject line is in the imperative mood ("Add X", not "Added X"), at most
  50 characters, and has no closing period.
- A blank line separates the subject from the body. Body lines are at most 72
  characters.
- Each commit is one logical change.

A release drop from upstream keeps its established form ("39.85", "Changes in
version 39.84"), so that the history lines up with the release tags.

## 2. Branches

- **`main`** is always releasable. It is protected: a change reaches it only
  through a reviewed pull request whose checks pass.
- **Version branches** (`39.99`) collect the work for one release. They exist in
  **both** repositories under the same name.
- **Work branches** are named for the version and the change
  (`39.99-menu-fix`), and branch from the version branch.

### Paired branches

A change to Sd often changes what the regression suite expects. Make such a
change on a branch with **the same name** in `sd` and in `sd-regression`. CI in
either repository checks out the other repository at the first of these that
exists there:

1. A ref given by hand to a manual run (`workflow_dispatch`).
2. For a tag push, the same tag.
3. The branch being built (for a pull request, its head branch).
4. For a pull request, its base branch.
5. `main`.

A branch therefore needs a partner only when the change alters test results. A
work branch with no partner is tested against its version branch's suite, and a
version branch with no partner is tested against `main`.

Merge the two pull requests of a pair together, so that neither repository's
`main` is ever tested against the other's unmerged change.

## 3. Pull Requests and Merging

- Every change reaches `main` and the version branches through a pull request.
- A pull request passes every check before merge.
- Prefer **squash merge** for work branches, so that the version branch has one
  commit per change.
- Delete the work branch after merge.

## 4. Tags and Releases

- Tag each release as `v<MAJOR>.<MINOR>` (`v39.85`) in **both** repositories,
  on the commits that were tested together.
- CI for a tag push in one repository checks out the same tag in the other.
