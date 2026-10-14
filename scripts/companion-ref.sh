#!/usr/bin/env bash
#
# Choose which ref of the companion repository to test against.
#
# sd and sd-regression are tested together. A change that needs both is made
# on a branch with the same name in each, so CI in one repository checks out
# the other at the first of these refs that exists there:
#
#   1. --override REF, when it is not empty (a ref typed into a manual run);
#   2. for a tag, the same tag;
#   3. the branch being built (for a pull request, its head branch);
#   4. for a pull request, its base branch;
#   5. --default, main unless given.
#
# The facts about the build come from the GitHub Actions environment
# (GITHUB_REF_TYPE, GITHUB_REF_NAME, GITHUB_HEAD_REF, and GITHUB_BASE_REF), or
# from the options below when run by hand.
#
# Usage:
#   scripts/companion-ref.sh --dir DIR [options]
#
# Options:
#   --dir DIR        A clone of the companion repository with all of its
#                    branches and tags fetched (actions/checkout with
#                    fetch-depth: 0)
#   --override REF   Use REF if it exists
#   --branch NAME    The branch being built (default: GITHUB_HEAD_REF, or
#                    GITHUB_REF_NAME for a branch push)
#   --base NAME      The pull request's base branch (default: GITHUB_BASE_REF)
#   --tag NAME       The tag being built (default: GITHUB_REF_NAME for a tag)
#   --default NAME   The last resort (default: main)
#   --checkout       Check the chosen ref out in DIR (detached)
#   -h, --help       Show this help
#
# Prints the chosen ref and why. Under GitHub Actions it also writes ref=<ref>
# to GITHUB_OUTPUT and a line to the job summary.

set -euo pipefail

usage() {
    sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

DIR=""
OVERRIDE=""
BRANCH=""
BASE="${GITHUB_BASE_REF:-}"
TAG=""
DEFAULT=main
CHECKOUT=false

if [[ "${GITHUB_REF_TYPE:-}" == tag ]]; then
    TAG="${GITHUB_REF_NAME:-}"
elif [[ -n "${GITHUB_HEAD_REF:-}" ]]; then
    BRANCH="$GITHUB_HEAD_REF"
else
    BRANCH="${GITHUB_REF_NAME:-}"
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dir)
            DIR="${2:?--dir needs a directory}"
            shift 2
            ;;
        --override)
            OVERRIDE="${2-}"
            shift 2
            ;;
        --branch)
            BRANCH="${2-}"
            shift 2
            ;;
        --base)
            BASE="${2-}"
            shift 2
            ;;
        --tag)
            TAG="${2-}"
            shift 2
            ;;
        --default)
            DEFAULT="${2:?--default needs a name}"
            shift 2
            ;;
        --checkout)
            CHECKOUT=true
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

if [[ -z "$DIR" || ! -d "$DIR/.git" ]]; then
    echo "error: --dir must name a git clone of the companion repository" >&2
    exit 2
fi

# exists KIND NAME -- print the full ref name if it exists in DIR.
exists() {
    local kind="$1" name="$2" ref
    [[ -z "$name" ]] && return 1
    case "$kind" in
        tag) ref="refs/tags/$name" ;;
        branch) ref="refs/remotes/origin/$name" ;;
        any)
            for ref in "refs/remotes/origin/$name" "refs/tags/$name"; do
                if git -C "$DIR" rev-parse --verify --quiet "$ref^{commit}" > /dev/null; then
                    echo "$ref"
                    return 0
                fi
            done
            # A commit hash.
            if git -C "$DIR" rev-parse --verify --quiet "$name^{commit}" > /dev/null; then
                echo "$name"
                return 0
            fi
            return 1
            ;;
    esac
    git -C "$DIR" rev-parse --verify --quiet "$ref^{commit}" > /dev/null && echo "$ref"
}

ref=""
why=""
if [[ -n "$OVERRIDE" ]]; then
    if ! ref=$(exists any "$OVERRIDE"); then
        echo "error: the requested ref '$OVERRIDE' does not exist in the companion repository" >&2
        exit 1
    fi
    why="requested for this run"
elif ref=$(exists tag "$TAG"); then
    why="the same tag"
elif ref=$(exists branch "$BRANCH"); then
    why="the same branch"
elif ref=$(exists branch "$BASE"); then
    why="the pull request's base branch ('$BRANCH' has no partner)"
elif ref=$(exists branch "$DEFAULT"); then
    tried="'${TAG:-$BRANCH}'"
    [[ -n "$BASE" ]] && tried+=" or '$BASE'"
    why="the default branch (no partner for $tried)"
else
    echo "error: none of the candidate refs exists in the companion repository" >&2
    exit 1
fi

short="${ref#refs/remotes/origin/}"
short="${short#refs/tags/}"
commit=$(git -C "$DIR" rev-parse --short "$ref^{commit}")
echo "companion-ref: $short ($commit) -- $why"

if [[ "$CHECKOUT" == true ]]; then
    git -C "$DIR" checkout --quiet --detach "$ref"
fi
if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
    echo "ref=$short" >> "$GITHUB_OUTPUT"
fi
if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    echo "Companion repository checked out at \`$short\` ($commit): $why." >> "$GITHUB_STEP_SUMMARY"
fi
