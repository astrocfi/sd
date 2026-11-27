#!/usr/bin/env bash
#
# Run the checks for this repository: the single source of truth for which
# checks exist. CI runs the same checks, through this script.
#
# Usage:
#   scripts/run-all-checks.sh [options]
#
# Scopes (with none, everything except the regression suite runs):
#   -c, --code         Build Sdtty and the call database, and smoke-test them
#   -d, --docs         Build the manuals and the developer guide, and run the
#                      text checks
#   -m, --text         The text checks only: spelling, prose, Markdown, shell
#                      scripts, and workflows
#   -r, --regression   Run the regression suite (needs both repositories)
#
# Single checks (may be combined with each other and with the scopes):
#   --build            scripts/build-sd.sh
#   --docs-build       scripts/build-docs.sh
#   --dev-guide        scripts/build-dev-guide.sh
#   --codespell        scripts/check-spelling.sh
#   --prose            scripts/check-doc-prose.sh
#   --pymarkdown       PyMarkdown over the Markdown in scripts/checks.conf
#   --shellcheck       ShellCheck over scripts/*.sh
#   --actionlint       actionlint over .github/workflows
#
# Other options:
#   -p, --parallel     Run the checks in parallel (the default)
#   -s, --sequential   Run the checks one after another, with their output as
#                      it happens (easier to read when one fails)
#   --setup            Create the lint tools' virtual environment (.build/venv)
#                      from scripts/requirements.txt and the developer guide's
#                      requirements.txt, then exit
#   -h, --help         Show this help
#
# Environment:
#   VENV         The lint tools' virtual environment (default: .build/venv);
#                "none" uses the tools on PATH, as CI does
#   SD_MAKEFILE  Passed through to scripts/build-sd.sh and
#                scripts/run-regression-tests.sh
#
# Exit status: 0 when every check passed, 1 when any failed, 2 for a usage
# error. scripts/checks.conf says what each check reads.

set -uo pipefail

# shellcheck source=lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
load_checks_conf || exit 1

RED='\033[0;31m'
GREEN='\033[0;32m'
BOLD='\033[1m'
RESET='\033[0m'
if [[ ! -t 1 ]]; then
    RED='' GREEN='' BOLD='' RESET=''
fi

usage() {
    sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

# The checks, in the order they run and are reported.
ALL_CHECKS=(build docs-build dev-guide codespell prose pymarkdown shellcheck actionlint regression)
TEXT_CHECKS=(codespell prose pymarkdown shellcheck actionlint)
declare -A WANT=()
PARALLEL=true
SETUP=false

want() {
    local c
    for c in "$@"; do
        WANT[$c]=1
    done
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c | --code) want build ;;
        -d | --docs) want docs-build dev-guide "${TEXT_CHECKS[@]}" ;;
        -m | --text | --markdown) want "${TEXT_CHECKS[@]}" ;;
        -r | --regression) want regression ;;
        --build) want build ;;
        --docs-build) want docs-build ;;
        --dev-guide) want dev-guide ;;
        --codespell) want codespell ;;
        --prose) want prose ;;
        --pymarkdown) want pymarkdown ;;
        --shellcheck) want shellcheck ;;
        --actionlint) want actionlint ;;
        -p | --parallel) PARALLEL=true ;;
        -s | --sequential) PARALLEL=false ;;
        --setup) SETUP=true ;;
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

if [[ "$SETUP" == true ]]; then
    venv="${VENV:-${VENV_PATH:-$BUILD_ROOT/venv}}"
    if [[ "$venv" == none ]]; then
        echo "error: VENV=none means the tools are installed already" >&2
        exit 2
    fi
    requirements=(-r "$PROJECT_ROOT/scripts/requirements.txt")
    requirements+=(-r "$PROJECT_ROOT/$DEV_GUIDE_DIR/requirements.txt")
    ensure_build_dir "$BUILD_ROOT"
    python3 -m venv --upgrade-deps "$venv" &&
        "$venv/bin/pip" install --quiet "${requirements[@]}" &&
        echo "run-all-checks: installed the lint tools in $venv"
    exit $?
fi

