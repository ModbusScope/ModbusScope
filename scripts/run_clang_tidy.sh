#!/bin/bash
set -euo pipefail

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

echo "=== Running clang-tidy ==="
if [[ -n "${SINGLE_FILE}" ]]; then
    clang-tidy -quiet -p "${BUILD_DIR}" "${SINGLE_FILE}"
else
    run-clang-tidy -quiet -p "${BUILD_DIR}" -j "$(nproc)" "$(pwd)/src/.*\.cpp\$" 2>/dev/null
fi
