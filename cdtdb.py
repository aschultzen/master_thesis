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

# BUG: Does not care if file is
# bigger than memory. Could cause
# "memoutofbound"

def insert_file(path):
    result = open(path, 'r')
    resultList = result.read()
    result.close()
    t_print("File " + path + " : loaded.")
    resultList = resultList.replace(",",".")
    resultList = resultList.splitlines()
    dbInsert(resultList)

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

def dbInsert(data):
    dbc = dbConnect()
    cursor = dbc.cursor(buffered=True)
    #rows = data.splitlines()
    row_count = len(data)
    columns = (config['columns']['list'])
    columns = columns.split()
    sources = (config['sources']['list'])
    sources = sources.split()
    sources_count = len(sources)

    counter = 0
    operation_counter = 0
    while(counter < row_count):  
        source_counter = 0
        cell = data[counter].split()
        cell[0] = format_date_string(cell[0])
        while(source_counter < sources_count):
            insert_measurement = ("INSERT INTO " + 
        config['db']['tablename'] + " " + 
        "(" + ''.join(columns) + ")" 
        + " VALUES (" + "'" + cell[0] + "'" + "," + "'" +  cell[1] + "'" + "," + cell[2] + "," +  "'" + sources[source_counter] + "'" + "," + cell[3 + source_counter] + "," +  "'" + config['data']['ref_clock'] + "'" + "," +  "'" + config['data']['measurerID'] + "'" + ");")
            try: 
                cursor.execute(insert_measurement)
                dbc.commit
                source_counter = source_counter + 1
                operation_counter = operation_counter + 1
                update_progress(operation_counter, row_count*22)
            except mysql.connector.Error as err:
                print("Something went wrong: {}".format(err))
        counter = counter + 1
    print()
    cursor.close
    dbClose(dbc)
    t_print("Inserted " + str(operation_counter) + " lines into DB (" + str(row_count) + " rows in file)" )

def initConfig():
    configFile = "config.ini"
    config.read(configFile)
    #t_print("Config file " + configFile + " : loaded.")

def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + current_time.isoformat() + "] " +"[" + message + "]"
    print(complete_message)
    if(config['logs']['enabled'] == "yes"):
        with open(config['logs']['path'], "a+") as log:
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

def calculate_file_name():
    start_mjd = ( int(int(get_today_mjd()) / 60) * 60 )
    stop_mjd = start_mjd + 59;
    filename = config['files']['file_prefix'] + " " + str(start_mjd) + " - " + str(stop_mjd) + ".dat"
    return filename

def get_full_path():
    calculate_file_name()
    return str(config['files']['folder'] + calculate_file_name())

def get_last_db_line_mjd():
    dbc = dbConnect()
    cursor = dbc.cursor(buffered=True)
    query = "SELECT mjd FROM clock_measurements ORDER BY clck_msrmID DESC LIMIT 1"
    cursor.execute(query)
    result = cursor.fetchall()
    dbClose(dbc)
    if(result): 
        return result[0][0]
    else:
        return -1
    
    ## Changed open with to 'r'
def find_new_lines(db_last_mjd):
    file_full_path = get_full_path()
    last_db_mjd = str(db_last_mjd).replace(".",",")

    line_found_switch = False;
    new_lines = []

    t_print("Opening file: " + file_full_path)
    with open(file_full_path, 'r') as openfile: 
        for line in openfile:
            if(line_found_switch == True):
                new_lines.append(line.replace(",","."))
            if line.find(last_db_mjd) != -1:
                line_found_switch = True

    t_print("Found " + str(len(new_lines)) + " new line(s), base is up to date!")
    new_lines = new_lines            
    return new_lines

def disable_file_insert():
    for line in fileinput.input(["config.ini"], inplace=True):
        line = line.replace("file_insert: 1", "file_insert: 0")
        sys.stdout.write(line)

def main_routine():
    time_start = time.time()
    initConfig()
    t_print("Starting up...")

    
    if(config['modes']['file_insert'] == "1"):
        if(config['modes']['are_you_sure_you_want_to_insert'] == "1"):
            insert_file(config['files']['insert_mode_path'])
            disable_file_insert()

    else:
        last_mjd = get_last_db_line_mjd()
        if(last_mjd != -1):
            new_lines = find_new_lines(last_mjd)
            if(len(new_lines) > 0):
                dbInsert(new_lines)
        else:
            t_print("The DB is empty. Inserting the whole file")
            insert_file(get_full_path())

    seconds = "{0:.2f}".format(float(time.time() - time_start))
    t_print("Elapsed time: " + str(seconds) + "s")
            
if __name__ == '__main__':
    main_routine()
   