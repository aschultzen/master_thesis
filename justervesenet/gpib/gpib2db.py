# TODO:
# Initialize config and return config object, pass this on.
# Verify config name. If no name exists like param name, ask for GPIB channel instead.

# 213 Init ignored
# 108 param not allowed
# 109 missing param
# 1160 measurement broken off

import gpib
import time
from ConfigParser import SafeConfigParser
import fileinput, sys
import datetime
import os
import sys
import ctypes
import mysql.connector
import hashlib

GPIB_IDENTIFY = "*IDN?"
GPIB_RESET = "*RST"
GPIB_CLEAR = "*CLS"
GPIB_READ = ":READ?"
GPIB_MEASURE = GPIB_READ
GPIB_ERROR = ":SYSTem:ERRor?"
LOG_LEVEL = 0
CONFIG_PATH = "config.ini"
GPIB_CONFIG_FILE = "/etc/gpib.conf"

def dbConnect(c_parser):
    try:    
        dbConnection = mysql.connector.connect(
            host= c_parser.get('db','ip'),
            database= c_parser.get('db','database'),
            user= c_parser.get('db','user'),
            password= c_parser.get('db','password'),
            autocommit=True
            )
        
        if dbConnection.is_connected():
            return dbConnection
        else:
        	return 0

    except mysql.connector.errors.InterfaceError as e:
            return 0

def dbClose(dbConnection):
    dbConnection.close()

    if dbConnection.is_connected():
    	return 0
    else:
    	return 1 

def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + current_time.isoformat() + "] " + message
    print(complete_message)
    if(LOG_LEVEL > 0):
        with open(config['logs']['log_path'], "a+") as log:
            log.write(complete_message + "\n")

def gpib_query(handle, command, numbytes = 100):
	gpib.write(handle, command)
	time.sleep(0.1)
	response = "NULL"
	try:
		response = gpib.read(handle, numbytes)
	except gpib.GpibError:
		t_print("Read error, returning NULL")
	return response

def gpib_reset(handle, sleep):
	gpib.write(handle, GPIB_RESET)
	time.sleep(sleep)

def gpib_clear(handle, sleep):
	gpib.write(handle, GPIB_CLEAR)
	time.sleep(sleep)

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

if __name__ == '__main__':
	t_print("gpib2db started!")
	
	# Init config
	config_parser = SafeConfigParser()
	conf_status = config_parser.read(CONFIG_PATH)

	if(len(conf_status) == 0):
		t_print("Failed to load " + CONFIG_PATH + ". Aborting")
		sys.exit()
	else:
		t_print("Config loaded from " + CONFIG_PATH)

	# Connects to database
	connection_attempts = 1
	connection_attempts_max = int(config_parser.get('db','connection_attempts_max'))
	db_con = dbConnect(config_parser)
	while( db_con == 0 ):
		time.sleep(1)
		t_print("DB connection attempt " + str(connection_attempts) + " failed.")
		db_con = dbConnect(config_parser)
		connection_attempts = connection_attempts + 1
		if(connection_attempts_max > 1 and connection_attempts > connection_attempts_max):
			t_print("Reached maximum attempts at connecting to DB. Aborting")
			sys.exit()

	t_print("Connection to database established after "
			+ str(connection_attempts) + " attempt(s)")

	# Connecting to counter/analyzer
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
	device_handle = get_handle(device_name)
	
	## Clearing device (RST, CLS)
	gpib_clear(device_handle, 0.1)
	gpib_reset(device_handle, 0.1)
	
	## Querying for ID
	device_id = gpib_query(device_handle, GPIB_IDENTIFY)
	device_id = device_id.rstrip("\r\n")
	t_print("Connection to GPIB device established!")
	t_print("Device ID: " + device_id)

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
			time.sleep(0.2)
			gpib.write(device_handle, file_content[x])

	error_list = gpib_get_errors(device_handle)
	if(len(error_list) != 0):
		t_print("Following errors where reported by the counter:")
		for x in range(0, len(error_list)):
			t_print("Error[" + str(x + 1) + "] " + error_list[x])

	print gpib_query(device_handle, GPIB_READ)

	# Closing connection to database
	close_status = dbClose(db_con)
	if(close_status == 1):
		t_print("Connection to database closed")
	else:
		t_print("Failed to close connection to database. Aborting anyway")
