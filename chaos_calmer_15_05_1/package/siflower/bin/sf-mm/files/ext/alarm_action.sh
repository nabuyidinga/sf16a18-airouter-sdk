#! /bin/sh
# $1 : object
# $2 : event
# $3 : delay
# $4 : md5sum (may not exist)
echo "alarm_action object : ${1}" > /dev/ttyS0
echo "alarm_action event : ${2}" > /dev/ttyS0
echo "alarm_action delay : ${3}" > /dev/ttyS0
echo "alarm_action year : ${4}" > /dev/ttyS0
echo "alarm_action vid : ${5}" > /dev/ttyS0
if [ $3 -ne 0 ] ; then
	sleep $3
fi

time_text=""
obj=""
event=""
text=""
cron_string=""
search=""
hour=$(date "+%H")
if [ $hour -le 11 ];then
	time_text="上午"
elif [ $hour -le 13 ]; then
	time_text="中午"
else
	time_text="下午"
	let hour=hour-12
fi
time_text="${time_text}${hour}点$(date "+%M")分"

if [ "$1" == "闹钟" ]; then
	obj="闹钟"
elif [ "$1" == "invalid" ]; then
	obj="提醒"
else
	obj="$1"
fi

if [ "$obj" != "闹钟" ] && [ "$2" != "invalid" ]; then
	event="，内容是${2}"
fi

text="现在是${time_text}，你有一个${obj}时间到了${event}"
ubus call sf_mm_ubus alarm_notify "{\"text\":\"${text}\"}"
if [ ! -z "$5" ]; then
	crontab -l | grep "$5" | grep -qF "*"
	##not a repeater alarm, delete it
	if [ $? -eq 1 ] ;then
		sed -i "/$5/d" /etc/crontabs/admin
		/etc/init.d/cron restart
	fi
fi
