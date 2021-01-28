#!/usr/bin/bash

alias compile='arduino-cli compile --fqbn arduino:avr:uno task3'
alias upload='arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno task3'
