#!/usr/bin/env bash
#
# Build Sdtty and the call database out of the source tree, and smoke-test them.
#
# Copies the sources of an sd checkout into a build directory, runs one of the
# makefiles in src/ there, and checks that it produced sdtty, mkcalls, and
# sd_calls.dat. Then runs `sdtty -help` and a short scripted sequence, so that a
# binary that builds but cannot start fails here. The checkout itself is left
# untouched.
#
# Usage:
#   scripts/build-sd.sh [options]
#
# Options:
#   --sd DIR        The sd checkout to build (default: this repository in sd,
#                   ../sd beside this repository in sd-regression, or
#                   $SD_DIR when it is set)
#   -f, --makefile NAME
#                   The makefile in src/ to use (default: makefile.linux64 on
#                   Linux, makefile.mac on macOS, or $SD_MAKEFILE)
#   -o, --out DIR   Build directory (default: .build/sd)
#   -h, --help      Show this help
#
# On success the build directory holds src/sdtty, src/mkcalls, and
# src/sd_calls.dat. The compiler's warnings, if any, are counted and listed;
# they do not fail the build, since the makefiles choose the warnings.

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

SD="${SD_DIR:-}"
MAKEFILE="${SD_MAKEFILE:-$default_makefile}"
OUT="$BUILD_ROOT/sd"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --sd)
            SD="${2:?--sd needs a directory}"
            shift 2
            ;;
        -f | --makefile)
            MAKEFILE="${2:?--makefile needs a name}"
            shift 2
            ;;
        -o | --out)
            OUT="${2:?--out needs a directory}"
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

if [[ -z "$SD" ]]; then
    if [[ "$REPO_KIND" == sd ]]; then
        SD="$PROJECT_ROOT"
    else
        SD="$(dirname "$PROJECT_ROOT")/sd"
    fi
fi
if [[ ! -f "$SD/src/sd_calls.txt" || ! -d "$SD/src" ]]; then
    echo "error: $SD is not an sd checkout (no src/sd_calls.txt)." >&2
    echo "Clone sd beside this repository, or name it with --sd or SD_DIR." >&2
    exit 1
fi
SD="$(cd "$SD" && pwd)"
if [[ ! -f "$SD/src/$MAKEFILE" ]]; then
    echo "error: $SD/src has no $MAKEFILE" >&2
    exit 1
fi
require_tool make "Install make and a C++ compiler."

# Copy the sources -- tracked files, and new ones that are not ignored -- but
# none of the objects or binaries a build in place may have left in src/.
ensure_build_dir "$OUT"
# The build runs in $OUT/src, so a relative path would no longer name it.
OUT="$(cd "$OUT" && pwd)"
rm -rf "$OUT/src"
mkdir -p "$OUT/src"
if git -C "$SD" rev-parse --is-inside-work-tree > /dev/null 2>&1; then
    git -C "$SD" ls-files -z --cached --others --exclude-standard -- src |
        while IFS= read -r -d '' f; do
            [[ -f "$SD/$f" ]] && cp "$SD/$f" "$OUT/src/"
        done
else
    cp -R "$SD/src/." "$OUT/src/"
fi

jobs=$(getconf _NPROCESSORS_ONLN 2> /dev/null || echo 2)
LOG="$OUT/build.log"
echo "build-sd: building $SD/src with $MAKEFILE in $OUT/src"
if ! (cd "$OUT/src" && make -j "$jobs" -f "$MAKEFILE" > "$LOG" 2>&1); then
    echo "build-sd: FAILED -- make stopped with an error. The end of $LOG:" >&2
    tail -40 "$LOG" >&2
    exit 1
fi

for product in sdtty mkcalls sd_calls.dat; do
    if [[ ! -s "$OUT/src/$product" ]]; then
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
# -diagnostic keeps it from reading or writing an initialization file.
version=$("$OUT/src/sdtty" -help 2>&1 | head -1) || {
    echo "build-sd: FAILED -- sdtty -help did not run" >&2
    exit 1
}
smoke="$OUT/smoke"
rm -rf "$smoke"
mkdir -p "$smoke"
if ! (cd "$smoke" && printf 'heads start\nquit\ny\n' |
    "$OUT/src/sdtty" c4 -db "$OUT/src/sd_calls.dat" \
        -diagnostic -no_console -no_graphics -no_color > transcript 2>&1); then
    echo "build-sd: FAILED -- sdtty could not start a sequence. Its output:" >&2
    cat "$smoke/transcript" >&2
    exit 1
fi
if ! grep -q '^HEADS' "$smoke/transcript"; then
    echo "build-sd: FAILED -- sdtty did not accept 'heads start'. Its output:" >&2
    cat "$smoke/transcript" >&2
    exit 1
fi

echo "build-sd: built $version"
echo "build-sd: passed"
