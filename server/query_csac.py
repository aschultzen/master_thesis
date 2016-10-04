import ctypes
import fileinput, sys
import datetime
import time
import io
import os
import serial

def main_routine():
	# Opening serial stream, use ASCII
	ser = serial.Serial("/dev/ttyUSB0",57600, timeout=0.1)
	sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser),encoding='ascii',newline="\r\n")

	# Open log file, mostly used for debug
	log_file = open("query_csac.txt", "a+")

	# The query to use 
	query = sys.argv[1].strip("\r\n")

	# How long to sleep between read from serial con.
	sleep_time = 0.2	

	# The minimum length of the answer
	# for the given query.
	minimum_len = 0

	if(query == '^' or query == '6'):
		minimum_len = 80
	elif(query == 'F'):
		sleep_time = 0.5
		minimum_len = 10
	elif(query == 'M'):
		minimum_len = 6
	elif (query == 'S'):
		sleep_time = 3
		minimum_len = 2
	else: 
		minimum_len = 1

	response_len = 0

	if(len(query) > 1):
		query = "!" + query + "\r\n"

	retry_count = 0

	while (response_len < minimum_len):
		ser.write(bytes(query))
		time.sleep(sleep_time)
		response = sio.readline()
		response = response.strip("\r\n\x00")
		response_len = len(response)
		retry_count = retry_count + 1

	print(response)
	ser.close()
	query = query.strip("\r\n")
	log_string = ("Issued query " + "'" +  query + "' " + str(retry_count) + " times\n")	
	log_file.write(log_string)
if __name__ == '__main__':
	main_routine()

