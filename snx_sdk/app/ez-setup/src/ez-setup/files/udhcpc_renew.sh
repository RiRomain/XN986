#!/bin/sh

DEST="www.yahoo.com.tw"
DEST=$1
PID_FILE=$2
#PID=`cat /var/run/udhcpc.wlan0.pid`

#while [ : ]
#do
	if [ -e $2 ]
	then
		PID=`cat $2`
#		echo "udhcpc pid == $PID"
		if ! ping -q -w 2 -c 1 "$DEST" &>/dev/null # &> means both stdout+stderr
		then # ping failed
			echo "ping failed"
			kill -SIGUSR2 $PID
			sleep 2
			kill -SIGUSR1 $PID	
		fi
	fi
#done
