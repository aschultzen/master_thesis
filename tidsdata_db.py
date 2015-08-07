import win32service
import win32serviceutil
import win32event
import ctypes  # An included library with Python install.

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
        
if __name__ == '__main__':
    win32serviceutil.HandleCommandLine(PySvc)
