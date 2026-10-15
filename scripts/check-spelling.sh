#!/usr/bin/env bash
#
# Check spelling with codespell, against a baseline of upstream lines.
#
# codespell reads its settings from .codespellrc. This script chooses the files
# and keeps the baseline. It checks two kinds of file, both set in
# scripts/checks.conf:
#
#   SPELL_FILES          every word: the documentation and the tooling;
#   COMMENT_SPELL_FILES  the comments only: the C++ sources. The code and its
#                        strings are not checked. Each file is copied to
#                        .build/spelling with everything but its comments
#                        blanked, line for line, and codespell reads the copy;
#                        its findings are reported against the source.
#
# The manuals are upstream text, and the lines that were misspelled before this
# check existed are listed, whole, in scripts/codespell-baseline.txt, which
# codespell skips. (For a source file, the line is listed as the copy has it:
# the comment, with any code before it blanked.) A misspelling on any other line
# fails the check -- including one added to a line that was in the baseline,
# since the edited line no longer matches.
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
COMMENT_DIR="$BUILD_ROOT/spelling"

UPDATE_BASELINE=false
ARGS=()
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
        *) ARGS+=("$arg") ;;
    esac
done

# The files whose comments alone are checked.
declare -A IS_COMMENT_FILE=()
while IFS= read -r f; do
    IS_COMMENT_FILE[$f]=1
done < <(list_files "${COMMENT_SPELL_FILES[@]}")

FILES=()
COMMENT_FILES=()
if [[ ${#ARGS[@]} -eq 0 ]]; then
    mapfile -t FILES < <(list_files "${SPELL_FILES[@]}")
    mapfile -t COMMENT_FILES < <(list_files "${COMMENT_SPELL_FILES[@]}")
else
    for f in "${ARGS[@]}"; do
        if [[ -n "${IS_COMMENT_FILE[$f]:-}" ]]; then
            COMMENT_FILES+=("$f")
        else
            FILES+=("$f")
        fi
    done
fi
if [[ $((${#FILES[@]} + ${#COMMENT_FILES[@]})) -eq 0 ]]; then
    echo "check-spelling: no files to check"
    exit 0
fi

# Copy each source file's comments to COMMENT_DIR, at the same relative path.
COPIES=()
if [[ ${#COMMENT_FILES[@]} -gt 0 ]]; then
    rm -rf "$COMMENT_DIR"
    ensure_build_dir "$COMMENT_DIR"
    python3 - "$COMMENT_DIR" "${COMMENT_FILES[@]}" << 'EOF'
import sys
from pathlib import Path


def comments_only(text):
    """Blank everything in C++ source text but its comments, line for line.

    The comment delimiters are kept, so a line that is all comment is copied
    unchanged. String and character literals are blanked, and a comment marker
    inside one is not a comment.
    """
    out = []
    i, n = 0, len(text)
    state = None  # None (code), "line", "block", or the quote of a literal
    while i < n:
        c = text[i]
        if c == "\n":
            out.append(c)
            if state == "line":
                state = None
            i += 1
        elif state is None:
            if text.startswith("//", i) or text.startswith("/*", i):
                state = "line" if text[i + 1] == "/" else "block"
                out.append(text[i:i + 2])
                i += 2
            else:
                if c in "\"'":
                    state = c
                out.append("\t" if c == "\t" else " ")
                i += 1
        elif state == "line":
            out.append(c)
            i += 1
        elif state == "block":
            if text.startswith("*/", i):
                state = None
                out.append("*/")
                i += 2
            else:
                out.append(c)
                i += 1
        else:
            # Inside a literal: skip an escaped character, end at the quote.
            if c == "\\" and i + 1 < n and text[i + 1] != "\n":
                out.append("  ")
                i += 2
                continue
            if c == state:
                state = None
            out.append(" ")
            i += 1
    return "".join(out)


out_dir = Path(sys.argv[1])
for name in sys.argv[2:]:
    data = Path(name).read_bytes()
    # Decode as codespell does: UTF-8, falling back to Latin-1.
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError:
        text = data.decode("iso-8859-1")
    copy = out_dir / name
    copy.parent.mkdir(parents=True, exist_ok=True)
    copy.write_text(comments_only(text), encoding="utf-8")
EOF
    for f in "${COMMENT_FILES[@]}"; do
        COPIES+=("$COMMENT_DIR/$f")
    done
fi

# Report a finding in a copy against its source file.
to_sources() {
    sed "s|^$COMMENT_DIR/||"
}

# codespell fails when its exclude-file is missing.
[[ -f "$BASELINE" ]] || : > "$BASELINE"

if [[ "$UPDATE_BASELINE" == true ]]; then
    # Find every misspelled line with no baseline in force, and record each
    # line exactly as codespell will compare it: without trailing whitespace.
    findings=$(codespell --exclude-file /dev/null "${FILES[@]}" "${COPIES[@]}" || true)
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

echo "check-spelling: ${#FILES[@]} file(s), and the comments in ${#COMMENT_FILES[@]} source file(s)"
if ! codespell "${FILES[@]}" "${COPIES[@]}" | to_sources; then
    echo "check-spelling: FAILED -- fix the spelling. If the word is right, end the line" >&2
    echo "with a 'codespell:ignore <word>' comment, or add the word to ignore-words-list" >&2
    echo "in .codespellrc with a comment saying why." >&2
    exit 1
fi
echo "check-spelling: passed"
