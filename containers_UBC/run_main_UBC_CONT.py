#!/usr/bin/python

import sys
import subprocess
import datetime
from timeit import default_timer as timer
import time



if __name__ == '__main__':
	start=timer()
	file_list=['B_heuristic_depth2_076_080_8388608_D0_P0.csv']#
	M_list=[30,40,50]#
	Epsilon_list=[2,5,10]#
	output_file='my_tests_tests.csv'#
	time_limit=40#
	seed=30#
	threads=1#
	container_grouping_factor=1#
	write_sol='yes'


	handler_output_file=open(output_file, 'w')
	handler_output_file.write('start: '+str(datetime.datetime.now())+'\n')
	handler_output_file.write('Input file,Deduplication level,Depth,System start,System end,filter/grouping factor,avg block/container size,Number of blocks/containers,Number of files,M fraction,M,M fraction actual,M actual,Epsilon fraction,Epsilon,Replication,Replication fraction,Seed,Threads,Time limit,Status,Total time,Solver time\n')
	handler_output_file.flush()
	handler_output_file.close


	for file in file_list:
		for M in M_list:
			for Epsilon in Epsilon_list:
				container_size=file[27:34]#
				depth=file[17]#
				file_system_start=file[19:22]#
				file_system_end=file[23:26]#
																					#./main_UBC_CONT {file name} {benchmarks output file name} {M} {epsilon} {yes/no (write solution to optimization_solution.txt)} {container grouping factor} {model time limit in seconds} {seed} {threads} {container size} {depth} {file_system_start} {file_system_end}
				command='./main_UBC_CONT {0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} {11} {12}'.format(file, output_file,                 M, Epsilon, write_sol,													container_grouping_factor, 	time_limit, 					seed, threads, container_size, 	depth, 	file_system_start, 	file_system_end)
				p = subprocess.Popen([command], shell=True)
				p.wait()
				print('just finished: ',command)

	end = timer()
	handler_output_file=open(output_file, 'a+')
	handler_output_file.write('end: '+str(datetime.datetime.now())+'\n')
	handler_output_file.write('the test ran for: '+str(end-start)+'\n')
	handler_output_file.flush()
	handler_output_file.close

