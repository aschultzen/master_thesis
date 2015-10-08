# NOTES:
# Used Python v.3.4.3
# and mysql-connector-python-2.0.4-py3.4

import ctypes
import mysql.connector
import configparser
import fileinput, sys
import datetime
import time
import jdutil
import os

# Something should be done about these!
config = configparser.ConfigParser()    # Global variable for configparser.

def dbConnect():
    try:    
        dbConnection = mysql.connector.connect(
            host= config['db']['ip'],
            database= config['db']['database'],
            user= config['db']['user'],
            password= config['db']['password'],
            autocommit=True
            )
        
        if dbConnection.is_connected():
            t_print("Connection to database (" + dbConnection.server_host +":" 
                + str(dbConnection.server_port) + ") established")

    except Error as e:
            t_print(e)

    return dbConnection    

def dbClose(dbConnection):
    dbConnection.close()

    if dbConnection.is_connected():
            t_print("Connection to database (" + dbConnection.server_host +":" 
                + str(dbConnection.server_port) + ") NOT closed") 
    else:
        t_print("Connection to database closed")

def initConfig():
    configFile = "config.ini"
    config.read(configFile)
    #t_print("Config file " + configFile + " : loaded.")

def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + current_time.isoformat() + "] " +"[" + message + "]"
    print(complete_message)
    if(config['logs']['enable_logging'] == "yes"):
        with open(config['logs']['log_path'], "a+") as log:
            log.write(complete_message + "\n")

def format_date_string(date_s):
    split = date_s.split(".")
    split = split[::-1]
    split = ''.join(split)
    return split

def update_progress(current, goal):
    progress = (current / goal) * 100
    print ("\r      Inserting lines: " + str(current) + "/" + str(goal),end="",flush=True)

def get_today_mjd():
    today = datetime.datetime.utcnow()
    return jdutil.jd_to_mjd(jdutil.datetime_to_jd(today))

def main_routine():
    while(True):
        initConfig()
        print("\n")

        t_print("GPS 2 DB starting up")
        time_start = time.time()

        seconds = "{0:.2f}".format(float(time.time() - time_start))
        t_print("Elapsed time: " + str(seconds) + "s")
        time.sleep(float(config['general']['interval']))
            
if __name__ == '__main__':
    main_routine()
   