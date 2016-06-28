# TODO:
# Initialize config and return config object, pass this on.
# Verify config name. If no name exists like param name, ask for GPIB channel instead.

import gpib
import time
from ConfigParser import SafeConfigParser
import fileinput, sys
import datetime
import os
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
            t_print("Connection to database (" + dbConnection.server_host +":" 
                + str(dbConnection.server_port) + ") established")

    except RuntimeError as e:
            t_print(e)

    return dbConnection    

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
	t_print("Starting up...")
	t_print("Loading config...")
	config_parser = SafeConfigParser()
	config_parser.read(CONFIG_PATH)
	t_print("Config loaded from " + CONFIG_PATH + ".")
	t_print("Connecting to database...")
	dbConnect(config_parser)