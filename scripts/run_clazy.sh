#!/bin/bash
set -euo pipefail

BUILD_DIR="${1:-build}"
QT_PREFIX="${2:-/opt/Qt/6.8.3/gcc_64}"

echo "=== Configuring (compile_commands.json) ==="
cmake -GNinja \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
    -B "${BUILD_DIR}"

echo "=== Generating autogen headers ==="
ninja -C "${BUILD_DIR}" src/ScopeSource_autogen

echo "=== Running clazy ==="
find "$(pwd)/src" -name "*.cpp" -print0 | \
    xargs -0 -P "$(nproc)" -I{} clazy-standalone --only-qt --header-filter="src/.*" -p "${BUILD_DIR}" {}
