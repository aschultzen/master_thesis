#! /bin/bash
echo Resetting ttyACM0
stty -F /dev/ttyACM0 icanon
echo Done

echo Resetting ttyUSB0
stty -F /dev/ttyUSB0 icanon
echo Done

