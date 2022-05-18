#!/bin/sh

############################################################################################################
# Based on: https://kelvinsp.medium.com/generating-code-coverage-with-qt-5-and-gcov-on-mac-os-4999857f4676
#
# Prerequirements:  Build with coverage info
#                   cmake . -GNinja -DUSE_GCOV=ON
#
# Usage:    ./process_coverage.sh [build dir with covera info]
#           ./process_coverage.sh ../build-ModbusScope-Desktop_Qt_6_2_4_GCC_64bit-Debug/
#
############################################################################################################

# SRC_DIR is the directory containing the .gcno files
SRC_DIR=$1

# COV_DIR is the directory where the coverage results will be stored
COV_DIR="./coverage"

############################################################################################################

# Path where the HTML files should be saved
HTML_RESULTS="${COV_DIR}""/html"

# Create the html folder if it does not exists
mkdir -p ${HTML_RESULTS}

# Generate our initial info
lcov -d "${SRC_DIR}" -c -o "${COV_DIR}/coverage.info"
 
# Remove some paths/files which we don't want to calculate the code coverage (e.g. third party libraries) and generate a new coverage file filtered (feel free to edit it when necessary)
lcov -r "${COV_DIR}/coverage.info" "*.h" "*Qt*.framework*" "*.moc" "*moc_*.cpp" "*/libraries/*" "*/tests/*" "/usr/include/*" -o "${COV_DIR}/coverage-filtered.info"
 
# Generate the HTML files
genhtml -o "${HTML_RESULTS}" "${COV_DIR}/coverage-filtered.info"
 
# Reset our counts
lcov -d "${COV_DIR}" -z
 
# Open the index.html
open "${HTML_RESULTS}/index.html" &