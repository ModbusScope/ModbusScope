#!/bin/bash
set -uo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
QT_PREFIX="${QT_PREFIX:-/opt/Qt/6.8.3/gcc_64}"
SINGLE_FILE="${1:-}"

echo "=== Configuring (compile_commands.json) ==="
cmake -GNinja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
    -B "${BUILD_DIR}"

echo "=== Generating autogen headers ==="
ninja -C "${BUILD_DIR}" src/ScopeSource_autogen

echo "=== Running clazy ==="
if [[ -n "${SINGLE_FILE}" ]]; then
    OUTPUT=$(clazy-standalone --only-qt --header-filter="src/.*" -p "${BUILD_DIR}" "${SINGLE_FILE}" 2>&1 || true)
else
    OUTPUT=$(find "$(pwd)/src" -name "*.cpp" -print0 | \
        xargs -0 -P "$(nproc)" -I{} clazy-standalone --only-qt --header-filter="src/.*" -p "${BUILD_DIR}" {} 2>&1 || true)
fi

# Check for violations in project source files, ignoring vendored qcustomplot library
PROJECT_VIOLATIONS=$(echo "${OUTPUT}" | grep -E "^.+:[0-9]+:[0-9]+: (error|warning):" | grep -v "qcustomplot" || true)

if [[ -n "${PROJECT_VIOLATIONS}" ]]; then
    echo "${OUTPUT}"
    exit 1
fi
