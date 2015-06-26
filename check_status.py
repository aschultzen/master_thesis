# This is a simple script that uses a LED connected to
# GPIO PIN 12 to show the status of the NTP server.
#
# Description:
# Rapid blinking: PPS Source not connected
# Solid: PPS source marked as false ticker
# Periodic short blink: None of the above, all is well.

import RPi.GPIO as GPIO
import subprocess
import time
import commands
import os
from multiprocessing import Process, Value, Array

red_led = 12
sleep_t = 0.1
GPIO.setmode(GPIO.BCM)
GPIO.setup(red_led, GPIO.OUT)
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

def run_command(command):
    p = subprocess.Popen(command,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)
    return iter(p.stdout.readline, b'')

def error_led():
        counter = 10
        while (counter > 0):
                counter = counter -1
                GPIO.output(red_led,1)
                time.sleep(0.07)
                GPIO.output(red_led,0)
                time.sleep(0.07)

def clear_led():
        GPIO.output(red_led,0)

def solid_led():
        GPIO.output(red_led,1)

def blink_led():
         clear_led()
         time.sleep(0.1)
         solid_led()
         time.sleep(0.1)
         clear_led()

def pps_test():
    command = "ppstest /dev/pps0"
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    # Poll process for new output until finished
    start = time.time()
    counter = 0
    while (time.time() - start < 1):
        counter = counter + 1
        nextline = process.stdout.readline()
        if nextline == '' and process.poll() != None:
            break

    if((counter / 5) <= 1):
        return 0
    else:
        return 1        # Not connected to source, picking up noise/50Hz
        
def query_ntp():
    command = 'ntpq -p'.split()
    for line in run_command(command):
            sub = line.split()
            if any("xPPS" in s for s in sub): # Look for false tickers
                    return 1
    return 0

if __name__ == '__main__':
        while(1 > 0):
                if(pps_test() == 1):
                        error_led()
                elif(query_ntp() == 1):
                        solid_led()
                else:
                        blink_led()

                time.sleep(0.1)

