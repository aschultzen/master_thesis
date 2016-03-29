#! /bin/bash
echo Resetting ttyACM0
stty -F /dev/ttyACM0 icanon
echo Done

