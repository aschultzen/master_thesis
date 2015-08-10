# NOTES:
# Used Python v.3.4.3
# and mysql-connector-python-2.0.4-py3.4

import win32service
import win32serviceutil
import win32event 
import ctypes
import mysql.connector


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
    return result.read()
    close.result

def dbConnect():
    try:    
        dbConnection = mysql.connector.connect(host='10.1.0.232',database='clock_data',user='monitor',password='monitor')
        
        if dbConnection.is_connected():
            print("Connection to database (" + dbConnection.server_host +":" + str(dbConnection.server_port) + ") established")

    except Error as e:
            print(e)

    return dbConnection    

def dbClose(dbConnection):
    dbConnection.close()

    if dbConnection.is_connected():
            print("Connection to database (" + dbConnection.server_host +":" + str(dbConnection.server_port) + ") NOT closed") 
    else:
        print("Connection to database closed")

if __name__ == '__main__':
    #win32serviceutil.HandleCommandLine(PySvc)
    #somelist = path2list("C:\\test2.dat")
    #print (somelist)
    db_con = dbConnect()
    dbClose(db_con)