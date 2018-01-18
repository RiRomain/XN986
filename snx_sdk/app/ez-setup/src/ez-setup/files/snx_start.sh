#!/bin/sh

echo "snx_start.sh"
minissdpd -i wlan0
cstreamer -i wlan0 -c /etc/SNIP39/EyeOn39.conf -p http://121.199.65.105:80/v1 http://121.199.65.105:80/v1 &
