'''
:Author: Aril Schultzen
:Email: aschultzen@gmail.com 
'''

"""
GPS Logger requires:
- Python v.2.7
- python-mysqldb

EXPECTED TABLE
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
	mjd VARCHAR(50),
	alt DECIMAL(5,2),
	PRIMARY KEY (id) );
"""

import ctypes
import MySQLdb as mdb
import ConfigParser
import fileinput
import sys
import datetime
import time
import io
import os
import serial
import jdutil
from subprocess import call

config = ConfigParser.ConfigParser() 


def dbConnect():
    con = mdb.connect(config.get('db', 'ip'), config.get('db', 'user'),
                      config.get('db', 'password'), config.get('db', 'database'))
    return con


def dbClose(dbConnection):
    dbConnection.close()
    t_print("Connection to database closed")


def initConfig():
    configFile = "config.ini"
    config.read(configFile)


def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + str(
        current_time.isoformat(
        )) + "] " + "[" + message + "]"
    print(complete_message)


def format_date_string(date_s):
    split = date_s.split(".")
    split = split[::-1]
    split = ''.join(split)
    return split


def insert(con, data):
    st = data
    temp = st[12]
    checksum = temp[1] + temp[2] + temp[3]
    faa = temp[0]
    x = con.cursor()
    date = st[9][4:6] + st[9][2:4] + st[9][0:2]
    st[9] = date

    try:
        query = ("INSERT INTO " + config.get('db', 'table') +
        " (sensorID, fix_time, recv_warn, latitude, la_dir, longitude, lo_dir, ) " +
		"(speed, course, fix_date, variation, var_dir, faa, checksum, mjd, alt) VALUES " +
        "(" + config.get('general', 'sensorID') + "," + st[1] + ",'" + st[2] + 
		"'," + st[3] + ",'" + st[4] + "'," + st[5] + ",'" + st[6] + "','" + st[7] + 
		"','" + st[8] + "','" + st[9] + "','" + st[10] + "','" + st[11] + 
		"','" + faa + "','" + checksum + "'," + st[14] + ",'" + st[13] + "');")
        x.execute(query)
        con.commit()
    except:
        con.rollback()

# Function used to reset the serial configuration
# in Linux in case its mangled by something'


def reset_serial():
    call("stty -F " + config.get('gps', 'port') + " icanon", shell=True)


def get_today_mjd():
    today = datetime.datetime.utcnow()
    return jdutil.jd_to_mjd(jdutil.datetime_to_jd(today))


def main_routine():
    initConfig()
    t_print("GPS logger started!")
    reset_serial()
    con = dbConnect()
    counter = 0
    data = ""

    while(True):
        ser = serial.Serial(
            config.get('gps',
                       'port'),
            config.get('gps',
                       'baud'),
            timeout=0.1)
        sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser), newline="\r")
        time.sleep(1)
        while True:
            temp = sio.readline()
            if(temp.find("GNRMC") == 1):
                data = temp
                data = data.split(",")
                sio.readline()  # Reading forward manually
                temp = sio.readline()
                temp = temp.split(",")
                data.append(str(temp[9]))
                data.append(str(get_today_mjd()))
                counter = counter + 1
                if(counter == int(config.get('general', 'discard_interval'))):
                    insert(con, data)
                    counter = 0
    dbClose(con)

if __name__ == '__main__':
    main_routine()
