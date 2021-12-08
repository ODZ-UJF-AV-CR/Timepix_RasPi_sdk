#!/bin/bash
gcc -o build-out ex-dataDriven.cpp -Wno-write-strings -L. -lpxcore -lminipix  -ldl -lm -lc -g
[ ! -d "./output-files" ] && mkdir ./output-files
# build the Data Driven Mode example (ex-dataDriven.cpp) and create dirrectory for data saving
