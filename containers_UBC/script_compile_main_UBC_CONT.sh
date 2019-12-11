#!/bin/bash
g++ -std=c++11 -Wall main_UBC_CONT.cpp -o main_UBC_CONT -I/opt/gurobi811/linux64/include -L/opt/gurobi811/linux64/lib -lgurobi_c++ -lgurobi81
