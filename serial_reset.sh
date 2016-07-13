#! /bin/bash
echo Resetting ttyACM0
stty -F /dev/ttyACM0 icanon
echo Done

echo Resetting ttyUSB1
stty -F /dev/ttyUSB1 icanon
echo Done

