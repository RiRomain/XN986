#!/bin/sh

ifconfig eth0 down

script_dir=`cd "$(dirname "$0")";pwd`

if [ -d "/etc/ppp" ]
then
	cp -rf $script_dir/configs/chat /etc/ppp/
	cp -rf $script_dir/configs/ppp/peers/* /etc/ppp/peers/
fi

while :
do
	if [ -e "/dev/ttyACM0" ]
	then
		while :
		do
			$script_dir/ATtest /dev/ttyACM0 "AT+COPS?" > /tmp/ATresult.txt
			ATCMD=`sed -n 's/^+COPS: //p' /tmp/ATresult.txt `
			SIMTYPE=`echo ${ATCMD##*,}`
			if [ $SIMTYPE -eq 2 ]
			then
				cd /etc/ppp/peers
				pppd call cmnet
				sleep 8
				IPADDR=`/sbin/ifconfig ppp0 |sed -n '/inet addr/s/^[^:]*:\([0-9.]\{7,15\}\) .*/\1/p'`
				cd $script_dir
				if [ $IPADDR != "" ]
				then
					echo "IPADDRESS: $IPADDR" 
					break
				fi
			fi
			sleep 1
		done
		break
	fi
	sleep 1
done

ifconfig eth0 down
sleep 1

ifconfig ppp0
