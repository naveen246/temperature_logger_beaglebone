
#run this script once to call temp_logger_init.sh everytime on startup
#also add capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN to uEnv.txt to disable HDMI capes

chmod u+x /home/root/temp_logger/scripts/temp_logger_init.sh
cp /home/root/temp_logger/scripts/temp_logger_init.service /lib/systemd/

ln /lib/systemd/temp_logger_init.service /etc/systemd/system/temp_logger_init.service

systemctl daemon-reload
systemctl start temp_logger_init.service
systemctl enable temp_logger_init.service

#to add usb_automount rules; find usb drive uuid using - ls /dev/disk/by-uuid
mkdir /media/usb_device
cp /home/root/temp_logger/scripts/usb_automount.rules /etc/udev/rules.d/
udevadm control --reload-rules