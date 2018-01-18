#!/bin/sh

ifconfig eth0 down

modprobe lc_ltencm
modprobe lc_ltetty

#insmod lc_ltencm.ko
#insmod lc_ltetty.ko

main_lc5761  /dev/ttyUSB2 "AT+CFUN=1"
usleep 500000
main_lc5761  /dev/ttyUSB2 "AT^DUSIMR=1"
usleep 500000
main_lc5761  /dev/ttyUSB2 "AT+COPS?"
usleep 500000
main_lc5761  /dev/ttyUSB2 "AT+CREG?"
usleep 500000
main_lc5761  /dev/ttyUSB2 "AT+CGREG?"
usleep 500000
main_lc5761  /dev/ttyUSB2 "AT+CSQ"
usleep 500000
main_lc5761  /dev/ttyUSB2 "AT+CGACT=1,1"
usleep 500000
main_lc5761  /dev/ttyUSB2 "AT+CGDATA=\"M-0000\",1"
usleep 500000

ifconfig eth0 down
sleep 1

ifconfig usb0 up

sleep 4

dmesg > /tmp/ipconfig.txt

grep "lient_ip" /tmp/ipconfig.txt > /tmp/ip.txt

IP=`sed 's/^.*=//g' /tmp/ip.txt`

grep "gateway_ip" /tmp/ipconfig.txt > /tmp/gw.txt

GWIP=`sed 's/^.*=//g' /tmp/gw.txt`

ifconfig usb0 $IP

usleep 500000

echo "nameserver 221.179.38.7" > /etc/resolv.conf
echo "nameserver 120.196.165.7" >> /etc/resolv.conf
route add default gw $GWIP dev usb0

sleep 1

ifconfig usb0
