#!/bin/bash          
#enable ADC driver
echo cape-bone-iio > /sys/devices/bone_capemgr.8/slots

#read rtc (ds1307) time and update system time
#ds1307 address - 68; its connected to BeagleBone Black throuch i2c
echo ds1307 0x68 > /sys/class/i2c-adapter/i2c-1/new_device
hwclock -s -f /dev/rtc1						#Set the System Time from the Hardware Clock
hwclock -w							#Set the Hardware Clock to the current System Time

# run temp_logger in the background
echo "Starting temp_logger:"
/home/root/programs/temp_logger/temp_logger &
