import ctypes
import fileinput, sys
import datetime
import time
import io
import os
import serial

def main_routine():
    ser = serial.Serial("/dev/ttyS0",57600, timeout=0.1)
    sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser),encoding='ascii',newline="\r\n")

    log_file = open("telemetry.txt", "a+")

    telemetry_len = 0
    while (telemetry_len < 60):
   	ser.write(b'!^\r\n')
    	time.sleep(0.01)
    	telemetry = sio.readline()
    	telemetry = telemetry.strip("\r\n\x00")
    	telemetry_len = len(telemetry)

    print(telemetry)
    ser.close()
    log_file.write(telemetry + "\n")	
if __name__ == '__main__':
    main_routine()

