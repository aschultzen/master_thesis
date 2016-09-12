import ctypes
import fileinput, sys
import datetime
import time
import io
import os
import serial

def main_routine():
    ser = serial.Serial("/dev/ttyUSB0",57600, timeout=0.1)
    sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser),encoding='ascii',newline="\r")

    log_file = open("csac.txt", "a+")
    ser.write(b'^')
    time.sleep(0.1)
    telemetry = sio.readline()

    telemetry_len = 0

    while (telemetry_len < 50):
    	ser.write(b'^')
    	time.sleep(0.1)
    	telemetry = sio.readline()
	telemetry = telemetry.strip("\r\n")
        telemetry_len = len(telemetry)
	
    print(telemetry)        

if __name__ == '__main__':
    main_routine()

