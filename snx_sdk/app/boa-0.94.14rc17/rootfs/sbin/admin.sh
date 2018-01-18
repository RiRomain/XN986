# /bin/sh
# 启动Web Admin Service
# 默认使用日文，否则使用中文
#  李志杰 2006.07.25
# 使用的方式为"admin.sh JA/zh_CN.GB2312"

echo "Start WebAdmin Servers"

WEB_ADMIN_MODULES="boa"

export LC_ALL=

WEB_ADMIN_LOCALE=/etc/boa/admin.locale
if [ ! -f "$WEB_ADMIN_LOCALE" ] ; then
	echo " Locale Defination for Web Admin is missed, Default is \"Chinese\"" >& 2
else
	. $WEB_ADMIN_LOCALE
fi


if [ $ADMIN_LOCALE = "JA" ] ; then
	LC_ALL=ja
	echo "     web language: \"Japanese\""
else 
	if [ $ADMIN_LOCALE = "zh_CN.GB2312" ] ; then
		LC_ALL=zh_CN.GB2312
		echo "     Default web language: \"Chinese\""
	else
		LC_ALL=C
		echo "     Default web language: \"English\""
	fi
fi

echo "     Startup \"$WEB_ADMIN_MODULES\" ....."
mkdir -p /tmp/log/boa
for WEB_ADMIN in $WEB_ADMIN_MODULES; do
	name=$(eval "echo /tmp/log/boa/error_log")
	echo "        Startup \"$WEB_ADMIN\" module ....."
	touch    $name
	echo 0 > $name
	name=$(eval "echo /tmp/log/boa/access_log")
	touch    $name
	echo 0 > $name

	/usr/bin/boa -f /etc/boa/boa.conf 
done
echo "     Startup All WebAdmin Servers Successfully!"
echo ""
echo ""
