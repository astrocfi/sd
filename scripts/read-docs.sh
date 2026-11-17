#!/usr/bin/env bash
#
# Build the Sd manuals as HTML and open the main one in a browser.
#
# Builds through scripts/build-docs.sh, which builds out of the source tree and
# checks the warnings against the baseline, as CI does. Then opens
# .build/docs/output/sd_doc.html with the platform's default handler.
#
# Usage:
#   scripts/read-docs.sh                build the HTML, then open it
#   scripts/read-docs.sh --open         open what is already built, without building
#   scripts/read-docs.sh --build        build without opening
#   scripts/read-docs.sh --manual NAME  open NAME.html instead of sd_doc.html
#                                       (faq, sessions, demo, relnotes, ...)
#
# Environment:
#   BROWSER  a command to open the page with, instead of the platform default

set -euo pipefail

# shellcheck source=lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
load_checks_conf

should_build=yes
should_open=yes
manual=sd_doc

while [[ $# -gt 0 ]]; do
    case "$1" in
        --open)
            should_build=no
            shift
            ;;
        --build)
            should_open=no
            shift
            ;;
        --manual)
            manual="${2:?--manual needs a name}"
            shift 2
            ;;
        -h | --help)
            sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "Usage: $0 [--open | --build] [--manual NAME]" >&2
            exit 2
            ;;
    esac
done

HTML="$BUILD_ROOT/docs/output/$manual.html"

open_html() {
    local path=$1

    if [[ -n "${BROWSER:-}" ]]; then
        "$BROWSER" "$path"
        return
    fi

    case "$(uname -s)" in
        Linux)
            # A headless shell has no handler to hand the page to; say where it
            # is rather than failing on a missing xdg-open or a missing DISPLAY.
            if [[ -z "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ]] || ! command -v xdg-open > /dev/null 2>&1; then
                echo "No browser to open here. The manual is at:"
                echo "  file://$path"
                return
            fi
            xdg-open "$path"
            ;;
        Darwin)
            open "$path"
            ;;
        CYGWIN* | MINGW* | MSYS*)
            if command -v cygpath > /dev/null 2>&1; then
                MSYS_NO_PATHCONV=1 cmd.exe //C start "" "$(cygpath -w "$path")"
            else
                cmd.exe //C start "" "$path"
            fi
            ;;
        *)
            echo "Unsupported platform $(uname -s). The manual is at:"
            echo "  file://$path"
            ;;
    esac
}

if [[ "$should_build" == yes ]]; then
    "$PROJECT_ROOT/scripts/build-docs.sh" --formats html
fi

if [[ ! -f "$HTML" ]]; then
    echo "Error: no built manual at $HTML" >&2
    echo "Run '$0' without --open to build it first." >&2
    exit 1
fi

if [[ "$should_open" == yes ]]; then
    open_html "$HTML"
fi
