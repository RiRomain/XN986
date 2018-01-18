#!/bin/sh
limit_size=204800
error_log="/var/log/error_log"
error_log_bak=${error_log}.1
access_log="/var/log/access_log"
access_log_bak=${access_log}.1
log_size=`/usr/bin/wc -c ${error_log} | cut -d " " -f 1`
if [ $log_size -gt $limit_size ]; then cp ${error_log} ${error_log_bak};echo >${error_log};fi
log_size=`wc -c ${access_log} | cut -d " " -f 1`
if [ $log_size -gt $limit_size ]; then cp ${access_log} ${access_log_bak};echo >${access_log};fi

