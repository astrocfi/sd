#!/usr/bin/env bash
#
# Run the checks for this repository: the single source of truth for which
# checks exist. CI runs the same checks, through this script.
#
# Usage:
#   scripts/run-all-checks.sh [options]
#
# Scopes (with none, everything runs):
#   -c, --code         Build Sdtty and the call database, and smoke-test them
#   -d, --docs         Build the manuals, and run the text checks
#   -m, --text         The text checks only: spelling, prose, Markdown, shell
#                      scripts, and workflows
#
# Single checks (may be combined with each other and with the scopes):
#   --build            scripts/build-sd.sh
#   --docs-build       scripts/build-docs.sh
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
#                      from scripts/requirements.txt, then exit
#   -h, --help         Show this help
#
# Environment:
#   VENV         The lint tools' virtual environment (default: .build/venv);
#                "none" uses the tools on PATH, as CI does
#   SD_DIR, SD_MAKEFILE
#                Passed through to scripts/build-sd.sh
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
ALL_CHECKS=(build docs-build codespell prose pymarkdown shellcheck actionlint)
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
        -d | --docs) want docs-build "${TEXT_CHECKS[@]}" ;;
        -m | --text | --markdown) want "${TEXT_CHECKS[@]}" ;;
        --build) want build ;;
        --docs-build) want docs-build ;;
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
    ensure_build_dir "$BUILD_ROOT"
    python3 -m venv --upgrade-deps "$venv" &&
        "$venv/bin/pip" install --quiet -r "$PROJECT_ROOT/scripts/requirements.txt" &&
        echo "run-all-checks: installed the lint tools in $venv"
    exit $?
fi

# With no scope, run everything.
if [[ ${#WANT[@]} -eq 0 ]]; then
    want build docs-build "${TEXT_CHECKS[@]}"
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
    declare -A PIDS=()
    for c in "${SELECTED[@]}"; do
        "check_$c" > "$LOG_DIR/$c.log" 2>&1 &
        PIDS[$c]=$!
    done
    for c in "${SELECTED[@]}"; do
        if wait "${PIDS[$c]}"; then
            STATUS[$c]=0
        else
            STATUS[$c]=1
        fi
    done
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
