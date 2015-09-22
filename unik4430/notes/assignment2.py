import ctypes
import configparser
import fileinput, sys
import datetime
import time
import os
import sys

def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + current_time.isoformat() + "] " +"[" + message + "]"
    print(complete_message)

def getConfVal(column, value):
	config = configparser.ConfigParser()
	configFile = "assignment2.ini"
	config.read(configFile)
	conf_value = config[column][value]
	return conf_value

class average:
	count = 0.0 # Number of times called; n.
	total = 0.0	# n-1 + n + n +1
	avg = 0.0	# The current average
		
	def pushpeek(self, value_n):
		self.push(value_n) 
		return self.avg

	def get_count(self):
		return self.count
	
	def peek(self):
		return self.avg

	def push(self, value_n):
		self.count = self.count + 1
		self.total = self.total + value_n
		self.avg = self.total / self.count

	def get_total(self):
		return self.total

class smart_average:
	count = 0.0 # Number of times called; n.
	avg = 0.0	# The current average
	prev_avg = 0.0 # The previous average
		
	def pushpeek(self, value_n):
		self.push(value_n) 
		return self.avg

	def get_count(self):
		return self.count
	
	def peek(self):
		return self.avg

	def push(self, value_n):
		if(self.count == 0.0):
			self.avg = value_n
		else:
			self.prev_avg = self.avg
			self.avg = ( ( (self.count-1) * self.prev_avg) + value_n ) / self.count
		self.count = self.count + 1

def getFile(path):
	result = open(path, 'r')
	resultList = result.read()
	resultList = resultList.splitlines()
	result.close()
	resultList = [float(i) for i in resultList]
	return resultList


## Debug method
def write_average_file(input_file, output_name, type):
	t_print("Making average file...")
	if(type == "smart"):
		t_print("Using smart_average")
		avg = smart_average()
	else:
		t_print("Using average")
		avg = average()
	
	length = len(input_file)
	file_out = open(output_name, 'w+')
	count = 0
	while(count < length):
		file_out.write(str(avg.pushpeek(input_file[count])) + "\n")
		count = count + 1
	file_out.close()
	t_print("Finished")

if __name__ == '__main__':
	file_1 = getFile(getConfVal("files","file2"))
	write_average_file(file_1, "ocxo_avg_smart", "smart")
	write_average_file(file_1, "ocxo_avg_dumb", "dumb")

