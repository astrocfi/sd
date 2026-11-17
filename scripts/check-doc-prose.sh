#!/usr/bin/env bash
#
# Check the documentation prose against .claude/rules/doc_prose_style.md.
#
# Two tiers:
#   ERRORS   -- high-confidence violations. A new one fails the script.
#   WARNINGS -- heuristics that need a human to judge. Reported, never fatal.
#
# The manuals are upstream text, and many of their lines predate these rules.
# Every error that exists in them is recorded in scripts/doc-prose-baseline.txt,
# keyed by file, check, and line text (not line number), and is not reported
# again. An error that is not in the baseline fails the script.
#
# Usage:
#   scripts/check-doc-prose.sh                    # check every documentation file
#   scripts/check-doc-prose.sh FILE...            # check only the named files
#   scripts/check-doc-prose.sh --warnings         # include the heuristic tier
#   scripts/check-doc-prose.sh --update-baseline  # accept every current error
#
# The files checked are set in scripts/checks.conf. The rule that defines each
# check, and the review checklist no script can run, is
# .claude/rules/doc_prose_style.md.

set -uo pipefail

# shellcheck source=lib.sh
if ! source "$(dirname "$0")/lib.sh" || ! load_checks_conf; then
    echo 'check-doc-prose: FAILED -- cannot load scripts/lib.sh and scripts/checks.conf' >&2
    exit 1
fi

# Without errexit a failed cd would carry on and check the caller's directory,
# or find nothing and report success.
if ! cd "$PROJECT_ROOT"; then
    echo 'check-doc-prose: FAILED -- cannot reach the repository root' >&2
    exit 1
fi

if [[ ! -f .claude/rules/doc_prose_style.md ]]; then
    echo 'check-doc-prose: FAILED -- .claude/rules/doc_prose_style.md is missing' >&2
    exit 1
fi

BASELINE=scripts/doc-prose-baseline.txt
SHOW_WARNINGS=0
UPDATE_BASELINE=0
EXPLICIT=0
FILES=()
for arg in "$@"; do
    case "$arg" in
        --warnings | -w) SHOW_WARNINGS=1 ;;
        --update-baseline) UPDATE_BASELINE=1 ;;
        -h | --help)
            sed -n '3,/^$/p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        -*)
            echo "check-doc-prose: FAILED -- unknown option $arg" >&2
            exit 2
            ;;
        *)
            FILES+=("$arg")
            EXPLICIT=1
            ;;
    esac
done

if [[ $UPDATE_BASELINE -eq 1 && $EXPLICIT -eq 1 ]]; then
    echo 'check-doc-prose: FAILED -- --update-baseline checks every file; name none' >&2
    exit 2
fi

