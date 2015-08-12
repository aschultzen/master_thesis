## FIx date bug
## ADD "in progress" message
## TEst service 


# NOTES:
# Used Python v.3.4.3
# and mysql-connector-python-2.0.4-py3.4

import win32service
import win32serviceutil
import win32event 
import ctypes
import mysql.connector
import configparser
import datetime
import time

# Something should be done about these!
config = configparser.ConfigParser()    # Global variable for configparser.


class PySvc(win32serviceutil.ServiceFramework):
    # you can NET START/STOP the service by the following name
    _svc_name_ = "TimedataToDB"
    # this text shows up as the service name in the Service
    # Control Manager (SCM)
    _svc_display_name_ = "TimedataToDB"
    # this text shows up as the description in the SCM
    _svc_description_ = "LabvView time data to DB monitor."
    
    def __init__(self, args):
        win32serviceutil.ServiceFramework.__init__(self,args)
        # create an event to listen for stop requests on
        self.hWaitStop = win32event.CreateEvent(None, 0, 0, None)
    
    # core logic of the service   
    def SvcDoRun(self):
        import servicemanager

        rc = none
        while rc != win32event.WAIT_OBJECT_0:
            # This is the main loop. Put whatever
            # needs to be done in here.
            # block for 5 seconds and listen for a stop event
            rc = win32event.WaitForSingleObject(self.hWaitStop, 5000)
        
    # called when we're being shut down    
    def SvcStop(self):
        # tell the SCM we're shutting down
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)
        # fire the stop event
        win32event.SetEvent(self.hWaitStop)

# BUG: Does not care if file is
# bigger than memory. Could cause
# "memoutofbound"
def path2list(path):
    result = open(path, 'r')
    resultList = result.read()
    result.close()
    t_print("File " + path + " : loaded.")
    resultList = resultList.replace(",",".")
    return resultList

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
    rows = data.splitlines()
    row_count = len(rows)
    columns = (config['columns']['list'])
    columns = columns.split()
    sources = (config['sources']['list'])
    sources = sources.split()
    sources_count = len(sources)

    counter = 0
    operation_counter = 0
    while(counter < row_count):  
        source_counter = 0
        cell = rows[counter].split()
        cell[0] = format_date_string(cell[0])
        while(source_counter < sources_count):
            insert_measurement = ("INSERT INTO " + 
        config['db']['tablename'] + " " + 
        "(" + ''.join(columns) + ")" 
        + " VALUES (" + "'" + cell[0] + "'" + "," + "'" +  cell[1] + "'" + "," + cell[2] + "," +  "'" + sources[source_counter] + "'" + "," + cell[3 + source_counter] + ");")
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
    t_print("Inserted " + str(operation_counter) + " lines")

def initConfig():
    configFile = "config.ini"
    config.read(configFile)
    t_print("Config file " + configFile + " : loaded.")

def t_print(message):
    current_time = datetime.datetime.now().time()
    print("[" + current_time.isoformat() + "] " +"[" + message + "]")

def format_date_string(date_s):
    split = date_s.split(".")
    split = split[::-1]
    split = ''.join(split)
    return split


def update_progress(current, goal):
    progress = (current / goal) * 100
    print ("\rInserting lines: " + str(current) + "/" + str(goal),end="",flush=True)

if __name__ == '__main__':
    t_print("Starting up...")
    time_start = time.time()
    #win32serviceutil.HandleCommandLine(PySvc)
    initConfig()
    data = path2list(config['data']['path'])
    dbInsert(data)
    seconds = int(time.time() - time_start)
    t_print("Elapsed time: " + str(seconds) + "s")