#!/usr/bin/env bash
#
# Build the developer guide (a Sphinx project) as HTML, with every warning an
# error.
#
# The guide's sources are in the directory that DEV_GUIDE_DIR in
# scripts/checks.conf names (docs/). The build goes to
# .build/dev-guide/html, so the source directory is left untouched. Sphinx's
# warnings include a cross-reference that does not resolve, a page that no
# table of contents lists, and a skipped heading level.
#
# Usage:
#   scripts/build-dev-guide.sh [options]
#
# Options:
#   -o, --out DIR   Build directory (default: .build/dev-guide)
#   --open          Open the built guide in a browser afterwards
#   -h, --help      Show this help
#
# Environment:
#   BROWSER  a command to open the page with, instead of the platform default
#
# Requirements: the packages in <DEV_GUIDE_DIR>/requirements.txt, which
# `scripts/run-all-checks.sh --setup` installs. Read the Docs builds the same
# project, with the same settings, from .readthedocs.yaml.

set -euo pipefail

# shellcheck source=lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
load_checks_conf

usage() {
    sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

OUT="$BUILD_ROOT/dev-guide"
OPEN=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -o | --out)
            [[ $# -ge 2 ]] || { echo "error: $1 needs a directory" >&2; exit 2; }
            OUT="$2"
            shift
            ;;
        --open) OPEN=true ;;
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
    shift
done

SRC="$PROJECT_ROOT/$DEV_GUIDE_DIR"
if [[ ! -f "$SRC/conf.py" ]]; then
    echo "dev-guide: FAILED -- $SRC/conf.py is missing" >&2
    exit 1
fi

activate_venv
require_tool sphinx-build "Run scripts/run-all-checks.sh --setup." || exit 1

ensure_build_dir "$OUT"
HTML="$OUT/html"
# A fresh build each time: Sphinx reports a page's warnings only when it reads
# the page, and an incremental build skips the pages that have not changed.
rm -rf "$HTML" "$OUT/doctrees"

echo "dev-guide: building $DEV_GUIDE_DIR into $HTML"
if ! sphinx-build -W --keep-going -q -b html -d "$OUT/doctrees" "$SRC" "$HTML"; then
    echo "dev-guide: FAILED -- fix the warnings above" >&2
    exit 1
fi
echo "dev-guide: passed"

if [[ "$OPEN" == true ]]; then
    page="$HTML/index.html"
    if [[ -n "${BROWSER:-}" ]]; then
        "$BROWSER" "$page"
    elif command -v xdg-open > /dev/null 2>&1; then
        xdg-open "$page"
    elif command -v open > /dev/null 2>&1; then
        open "$page"
    else
        echo "dev-guide: open $page in a browser"
    fi
fi
