#!/bin/bash
set -euo pipefail

CHECK_MODE=false
if [ "${1:-}" = "--check" ]; then
    CHECK_MODE=true
fi

MERGE_BASE=$(git merge-base HEAD master 2>/dev/null || git merge-base HEAD main 2>/dev/null || echo "")

CHANGED_FILES=$(
    {
        if [ -n "$MERGE_BASE" ]; then
            git diff --name-only --diff-filter=d "$MERGE_BASE" HEAD
        fi
        git diff --name-only --diff-filter=d HEAD
        git ls-files --others --exclude-standard
    } \
    | grep -E '\.(cpp|h)$' \
    | sort -u \
    || true
)

if [ -z "$CHANGED_FILES" ]; then
    echo "No changed C++ files."
    exit 0
fi

echo "=== Files ==="
echo "$CHANGED_FILES"
echo "=== Running clang-format ==="

if [ "$CHECK_MODE" = true ]; then
    echo "$CHANGED_FILES" | xargs clang-format --dry-run --Werror
else
    echo "$CHANGED_FILES" | xargs clang-format -i
fi
