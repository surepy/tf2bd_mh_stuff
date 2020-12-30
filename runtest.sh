#!/bin/bash
set -e

rm -rf out_test_linux
mkdir out_test_linux
cd out_test_linux

echo Configuring...
CXX=g++-10 cmake ../ -G Ninja

echo Building...
cmake --build .

echo Running CTest...
ctest --output-on-failure

echo Running gcovr...
# cd test/CMakeFiles/text_stringops_test.dir
# cd ../
gcovr --root "../" --exclude ".*/catch.hpp" --sort-percentage --html-details "results.html" .
