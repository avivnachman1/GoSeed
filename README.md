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
2. Right after comes the files description, these lines begin with the character 'F' and contains information such as serial number, number of blocks, serial numbers of the blocks related and their sizes in bytes, etc.
   - Example: "F,33,002_28147,0,3,0,2048,1,4096,2,8192"
   - Interpretation: file with serial number 33, ID of 002_28147, in a directory with serial number of 0, this file is built out of 3 blocks, their serial numbers are 0,1,2 and the sizes are 2kb, 4kb and 8kb respectively.
3. Right after comes the block description, these lines begin with the character 'B' and contains information such as serial number, block fingerprint value (hexadecimal) ,number of files related and the serial numbers of these files.
   - Example: "B,0,9029760f86,4,1,22,33"
   - Interpretation: block with serial number 0, fingerprint of 9029760f86, 4 files related with serial numbers of 4,1,22,33.
