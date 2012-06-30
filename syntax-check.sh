#!/bin/bash

for i in src/rusql/*.hpp; do
	gcc --std=c++0x -fsyntax-only ${i}
done

