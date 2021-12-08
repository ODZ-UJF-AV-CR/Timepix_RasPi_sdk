#!/bin/bash
gcc -o build-out ex-otherFns.cpp -Wno-write-strings -L. -lpxcore -lminipix  -ldl -lm -lc -g
# build the Other Functions example (ex-otherFns.cpp)
