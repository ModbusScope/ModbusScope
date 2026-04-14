#!/bin/bash
set -ex

mkdir -p release
cd release

cmake -GNinja -DCMAKE_BUILD_TYPE="${BUILD_TYPE:-Debug}" ..
ninja

ctest --output-on-failure