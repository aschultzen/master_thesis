"""
README
---------
 
This script is used to store NMEA->GPRMC data from
a GNSS chip to a database. It was tested using mysql 
running on a Raspberry Pi (1). All the settings like
DB, GPS interface (TTY) and such, is defined in the
config file using ConfigParser. 

The point of the script was to be able to log GPS
data for later analysis in a system with a DB
readily available.

BUGS
---------  
- No NULL error checks are done. I'm letting
the database do my dirtywork for me. I know,
I'm a bad person.

EXPECTED TABLE (14.10.2015):
---------

create table gprmc (  
	id INT NOT NULL AUTO_INCREMENT, 
	sensorID INT ,
	fix_time TIME,  
	recv_warn VARCHAR(5),  
	latitude DECIMAL(10,5),  
	la_dir VARCHAR(5),  
	longitude DECIMAL(10,5),  
	lo_dir VARCHAR(5),  
	speed DECIMAL(10,5),  
	course DECIMAL(5,2),  
	fix_date DATE,  
	variation DECIMAL(5,2),
	var_dir VARCHAR(5),  
	faa VARCHAR(5),  
	checksum VARCHAR(5),
	PRIMARY KEY (id) );
"""

# NOTES:
# Used Python v.2.7
# python-mysqldb

import ctypes
import MySQLdb as mdb
import ConfigParser
import fileinput, sys
import datetime
import time
import io
import os
import serial
from subprocess import call

# Something should be done about these!
config = ConfigParser.ConfigParser()    # Global variable for configparser.

def dbConnect():
	con = mdb.connect(config.get('db','ip'), config.get('db','user'), config.get('db','password'), config.get('db','database')); 
	return con    

def dbClose(dbConnection):
    dbConnection.close()
    t_print("Connection to database closed")

def initConfig():
    configFile = "config.ini"
    config.read(configFile)
    #t_print("Config file " + configFile + " : loaded.")

def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + str(current_time.isoformat()) + "] " +"[" + message + "]"
    print(complete_message)

def format_date_string(date_s):
    split = date_s.split(".")
    split = split[::-1]
    split = ''.join(split)
    return split

# Do not look directly at this horrible function
def insert(con, data):
	# Removing newline and return carriage
	st = data.replace(",", " ")
	st = st.split(" ")
	temp = st[12]
	checksum = temp[1] + temp[2] + temp[3] 
	faa = temp[0]
	x = con.cursor()
	date = st[9][4:6] + st[9][2:4] + st[9][0:2]
	st[9] = date
	
	#return 0
	try:
		query = ("INSERT INTO " + config.get('db','table') +
		" (sensorID, fix_time, recv_warn, latitude, la_dir, longitude, lo_dir, speed, course, fix_date, variation, var_dir, faa, checksum) VALUES " +
		"(" + config.get('general','sensorID') + "," + st[1] + ",'" + st[2] + "'," + st[3] + ",'" + st[4] + "'," + st[5] + ",'" + st[6] + "','" + st[7] + "','" + st[8] + "','" + st[9] + "','" + st[10] + "','" + st[11] + "','" + faa + "','" + checksum + "');")
		#print(query)
   		x.execute(query)
   		con.commit()
	except:
   		con.rollback()

# Function used to reset the serial configuration
# in Linux in case its mangled by something'
def reset_serial():
	call("stty -F " + config.get('gps','port') + " icanon", shell=True)

def main_routine():
    initConfig()	
    t_print("GPS 2 DB started")
    reset_serial()
    con = dbConnect()
    counter = 0
    while(True):
	ser = serial.Serial(config.get('gps','port'),config.get('gps','baud'),timeout=0.1)
	sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser),newline="\r")
	time.sleep(1)
	while 1:
	   	temp = sio.readline()
		if(temp.find("GNRMC") == 1):
			counter = counter + 1
			if(counter == int(config.get('general','discard_interval'))):
				insert(con, temp)
				t_print("Insertion succeeded!")
				counter = 0					
    dbClose(con)
            
if __name__ == '__main__':
    main_routine()
   
