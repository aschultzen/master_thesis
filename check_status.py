import RPi.GPIO as GPIO
import subprocess
import time
import commands
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
        while (1 > 0):
                GPIO.output(red_led,1)
                time.sleep(sleep_t)
                GPIO.output(red_led,0)
                time.sleep(sleep_t)

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

def check_status():
        command = 'ntpq -p'.split()
        for line in run_command(command):
                sub = line.split()
                if any("xPPS" in s for s in sub): # Look for false tickers
                        return 1
        return 0

if __name__ == '__main__':
        while(1 > 0):
                if(check_status() == 1):
                        solid_led()
                else:
                        blink_led()

                time.sleep(0.5)
