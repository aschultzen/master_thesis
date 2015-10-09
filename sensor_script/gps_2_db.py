# NOTES:
# Used Python v.2.7
# python-mysqldb

import ctypes
import MySQLdb as mdb
import ConfigParser
import fileinput, sys
import datetime
import time
import os
import serial

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
def insert(con, text):
	x = con.cursor()
	try:
   		x.execute("INSERT INTO " + config.get('db','table') + " VALUES " + text)
   		conn.commit()
	except:
   		con.rollback()

def main_routine():
    initConfig()	
    t_print("GPS 2 DB starting up")
    con = dbConnect()
    while(True):
        time_start = time.time()
	ser = serial.Serial(config.get('gps','port'),config.get('gps','baud'),timeout=1)
	while 1:
   		temp = ser.readline()
		if(temp.find("GPRMC") == 1):
			print(temp)
			insert(con, temp)			
        seconds = "{0:.2f}".format(float(time.time() - time_start))
        t_print("Elapsed time: " + str(seconds) + "s")
    dbClose(con)
            
if __name__ == '__main__':
    main_routine()
   
