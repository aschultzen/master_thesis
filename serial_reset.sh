#! /bin/bash
echo Resetting ttyACM0
stty -F /dev/ttyACM0 icanon
echo Done

echo Resetting ttyACM1
stty -F /dev/ttyACM1 icanon
echo Done

