# shellcheck shell=bash
#
# Helpers shared by the check scripts. Source this file; do not run it.
#
# What the checks read in this repository is set in scripts/checks.conf, which
# load_checks_conf reads.

# Python tools (codespell above all) print the Latin-1 web pages in doc/ as
# text; in a C locale that would fail part way through a run. UTF-8 mode makes
# their input and output UTF-8 whatever the locale.
export PYTHONUTF8=1

# The repository root, whatever the caller's working directory.
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Scratch space for builds, the regression run, and the lint virtual
# environment. It ignores itself (see ensure_build_dir), so nothing in it can
# be committed, and .gitignore does not need an entry for it.
BUILD_ROOT="${BUILD_ROOT:-$PROJECT_ROOT/.build}"

# Read scripts/checks.conf, and fail if it does not say which repository this
# is: every script needs to know.
load_checks_conf() {
    local conf="$PROJECT_ROOT/scripts/checks.conf"
    if [[ ! -f "$conf" ]]; then
        echo "error: $conf is missing" >&2
        return 1
    fi
    # shellcheck source=checks.conf
    source "$conf"
    case "${REPO_KIND:-}" in
        sd | sd-regression) ;;
        *)
            echo "error: REPO_KIND in $conf must be sd or sd-regression" >&2
            return 1
            ;;
    esac
}

# list_files PATHSPEC...
# Print the files that match the git pathspecs, one per line, relative to the
# repository root: tracked files plus new files that are not ignored. Build
# output in the tree is ignored, so it never matches.
list_files() {
    [[ $# -eq 0 ]] && return 0
    # A pathspec naming a directory that does not exist yet is not an error.
    git -C "$PROJECT_ROOT" ls-files --cached --others --exclude-standard -- "$@" 2> /dev/null |
        sort -u |
        while IFS= read -r f; do
            # A file deleted in the working tree is still in the index.
            [[ -f "$PROJECT_ROOT/$f" ]] && printf '%s\n' "$f"
        done
}

# ensure_build_dir DIR
# Create DIR, and BUILD_ROOT with a .gitignore that makes git ignore it. DIR
# is usually under BUILD_ROOT, but an --out option can put it elsewhere.
ensure_build_dir() {
    mkdir -p "$BUILD_ROOT" "$1"
    if [[ ! -f "$BUILD_ROOT/.gitignore" ]]; then
        printf '# Created by scripts/lib.sh: everything here is build output.\n*\n' \
            > "$BUILD_ROOT/.gitignore"
    fi
}

# activate_venv
# Put the lint tools' virtual environment first on PATH. VENV (or VENV_PATH)
# names it; the default is .build/venv, which `scripts/run-all-checks.sh
# --setup` creates. In CI the tools are installed directly, and no virtual
# environment is needed: set VENV=none.
activate_venv() {
    local venv="${VENV:-${VENV_PATH:-$BUILD_ROOT/venv}}"
    [[ "$venv" == none ]] && return 0
    if [[ -x "$venv/bin/python" ]]; then
        PATH="$venv/bin:$PATH"
        export PATH VIRTUAL_ENV="$venv"
    fi
}

# require_tool NAME HINT
# Fail with a hint when a tool is not on PATH.
require_tool() {
    if ! command -v "$1" > /dev/null 2>&1; then
        echo "error: $1 is not installed. $2" >&2
        return 1
    fi
}
