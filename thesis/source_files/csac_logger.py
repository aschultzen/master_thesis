import ctypes
import fileinput, sys
import datetime
import time
import io
import os
import serial
import jdutil

def get_today_mjd():
    today = datetime.datetime.utcnow()
    return jdutil.jd_to_mjd(jdutil.datetime_to_jd(today)) 

def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + str(current_time.isoformat()) + "] " +"[" + message + "]"
    print(complete_message)

def main_routine():
    log_file = open("dp.txt", "a+")
    t_print("Started CSAC logging script")
    ser = serial.Serial("/dev/ttyUSB0",57600, timeout=0.1)
    sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser),encoding='ascii',newline="\r")

    while(True):
        log_file = open("dp.txt", "a+")
        ser.write(b'^')
        time.sleep(0.1)
        telemetry = sio.readline()
        output = str(get_today_mjd()) + "," + telemetry
        log_file.write(output)
        log_file.close()
        time.sleep(1)

if __name__ == '__main__':
    main_routine()


