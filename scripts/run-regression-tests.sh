#!/usr/bin/env bash
#
# Run the Sd regression suite from sd-regression against a build of sd.
#
# sd and sd-regression are cloned side by side, under the same parent
# directory, and the suite's scripts run ../../sd/src/sdtty from
# sd-regression/regression_tests. This script uses that layout as it is: it
#
#   1. builds sd in place with scripts/build-sd.sh;
#   2. runs regression_tests/doit.sh in sd-regression, exactly as a developer
#      does, sending its output (which includes the differences, and therefore
#      the tests' contents) to regression_tests/doit.log, which is not printed;
#   3. compares every reference transcript (*.ref) with the transcript the run
#      wrote (*.tmp), and reports each test by name: passed, FAILED, or not run.
#
# Everything the build and the run write is ignored by git in both checkouts.
#
# The tests are private: most are transcriptions of choreography called at real
# dances. This script prints test names and the number of differing lines, and
# never their contents, unless --show-diffs is given. In the sd repository on
# GitHub Actions, whose logs are public, it refuses --show-diffs.
#
# Usage:
#   scripts/run-regression-tests.sh [options]
#
# Options:
#   --show-diffs        Print the differences for each failed test (also
#                       REGRESSION_SHOW_DIFFS=true, which run-all-checks.sh
#                       passes through)
#   -h, --help          Show this help
#
# SD_MAKEFILE is passed through to scripts/build-sd.sh.
#
# Exit status: 0 when every test that ran passed, 1 when any failed or none
# ran, 2 for a usage error.

set -euo pipefail

# shellcheck source=lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
load_checks_conf

usage() {
    sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

SHOW_DIFFS="${REGRESSION_SHOW_DIFFS:-false}"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --show-diffs)
            SHOW_DIFFS=true
            shift
            ;;
        -h | --help)
            usage
            exit 0
            ;;
        *)
            echo "error: unknown option $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ "$SHOW_DIFFS" == true && "$REPO_KIND" == sd && "${GITHUB_ACTIONS:-}" == true ]]; then
    echo "error: --show-diffs would publish private test content in sd's public CI log" >&2
    exit 2
fi

# The suite's scripts name ../../sd/src, so the two checkouts must be siblings
# named sd and sd-regression.
parent="$(dirname "$PROJECT_ROOT")"
SD="$parent/sd"
REGRESSION="$parent/sd-regression"
if [[ "$PROJECT_ROOT" != "$SD" && "$PROJECT_ROOT" != "$REGRESSION" ]]; then
    echo "error: this checkout is $PROJECT_ROOT; the regression suite needs it to be" >&2
    echo "named sd or sd-regression, beside the other." >&2
    exit 1
fi
TESTS="$REGRESSION/regression_tests"
if [[ ! -f "$TESTS/doit.sh" ]]; then
    echo "error: $TESTS/doit.sh not found." >&2
    echo "Clone the private sd-regression repository beside sd, under the same parent" >&2
    echo "directory." >&2
    exit 1
fi

# The two checkouts should be on partner branches; say so when two named
# branches differ, since a mismatch is the usual reason for a surprising
# result. (CI checks both out detached, after scripts/companion-ref.sh has
# chosen the partner.)
branch_of() {
    git -C "$1" symbolic-ref --quiet --short HEAD 2> /dev/null ||
        git -C "$1" describe --tags --exact-match 2> /dev/null ||
        git -C "$1" rev-parse --short HEAD 2> /dev/null ||
        echo unknown
}
sd_branch=$(branch_of "$SD")
regression_branch=$(branch_of "$REGRESSION")
echo "regression: sd at $sd_branch ($SD)"
echo "regression: sd-regression at $regression_branch ($REGRESSION)"
if git -C "$SD" symbolic-ref --quiet HEAD > /dev/null 2>&1 &&
    git -C "$REGRESSION" symbolic-ref --quiet HEAD > /dev/null 2>&1 &&
    [[ "$sd_branch" != "$regression_branch" ]]; then
    echo "regression: note -- the two checkouts are on different branches"
fi

# 1. The build, in sd/src, where the suite's scripts look for it.
"$PROJECT_ROOT/scripts/build-sd.sh"

# 2. The run. A transcript left by an earlier run would count as this run's,
# so the old ones go first. doit.sh warms the menu cache itself. Its output
# holds the tests' contents, so it goes to a file.
rm -f "$TESTS"/*.tmp "$TESTS/doit.log"
LOG="$TESTS/doit.log"
echo "regression: running doit.sh (this takes several minutes)"
start=$(date +%s)
(cd "$TESTS" && sh ./doit.sh > "$LOG" 2>&1) || true
elapsed=$(($(date +%s) - start))

# 3. The results.
passed=()
failed=()
not_run=()
declare -A differing=()
shopt -s nullglob
for ref in "$TESTS"/*.ref; do
    name=$(basename "$ref" .ref)
    tmp="$TESTS/$name.tmp"
    if [[ ! -f "$tmp" ]]; then
        not_run+=("$name")
    elif cmp -s "$tmp" "$ref"; then
        passed+=("$name")
    else
        failed+=("$name")
        differing[$name]=$(diff "$tmp" "$ref" | grep -c '^[<>]' || true)
    fi
done
shopt -u nullglob

echo "regression: doit.sh ran in ${elapsed}s"
echo "regression: ${#passed[@]} passed, ${#failed[@]} failed, ${#not_run[@]} not run by doit.sh"
for name in "${failed[@]}"; do
    echo "  FAILED   $name (${differing[$name]} line(s) differ)"
    if [[ "${GITHUB_ACTIONS:-}" == true ]]; then
        echo "::error title=Regression test failed::$name: ${differing[$name]} line(s) differ from $name.ref"
    fi
    if [[ "$SHOW_DIFFS" == true ]]; then
        diff "$TESTS/$name.tmp" "$TESTS/$name.ref" | sed 's/^/      /' || true
    fi
done
for name in "${not_run[@]}"; do
    echo "  not run  $name (doit.sh does not run it)"
done

if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    tick='`'
    {
        echo "### Regression tests"
        echo
        echo "sd at \`$sd_branch\`, sd-regression at \`$regression_branch\`."
        echo
        echo "| Passed | Failed | Not run |"
        echo "|---|---|---|"
        echo "| ${#passed[@]} | ${#failed[@]} | ${#not_run[@]} |"
        if [[ ${#failed[@]} -gt 0 ]]; then
            echo
            echo "Failed: $(printf "$tick%s$tick " "${failed[@]}")"
        fi
        if [[ ${#not_run[@]} -gt 0 ]]; then
            echo
            echo "Not run by \`doit.sh\`: $(printf "$tick%s$tick " "${not_run[@]}")"
        fi
    } >> "$GITHUB_STEP_SUMMARY"
fi

if [[ ${#passed[@]} -eq 0 && ${#failed[@]} -eq 0 ]]; then
    echo "regression: FAILED -- no test ran. The run's log is $LOG" >&2
    exit 1
fi
if [[ ${#failed[@]} -gt 0 ]]; then
    echo "regression: FAILED -- ${#failed[@]} test(s) differ from their references."
    echo "regression: compare $TESTS/NAME.tmp with NAME.ref to see why."
    exit 1
fi
echo "regression: passed"
