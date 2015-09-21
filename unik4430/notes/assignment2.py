# Typical usage
# First argument: Reference data
# Second argument: Data to filter based on reference
# The program will also make a file in the programs root
# with the result file 
#python .\assignment2.py ..\Exercises\GPSDO\data\gps.dat ..\Exercises\GPSDO\data\ocxo.dat

import ctypes
import configparser
import fileinput, sys
import datetime
import time
import os
import sys

class average:
	count = 0 # Number of times called; n.
	avg = 0

	def __init__(self):
		average.count = average.count + 1

	def get_average(value_n):
		average.avg = (average.avg + value_n) / average.count 
		return average.count

def getFile(path):
	result = open(path, 'r')
	resultList = result.read()
	resultList = resultList.splitlines()
	result.close()
	resultList = [float(i) for i in resultList]
	return resultList

def filter(file1):
	length = len(file1)
	f_out = open('output', 'w+')
	count = 0
	while(count < length):
		f_out.write(str(file1[count]) + "\n")
		count = count + 1
	f_out.close()

if __name__ == '__main__':
	file_1 = getFile(sys.argv[1])
	#file_2 = getFile(sys.argv[2])
	filter(file_1)
