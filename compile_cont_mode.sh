#!/bin/bash
g++ -std=c++11 -Wall GoSeedCont.cpp -o GoSeedCont -I/opt/gurobi811/linux64/include -L/opt/gurobi811/linux64/lib -lgurobi_c++ -lgurobi81
