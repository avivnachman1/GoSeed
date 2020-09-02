# GoSeed
ILP based migration plan generator for deduplication storage.

This repository is attached to the paper: "GoSeed: Generating an Optimal Seeding Plan for Deduplicated Storage".<br/>
Authors: Aviv Nachman, Gala Yadgar and Sarai Sheinvald.<br/>
Conference: FAST20, 18th USENIX Conference on File and Storage Technologies.<br/>
Link: https://www.usenix.org/conference/fast20/presentation/nachman

In order to compile and run the project you must have g++, boost and gurobi (I was using version 8.1.1).

The Repositry Contains:
1. A readme file.
2. Two source code files:
   - GoSeedBlocks.cpp: The source code when blocks are the basic migration unit.
   - GoSeedCont.cpp: The source code when containers are the basic migration unit, there are few optimizations that can be applied when migrating full containers.
3. Two bash scripts for compilation, one for each source.

Input format description:
1. The input begins with multiple header lines each one starting with the character '#', These lines contain general information about the workload such as: number of files/blocks/directories, deduplication level, traces ingested, etc.
2. 

