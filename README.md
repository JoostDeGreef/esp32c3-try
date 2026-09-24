
This project uses IDF.

source ~/.espressif/tools/activate_idf_v6.1.sh 

Do this once for new targets (existing target is esp32c3)
idf.py set-target esp32c3 : select esp32c3
idf.py menuconfig : create sdk file

build:
idf.py build

flash/upload:
idf.py -p /dev/ttyACM0 flash

open serial monitor:
idf.py -p /dev/ttyACM0 monitor