if [[ ${#FILES[@]} -eq 0 ]]; then
    if ! found=$(list_files "${TEXINFO_FILES[@]}" "${PROSE_MARKDOWN_FILES[@]}"); then
        echo 'check-doc-prose: FAILED -- could not list the documentation files' >&2
        exit 1
    fi
    if [[ -n "$found" ]]; then
        mapfile -t FILES <<< "$found"
    fi
fi

# A user reads the manuals and the README; the developer documentation and the
# GitHub templates are for contributors. The audience-separation checks read
# only what a user reads.
user_facing() {
    case "$1" in
        doc/*.txi | README.md) return 0 ;;
        *) return 1 ;;
    esac
}

ERRORS=0
WARNINGS=0

# Every error found, keyed as in the baseline, whether or not the baseline
# knows it: --update-baseline writes this, and the stale-entry report reads it.
KEYS_FILE=$(mktemp)
trap 'rm -f "$KEYS_FILE"' EXIT

# The baseline holds "check<TAB>path<TAB>text" lines.
KNOWN=""
if [[ -f "$BASELINE" ]]; then
    KNOWN=$(grep -v '^#' "$BASELINE")
fi

# report TIER ID MESSAGE MATCHES  -- MATCHES holds "path:line:text" lines.
# The matches arrive as an argument rather than on stdin: a pipeline would run
# this function in a subshell and the counters would not survive it.
report() {
    local tier="$1" id="$2" msg="$3" matches="$4"
    [[ "$matches" != *[![:space:]]* ]] && return 0
    if [[ "$tier" == ERROR ]]; then
        # Key each match by check, path, and text -- not line number -- record
        # the key, and keep only the matches the baseline does not know.
        local fresh
        fresh=$(printf '%s\n' "$matches" | ID="$id" KNOWN="$KNOWN" KEYS="$KEYS_FILE" python3 -c '
import os
import sys

check = os.environ["ID"]
known = set(os.environ["KNOWN"].splitlines())
with open(os.environ["KEYS"], "a", encoding="utf-8") as keys:
    for raw in sys.stdin:
        parts = raw.rstrip("\n").split(":", 2)
        if len(parts) != 3:
            continue
        path, _line, text = parts
        key = f"{check}\t{path}\t{text}"
        keys.write(key + "\n")
        if key not in known:
            print(raw.rstrip("\n"))
')
        [[ "$fresh" != *[![:space:]]* ]] && return 0
        matches="$fresh"
    fi
    [[ $UPDATE_BASELINE -eq 1 ]] && return 0
    printf '\n%s [%s]: %s\n' "$tier" "$id" "$msg"
    printf '%s\n' "$matches" | sed 's/^/  /'
    if [[ "$tier" == ERROR ]]; then
        ERRORS=$((ERRORS + 1))
    else
        WARNINGS=$((WARNINGS + 1))
    fi
}

# prose_stream FILE...
# Emit "path:line:text" for the lines of prose only. Code, examples, and
# comments are evidence or markup, not prose, so the checks must not read them.
# In Texinfo, the arguments of the literal commands (@code, @samp, @kbd, @file,
# and their kind) and of the cross-reference commands are replaced by "X" or
# "x", matching the case of their first letter, and
# the other brace commands (@emph, @strong, ...) are reduced to their text.
prose_stream() {
    [[ $# -eq 0 ]] && return 0
    # The program is Python in single quotes; nothing in it is for the shell.
    # shellcheck disable=SC2016
    python3 -c '
import re
import sys

failed = []

# Texinfo environments whose content is not prose.
TEXI_SKIP_BLOCKS = {
    "example", "smallexample", "lisp", "smalllisp", "display", "smalldisplay",
    "format", "smallformat", "verbatim", "ignore", "tex", "html", "iftex",
    "macro", "menu", "titlepage", "copying", "direntry", "docbook", "latex",
    "xml",
}
# Brace commands whose argument is literal text or a node name.
TEXI_LITERAL = {
    "code", "samp", "kbd", "key", "file", "var", "command", "env", "option",
    "url", "uref", "email", "xref", "pxref", "ref", "inforef", "verb", "t",
    "value", "w", "image", "footnote", "anchor", "acronym", "abbr",
}
# Line commands with no prose on the line.
TEXI_LINE_COMMANDS = re.compile(
    r"@(c|comment|node|set|clear|setfilename|settitle|sp|page|need|vskip|"
    r"headings|oddheading|evenheading|oddfooting|evenfooting|everyheading|"
    r"everyfooting|thispage|thisfile|finalout|setchapternewpage|"
    r"paragraphindent|exampleindent|footnotestyle|input|include|bye|top|"
    r"printindex|[cfkptv]index|contents|shortcontents|summarycontents|"
    r"end|noindent|center|titlefont|tab|columnfractions|dircategory|"
    r"defindex|defcodeindex|syncodeindex|synindex|documentencoding|"
    r"documentlanguage|frenchspacing|allowcodebreaks|refill|group|"
    r"cartouche|quotation|itemize|enumerate|table|ftable|vtable|"
    r"multitable|ifset|ifclear|ifinfo|ifnotinfo|ifhtml|ifnothtml|"
    r"ifnottex|ifplaintext|ifnotplaintext|ifdocbook|ifnotdocbook|"
    r"ifxml|ifnotxml|iflatex|ifnotlatex|ifcommanddefined|"
    r"ifcommandnotdefined)\b"
)


def texi_inline(text):
    """Reduce Texinfo inline markup to plain text."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        ch = text[i]
        if ch == "@" and i + 1 < n:
            nxt = text[i + 1]
            if nxt in "@{}":
                out.append(nxt)
                i += 2
                continue
            if not (nxt.isalpha()):
                # @. @: @! @? @* @- @, @" and the like.
                if nxt in ".!?":
                    out.append(nxt)
                i += 2
                continue
            m = re.match(r"@([A-Za-z]+)", text[i:])
            name = m.group(1)
            j = i + len(m.group(0))
            if j < n and text[j] == "{":
                depth = 0
                k = j
                while k < n:
                    if text[k] == "@" and k + 1 < n:
                        k += 2
                        continue
                    if text[k] == "{":
                        depth += 1
                    elif text[k] == "}":
                        depth -= 1
                        if depth == 0:
                            break
                    k += 1
                inner = text[j + 1:k]
                if name in TEXI_LITERAL:
                    # Keep the case of the first letter, which the
                    # sentence-spacing check reads.
                    first = re.search(r"[A-Za-z]", inner)
                    out.append("X" if first and first.group().isupper() else "x")
                elif name in ("dots", "enddots", "bullet", "minus", "copyright",
                              "registeredsymbol", "result", "expansion",
                              "print", "error", "equiv", "point", "tie"):
                    out.append("...")
                else:
                    out.append(texi_inline(inner))
                i = k + 1
                continue
            # A command without braces, such as @item or @strong at the end of
            # a line: drop the command and keep the rest.
            out.append(" ")
            i = j
            continue
        out.append(ch)
        i += 1
    return "".join(out)


def texi(path, lines):
    stack = []
    for n, line in enumerate(lines, 1):
        stripped = line.strip()
        m = re.match(r"@end\s+(\w+)", stripped)
        if m:
            if stack and stack[-1] == m.group(1):
                stack.pop()
            continue
        m = re.match(r"@(\w+)\b", stripped)
        if m and m.group(1) in TEXI_SKIP_BLOCKS:
            # @iftex and its kind open a block; so does every skipped
            # environment. A one-line "@ignore ... @end ignore" does not occur.
            stack.append(m.group(1))
            continue
        if stack:
            continue
        if not stripped or stripped.startswith("\\input"):
            continue
        if TEXI_LINE_COMMANDS.match(stripped):
            continue
        # Section titles and @item lines are prose; drop the command.
        text = re.sub(r"^@(chapter|section|subsection|subsubsection|unnumbered|"
                      r"unnumberedsec|unnumberedsubsec|appendix|appendixsec|"
                      r"appendixsubsec|heading|subheading|subsubheading|"
                      r"majorheading|chapheading|item|itemx|title|subtitle|"
                      r"author)\b\s*", "", stripped)
        text = texi_inline(text)
        # A trailing @c comment is not prose.
        text = re.sub(r"\s@c(omment)?\b.*$", "", text)
        print(f"{path}:{n}:{text}")


def markdown(path, lines):
    fence = None
    front = lines[:1] == ["---"]
    comment = False
    for n, line in enumerate(lines, 1):
        stripped = line.strip()
        if front:
            # YAML front matter, as in a skill or an issue template.
            if n > 1 and stripped == "---":
                front = False
            continue
        # CommonMark allows ``` and ~~~ fences; a fence closes only on a run of
        # the same character at least as long, and a run indented by four or
        # more spaces is code-block content rather than a fence.
        indent = len(line) - len(line.lstrip())
        m = re.match(r"(`{3,}|~{3,})", stripped) if indent < 4 else None
        if m and fence is None:
            fence = m.group(1)
            continue
        if fence and m and re.fullmatch(re.escape(fence[0]) + "{%d,}" % len(fence), stripped):
            fence = None
            continue
        if fence:
            continue
        if line.startswith("    ") and stripped and not re.match(r"[-*+]|\d+\.", stripped):
            continue
        # HTML comments, which may span lines.
        text = line
        if comment:
            if "-->" not in text:
                continue
            text = text.split("-->", 1)[1]
            comment = False
        text = re.sub(r"<!--.*?-->", " ", text)
        if "<!--" in text:
            text = text.split("<!--", 1)[0]
            comment = True
        # Inline code and URLs are not prose.
        text = re.sub(r"`[^`]*`", "X", text)
        text = re.sub(r"\]\([^)]*\)", "]", text)
        text = re.sub(r"<?https?://\S+>?", "X", text)
        if not text.strip():
            continue
        print(f"{path}:{n}:{text}")


for path in sys.argv[1:]:
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            lines = f.read().replace("\r\n", "\n").split("\n")
    except OSError as exc:
        failed.append(f"{path}: {exc}")
        continue
    if path.endswith(".txi"):
        texi(path, lines)
    else:
        markdown(path, lines)

if failed:
    for entry in failed:
        print(f"check-doc-prose: cannot read {entry}", file=sys.stderr)
    raise SystemExit(1)
' "$@"
}

# hits PATTERN STREAM  -- PCRE, case sensitive, over a prose stream
hits() {
    local pat="$1" stream="$2"
    [[ -z "$stream" ]] && return 0
    printf '%s\n' "$stream" | grep -P -- "$pat" 2> /dev/null
    return 0
}

ALL_FILES=()
USER_FILES=()
TEXI_FILES=()
MD_FILES=()
DEV_FILES=()
missing=()
for f in "${FILES[@]}"; do
    if [[ ! -f "$f" ]]; then
        missing+=("$f")
        continue
    fi
    # A path spelled ./doc/... or /abs/path/doc/... names the same file as
    # doc/..., and the audience rules and the baseline key on the path.
    if abs=$(cd -- "$(dirname -- "$f")" && pwd -P); then
        rel="${abs#"$(pwd -P)"}"
        rel="${rel#/}"
        f="${rel:+$rel/}$(basename -- "$f")"
    fi
    ALL_FILES+=("$f")
    case "$f" in
        *.txi) TEXI_FILES+=("$f") ;;
        *) MD_FILES+=("$f") ;;
    esac
    if user_facing "$f"; then
        USER_FILES+=("$f")
    else
        DEV_FILES+=("$f")
    fi
done

# Silently dropping a path a caller named turns a typo into a clean pass.
if [[ ${#missing[@]} -gt 0 && $EXPLICIT -eq 1 ]]; then
    printf 'check-doc-prose: FAILED -- no such file: %s\n' "${missing[@]}" >&2
    exit 1
fi

if [[ ${#ALL_FILES[@]} -eq 0 ]]; then
    echo 'check-doc-prose: no documentation files to check'
    exit 0
fi

echo "check-doc-prose: ${#ALL_FILES[@]} file(s), ${#USER_FILES[@]} user-facing"

# A filter that fails would hand every check an empty stream, and every check
# would pass. Its exit status is therefore load-bearing.
# out names the caller's variable, which is assigned through it.
# shellcheck disable=SC2034
stream_of() {
    local -n out="$1"
    shift
    [[ $# -eq 0 ]] && return 0
    if ! out="$(prose_stream "$@")"; then
        echo 'check-doc-prose: FAILED -- could not read the documentation prose' >&2
        exit 1
    fi
}
PROSE_ALL=""
PROSE_USER=""
PROSE_TEXI=""
PROSE_MD=""
PROSE_DEV=""
stream_of PROSE_ALL "${ALL_FILES[@]}"
stream_of PROSE_USER "${USER_FILES[@]}"
stream_of PROSE_TEXI "${TEXI_FILES[@]}"
stream_of PROSE_MD "${MD_FILES[@]}"
stream_of PROSE_DEV "${DEV_FILES[@]}"
if [[ "$PROSE_ALL" != *[![:space:]]* ]]; then
    echo 'check-doc-prose: FAILED -- the prose stream is empty' >&2
    exit 1
fi

# The release notes record history, so the time-anchoring check skips them.
PROSE_CURRENT=$(printf '%s\n' "$PROSE_ALL" | grep -v -E '^doc/relnotes[0-9]*\.txi:')

# ------------------------------------------------------------------ errors ----

# "nowhere near" is ordinary English; the rule is about doing something nowhere.
report ERROR nowhere 'you cannot do something "nowhere" -- say it is not done anywhere' \
    "$(hits '\bnowhere\b(?! near)' "$PROSE_ALL")"

# Texinfo and TeX treat a period followed by two spaces as the end of a
# sentence, and the manuals put two spaces there. An initial ("William B.
# Ackerman") and an abbreviation take one space and are excluded.
report ERROR texi-sentence-space 'Texinfo: end a sentence with two spaces' \
    "$(hits '[a-z0-9)][.?!](?<!\be\.g\.)(?<!\bi\.e\.)(?<!\betc\.)(?<!\bvs\.)(?<!\bMr\.)(?<!\bMrs\.)(?<!\bMs\.)(?<!\bDr\.)(?<!\bSt\.)(?<!\bNo\.)(?<!\bVol\.)(?<!\bcf\.) (?=[A-Z])' "$PROSE_TEXI")"

# Markdown renders any run of spaces as one; the convention there is one.
report ERROR md-sentence-space 'Markdown: end a sentence with one space' \
    "$(hits '[a-z0-9)][.?!]  +[A-Z]' "$PROSE_MD")"

if [[ ${#USER_FILES[@]} -gt 0 ]]; then
    # A user does not read the source; the files and functions of the engine
    # mean nothing to one.
    report ERROR internal-name 'source file or internal function name in user-facing prose' \
        "$(hits '\b(sd\w*|mkcalls|mapcachefile|common|database|paths)\.(cpp|h)\b|\b(toplevelmove|do_big_concept|basic_move|parse_block|setup_kind|iobase|iofull|ui_utils|matcher_class|run_program|open_session|calldefn|callarray|personrec)\b' "$PROSE_USER")"
fi

if [[ "$REPO_KIND" == sd && ${#DEV_FILES[@]} -gt 0 ]]; then
    # The sd repository is public; the regression suite is not. Its files and
    # their contents are named by test, never quoted, so a path into it, or a
    # reference transcript's name, is the mark of a quotation.
    report ERROR regression-content 'sd is public: do not quote the private regression suite' \
        "$(hits '\bregression_tests/[a-z]+[0-9]+t\b|\b[a-z]+[0-9]+t\.(ref|tmp)\b' "$PROSE_DEV")"
fi

# A negation goes on the verb, not on its object: "doesn't accept options",
# not "accepts no options". Idioms such as "has no effect" and "do nothing" are
# good English and are not flagged. scripts/wording.py finds a verb from its
# list followed by "no", "nothing", "nobody", "none", or "no one", and appends a
# suggested rewrite in brackets where the verb form makes one simple.
if ! NEGATIVE_OBJECTS=$(printf '%s\n' "$PROSE_ALL" | python3 "$PROJECT_ROOT/scripts/wording.py"); then
    echo 'check-doc-prose: FAILED -- scripts/wording.py did not run' >&2
    exit 1
fi
report ERROR negative-object 'put the negation on the verb -- "does not accept options", not "accepts no options"' \
    "$NEGATIVE_OBJECTS"

# ---------------------------------------------------------------- warnings ----

if [[ $SHOW_WARNINGS -eq 1 ]]; then
    # "the gate" is part of call names (fan the gate, push open the gate).
    report WARNING gate 'name what is being tested rather than calling it a gate' \
        "$(hits '(?<!the )\bgat(e|es|ed|ing)\b' "$PROSE_ALL")"

    report WARNING oxford-comma 'possible missing Oxford comma -- check whether this is a list of three' \
        "$(hits ',\s+[^,]{3,60}\s+(and|or)\s+\w' "$PROSE_ALL" | grep -vP ',\s+(and|or)\s')"

    report WARNING x-not-y 'X-not-Y: keep only if a reader would genuinely expect Y' \
        "$(hits ',\s+not\s+\w' "$PROSE_ALL")"

    report WARNING so-clause 'a clause after "so" that restates the obvious should be deleted' \
        "$(hits '\bso (what|that means|you|it) \w+' "$PROSE_ALL")"

    # The manuals quote with ``this'', which TeX and makeinfo turn into curly
    # quotes; a straight double quote prints as a straight quote in every
    # format. The FAQ and the release notes quote with straight quotes
    # throughout, so this is a warning rather than an error.
    report WARNING texi-straight-quote "Texinfo: quote with \`\`this'' -- not with straight double quotes" \
        "$(hits '"' "$PROSE_TEXI")"

    report WARNING time-anchor 'describe the current state -- no "recently", "currently", "no longer", or "legacy"' \
        "$(hits '(?i)\b(recently|currently|nowadays|legacy|no longer|as of now)\b' "$PROSE_CURRENT")"

    if [[ ${#USER_FILES[@]} -gt 0 ]]; then
        report WARNING level-case 'a level is written in capitals in prose (C4, A2) -- lower case only as typed, in markup' \
            "$(hits '(?<![\w@{/-])(c[1-4][abx]?|a[12])(?![\w}])' "$PROSE_USER")"

        report WARNING program-name 'the programs are Sd and Sdtty -- not SD or SDTTY' \
            "$(hits '\b(SD|SDTTY|SDtty)\b' "$PROSE_USER")"
    fi
fi

# ----------------------------------------------------------------- summary ----

if [[ $UPDATE_BASELINE -eq 1 ]]; then
    {
        echo "# Errors from scripts/check-doc-prose.sh that predate the check, and are accepted."
        echo "# Each line is: check<TAB>file<TAB>prose text. Regenerate with:"
        echo "#     scripts/check-doc-prose.sh --update-baseline"
        LC_ALL=C sort -u "$KEYS_FILE"
    } > "$BASELINE"
    echo "check-doc-prose: recorded $(grep -c -v '^#' "$BASELINE") error(s) in $BASELINE"
    exit 0
fi

if [[ -f "$BASELINE" && $EXPLICIT -eq 0 ]]; then
    stale=$(printf '%s\n' "$KNOWN" | grep -v '^$' | LC_ALL=C sort -u |
        LC_ALL=C comm -23 - <(LC_ALL=C sort -u "$KEYS_FILE"))
    if [[ -n "$stale" ]]; then
        echo
        echo "check-doc-prose: $(printf '%s\n' "$stale" | wc -l) baseline error(s) no longer occur;"
        echo "check-doc-prose: run with --update-baseline to drop them"
    fi
fi

plural() { [[ "$1" -eq 1 ]] && echo category || echo categories; }

echo
if [[ $ERRORS -gt 0 ]]; then
    echo "check-doc-prose: FAILED -- $ERRORS error $(plural $ERRORS)"
    [[ $WARNINGS -gt 0 ]] && echo "check-doc-prose: $WARNINGS warning $(plural $WARNINGS)"
    exit 1
fi

if [[ $WARNINGS -gt 0 ]]; then
    echo "check-doc-prose: passed with $WARNINGS warning $(plural $WARNINGS) to review by hand"
else
    echo 'check-doc-prose: passed'
    [[ $SHOW_WARNINGS -eq 0 ]] && echo 'check-doc-prose: rerun with --warnings for the heuristic tier'
fi
exit 0
