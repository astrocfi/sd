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
#
# A FILE is relative to the current directory, and must be in the repository.
# --update-baseline checks every configured file, so it takes no FILE.

set -euo pipefail

# shellcheck source=lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
load_checks_conf
activate_venv
require_tool codespell "Install the tools in scripts/requirements.txt."

CALLER_DIR=$PWD
cd "$PROJECT_ROOT"
ROOT=$(pwd -P)
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

if [[ "$UPDATE_BASELINE" == true && ${#ARGS[@]} -gt 0 ]]; then
    echo "error: --update-baseline checks every file; name none" >&2
    exit 2
fi

# Print FILE (relative to the caller's directory) relative to the repository
# root, or fail if it is missing or outside the repository.
repo_path() {
    local path=$1 dir
    [[ "$path" == /* ]] || path="$CALLER_DIR/$path"
    if [[ ! -f "$path" ]]; then
        echo "error: no such file: $1" >&2
        return 1
    fi
    dir=$(cd "$(dirname "$path")" && pwd -P)
    path="$dir/$(basename "$path")"
    if [[ "$path" != "$ROOT"/* ]]; then
        echo "error: not in the repository: $1" >&2
        return 1
    fi
    printf '%s\n' "${path#"$ROOT"/}"
}

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
    for arg in "${ARGS[@]}"; do
        f=$(repo_path "$arg") || exit 2
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

IDENT = frozenset("_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
RAW_PREFIXES = ("R", "LR", "uR", "UR", "u8R")


def word_before(text, i, also=""):
    """Return the identifier or number that ends just before text[i]."""
    j = i
    while j > 0 and (text[j - 1] in IDENT or text[j - 1] in also):
        j -= 1
    return text[j:i]


def continued(text, i):
    """Tell whether the newline at text[i] follows a backslash."""
    j = i - 1
    if j >= 0 and text[j] == "\r":
        j -= 1
    return j >= 0 and text[j] == "\\"


def blank(s):
    """Blank s, keeping its newlines and tabs."""
    return "".join(c if c in "\n\t" else " " for c in s)


def comments_only(text):
    """Blank everything in C++ source text but its comments, line for line.

    The comment delimiters are kept, so a line that is all comment is copied
    unchanged. String and character literals, raw strings among them, are
    blanked, and a comment marker inside one is not a comment. A backslash at
    the end of a line continues a // comment or a literal onto the next line.
    A digit separator (1'000) does not start a character literal, and a
    literal left open at the end of a line ends there, so that one stray quote
    (an apostrophe in an #if 0 block) cannot hide the rest of the file.
    """
    out = []
    i, n = 0, len(text)
    state = None  # None (code), "line", "block", or the quote of a literal
    while i < n:
        c = text[i]
        if c == "\n":
            out.append(c)
            if state in ("line", '"', "'"):
                if not continued(text, i):
                    state = None
            i += 1
        elif state is None:
            if text.startswith("//", i) or text.startswith("/*", i):
                state = "line" if text[i + 1] == "/" else "block"
                out.append(text[i:i + 2])
                i += 2
            elif c == '"' and word_before(text, i) in RAW_PREFIXES:
                # A raw string: R"delim( ... )delim", with no escapes.
                open_paren = text.find("(", i + 1, i + 18)
                if open_paren < 0:
                    state = c
                    out.append(" ")
                    i += 1
                    continue
                close = ")" + text[i + 1:open_paren] + '"'
                end = text.find(close, open_paren + 1)
                end = n if end < 0 else end + len(close)
                out.append(blank(text[i:end]))
                i = end
            elif c == "'" and word_before(text, i, ".'")[:1].isdigit():
                # A digit separator, inside a number.
                out.append(" ")
                i += 1
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
            # A backslash before a newline is left for the newline to see.
            if c == "\\" and i + 1 < n and text[i + 1] not in "\r\n":
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

# Report a finding in a copy against its source file. The prefix is matched
# as text, not as a pattern, since the build directory can be anywhere.
to_sources() {
    PREFIX="$COMMENT_DIR/" awk '
        BEGIN { prefix = ENVIRON["PREFIX"] }
        index($0, prefix) == 1 { $0 = substr($0, length(prefix) + 1) }
        { print }'
}

# codespell fails when its exclude-file is missing.
[[ -f "$BASELINE" ]] || : > "$BASELINE"

if [[ "$UPDATE_BASELINE" == true ]]; then
    # .codespellrc makes the baseline codespell's exclude-file, and a second
    # --exclude-file adds to it rather than replacing it. So empty the baseline
    # while codespell finds every misspelled line, and put it back if anything
    # fails before the new one is written.
    ensure_build_dir "$BUILD_ROOT"
    saved=$(mktemp "$BUILD_ROOT/codespell-baseline.XXXXXX")
    cp "$BASELINE" "$saved"
    trap 'cp "$saved" "$BASELINE"; rm -f "$saved" "$saved.new"' EXIT
    : > "$BASELINE"
    status=0
    findings=$(codespell "${FILES[@]}" "${COPIES[@]}") || status=$?
    # 65: misspellings found. Anything else is codespell failing.
    if [[ $status -ne 0 && $status -ne 65 ]]; then
        echo "error: codespell failed (exit $status); the baseline is unchanged" >&2
        exit 1
    fi
    # Record each line exactly as codespell will compare it: without trailing
    # whitespace.
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
' > "$saved.new"
    cp "$saved.new" "$BASELINE"
    rm -f "$saved" "$saved.new"
    trap - EXIT
    echo "check-spelling: recorded $(wc -l < "$BASELINE") line(s) in $BASELINE"
fi

echo "check-spelling: ${#FILES[@]} file(s), and the comments in ${#COMMENT_FILES[@]} source file(s)"
if ! codespell "${FILES[@]}" "${COPIES[@]}" | to_sources; then
    echo "check-spelling: FAILED -- fix the spelling. If the word is right, end the line" >&2
    echo "with a comment holding 'codespell:ignore <word>' after punctuation, such as" >&2
    echo "'// codespell:ignore <word>' (a bare 'codespell:ignore' for a hyphenated" >&2
    echo "word), or add the word to ignore-words-list in .codespellrc with a comment" >&2
    echo "saying why." >&2
    exit 1
fi
echo "check-spelling: passed"
