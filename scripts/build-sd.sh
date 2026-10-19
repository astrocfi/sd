#!/usr/bin/env bash
#
# Build Sdtty and the call database in place, and smoke-test them.
#
# Runs one of the makefiles in sd's src/, the way a developer does, and checks
# that it produced sdtty, mkcalls, and sd_calls.dat. Then runs `sdtty -help` and
# a short scripted sequence, so that a binary that builds but cannot start fails
# here. Everything the build writes in src/ is ignored by git.
#
# sd and sd-regression are cloned side by side, so in sd-regression this builds
# ../sd, the checkout the regression suite's scripts run.
#
# Usage:
#   scripts/build-sd.sh [options]
#
# Options:
#   -f, --makefile NAME
#                   The makefile in src/ to use (default: makefile.linux64 on
#                   Linux, makefile.mac on macOS, or $SD_MAKEFILE)
#   -h, --help      Show this help
#
# On success src/ holds sdtty, mkcalls, and sd_calls.dat. The compiler's
# warnings, if any, are counted and listed; they do not fail the build, since
# the makefiles choose the warnings.

set -euo pipefail

# shellcheck source=lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
load_checks_conf

usage() {
    sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

case "$(uname -s)" in
    Darwin) default_makefile=makefile.mac ;;
    *) default_makefile=makefile.linux64 ;;
esac

MAKEFILE="${SD_MAKEFILE:-$default_makefile}"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -f | --makefile)
            MAKEFILE="${2:?--makefile needs a name}"
            shift 2
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

if [[ "$REPO_KIND" == sd ]]; then
    SD="$PROJECT_ROOT"
else
    SD="$(dirname "$PROJECT_ROOT")/sd"
fi
SRC="$SD/src"
if [[ ! -f "$SRC/sd_calls.txt" ]]; then
    echo "error: $SD is not an sd checkout (no src/sd_calls.txt)." >&2
    echo "Clone sd and sd-regression side by side, under the same parent directory." >&2
    exit 1
fi
if [[ ! -f "$SRC/$MAKEFILE" ]]; then
    echo "error: $SRC has no $MAKEFILE" >&2
    exit 1
fi
require_tool make "Install make and a C++ compiler."

SCRATCH=$(mktemp -d)
trap 'rm -rf "$SCRATCH"' EXIT

jobs=$(getconf _NPROCESSORS_ONLN 2> /dev/null || echo 2)
LOG="$SCRATCH/build.log"
echo "build-sd: building $SRC with $MAKEFILE"
if ! (cd "$SRC" && make -j "$jobs" -f "$MAKEFILE" > "$LOG" 2>&1); then
    echo "build-sd: FAILED -- make stopped with an error. The end of its output:" >&2
    tail -40 "$LOG" >&2
    exit 1
fi

for product in sdtty mkcalls sd_calls.dat; do
    if [[ ! -s "$SRC/$product" ]]; then
        echo "build-sd: FAILED -- $MAKEFILE did not produce $product" >&2
        exit 1
    fi
done

warnings=$(grep -c 'warning:' "$LOG" || true)
if [[ "$warnings" -gt 0 ]]; then
    echo "build-sd: the compiler printed $warnings warning(s):"
    grep 'warning:' "$LOG" | sed 's/^/  /'
fi

# Smoke test: the program starts, reads the database, and can start a sequence.
# -diagnostic keeps it from reading or writing an initialization file, and the
# run is in a scratch directory, so the sequence file it writes is thrown away.
version=$("$SRC/sdtty" -help 2>&1 | head -1) || {
    echo "build-sd: FAILED -- sdtty -help did not run" >&2
    exit 1
}
if ! (cd "$SCRATCH" && printf 'heads start\nquit\ny\n' |
    "$SRC/sdtty" c4 -db "$SRC/sd_calls.dat" \
        -diagnostic -no_console -no_graphics -no_color > transcript 2>&1); then
    echo "build-sd: FAILED -- sdtty could not start a sequence. Its output:" >&2
    cat "$SCRATCH/transcript" >&2
    exit 1
fi
if ! grep -q '^HEADS' "$SCRATCH/transcript"; then
    echo "build-sd: FAILED -- sdtty did not accept 'heads start'. Its output:" >&2
    cat "$SCRATCH/transcript" >&2
    exit 1
fi

echo "build-sd: built $version"
echo "build-sd: passed"
