#!/bin/bash
gcc -o build-out high_energy_calibration.cpp -Wno-write-strings -L. -lpxcore -lminipix  -ldl -lm -lc -g
[ ! -d "./output-files" ] && mkdir ./output-files
# build the high-energy calibration example (high_energy_calibration.cpp) and create dirrectory for data saving