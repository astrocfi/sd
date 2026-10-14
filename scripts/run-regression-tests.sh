#!/usr/bin/env bash
#
# Run the Sd regression suite from sd-regression against a build of sd.
#
# The suite's own scripts decide what runs: this script runs
# regression_tests/doit.sh, exactly as a developer does, and only arranges the
# files around it and reads the results. It
#
#   1. builds sd with scripts/build-sd.sh (or uses --prebuilt binaries);
#   2. copies the suite into a work directory, beside a copy of the binaries,
#      laid out so that both conventions the suite's scripts use find them:
#        <work>/sd/src/sdtty              ../../sd/src/sdtty from the tests
#        <work>/sd-regression/sdtty       ../sdtty from the tests (a link)
#   3. warms the menu cache for every level, so that the first test run at a
#      level does not report a cache miss that its reference does not contain;
#   4. runs doit.sh, sending its output (which includes the differences, and
#      therefore the tests' contents) to a log file that is not printed;
#   5. compares every reference transcript (*.ref) with the transcript the run
#      wrote (*.tmp), and reports each test by name: passed, FAILED, or not run.
#
# Neither checkout is modified.
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
#   --sd DIR            The sd checkout (default: this repository in sd, or
#                       ../sd beside this repository; or $SD_DIR)
#   --regression DIR    The sd-regression checkout (default: this repository
#                       in sd-regression, or ../sd-regression beside this
#                       repository; or $SD_REGRESSION_DIR)
#   --prebuilt DIR      Use the sdtty and sd_calls.dat in DIR instead of
#                       building (for example .build/sd/src)
#   -o, --out DIR       Work directory (default: .build/regression)
#   --show-diffs        Print the differences for each failed test (also
#                       REGRESSION_SHOW_DIFFS=true, which run-all-checks.sh
#                       passes through)
#   -h, --help          Show this help
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

# Every level sdtty accepts that the suite might use. Warming one that no test
# uses costs about a second.
LEVELS=(m p a1 a2 c1 c2 c3a c3 c3x c4a c4 c4x)

SD="${SD_DIR:-}"
REGRESSION="${SD_REGRESSION_DIR:-}"
PREBUILT=""
OUT="$BUILD_ROOT/regression"
SHOW_DIFFS="${REGRESSION_SHOW_DIFFS:-false}"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --sd)
            SD="${2:?--sd needs a directory}"
            shift 2
            ;;
        --regression)
            REGRESSION="${2:?--regression needs a directory}"
            shift 2
            ;;
        --prebuilt)
            PREBUILT="${2:?--prebuilt needs a directory}"
            shift 2
            ;;
        -o | --out)
            OUT="${2:?--out needs a directory}"
            shift 2
            ;;
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

parent="$(dirname "$PROJECT_ROOT")"
if [[ -z "$SD" ]]; then
    if [[ "$REPO_KIND" == sd ]]; then SD="$PROJECT_ROOT"; else SD="$parent/sd"; fi
fi
if [[ -z "$REGRESSION" ]]; then
    if [[ "$REPO_KIND" == sd-regression ]]; then
        REGRESSION="$PROJECT_ROOT"
    else
        REGRESSION="$parent/sd-regression"
    fi
fi

TESTS="$REGRESSION/regression_tests"
if [[ ! -f "$TESTS/doit.sh" ]]; then
    echo "error: $TESTS/doit.sh not found." >&2
    echo "Clone the private sd-regression repository beside this one, or name it" >&2
    echo "with --regression or SD_REGRESSION_DIR." >&2
    exit 1
fi
SD="$(cd "$SD" && pwd)"
REGRESSION="$(cd "$REGRESSION" && pwd)"

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

ensure_build_dir "$OUT"
# The run happens in a directory under $OUT, so a relative path would no
# longer name it.
OUT="$(cd "$OUT" && pwd)"
rm -rf "$OUT/sd" "$OUT/sd-regression" "$OUT/doit.log"
mkdir -p "$OUT/sd/src" "$OUT/sd-regression"

# 1. The binaries.
if [[ -n "$PREBUILT" ]]; then
    BIN="$(cd "$PREBUILT" && pwd)"
else
    "$PROJECT_ROOT/scripts/build-sd.sh" --sd "$SD" --out "$OUT/build"
    BIN="$OUT/build/src"
fi
for product in sdtty sd_calls.dat; do
    if [[ ! -s "$BIN/$product" ]]; then
        echo "error: $BIN/$product is missing" >&2
        exit 1
    fi
    cp "$BIN/$product" "$OUT/sd/src/"
    ln -s "../sd/src/$product" "$OUT/sd-regression/$product"
done

# 2. The suite. cp -p keeps the scripts executable.
cp -Rp "$TESTS" "$OUT/sd-regression/regression_tests"
WORK="$OUT/sd-regression/regression_tests"
rm -f "$WORK"/*.tmp

# 3. The menu caches. The cache file is named after the database path as
# given, so each spelling the scripts use gets its own.
echo "regression: warming the menu cache for ${#LEVELS[@]} levels"
for level in "${LEVELS[@]}"; do
    for prefix in .. ../../sd/src; do
        (cd "$WORK" && "$prefix/sdtty" "$level" -db "$prefix/sd_calls.dat" \
            -diagnostic -no_console -no_graphics -no_color < /dev/null > /dev/null 2>&1) || true
    done
done

# 4. The run. Its output holds the tests' contents, so it goes to a file.
echo "regression: running doit.sh (this takes several minutes)"
start=$(date +%s)
(cd "$WORK" && sh ./doit.sh > "$OUT/doit.log" 2>&1) || true
elapsed=$(($(date +%s) - start))

# 5. The results.
passed=()
failed=()
not_run=()
declare -A differing=()
shopt -s nullglob
for ref in "$WORK"/*.ref; do
    name=$(basename "$ref" .ref)
    tmp="$WORK/$name.tmp"
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
        diff "$WORK/$name.tmp" "$WORK/$name.ref" | sed 's/^/      /' || true
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
    echo "regression: FAILED -- no test ran. The run's log is $OUT/doit.log" >&2
    exit 1
fi
if [[ ${#failed[@]} -gt 0 ]]; then
    echo "regression: FAILED -- ${#failed[@]} test(s) differ from their references."
    echo "regression: compare $WORK/NAME.tmp with NAME.ref to see why."
    exit 1
fi
echo "regression: passed"
