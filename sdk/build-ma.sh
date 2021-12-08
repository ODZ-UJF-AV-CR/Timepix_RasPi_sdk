#!/bin/bash
gcc -o build-out ex-maintenance.cpp -Wno-write-strings -L. -lpxcore -lminipix  -ldl -lm -lc -g
# build the Sensor Maintenance example (ex-maintenance.cpp)
