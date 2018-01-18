
#cat asccii16x16.bin > /proc/isp/osd/0/font
#cat asccii16x16.bin > /proc/isp/osd/0/font

cat unicode16x16.bin > /proc/isp/osd/0/font
cat unicode16x16.bin > /proc/isp/osd/1/font

cat template.utf8 >/proc/isp/osd/0/template
cat template.utf8 >/proc/isp/osd/1/template

cat txt.utf8  >/proc/isp/osd/0/1/txt
cat txt.utf8  >/proc/isp/osd/1/1/txt

echo 0x0 > /proc/isp/osd/0/timestamp
echo 0x0 > /proc/isp/osd/1/timestamp

while true
do

#tm=`date "+%Y/%m/%d %H:%M:%S"`

#echo "$tm"

date "+%Y-%m-%d %H:%M:%S">/proc/isp/osd/0/0/txt
date "+%Y-%m-%d %H:%M:%S">/proc/isp/osd/1/0/txt

sleep 1

done



