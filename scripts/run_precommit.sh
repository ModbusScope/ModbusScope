#!/bin/bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}/.."
PASS=0
FAIL=0

run_check()
{
    local name="$1"
    shift
    echo "=== $name ==="
    if "$@"; then
        echo "--- $name: PASS"
        ((PASS++))
    else
        echo "--- $name: FAIL"
        ((FAIL++))
    fi
    echo ""
}

run_check "clang-format" bash "${SCRIPT_DIR}/run_clang_format.sh" --check
run_check "clang-tidy"   bash "${SCRIPT_DIR}/run_clang_tidy.sh"
run_check "clazy"        bash "${SCRIPT_DIR}/run_clazy.sh"

echo "=== Results: ${PASS} passed, ${FAIL} failed ==="
[ "${FAIL}" -eq 0 ]
