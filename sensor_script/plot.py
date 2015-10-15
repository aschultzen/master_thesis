# NOTES:
# Used Python 3
# mysql-connector-python3 (FEDORA)
# python3-matplotlib (FEDORA)

import ctypes
import mysql.connector
import configparser
import fileinput, sys
import datetime
import time
import os
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

config = configparser.ConfigParser()   # Global variable for configparser.

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
			t_print("Connection to database (" + dbConnection.server_host +":" + str(dbConnection.server_port) + ") established")

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
    complete_message = "[" + str(current_time.isoformat()) + "] " +"[" + message + "]"
    print(complete_message)

#def latlon_to_ecef(data):
	#LAT = latitude * pi/180
    #LON = longitude * pi/180
    #x = -R * cos(LAT) * cos(LON)
    #y =  R * sin(LAT) 
    #z =  R * cos(LAT) * sin(LON)
 
def plot(data):
	fig = plt.figure()
	ax = fig.add_subplot(111, projection='3d')

	x =[1,2,3,4,5,6,7,8,9,10]
	y =[5,6,2,3,13,4,1,2,4,8]
	z =[2,3,3,3,5,7,9,11,9,10]

	ax.scatter(x, y, z, c='r', marker='o')

	ax.set_xlabel('X Label')
	ax.set_ylabel('Y Label')
	ax.set_zlabel('Z Label')

	plt.show()
	
def get_data(con):
	query = "select latitude,longitude,la_dir,lo_dir from gprmc;"
	cursor = con.cursor()
	cursor.execute(query)
	latitude = []
	longitude = []
	rows = cursor.fetchall()
	#rows[0][0] = Latitude
	#rows[0][1] = Longitude

	q_length = cursor.rowcount
	x = 0
	while(x < q_length-1):
		temp = str(rows[x][0])
		length = len(temp) -1
		p_point = temp.find(".") - 2
		post_p = temp[(p_point):]
		offsett = length - (length - p_point)
		pre_p = temp[:offsett]
		post_p = float(post_p)
		post_p = post_p/60
		result = float(pre_p) + post_p
		if(rows[x][2] != "N"):	#Checking la_dir
			result = result * -1
		latitude.append(result)
		x = x + 1

	x = 0	
	while(x < q_length-1):
		temp = str(rows[x][1])
		length = len(temp) -1
		p_point = temp.find(".") - 2
		post_p = temp[(p_point):]
		offsett = length - (length - p_point)
		pre_p = temp[:offsett]
		post_p = float(post_p)
		post_p = post_p/60
		result = float(pre_p) + post_p
		if(rows[x][3] != "W"):	#Checking la_dir
			result = result * -1
		longitude.append(result)
		x = x + 1

	print(q_length)	

	cursor.close()
	con.close()

def main_routine():
	initConfig()
	con = dbConnect();
	get_data(con)

if __name__ == '__main__':
    main_routine()