#!/usr/bin/env bash
#
# Check spelling with codespell, against a baseline of upstream lines.
#
# codespell reads its settings from .codespellrc. This script chooses the files
# (SPELL_FILES in scripts/checks.conf) and keeps the baseline: the manuals are
# upstream text, and the lines that were misspelled before this check existed
# are listed, whole, in scripts/codespell-baseline.txt, which codespell skips.
# A misspelling on any other line fails the check -- including one added to a
# line that was in the baseline, since the edited line no longer matches.
#
# Usage:
#   scripts/check-spelling.sh                    # check every configured file
#   scripts/check-spelling.sh FILE...            # check only the named files
#   scripts/check-spelling.sh --update-baseline  # accept every current finding

set -euo pipefail

# shellcheck source=lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
load_checks_conf
activate_venv
require_tool codespell "Install the tools in scripts/requirements.txt."

cd "$PROJECT_ROOT"
BASELINE=scripts/codespell-baseline.txt

UPDATE_BASELINE=false
FILES=()
for arg in "$@"; do
    case "$arg" in
        --update-baseline) UPDATE_BASELINE=true ;;
        -h | --help)
            sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        -*)
            echo "error: unknown option $arg" >&2
            exit 2
            ;;
        *) FILES+=("$arg") ;;
    esac
done

if [[ ${#FILES[@]} -eq 0 ]]; then
    mapfile -t FILES < <(list_files "${SPELL_FILES[@]}")
fi
if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "check-spelling: no files to check"
    exit 0
fi

# codespell fails when its exclude-file is missing.
[[ -f "$BASELINE" ]] || : > "$BASELINE"

if [[ "$UPDATE_BASELINE" == true ]]; then
    # Find every misspelled line with no baseline in force, and record each
    # line exactly as codespell will compare it: without trailing whitespace.
    findings=$(codespell --exclude-file /dev/null "${FILES[@]}" || true)
    printf '%s\n' "$findings" | python3 -c '
import sys


def read_lines(path):
    # Decode as codespell does: UTF-8, falling back to Latin-1.
    with open(path, "rb") as f:
        data = f.read()
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError:
        text = data.decode("iso-8859-1")
    return text.splitlines()


lines = set()
for raw in sys.stdin:
    parts = raw.split(":", 2)
    if len(parts) < 3 or not parts[1].isdigit():
        continue
    lines.add(read_lines(parts[0])[int(parts[1]) - 1].rstrip())
# codespell reads the baseline as UTF-8, whatever the locale.
sys.stdout.buffer.write("".join(line + "\n" for line in sorted(lines)).encode("utf-8"))
' > "$BASELINE"
    echo "check-spelling: recorded $(wc -l < "$BASELINE") line(s) in $BASELINE"
fi

echo "check-spelling: ${#FILES[@]} file(s)"
if ! codespell "${FILES[@]}"; then
    echo "check-spelling: FAILED -- fix the spelling. If the word is right, end the line" >&2
    echo "with a 'codespell:ignore <word>' comment, or add the word to ignore-words-list" >&2
    echo "in .codespellrc with a comment saying why." >&2
    exit 1
fi
echo "check-spelling: passed"
