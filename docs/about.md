# About This Guide

The guide is a Sphinx project in `docs/`, written in Markdown with the MyST
extensions. Build it with:

```bash
scripts/run-all-checks.sh --setup    # once: installs Sphinx with the lint tools
scripts/build-dev-guide.sh           # builds .build/dev-guide/html
```

`scripts/build-dev-guide.sh` treats every Sphinx warning as an error, as CI's
Docs job does. A broken cross-reference, a page missing from every table of
contents, and a heading level skipped are all warnings.

## Publishing

`.readthedocs.yaml`, at the repository root, configures Read the Docs to build
the guide with the same settings: the Sphinx versions pinned in
`docs/requirements.txt`, with every warning an error. A maintainer imports the
`sd` repository into Read the Docs once; it then rebuilds the guide on each
push.

## How It Is Organized

Each chapter is a directory under `docs/` with an `index.md` that introduces
the chapter and lists its pages in a `toctree`. The chapters are listed in the
`toctree` of `docs/index.md`. Continuous integration (`docs/ci/`) is the first
chapter.

To add a chapter:

1. Create `docs/<chapter>/index.md`, with a single top-level heading and a
   `toctree` of its pages.
2. Add `<chapter>/index` to the `toctree` in `docs/index.md`.
3. Run `scripts/run-all-checks.sh --docs`, which builds the guide and checks
   its spelling, its Markdown, and its prose.

Name files in lower case, with underscores between words.

## Writing for the Guide

The guide is developer documentation, and follows the same rules as
`CONTRIBUTING.md`:

- `.claude/rules/doc_prose_style.md` governs the prose. The guide may name
  source files, functions, and scripts freely.
- The `sd` repository is public, and so is this guide. Name a regression test
  (`t50t`), but never quote one, its reference transcript, or its output. See
  `.claude/rules/security.md`.
- Link to another page with a relative Markdown link
  (`[the check runner](checks.md)`), and to a section of one with its anchor
  (`[the deploy key](regression.md#the-deploy-key)`). Sphinx reports a link
  that does not resolve.
