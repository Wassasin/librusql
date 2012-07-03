#!/bin/bash
set -e
set +x

CC="clang"
CXX="clang++"

rm -rf build
mkdir -p build
pushd build
cmake .. -DCMAKE_CXX_COMPILER="${CXX}" -DCMAKE_CC_COMPILER="${CC}"
popd

