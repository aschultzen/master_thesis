#! /bin/bash
echo Resetting ttyACM0
stty -F /dev/ttyACM0 icanon
echo Done

echo Resetting ttyS0
stty -F /dev/ttyS0 icanon
echo Done

