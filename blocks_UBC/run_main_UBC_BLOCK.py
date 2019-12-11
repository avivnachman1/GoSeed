#!/usr/bin/python

import sys
import subprocess
import datetime
from timeit import default_timer as timer
import time



if __name__ == '__main__':
	start=timer()
	file_list=['B_dedup_220_220.csv']#
	M_list=[50]#
	Epsilon_list=[2,5,10,15]#
	output_file='my_block_tests.csv'#
	time_limit=0#
	seed=30#
	threads=1#
	k_filter_factor=0#
	write_sol='yes'


	handler_output_file=open(output_file, 'w')
	handler_output_file.write('start: '+str(datetime.datetime.now())+'\n')
	handler_output_file.write('Input file,Deduplication level,Depth,System start,System end,filter/grouping factor,avg block/container size,Number of blocks/containers,Number of files,M fraction,M,M fraction actual,M actual,Epsilon fraction,Epsilon,Replication,Replication fraction,Seed,Threads,Time limit,Status,Total time,Solver time\n')
	handler_output_file.flush()
	handler_output_file.close


	for file in file_list:
		for M in M_list:
			for Epsilon in Epsilon_list:
				avg_block_size=4096#
				depth=0#
				file_system_start=file[8:11]#
				file_system_end=file[12:15]#
																					#./main_UBC_CONT {file name} {benchmarks output file name} {M} {epsilon} {yes/no (write solution to optimization_solution.txt)} {k filter factor} {model time limit in seconds} {seed} {threads} {avg block size} {depth} {file_system_start} {file_system_end}
				command='./main_UBC_BLOCK {0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} {11} {12}'.format(file, 		output_file,                M, Epsilon, 	write_sol,											k_filter_factor, 	time_limit, 					seed, threads, avg_block_size, 	depth, 	file_system_start, 	file_system_end)
				p = subprocess.Popen([command], shell=True)
				p.wait()
				print('just finished: ',command)

	end = timer()
	handler_output_file=open(output_file, 'a+')
	handler_output_file.write('end: '+str(datetime.datetime.now())+'\n')
	handler_output_file.write('the test ran for: '+str(end-start)+'\n')
	handler_output_file.flush()
	handler_output_file.close

