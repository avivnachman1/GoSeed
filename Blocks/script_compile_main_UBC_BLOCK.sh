#!/bin/bash
g++ -std=c++11 -O3 -Wall GoSeedBlocks.cpp -o GoSeedBlocks -I/opt/gurobi811/linux64/include -L/opt/gurobi811/linux64/lib -lgurobi_c++ -lgurobi81
