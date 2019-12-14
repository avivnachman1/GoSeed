# GoSeed
ILP based migration plan generator for deduplication storage.

This project part of my MSC work at the technion and is attached to the paper "GoSeed: Generating an Optimal Seeding Plan for Deduplicated Storage" presented at FAST20 ([link to the paper](https://actual_link/)).

In order to compile and run the project you must have g++, boost and gurobi (I was using version 8.1.1).

The Repositry Contains:
1. A readme file.
2. Two source code files:
   - GoSeedBlocks.cpp: The source code when blocks are the most basic migration unit.
   - GoSeedCont.cpp: The source code when containers are the most basic migration unit, there are few optimizations that can be applied when migrating full containers.
3. Two bash scripts for compilation, one for each source.
