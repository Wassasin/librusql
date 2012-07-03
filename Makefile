all: build
	cd build && make

build:
	./cmake.sh

test: build
	cd build && make test

