#!/bin/bash

set -e

#source ~/.espressif/tools/activate_idf_v6.1.sh 

# idf.py set-target esp32c3 : select esp32c3
# idf.py menuconfig : create sdk file

idf.py build

idf.py -p /dev/ttyACM0 flash

idf.py -p /dev/ttyACM0 monitor
