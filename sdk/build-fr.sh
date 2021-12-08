#!/bin/bash
gcc -o build-out ex-frames.cpp -Wno-write-strings -L. -lpxcore -lminipix  -ldl -lm -lc -g
[ ! -d "./output-files" ] && mkdir ./output-files
# build the Frame Mode example (ex-frames.cpp) and create dirrectory for saving images
