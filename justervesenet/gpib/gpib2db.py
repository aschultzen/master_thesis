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
import jdutil
from datetime import date,timedelta

GPIB_IDENTIFY = "*IDN?"
GPIB_RESET = "*RST"
GPIB_CLEAR = "*CLS"
GPIB_READ = ":READ?"
GPIB_MEASURE = GPIB_READ
GPIB_ERROR = ":SYSTem:ERRor?"
LOG_LEVEL = 0
CONFIG_PATH = "config.ini"
GPIB_CONFIG_FILE = "/etc/gpib.conf"


class matrix_switch(object):
	''' A class for the matrix switch. It mostly contains state '''
	def __init__(self, handle):
		self.switch_handle = handle
		self.source_index = 0
		config_parser = SafeConfigParser()
		config_parser.read(CONFIG_PATH)
		self.number_of_ports = int(config_parser.get('matrix','num_of_ports'))
		self.load_config()

		self.switch_codes = [
		"ZEROINDX",
		"01010100",
		"01010200",
		"01010400",
		"01010800",
		"01011000",
		"01012000",
		"01014000",
		"01018000",
		"01010001",
		"01010002",
		"01010004",
		"01010008",
		"01010010",
		"01010020",
		"01010040",
		"01010080",
		]

	def load_config(self):
		config_parser.read(CONFIG_PATH)
		self.source_list = config_parser.get('sources','list')
		self.source_list = self.source_list.rstrip(" ")
		self.source_list = self.source_list.split("\n")

	def iterate_port(self):
		self.source_index += 1
		if(self.source_index > self.number_of_ports):
			self.source_index = 1
		return self.source_index

	def get_next_source(self):
		# Check if port is supposed to be switched to:
		empty_list_retry_time = int(config_parser.get('sources','empty_list_retry')) 
		line = "NULL"
		counter = 0
		while(True):
			port = self.iterate_port()
			for x in range(0, len(self.source_list)):
				line = self.source_list[x].split(",")
				if(line[1] == str(port) and line[0][0] != "#"):
					return line
			counter += 1
			if(counter > self.number_of_ports):
				t_print("No sources configured. Trying again in: " + str(empty_list_retry_time) + " seconds")
				time.sleep(empty_list_retry_time)
				self.load_config()
				counter = 0

	def switch(self):
		self.load_config()
		switch_touple = self.get_next_source()
		source_name = switch_touple[0]
		port_to_switch = int(switch_touple[1])
		#t_print("Switching: " + self.switch_codes[port_to_switch] + ", source: " + source_name)
		#gpib.write(switch_handle, self.switch_codes[port_to_switch])
		return (source_name, port_to_switch)


def get_today_mjd():
    today = datetime.datetime.utcnow()
    return jdutil.jd_to_mjd(jdutil.datetime_to_jd(today)) 

def get_date():
	return datetime.datetime.utcnow()

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

def format_date_string(date_s):
    split = date_s.split(".")
    split = split[::-1]
    split = ''.join(split)
    return split

def create_query(config_parser, switch_info, measurement):
	today = time.strftime("%Y/%m/%d")
	now = time.strftime("%H:%M:%S")
	measurement = measurement.rstrip("\r\n")

	data_measurement = {
		'date': today,
		'time': now,
		'mjd' : get_today_mjd(),
		'source': switch_info[0],
		'value' : measurement,
		'ref_clock': config_parser.get('general','ref_clock'),
		'measurerID': config_parser.get('general','measurerID'),
	}
	return data_measurement

def upload(db_con, data_measurement):
	cursor = db_con.cursor();

	add_measurement = ("INSERT INTO clock_measurements"
              "(date, time, mjd, source, value, ref_clock, measurerID) "
              "VALUES (%(date)s, %(time)s, %(mjd)s, %(source)s, %(value)s, %(ref_clock)s, %(measurerID)s)")

	cursor.execute(add_measurement, data_measurement)

	# Make sure data is committed to the database
	db_con.commit()
	cursor.close()

def measure(config_parser, counter_handle, matrix_switch, db_con):	
	switch_info = matrix_switch.switch()
	measurement = gpib_query(counter_handle, GPIB_MEASURE)
	data_measurement = create_query(config_parser, switch_info, measurement)
	upload(db_con, data_measurement)

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
	# this code was written for, does not  l
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
	ms = matrix_switch(11)

	# Begin measurements
	while(True):
		measure(config_parser, counter_handle, ms, db_con)
		time.sleep(1)

	# Closing connection to database
	close_status = dbClose(db_con)
	if(close_status == 1):
		t_print("Connection to database closed")
	else:
		t_print("Failed to close connection to database. Aborting anyway")
