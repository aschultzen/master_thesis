import mysql.connector
import time
from lmmm_common import (t_print, get_today_date, get_today_mjd,get_now_time)

def db_connect(c_parser):
    try:    
        dbConnection = mysql.connector.connect(
            host= c_parser.get('db','ip'),
            database= c_parser.get('db','database'),
            user= c_parser.get('db','user'),
            password= c_parser.get('db','password'),
            connection_timeout =0.1,
            autocommit=True
            )
        
        if dbConnection.is_connected():
            return dbConnection
        else:
        	return 0

    except mysql.connector.errors.InterfaceError as e:
            return 0

# Connects to the database using Connect()
# Implements error checking and retry 
def db_connector(cp):
	# Connects to database
	connection_attempts = 1
	connection_attempts_max = int(cp.get('db','connection_attempts_max'))
	db_con = db_connect(cp)
	while( db_con == 0 ):
		time.sleep(1)
		t_print("DB connection attempt " + str(connection_attempts) + " failed.")
		db_con = db_connect(cp)
		connection_attempts = connection_attempts + 1
		if(connection_attempts_max > 1 and connection_attempts > connection_attempts_max):
			t_print("Reached maximum attempts at connecting to DB. Aborting")
			return 0

	t_print("Connection to database established after "
			+ str(connection_attempts) + " attempt(s)")
	return db_con	

def dbClose(dbConnection):
    dbConnection.close()

    if dbConnection.is_connected():
    	return 0
    else:
    	return 1 

def create_query(config_parser, switch_info, measurement):
	measurement = measurement.rstrip("\r\n")

	data_measurement = {
		'date': get_today_date(),
		'time': get_now_time(),
		'mjd' : get_today_mjd(),
		'source': switch_info[0],
		'value' : measurement,
		'ref_clock': config_parser.get('general','ref_clock'),
		'measurerID': config_parser.get('general','measurerID'),
	}
	return data_measurement

def upload(db_con, data_measurement):
	error_string = "Connection to DB was down!"
	try:
		cursor = db_con.cursor();
	except mysql.connector.errors.OperationalError:
		t_print(error_string)
		return 0
	
	add_measurement = ("INSERT INTO clock_measurements"
              "(date, time, mjd, source, value, ref_clock, measurerID) "
              "VALUES (%(date)s, %(time)s, %(mjd)s, %(source)s, %(value)s, %(ref_clock)s, %(measurerID)s)")

	try:
		cursor.execute(add_measurement, data_measurement)
	except mysql.connector.errors.OperationalError:
		t_print(error_string)
		cursor.close()
		return 0

	# Make sure data is committed to the database
	try:
		db_con.commit()
	except mysql.connector.errors.OperationalError:
		t_print(error_string)
		return 0

	cursor.close()
	return 1