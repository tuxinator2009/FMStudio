#!/bin/sh
CXX_FLAGS="-Wall -g -std=gnu++1z"
LIBS="-lm"
CXX="g++"
for filename in *.cpp; do
	[ -e "$filename" ] || continue
	echo Compiing $(basename "$filename" .cpp) from $filename
	$CXX $CXX_FLAGS -o $(basename "$filename" .cpp) $filename $LIBS
done
