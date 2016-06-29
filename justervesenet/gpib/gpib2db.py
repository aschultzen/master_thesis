# TODO:
# Initialize config and return config object, pass this on.
# Verify config name. If no name exists like param name, ask for GPIB channel instead.

import gpib
import time
from ConfigParser import SafeConfigParser
import fileinput, sys
import datetime
import os
import sys
import ctypes
import mysql.connector

GPIB_IDENTIFY = "*IDN?"
GPIB_RESET = "*RST"
LOG_LEVEL = 0
CONFIG_PATH = "config.ini"

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
    complete_message = "[" + current_time.isoformat() + "] " +"[" + message + "]"
    print(complete_message)
    if(LOG_LEVEL > 0):
        with open(config['logs']['log_path'], "a+") as log:
            log.write(complete_message + "\n")

def query(handle, command, numbytes = 100):
	gpib.write(handle, command)
	time.sleep(0.1)
	response = gpib.read(handle, numbytes)
	return response

def reset(handle, sleep):
	gpib.write(handle, GPIB_RESET)
	time.sleep(sleep)	

def set_display(handle, value):
	if(value < 1):
		gpib.write(handle, "DISP:ENAB OFF") 
	else:
		gpib.write(handle, "DISP:ENAB ON") 

def get_handle(name):
		device = gpib.find(name)
		q_result = query(device, GPIB_IDENTIFY)
		return (device, q_result)

if __name__ == '__main__':
	t_print("gpib2db started!")
	
	config_parser = SafeConfigParser()
	conf_status = config_parser.read(CONFIG_PATH)

	if(len(conf_status) == 0):
		t_print("Failed to load " + CONFIG_PATH + ". Aborting")
		sys.exit()
	else:
		t_print("Config loaded from " + CONFIG_PATH)

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

	close_status = dbClose(db_con)
	if(close_status == 1):
		t_print("Connection to database closed")
	else:
		t_print("Failed to close connection to database. Aborting anyway")