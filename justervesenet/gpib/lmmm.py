# Make sure that the program continues normal execution even if the link goes down

import gpib
import time
import fileinput, sys
import os
import sys
import ctypes
import hashlib

# LMMM Modules
from matrix_switch import matrix_switch
from lmmm_common import (t_print, init_config)
from lmmm_db import (db_connector, dbClose, create_query, upload)

GPIB_IDENTIFY = "*IDN?"
GPIB_RESET = "*RST"
GPIB_CLEAR = "*CLS"
GPIB_READ = ":READ?"
GPIB_MEASURE = GPIB_READ
GPIB_ERROR = ":SYSTem:ERRor?"
LOG_LEVEL = 0
CONFIG_PATH = "lmmm_config.ini"
GPIB_CONFIG_FILE = "/etc/gpib.conf"

def gpib_query(handle, command, numbytes = 100):
	gpib.write(handle, command)
	time.sleep(0.1)
	response = "NULL"
	try:
		response = gpib.read(handle, numbytes)
	except gpib.GpibError:
		t_print("Read error, returning NULL")
	return response

def gpib_reset(handle):
	try:
		gpib.write(handle, GPIB_RESET)
	except gpib.GpibError:
		return 0
	return 1

def gpib_clear(handle):
	try:
		gpib.write(handle, GPIB_CLEAR)
	except gpib.GpibError:
		return 0
	return 1

# Reads all the errors from the counter into a list.
# If the returned list len = 0, no errors where returned.
def gpib_get_errors(handle):
	error_list = []
	while(True):
		gpib.write(handle, GPIB_ERROR)
		response = gpib.read(handle, 100)
		if(response.find("No error") == -1):
			response = response.rstrip("\r\n")
			error_list.append(response)
		else:
			break
	return error_list

def set_display(handle, value):
	if(value < 1):
		gpib.write(handle, "DISP:ENAB OFF") 
	else:
		gpib.write(handle, "DISP:ENAB ON") 

# Returns handle to GPIB device by either name or channel
def get_handle(name):
		if( isinstance(name, basestring) ):
			device = gpib.find(name)
		elif( isinstance(name, int)):
			device = gpib.dev(0,name)
		else:
			return 0
		return (device)

# Dump query (dm) to file
def dump(dm, path, upload_status):
	dump_file = open(path, "a+")
	output = (	dm['date'] + " " + dm['time'] + " " + 
				str(dm['mjd']) + " " + dm['source'] + " " + 
				dm['value'] + " " + 
				dm['ref_clock'] + " " + dm['measurerID'])
	# If upload to db failed, the query is marked
	if(upload_status == 0):
		output += " *\n"
	else:
		output += "\n"

	dump_file.write(output)
	dump_file.close()

def measure(config_parser, counter_handle, matrix_switch, db_con):	
	switch_info = matrix_switch.switch()
	measurement = gpib_query(counter_handle, GPIB_MEASURE)
	data_measurement = create_query(config_parser, switch_info, measurement)
	#upload_status = upload(db_con, data_measurement)
	upload_status = 1
	if(config_parser.get('general','dump_to_file') == "yes"):
		path = config_parser.get('general','dump_dir')
		path += config_parser.get('general','dump_file_name')
		path += time.strftime("%Y_%m_%d")
		dump(data_measurement, config_parser.get('general','dump_file_name'), upload_status)