# With no scope, run everything but the regression suite, which needs the
# private repository.
if [[ ${#WANT[@]} -eq 0 ]]; then
    want build docs-build dev-guide "${TEXT_CHECKS[@]}"
fi

activate_venv
cd "$PROJECT_ROOT" || exit 1

# Each check is a function that returns non-zero on failure.

check_build() {
    scripts/build-sd.sh
}

check_docs-build() {
    scripts/build-docs.sh
}

check_dev-guide() {
    scripts/build-dev-guide.sh
}

check_codespell() {
    scripts/check-spelling.sh
}

check_prose() {
    scripts/check-doc-prose.sh
}

check_pymarkdown() {
    require_tool pymarkdown "Run scripts/run-all-checks.sh --setup." || return 1
    local files
    mapfile -t files < <(list_files "${MARKDOWN_FILES[@]}")
    if [[ ${#files[@]} -eq 0 ]]; then
        echo "pymarkdown: no Markdown files to check"
        return 0
    fi
    echo "pymarkdown: ${#files[@]} file(s)"
    pymarkdown --config .pymarkdown.json scan "${files[@]}" && echo "pymarkdown: passed"
}

check_shellcheck() {
    require_tool shellcheck "Run scripts/run-all-checks.sh --setup." || return 1
    local files
    mapfile -t files < <(list_files "${SHELL_FILES[@]}")
    if [[ ${#files[@]} -eq 0 ]]; then
        echo "shellcheck: no shell scripts to check"
        return 0
    fi
    echo "shellcheck: ${#files[@]} file(s)"
    shellcheck "${files[@]}" && echo "shellcheck: passed"
}

check_actionlint() {
    require_tool actionlint "Run scripts/run-all-checks.sh --setup." || return 1
    local files
    mapfile -t files < <(list_files "${WORKFLOW_FILES[@]}")
    if [[ ${#files[@]} -eq 0 ]]; then
        echo "actionlint: no workflows to check"
        return 0
    fi
    echo "actionlint: ${#files[@]} file(s)"
    # actionlint runs ShellCheck over each run: block, which is on PATH here.
    actionlint "${files[@]}" && echo "actionlint: passed"
}

check_regression() {
    if [[ ! -x scripts/run-regression-tests.sh ]]; then
        echo "regression: scripts/run-regression-tests.sh is missing" >&2
        return 1
    fi
    scripts/run-regression-tests.sh
}

SELECTED=()
for c in "${ALL_CHECKS[@]}"; do
    [[ -n "${WANT[$c]:-}" ]] && SELECTED+=("$c")
done

LOG_DIR="$BUILD_ROOT/checks"
ensure_build_dir "$LOG_DIR"
declare -A STATUS=()

printf '%brun-all-checks: %s (%s)%b\n' "$BOLD" "${SELECTED[*]}" \
    "$([[ "$PARALLEL" == true ]] && echo parallel || echo sequential)" "$RESET"

if [[ "$PARALLEL" == true ]]; then
    # The build and the regression suite both build in src/, so the regression
    # suite waits for the others rather than run make beside the build.
    declare -A PIDS=()
    for c in "${SELECTED[@]}"; do
        [[ "$c" == regression ]] && continue
        "check_$c" > "$LOG_DIR/$c.log" 2>&1 &
        PIDS[$c]=$!
    done
    for c in "${!PIDS[@]}"; do
        if wait "${PIDS[$c]}"; then
            STATUS[$c]=0
        else
            STATUS[$c]=1
        fi
    done
    if [[ -n "${WANT[regression]:-}" ]]; then
        if check_regression > "$LOG_DIR/regression.log" 2>&1; then
            STATUS[regression]=0
        else
            STATUS[regression]=1
        fi
    fi
    for c in "${SELECTED[@]}"; do
        printf '\n%b==== %s ====%b\n' "$BOLD" "$c" "$RESET"
        cat "$LOG_DIR/$c.log"
    done
else
    for c in "${SELECTED[@]}"; do
        printf '\n%b==== %s ====%b\n' "$BOLD" "$c" "$RESET"
        if "check_$c"; then
            STATUS[$c]=0
        else
            STATUS[$c]=1
        fi
    done
fi

FAILED=()
printf '\n%bSummary%b\n' "$BOLD" "$RESET"
for c in "${SELECTED[@]}"; do
    if [[ "${STATUS[$c]}" -eq 0 ]]; then
        printf '  %bpassed%b  %s\n' "$GREEN" "$RESET" "$c"
    else
        printf '  %bFAILED%b  %s\n' "$RED" "$RESET" "$c"
        FAILED+=("$c")
    fi
done

if [[ ${#FAILED[@]} -gt 0 ]]; then
    printf '\n%brun-all-checks: FAILED -- %s%b\n' "$RED" "${FAILED[*]}" "$RESET"
    exit 1
fi
printf '\n%brun-all-checks: all checks passed%b\n' "$GREEN" "$RESET"
