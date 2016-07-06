import mysql.connector
from lmmm_common import (t_print, get_today_date, get_today_mjd,get_now_time)

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
	cursor = db_con.cursor();

	add_measurement = ("INSERT INTO clock_measurements"
              "(date, time, mjd, source, value, ref_clock, measurerID) "
              "VALUES (%(date)s, %(time)s, %(mjd)s, %(source)s, %(value)s, %(ref_clock)s, %(measurerID)s)")

	cursor.execute(add_measurement, data_measurement)

	# Make sure data is committed to the database
	db_con.commit()
	cursor.close()