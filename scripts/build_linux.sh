#!/bin/bash
set -ex

mkdir -p release
cd release

cmake -GNinja .. 
ninja

ctest --output-on-failure