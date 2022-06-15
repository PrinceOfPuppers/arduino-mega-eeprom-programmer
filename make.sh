#! /bin/bash
arduino-cli compile --fqbn "arduino:avr:mega" eeprom-programmer.ino

EXITCODE=$?
if [ $EXITCODE -eq 0 ]; then
    arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega eeprom-programmer
    cat /dev/ttyACM0
fi
