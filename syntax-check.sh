#!/bin/bash

[ -f "$(which $CXX)" ] || CXX=gcc

for i in src/rusql/*.hpp; do
	$CXX --std=c++0x -fsyntax-only ${i}
done

