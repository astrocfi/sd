#!/usr/bin/env bash
#
# Build the Sd manuals out of the source tree and check the build's warnings.
#
# The manuals in doc/ are Texinfo, built by doc/makefile. This script copies the
# documentation sources into .build/docs/src, runs that makefile there, and so
# leaves doc/ untouched. It then compares the warnings that makeinfo and TeX
# print with scripts/doc-warnings-baseline.txt: a warning in the baseline is
# known and accepted, and any other warning fails the build. Warnings are
# compared without their line numbers, so an edit elsewhere in a manual does
# not disturb the baseline.
#
# Usage:
#   scripts/build-docs.sh [options]
#
# Options:
#   -f, --formats LIST   Comma-separated formats to build (default: all):
#                        html, txt, info, pdf, ps, install
#   -o, --out DIR        Build directory (default: .build/docs)
#   --update-baseline    Record the current warnings as the baseline
#   -h, --help           Show this help
#
# The built manuals are collected in <out>/output.
#
# Requirements: make, a C compiler (for the ps format), makeinfo (texinfo), and
# TeX with texinfo.tex for the pdf and ps formats (texlive-base on Debian and
# Ubuntu).

set -euo pipefail

# shellcheck source=lib.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"
load_checks_conf

usage() {
    sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

ALL_FORMATS="html,txt,info,pdf,ps,install"
FORMATS="$ALL_FORMATS"
OUT="$BUILD_ROOT/docs"
UPDATE_BASELINE=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        -f | --formats)
            FORMATS="${2:?--formats needs a value}"
            shift 2
            ;;
        -o | --out)
            OUT="${2:?--out needs a value}"
            shift 2
            ;;
        --update-baseline)
            UPDATE_BASELINE=true
            shift
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

if [[ "$UPDATE_BASELINE" == true && "$FORMATS" != "$ALL_FORMATS" ]]; then
    echo "error: --update-baseline builds every format; do not combine it with --formats" >&2
    exit 2
fi

BASELINE="$PROJECT_ROOT/scripts/doc-warnings-baseline.txt"

# Map each format to its doc/makefile target.
TARGETS=()
NEED_TEX=false
NEED_CRFIX=false
IFS=',' read -r -a FORMAT_LIST <<< "$FORMATS"
for format in "${FORMAT_LIST[@]}"; do
    case "$format" in
        html) TARGETS+=(html.all) ;;
        txt) TARGETS+=(text.all) ;;
        info) TARGETS+=(info.all) ;;
        pdf)
            TARGETS+=(pdf.all)
            NEED_TEX=true
            ;;
        ps)
            TARGETS+=(ps.all)
            NEED_TEX=true
            NEED_CRFIX=true
            ;;
        install) TARGETS+=(install) ;;
        *)
            echo "error: unknown format '$format'" >&2
            exit 2
            ;;
    esac
done

hint="Install texinfo, and texlive-base for the pdf and ps formats."
require_tool make "$hint"
require_tool makeinfo "$hint"
if [[ "$NEED_TEX" == true ]]; then
    require_tool tex "$hint"
    require_tool pdftex "$hint"
fi
if [[ "$NEED_CRFIX" == true ]]; then
    require_tool dvips "$hint"
    require_tool cc "Install a C compiler."
fi

