#!/bin/bash

if [ -d /sw/ ]; then
	./cmake-fink.sh
else
	echo "Fallback to linux make script!"
	./cmake-linux.sh
fi