if __name__ == '__main__':
	t_print("Lean Mean Measuring Machine (LMMM) started!")
	
	# Initializes config
	config_parser = init_config(CONFIG_PATH)
	if(config_parser == 0):
		sys.exit()

	# Connecting to the database
	db_con = db_connector(config_parser)
	if(db_con == 0):
		sys.exit()

	# Connecting to counter/analyzer
	# This could be put in a function, but it is only 
	# called once throughout the program
	device_name_config = config_parser.get('counter','name')
	device_name = device_name_config
	name_found_in_config = -1

	try:
		gpib_conf_file = open(GPIB_CONFIG_FILE, "r")
		file_content = gpib_conf_file.read()
		gpib_conf_file.close()
		name_found_in_config = file_content.find(device_name_config)
	except IOError:
		t_print("Could not load " + GPIB_CONFIG_FILE + ", trying anyway...")

	if( name_found_in_config < 0 ):
		t_print("Did not find " + device_name_config + " in /etc/gpib.conf, falling back to config.ini")
		device_name = int(config_parser.get('counter','channel'))

	## Retrieving handle
	counter_handle = get_handle(device_name)
	
	## Clearing device (RST, CLS)
	time.sleep(1)
	if(gpib_clear(counter_handle) == 0):
		counter = 1
		give_up = int(config_parser.get('counter','cls_rst_retry_count'))
		
		while(counter <= give_up):
			t_print("Attempt " +  str(counter) + " to CLEAR counter failed, retrying...")
			time.sleep(1)
			if(gpib_clear(counter_handle) == 1):
				break;
			counter += 1
		t_print("FAILED to clear to counter, confirm that the counter is powered on. Aborting.")
		sys.exit()
	else:
		t_print("Counter CLEARED")

	time.sleep(1)
	if(gpib_reset(counter_handle) == 0):
		counter = 1
		give_up = int(config_parser.get('counter','cls_rst_retry_count'))
		
		while(counter <= give_up):
			t_print("Attempt " +  str(counter) + " to RESET counter failed, retrying...")
			time.sleep(1)
			if(gpib_reset(counter_handle) == 1):
				break;
			counter += 1
		t_print("FAILED to reset to counter, confirm that the counter is powered on. Aborting.")
		sys.exit()
	else:
		t_print("Counter RESET")

	gpib_reset(counter_handle)
	
	## Querying for ID
	device_id = gpib_query(counter_handle, GPIB_IDENTIFY)
	device_id = device_id.rstrip("\r\n")
	t_print("Counter ID: " + device_id)

	# Uploading GPIB configuration
	try:
		device_gpib_conf_path = config_parser.get('counter','device_gpib_conf')
		device_gpib_conf_file = open(device_gpib_conf_path,'r')
		file_content = device_gpib_conf_file.read()
		hasher = hashlib.sha1	()
		hasher.update(file_content)
		if(hasher.hexdigest() == config_parser.get('counter','device_gpib_conf_checksum').rstrip()):
			t_print("Checksum check PASSED for: " + device_gpib_conf_path)
		else:
			t_print("Checksum check FAILED for: " + device_gpib_conf_path)
			if(config_parser.get('counter','quit_on_failed_checksum') == "yes"):
				t_print("Aborting because of failed checksum. See config.ini")
				sys.exit()	
	except IOError:
		t_print("Could not load device config file: " + config_parser.get('counter','device_gpib_conf') + ". Aborting...")
		sys.exit()	

	file_content = file_content.split("\n")

	for x in range(0, len(file_content)):
		if( file_content[x][0] != "#" and file_content[x][0] != "\n"):
			time.sleep(1)
			gpib.write(counter_handle, file_content[x])

	error_list = gpib_get_errors(counter_handle)
	if(len(error_list) != 0):
		t_print("Following errors where reported by the counter:")
		for x in range(0, len(error_list)):
			t_print("Error[" + str(x + 1) + "] " + error_list[x])
	else:
		t_print("No errors collected from counter.")

	# Connecting to the Matrix switch
	'''
	matrix_name_config = config_parser.get('matrix','name')
	matrix_name = matrix_name_config
	name_found_in_config = -1

	try:
		gpib_conf_file = open(GPIB_CONFIG_FILE, "r")
		file_content = gpib_conf_file.read()
		gpib_conf_file.close()
		name_found_in_config = file_content.find(matrix_name_config)
	except IOError:
		t_print("Could not load " + GPIB_CONFIG_FILE + ", trying anyway...")

	if( name_found_in_config < 0 ):
		t_print("Did not find " + matrix_name_config + " in /etc/gpib.conf, falling back to config.ini")
		matrix_name = int(config_parser.get('counter','channel'))

	## Retrieving handle
	matrix_handle = get_handle(matrix_name)

	## Testing connection 

	# NOTE! The CE 1017 Matrix switch that 
	# this code was written for, does not
	# respond to the "*IDN?" command.
	# However, an "gpib.GpibError: write() failed:"
	# error will be raised if there is no device 
	# connected to the channel used by the handler.
	# The message could in other words just be garbage.
	# This will however work as advertized on a switch
	# that has the "*IDN?" command implemented
	try:
		gpib.write(matrix_handle, "*IDN?")
	except gpib.GpibError:
			t_print("Error when writing to the switches channel, check configuration and try again.")
			t_print("Aborting")
			sys.exit()
	'''		
	ms = matrix_switch(11, CONFIG_PATH)

	t_print("Commencing measurements...")
	# Begin measurements
	while(True):
		measure(config_parser, counter_handle, ms, db_con)
		#time.sleep(1)

	# Closing connection to database
	close_status = dbClose(db_con)
	if(close_status == 1):
		t_print("Connection to database closed")
	else:
		t_print("Failed to close connection to database. Aborting anyway")
