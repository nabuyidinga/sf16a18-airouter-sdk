#!/bin/sh

##select no repeat alarm
crontab -l | grep "alarm_action.sh" | grep -F -v "*" | while read line
do
	now_year=$(date "+%Y");
	now_time=$(date "+%s")
	cron_date=$(echo "$line" | awk -v y=$now_year '{printf"%d-%d-%d",y,$4,$3}')
	cron_time=$(date -d "$cron_date" "+%s")
	cron_hour=$(echo "$line" | awk '{print $2}')
	let cron_time=cron_time+cron_hour*3600
	cron_minute=$(echo "$line" | awk '{print $1}')
	let cron_time=cron_tmie+cron_hour*60
	echo "cron_time:${cron_time}, now_time: ${now_time}" > /dev/ttyS0
	##31536000 = 365 * 24 * 3600
	##2592000 = 30 * 24 * 3600
	if [ $cron_time -lt $now_time ]  && [ $(expr $now_time - $cron_time - 31536000) -lt -2592000 ]; then
		md5=$(echo "$line" | awk '{print $10}')
		sed -i "/$md5/d" /etc/crontabs/admin
		/etc/init.d/cron restart
	fi
done