# Copy the documentation sources, and nothing that a build in place may have
# left in doc/.
ensure_build_dir "$OUT"
# The build runs in $OUT/src, so a relative path would no longer name it.
OUT="$(cd "$OUT" && pwd)"
SRC="$OUT/src"
rm -rf "$SRC" "$OUT/output"
mkdir -p "$SRC" "$OUT/output"
mapfile -t doc_files < <(list_files 'doc/*')
if [[ ${#doc_files[@]} -eq 0 ]]; then
    echo "error: no documentation sources found under doc/" >&2
    exit 1
fi
for f in "${doc_files[@]}"; do
    cp "$PROJECT_ROOT/$f" "$SRC/"
done

if [[ "$NEED_CRFIX" == true ]]; then
    cc -O2 -o "$SRC/crfix" "$SRC/crfix.c"
fi

# TeX stops at its first error to ask what to do; nonstopmode makes it report
# the error and exit non-zero instead, which fails the make.
LOG="$OUT/build.log"
echo "build-docs: building ${TARGETS[*]} in $SRC"
if ! (cd "$SRC" && make -k -f makefile "${TARGETS[@]}" \
    TXI2DVI="tex -interaction=nonstopmode" \
    TXI2PDF="pdftex -interaction=nonstopmode" \
    CRFIX=./crfix < /dev/null > "$LOG" 2>&1); then
    echo "build-docs: FAILED -- the build stopped with an error. The end of $LOG:" >&2
    tail -40 "$LOG" >&2
    exit 1
fi

# Collect the warnings that matter, without line numbers:
#   makeinfo:  doc.txi:123: warning: ...  (or doc.txi: warning: ...)
#   TeX:       Undefined cross reference ..., and LaTeX-style warnings
# TeX's overfull and underfull box messages are about line breaking only, so
# they are counted but not gated.
CURRENT="$OUT/warnings.txt"
{
    grep -E '^[^ :]+\.txi(:[0-9]+)?: (warning|error): ' "$LOG" |
        sed -E 's/^([^ :]+\.txi):[0-9]+: /\1: /' || true
    grep -E 'Undefined cross reference|^! ' "$LOG" | sed -E 's/ on input line [0-9]+//' || true
} | LC_ALL=C sort > "$CURRENT"
boxes=$(grep -c -E '^(Overfull|Underfull) ' "$LOG" || true)

# Collect what was built.
shopt -s nullglob
for built in "$SRC"/*.html "$SRC"/*.txt "$SRC"/*.info "$SRC"/*.pdf "$SRC"/*.ps "$SRC"/install; do
    [[ -f "$built" ]] || continue
    # The sources include hand-written HTML and text; collect only what make wrote.
    base=$(basename "$built")
    if [[ " ${doc_files[*]##*/} " != *" $base "* ]]; then
        cp "$built" "$OUT/output/"
    fi
done
shopt -u nullglob

if [[ "$UPDATE_BASELINE" == true ]]; then
    {
        echo "# Warnings from scripts/build-docs.sh that are known and accepted."
        echo "# Regenerate with: scripts/build-docs.sh --update-baseline"
        cat "$CURRENT"
    } > "$BASELINE"
    echo "build-docs: recorded $(wc -l < "$CURRENT") warning(s) in ${BASELINE#"$PROJECT_ROOT"/}"
    exit 0
fi

KNOWN="$OUT/warnings-baseline.txt"
if [[ -f "$BASELINE" ]]; then
    grep -v '^#' "$BASELINE" | LC_ALL=C sort > "$KNOWN" || true
else
    : > "$KNOWN"
fi

new=$(LC_ALL=C comm -23 "$CURRENT" "$KNOWN")
gone=$(LC_ALL=C comm -13 "$CURRENT" "$KNOWN")

echo "build-docs: built $(find "$OUT/output" -type f | wc -l) file(s) into $OUT/output"
echo "build-docs: $(wc -l < "$CURRENT") warning(s), $boxes overfull/underfull box message(s) (not checked)"
# A build of some formats does not produce every warning, so only a build of
# all of them can tell that a baseline warning is gone.
if [[ -n "$gone" && "$FORMATS" == "$ALL_FORMATS" ]]; then
    echo "build-docs: these baseline warnings no longer occur; run with --update-baseline to drop them:"
    printf '%s\n' "$gone" | sed 's/^/  /'
fi
if [[ -n "$new" ]]; then
    echo "build-docs: FAILED -- new warning(s) not in the baseline:" >&2
    printf '%s\n' "$new" | sed 's/^/  /' >&2
    echo "Fix them, or, if one cannot be fixed, run scripts/build-docs.sh --update-baseline" >&2
    echo "and explain the addition in the pull request." >&2
    exit 1
fi
echo "build-docs: passed"
