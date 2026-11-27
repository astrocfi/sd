# Maintaining CI

## Adding a Check

1. Write the check as a script in `scripts/`, or as a function in
   `scripts/run-all-checks.sh` that runs a tool. Make it pass on the
   repository as it stands, with a baseline if the existing files have
   findings (see [Baselines](baselines.md)).
2. Add it to `ALL_CHECKS` in `scripts/run-all-checks.sh`, give it an option,
   and add it to the scopes it belongs to.
3. If it reads a list of files, add a setting to `scripts/checks.conf`, and
   read the setting in the script.
4. Run it in a workflow job. A text check joins the Lint job by joining the
   `--text` scope; anything else needs a job, or a step in one, that installs
   its tools and runs `scripts/run-all-checks.sh` with its option.
5. If `sd-regression` needs the check too, add it there the same way.
6. Describe it in [The Check Runner](checks.md), in `CONTRIBUTING.md`, and in
   `.claude/skills/run-all-checks`. Both repositories have that skill, and it
   is identical in the two, so change it in `sd-regression` as well.

## Adding a Build

The Build job's matrix in `sd`'s workflow lists the runner and the makefile
for each build. To add one, add a row:

```yaml
- os: ubuntu-latest
  makefile: makefile.linux32
```

`scripts/build-sd.sh` needs only the makefile's name.

## Updating the Tools

The Python tools are pinned in `scripts/requirements.txt` (the lint tools) and
`docs/requirements.txt` (Sphinx), so that a local run and CI report the same
findings. Dependabot does not update them. To update one:

1. Change its version in the requirements file.
2. Run `scripts/run-all-checks.sh --setup` and then `scripts/run-all-checks.sh`.
3. Fix, or record in a baseline, anything the new version finds.
4. If `sd-regression` uses the tool too (PyMarkdown, ShellCheck, or
   actionlint), make the same change to its `scripts/requirements.txt`.

Dependabot proposes new major versions of the GitHub Actions each week. Review
the action's release notes, and merge its pull requests in both repositories.

## Rotating the Deploy Key

Replace the key when someone who could read it leaves the project, when it may
have been exposed (in a log, a commit, or a message), or as routine. Deleting
an exposed copy does not revoke the key; only removing it from `sd-regression`
does.

A maintainer with admin access to both repositories runs:

```bash
ssh-keygen -t ed25519 -N '' -C 'sd CI: read sd-regression' -f sd-regression-key
gh repo deploy-key add sd-regression-key.pub --repo astrocfi/sd-regression \
    --title 'sd CI (read-only)'
gh secret set SD_REGRESSION_SSH_KEY --repo astrocfi/sd < sd-regression-key
rm sd-regression-key sd-regression-key.pub
```

`gh repo deploy-key add` creates a read-only key unless it is given
`--allow-write`; never give it write access. Then remove the old key:

```bash
gh repo deploy-key list --repo astrocfi/sd-regression
gh repo deploy-key delete <id> --repo astrocfi/sd-regression
```

Rerun a regression job to confirm the new key works. Its "Check for the deploy
key" step passes, and the checkout of `sd-regression` succeeds.
