#!/bin/bash
set -e
set +x

CC="/sw/bin/gcc-fsf-4.6"
CXX="/sw/bin/g++-fsf-4.6"

rm -rf build
mkdir -p build
pushd build
cmake .. -DCMAKE_CXX_COMPILER="${CXX}" -DCMAKE_CC_COMPILER="${CC}"
popd

