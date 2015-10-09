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

def main_routine():
    initConfig()	
    t_print("GPS 2 DB starting up")
    con = dbConnect()
    while(True):

        time_start = time.time()
	t_print("Do something here")
	time.sleep(1)	
        seconds = "{0:.2f}".format(float(time.time() - time_start))
        t_print("Elapsed time: " + str(seconds) + "s")

    dbClose(con)
            
if __name__ == '__main__':
    main_routine()
   
