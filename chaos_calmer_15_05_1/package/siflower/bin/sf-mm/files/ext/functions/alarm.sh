#!/bin/sh

alarm_file_lock="/tmp/.alarm_file_lock"
check_alarm_file_lock() {
	local tmp=0
	while [ $tmp -lt 50 ]; do
		if [ -f "$alarm_file_lock" ]; then
			#sleep 20ms
			let tmp=tmp+1
			msleep 20
		else
			return 0
		fi
	done
	return 1
}
lock_alarm_file_lock() {
	touch $alarm_file_lock
}
unlock_alarm_file_lock() {
	rm $alarm_file_lock
}

check_invalid() {
	params="$1"
	start_char=${params:0:1}
	if [ "$start_char" = "#" ]; then
		return 1
	else 
		return 0
	fi
}

check_date() {
	time=$1
	date=$2

	if [ $(echo "$time" | grep -c "<") -ne 0 ]; then
		echo "unsupport time $time" > /dev/ttyS0
		return 1
	fi

	if [ $(echo "$date" | grep -c "<") -ne 0 ]; then
		echo "unsupport date $date" > /dev/ttyS0
		return 2
	fi
	return 0
}

start_del_alarm_timer(){
	del_alarm="0 * * * * /usr/sbin/del_alarm.sh"
	if [ $(crontab -l | grep -Fc "$del_alarm") -eq 0 ]; then
		echo "$del_alarm" >> etc/crontabs/admin
	fi
}

__set_alarm() {
	json_get_vars event object date period time repeat 
	json_get_vars festival relativeTime timeLocation dateChina solarterm date1 date2
	local second=*
	local minute=*
	local hour=*
	local dom=*
	local month=*
	local year=*
	local dow=*
	local crontab_info
	local start_period

	local now_date=$(date +"%Y%m%d")
	local now_time=$(date +"%H%M%S" | awk '{printf"%d", $1}')
	local tmp
	local text=""
	local response=""

	echo "event is  $event" > /dev/ttyS0
	echo "object is $object" > /dev/ttyS0
	echo "date is $date" > /dev/ttyS0
	echo "period is $period" > /dev/ttyS0
	echo "time is $time" > /dev/ttyS0
	echo "festival is $festival" > /dev/ttyS0
	echo "repeat is $repeat" > /dev/ttyS0
	echo "relativeTime is $relativeTime " > /dev/ttyS0

	check_date $time $date
	if [ $? -ne 0 ]; then
		response="暂不支持多个时间设置闹钟哦"
		common_json_output_status "error" $response
		return 0
	fi

	check_invalid $event
	if [ $? -eq 1 ]; then
		event="invalid"
	fi

	check_invalid $object
	if [ $? -eq 1 ]; then
		object="invalid"
	fi

	check_invalid $relativeTime
	if [ $? -eq 1 ]; then
		check_invalid $repeat
		if [ $? -eq 1 ]; then
			repeat="invalid"
		fi
		case "$repeat" in
			invalid)
				#"20181201<20181202"
				check_invalid $date
				if [ $? -ne 1 ]; then
					if [ $date -lt $now_date ]; then
						response="主人，你说的我做不到呀，我可没法穿越时空去提醒你"
						common_json_output_status "error" "$response"
						return 0
					fi

					dom=$(echo ${date:6:2} | awk '{printf"%d",$1}')
					month=$(echo ${date:4:2} | awk '{printf"%d",$1}')
					year=$(echo ${date:0:4} | awk '{printf"%d",$1}')
					dow=$(date -d "$year-$month-$dom" +"%w")
				else
					date=""
				fi
				;;
			everyday)
				dom=*
				month=*
				year=*
				dow=*
				;;
			w*)
				#w1|w2|w3|w4|w5
				dom=*
				month=*
				year=*
				dow=${repeat#w}
				;;
			d*)
				month=*
				year=*
				dow=*
				dom=${repeat#d}
				;;
		esac
		echo "dom is $dom" > /dev/ttyS0
		echo "month is $month" > /dev/ttyS0
		echo "year is $year" > /dev/ttyS0
		echo "dow is $dow" > /dev/ttyS0

		check_invalid $time
		if [ $? -eq 1 ]; then
			check_invalid $period
			if [ $? -eq 0 ]; then
				response="暂不支持多个时间设置闹钟哦"
			else
				response="主人,你说的时间已经超出了我的理解范围"
			fi
			common_json_output_status "error" "$response"
			return 0
		fi

		hour=$(echo ${time:0:2} | awk '{printf"%d", $1}')

		minute=$(echo ${time:3:2} | awk '{printf"%d", $1}')

		second=$(echo ${time:6:2}| awk '{printf"%d", $1}')

		if [ $hour -gt 24 ]; then
			error=1
			response="主人，你说的时间超出我的理解范围了呀"
			common_json_output_status "error" "$response"
			return 0
		fi



		check_invalid $period
		if [ $? -eq 1 ]; then
			period="invalid"
		fi
		if [ "$period" != "invalid" ]; then
			start_period=$(echo $period | awk -F '-' '{printf"%d", $1}')
			if [ $start_period -eq 12 ]; then
				hour=`expr ${hour} + ${start_period}`
			fi
			if [ $start_period -eq 18 ]; then
				hour=`expr ${hour} + 12`
			fi
		fi

		echo "after process , hour is $hour" > /dev/ttyS0
		if [ $hour -eq 24 ]; then
			response="当前不支持此种查询方式，请换一种方式再试"
			common_json_output_status "error" $response
			return 0
		elif [ $hour -gt 24 ]; then
			response="主人，你说的时间超出我的理解范围了呀"
			common_json_output_status "error" "$response"
			return 0
		fi

		## only time, unkown date
		if [ "$date" == "" ];then
			tmp=$(echo "" | awk -v h=$hour -v m=$minute -v s=$second '{printf"%d%02d%02d",h,m,s}')
			if [ $tmp -lt $now_time ]; then
				let tmp=tmp+120000
				## time has changed, we should get new value for hour,minute and second.
				if [ $tmp -gt $now_time ] && [ $tmp -lt 240000 ]; then
					date=$(date "+%Y%m%d")
					hour=$(echo ${tmp:0:2} | awk '{printf"%d",$1}')
					minute=$(echo ${tmp:2:2} | awk '{printf"%d",$1}')
					second=$(echo ${tmp:4:2} | awk '{printf"%d",$1}')
				fi
			fi
			##let day+1
			if [ "$date" == "" ]; then
				date=$(date "+%Y%m%d")
				let date=date+1
				date=$(date -d "${date:0:4}-${date:4:2}-${date:6:2}" "+%Y%m%d%w")
			fi

			dow=$(echo ${date:7:6} | awk '{printf"%d",$1}')
			dom=$(echo ${date:6:2} | awk '{printf"%d",$1}')
			month=$(echo ${date:4:2} | awk '{printf"%d",$1}')
			year=$(echo ${date:0:4} | awk '{printf"%d",$1}')
		fi
		##

		echo "hour is $hour" > /dev/ttyS0
		echo "minute is $minute" > /dev/ttyS0
		echo "second is $second " > /dev/ttyS0

		tmp=$(echo "" | awk -v h=$hour -v m=$minute -v s=$second '{printf"%d%02d%02d",h,m,s}')
		if [ $date -eq $now_date ]; then
			if [ $tmp -lt $now_time ]; then
				response="主人，你说的我做不到呀，我可没法穿越时空去提醒你"
				common_json_output_status "error" "$response"
				return 0
			fi
		fi


	else
		## relativeTime has a value
		## get now time and relative time
		hour=$(date "+%H")
		minute=$(date "+%M")
		second=$(date "+%S")
		relative_hour=$(echo "$relativeTime" | awk -F ':' '{printf"%d",$1}')
		relative_minute=$(echo "$relativeTime" | awk -F ':' '{printf"%d",$2}')
		relative_second=$(echo "$relativeTime" | awk -F ':' '{printf"%d",$2}')
		if [ $relative_hour -eq 0 ] && [ $relative_minute -eq 0 ] && [ $relative_second -gt 0 ]; then
			response="当前不支持此种方式，请换一种方式再试"
		else
			response="好的，主人,我会在"
			if [ $relative_hour -gt 0 ]; then	
				response="${response}${relative_hour}小时"
			fi
			if [ $relative_minute -gt 0 ]; then
				response="${response}${relative_minute}分钟"
			fi

			if [ "$event" == "invalid" ]; then
				response="${response}后提醒你"
			else
				response="${response}后提醒你${event}"
			fi
		fi
		## now get the real time
		let minute=minute+relative_minute
		if [ $minute -gt 60 ];then
			let minute=minute-60
			let hour=hour+1
		fi
		let hour=hour+relative_hour
		if [ $hour -ge 24 ]; then
			let hour=hour-24
			let now_date=now_date+1
			##use -d 2018-12-32 convet to standard format
			now_date=$(date -d "${now_date:0:4}-${now_date:4:2}-${now_date:6:2}" "+%Y%m%d%w")
		fi

		dow=$(echo ${now_date:7:6} | awk '{printf"%d",$1}')
		dom=$(echo ${now_date:6:2} | awk '{printf"%d",$1}')
		month=$(echo ${now_date:4:2} | awk '{printf"%d",$1}')
		year=$(echo ${now_date:0:4} | awk '{printf"%d",$1}')
	fi

	# m h dom mon dow user  command
	#crontab_info="${minute}"" ""${hour}"" ""${dom}"" ""${month}"" ""${dow}"" /usr/sbin/alarm_action.sh"
	crontab_info="${minute} ${hour} ${dom} ${month} ${dow} /usr/sbin/alarm_action.sh ${event} ${object}"

	#date text
	if [ "$month" != "*" ]; then	
		text=$(echo -n "${text}${month}月")	
		if [ "$dom" != "*" ]; then
			text=$(echo -n "${text}${dom}日")	
		fi
	else
		if [ "$dom" != "*" ]; then
			text=$(echo -n "${text}每个月${dom}日")	
		elif [ "$dow" != "*" ]; then
			if [ $dow -ne 7 ]; then
				text=$(echo -n "${text}每周${dow}")
			else
				text=$(echo -n "${text}每周日")
			fi
		else
			text=$(echo -n "${text}每天")
		fi
	fi
	#time text
	if [ $hour -lt 11 ]; then
		period="上午"
	elif [ $hour -le 13 ]; then
		period="中午"
	else
		period="下午"
	fi
	if [ $hour -ge 13 ]; then	
		let hour=hour-12
	fi
	text=$(echo -n "${text}${period}${hour}点")
	if [ $minute -ne 0 ]; then
		text=$(echo -n "${text}${minute}分")
	fi


	if [ "$event" == "invalid" ]; then
		event=""
	fi

	search_string=$(echo "$crontab_info" | sed "s/\*/\\\*/g")
	echo $search_string > /dev/ttyS0
	if [ $(crontab -l | grep -c "${search_string}") -ne 0 ];then
		response="你在${text}已有一个${object}"
	else
		check_alarm_file_lock
		if [ $? -eq 0 ]; then
			lock_alarm_file_lock
			echo "$crontab_info" >> etc/crontabs/admin
			start_del_alarm_timer
			unlock_alarm_file_lock
			/etc/init.d/cron restart
			if [ "$response" == "" ]; then
				if [ $object == "闹钟" ]; then
					response="好的，主人，已经为你设置了${text}的闹钟"
				else
					response="好的，主人,我会在这里静静地等你到${text}提醒你${event}"
				fi
			fi
		fi
	fi
	common_json_output_status "ok" "$response"
}

alarm_close()
{
	#TODO should check the alarm status and give different reponse ?
	common_json_output_status "ok"
}

set_alarm()
{
	json_get_vars option
	case "$option" in
		"close" )
		alarm_close
		;;
		* )
		__set_alarm
		;;
	esac
}


get_alarm()
{
	json_get_vars event date object time period repeat date_origin repeat_origin
	echo "event : ${event}" > /dev/ttyS0
	echo "date : ${date}" > /dev/ttyS0
	echo "object : ${object}" > /dev/ttyS0
	echo "time : ${time}" > /dev/ttyS0
	echo "period : ${period}" > /dev/ttyS0
	local second=*
	local minute=*
	local hour=*
	local dom=*
	local month=*
	local year=*
	local dow=*
	local num=0
	local time_s=""
	local time_e=""
	local re=0
	local content=""
	local text=""
	local tmp=""

	time=$(echo $time | sed "s/://g")
	check_date $time $date
	re=$?
	if [ $re -eq 2 ]; then
		response="当前不支持此种查询方式，请换一种方式再试"
		common_json_output_status "error" $response
		return 0
	elif [ $re -eq 1 ]; then
		time_s=$(echo $time | awk -F '<' '{printf"%d",$1}')
		time_e=$(echo $time | awk -F '<' '{printf"%d",$2}')
		if [ $time_s -gt $time_e ]; then
			let time_e=time_e+120000
			if [ $time_s -gt $time_e ] || [ $time_e -gt 240000 ]; then
				response="主人,你说的时间已经超出了我的理解范围"
				common_json_output_status "error" $response
				return 0
			fi
		elif [ $time_s -eq $time_e ]; then
			time=$time_s
		fi
	fi

	check_invalid $time
	if [ $? -eq 1 ]; then
		check_invalid $period
		if [ $? -ne 1 ]; then
			hour=$period
			minute=*
			second=*
		fi
	else

		if [ "${time_s}" != "" ]; then
			if [ $(expr $time_s % 10000) -ne 0 ] || [ $(expr $time_e % 10000) -ne 0 ]; then
				response="当前不支持此种查询方式，请换一种方式再试"
				common_json_output_status "error" $response
				return 0
			fi
			time_s=$(echo $time_s | awk '{printf"%06d", $1}')
			time_e=$(echo $time_e | awk '{printf"%06d", $1}')
			hour=$(echo ${time_s:0:2} | awk '{printf"%d",$1}')"-"$(echo ${time_e:0:2} | awk '{printf"%d",$1}')
			minute=$(echo ${time_s:2:2} | awk '{printf"%d",$1}')
			second=$(echo ${time_s:4:2} | awk '{printf"%d",$1}')
		else
			time=$(echo $time | awk '{printf"%06d", $1}')
			hour=$(echo ${time:0:2} | awk '{printf"%d",$1}')
			if [ $hour -eq 24 ]; then
				response="当前不支持此种查询方式，请换一种方式再试"
				common_json_output_status "error" $response
				return 0
			elif [ $hour -gt 24 ];then
				response="主人,你说的时间已经超出了我的理解范围"
				common_json_output_status "error" $response
				return 0
			fi
			minute=$(echo ${time:2:2} | awk '{printf"%d",$1}')
			second=$(echo ${time:4:2} | awk '{printf"%d",$1}')
		fi
	fi

	check_invalid $date
	if [ $? -eq 0 ]; then
		dom=${date:6:2}
		if [ "${dom:0:1}" = "0" ]; then
			dom=${dom:1:1} 
		fi
		month=${date:4:2}
		if [ "${month:0:1}" = "0" ]; then
			month=${month:1:1} 
		fi
		year=${date:0:4}
		dow=$(date -d "${year}-${month}-${dom}" "+%w")
	else
		check_invalid $repeat
		if [ $? -eq 1 ]; then
			response="主人,你说的时间已经超出了我的理解范围"
			common_json_output_status "error" $response
			return 0
		else
			case "$repeat" in
				everyday)
					dom=*
					month=*
					year=*
					dow=*
					;;
				w*)
					dom=*
					month=*
					year=*
					dow=${repeat#w}
					;;
				d*)
					dom=${repeat#d}
					month=*
					year=*
					dow=*
					;;
				*)
					response="主人,你说的时间已经超出了我的理解范围"
					common_json_output_status "error" $response
					return 0
					;;
			esac
		fi
	fi

	check_invalid $event
	if [ $? -eq 1 ]; then
		event="invalid"
	fi
	check_invalid $object
	if [ $? -eq 1 ]; then
		response="主人,你说的时间已经超出了我的理解范围"
		common_json_output_status "error" $response
		return 0
	fi

	if [ "$object" != "闹钟" ] && [ "$event" == "invalid" ]; then
		event=".*"
	fi

	#crontab_info="${minute} ${hour} ${dom} ${month} ${dow} /usr/sbin/alarm_action.sh ${event} ${object}"
	#search_string=$(echo "$crontab_info" | sed "s/\*/\\\*/g")
	search_string=$(echo -n "${minute} ${hour} ${dom} ${month} ${dow} " | sed "s/\*/\\\*/g" && echo -n "/usr/sbin/alarm_action.sh ${event} ${object}")
	echo $search_string > /dev/ttyS0
	num=$(crontab -l | grep -c "${search_string}")
	if [ $num -eq 1 ]; then
		content=$(echo "${content}$minute $hour $event")
	fi
	while read line
	do
		if [ $(echo "$line" | grep -c "alarm_action.sh ${event} ${object}") -eq 0 ]; then
			continue
		fi
		## get event
		tmp=$(echo "$line" | awk -F ' ' '{print $7}')
		line=$(echo "$line" | awk -F '/' '{print $1}')
		cron_check -a "$line" -b "${minute} ${hour} ${dom} ${month} ${dow}"
		if [ $? -eq 1 ]; then
			if [ "$content" == "" ]; then
				content=$(echo "$line" | awk -v t=$tmp -F ' ' '{printf"%02d%02d %s",$2,$1,t}')
			else
				content=$(echo "${content}" && echo "$line" | awk -v t=$tmp -F ' ' '{printf"%02d%02d %s",$2,$1,t}')
			fi
			let num=num+1
		fi
	done < /etc/crontabs/admin
	check_invalid $date_origin
	if [ $? -eq 1 ]; then
		date_origin=""
	fi

	check_invalid $repeat_origin
	if [ $? -eq 1 ]; then
		repeat_origin=""
	fi

	if [ $num -gt 0 ];then
		##show content
		echo "###############" > /dev/ttyS0
		echo "$content" > /dev/ttyS0
		echo "###############" > /dev/ttyS0
		##convert content to text string
		echo "$content" > /tmp/alarm.tmp 
		content=$(cat /tmp/alarm.tmp | sort -n)
		echo "$content" > /tmp/alarm.tmp 
		while read line
		do
			echo "line is $line" > /dev/ttyS0
			tmp=$(echo $line | awk '{print $1}')
			hour=$(echo "${tmp:0:2}" | awk '{printf"%d",$1}')
			minute=$(echo "${tmp:2:2}" | awk '{printf"%d",$1}')
			event=$(echo $line | awk '{print $2}')
			if [ $hour -lt 11 ]; then
				period="上午"
			elif [ $hour -le 13 ]; then
				period="中午"
			else
				period="下午"
			fi

			text=$(echo "${text}${date_origin}${period}${hour}点")	
			if [ $minute -ne 0 ]; then
				text=$(echo "${text}${minute}分,")	
			fi

			if [ "$object" != "闹钟" ] && [ "$event" != "invalid" ];then
				text=$(echo "${text}内容是${event}")
			fi
		done < /tmp/alarm.tmp
		echo "text is ${text}" > /dev/ttyS0
		if [ $num -ne 1 ]; then
			response="已为你找到${num}个${object}，分别是${text}"
		else	
			response="已为你找到${num}个${object}，是${text}"
		fi
		common_json_output_status "okay" "$response"
	else
		tmp=$hour
		if [ $(echo "$hour" | grep -c "-") -ne 0 ]; then
			##hour is a range, such as 8-10
			tmp=$(echo "$hour" | awk -F '-' '{print $1}')
			if [ $tmp -lt 11 ]; then
				period="上午"
			elif [ $tmp -le 13 ]; then
				period="中午"
			else
				period="下午"
			fi
			if [ $tmp -ge 13 ]; then	
				let tmp=tmp-12
			fi
			text="${period}${tmp}点到"
			tmp=$(echo "$hour" | awk -F '-' '{print $2}')
		else
			if [ $tmp -lt 11 ]; then
				period="上午"
			elif [ $tmp -le 13 ]; then
				period="中午"
			else
				period="下午"
			fi
			if [ $tmp -ge 13 ]; then	
				let tmp=tmp-12
			fi
			text="${text}${period}${tmp}点"
		fi

		response="主人，你${date_origin}${repeat_origin}${text}的${object}还没有被设置"
		common_json_output_status "okay" "$response"
	fi
	return 0
}


del_alarm()
{
	json_get_vars event date object time period repeat date_origin repeat_origin
	echo "event : ${event}" > /dev/ttyS0
	echo "date : ${date}" > /dev/ttyS0
	echo "object : ${object}" > /dev/ttyS0
	echo "time : ${time}" > /dev/ttyS0
	echo "period : ${period}" > /dev/ttyS0
	local second=*
	local minute=*
	local hour=*
	local dom=*
	local month=*
	local year=*
	local dow=*
	local num=0
	local time_s=""
	local time_e=""
	local res=0
	local loop=0
	local index=0
	local tmp
	local lines=""

	time=$(echo $time | sed "s/://g")
	check_date $time $date
	res=$?
	if [ $res -eq 2 ]; then
		response="我还不会删除多个时间的闹钟这个技能了"
		common_json_output_status "error" $response
		return 0
	elif [ $res -eq 1 ]; then
		time_s=$(echo $time | awk -F '<' '{printf"%d",$1}')
		time_e=$(echo $time | awk -F '<' '{printf"%d",$2}')
		if [ $time_s -gt $time_e ]; then
			let time_e=time_e+120000
			if [ $time_s -gt $time_e ] || [ $time_e -gt 240000 ]; then
				response="主人,你说的时间已经超出了我的理解范围"
				common_json_output_status "error" $response
				return 0
			fi
		elif [ $time_s -eq $time_e ]; then
			time=$time_s
		fi
	fi

	check_invalid $time
	if [ $? -eq 1 ]; then
		check_invalid $period
		if [ $? -eq 0 ]; then	
			hour=$period
		fi
	else
		if [ "${time_s}" != "" ]; then
			if [ $(expr $time_s % 10000) -ne 0 ] || [ $(expr $time_e % 10000) -ne 0 ]; then
				response="当前不支持此种方式的删除，请换种说法再试"
				common_json_output_status "error" "$response"
				return 0
			fi
			time_s=$(echo $time_s | awk '{printf"%06d", $1}')
			time_e=$(echo $time_e | awk '{printf"%06d", $1}')
			hour=$(echo ${time_s:0:2} | awk '{printf"%d",$1}')"-"$(echo ${time_e:0:2} | awk '{printf"%d",$1}')
			minute=$(echo ${time_s:2:2} | awk '{printf"%d",$1}')
			second=$(echo ${time_s:4:2} | awk '{printf"%d",$1}')
		else
			time=$(echo $time | awk '{printf"%06d", $1}')
			hour=$(echo ${time:0:2} | awk '{printf"%d",$1}')
			if [ $hour -eq 24 ]; then
				response="当前不支持此种说法，请换一种方式再试"
				common_json_output_status "error" $response
				return 0
			elif [ $hour -gt 24 ];then
				response="主人,你说的时间已经超出了我的理解范围"
				common_json_output_status "error" $response
				return 0
			fi
			minute=$(echo ${time:2:2} | awk '{printf"%d",$1}')
			second=$(echo ${time:4:2} | awk '{printf"%d",$1}')
		fi
	fi

	check_invalid $date
	if [ $? -eq 0 ]; then
		dom=${date:6:2}
		if [ "${dom:0:1}" = "0" ]; then
			dom=${dom:1:1} 
		fi
		month=${date:4:2}
		if [ "${month:0:1}" = "0" ]; then
			month=${month:1:1} 
		fi
		year=${date:0:4}
		dow=*
	else
		check_invalid $repeat
		if [ $? -eq 1 ]; then
			response="主人,你说的时间已经超出了我的理解范围"
			common_json_output_status "error" $response
			return 0
		else
			case $repeat in
				everyday)
					dom=*
					month=*
					year=*
					dow=*
					;;
				w*)
					dom=*
					month=*
					year=*
					dow=${repeat#w}
					;;
				d*)
					dom=${repeat#d}
					month=*
					year=*
					dow=*
					;;
				*)
					response="主人,你说的时间已经超出了我的理解范围"
					common_json_output_status "error" $response
					return 0
					;;
			esac
		fi
	fi

	check_invalid $event
	if [ $? -eq 1 ]; then
		event="invalid"
	fi
	check_invalid $object
	if [ $? -eq 1 ]; then
		object="invalid"
	fi

	if [ "$object" != "闹钟" ] && [ "$event" == "invalid" ]; then
		event=".*"
	fi
	#crontab_info="${minute} ${hour} ${dom} ${month} ${dow} /usr/sbin/alarm_action.sh ${event} ${object}"
	search_string=$(echo -n "${minute} ${hour} ${dom} ${month} ${dow} " | sed "s/\*/\\\*/g" && echo -n "/usr/sbin/alarm_action.sh ${event} ${object}")
	echo $search_string > /dev/ttyS0
	num=$(crontab -l | grep -c "${search_string}")
	if [ $num -gt 0 ]; then
		lines=$(crontab -l | grep -n "${search_string}" | awk -F ':' '{print $1}' | sort -r)
		for i in $lines
		do
			##convert i to number
			i=$(echo "" | awk -v t=$i '{printf"%d",t}')
			sed -i ${i}d /etc/crontabs/admin
		done
	fi
	if [ "$tmp" != "" ]; then
		/etc/init.d/cron restart
	fi
	index=0
	while read line
	do
		let index=index+1
		if [ $(echo "$line" | grep -c "alarm_action.sh ${event} ${object}") -eq 0 ]; then
			continue
		fi
		line=$(echo "$line" | awk -F '/' '{print $1}')
		cron_check -a "$line" -b "${minute} ${hour} ${dom} ${month} ${dow}" -i
		if [ $? -eq 1 ]; then
			if [ $(echo "$line" | awk -F ' ' '{print $3 $4}' | grep -c "\*") -eq 0 ]; then
				let num=num+1
			else
				let loop=loop+1
			fi
			lines="${lines}${index}\n"
		fi
	done < /etc/crontabs/admin

	check_alarm_file_lock
	if [ $? -eq 0 ]; then
		lock_alarm_file_lock
		lines=$(echo $lines | sort -r )
		for i in $lines
		do
			##convert i to number
			i=$(echo "" | awk -v t=$i '{printf"%d",t}')
			sed -i ${i}d /etc/crontabs/admin
		done
		unlock_alarm_file_lock
		if [ "$lines" != "" ]; then
			/etc/init.d/cron restart
		fi
	fi

	check_invalid $date_origin
	if [ $? -eq 1 ]; then
		date_origin=""
	fi

	check_invalid $repeat_origin
	if [ $? -eq 1 ]; then
		repeat_origin=""
	fi

	if [ $(expr $num + $loop) -gt 0 ];then
		if [ $num -gt 0 ] ;then
			response="好的，已为你删除${date_origin}${repeat_origin}的$(expr $num + $loop)个${object}"
			if [ $loop -gt 0 ]; then
				response="${response},其中包含${loop}个循环${object}"
			fi
		elif [ $loop -gt 0 ]; then
			response="好的,已为你删除${date_origin}${repeat_origin}的${loop}个循环${object}"
		fi
		common_json_output_status "okay" "$response"
	else
		tmp=$hour
		if [ $(echo "$hour" | grep -c "-") -ne 0 ]; then
			##hour is a range, such as 8-10
			tmp=$(echo "$hour" | awk -F '-' '{print $1}')
			if [ $tmp -lt 11 ]; then
				period="上午"
			elif [ $tmp -le 13 ]; then
				period="中午"
			else
				period="下午"
			fi
			if [ $tmp -ge 13 ]; then	
				let tmp=tmp-12
			fi
			text="${period}${tmp}点到"
			tmp=$(echo "$hour" | awk -F '-' '{print $2}')
		else
			if [ $tmp -lt 11 ]; then
				period="上午"
			elif [ $tmp -le 13 ]; then
				period="中午"
			else
				period="下午"
			fi
			if [ $tmp -ge 13 ]; then	
				let tmp=tmp-12
			fi
			text="${text}${period}${tmp}点"
		fi

		response="主人，你${date_origin}${repeat_origin}${text}的${object}还没有被设置"
		common_json_output_status "okay" "$response"
	fi
	return 0
}

set_new_alarm()
{
	echo "inset new alarm" > /dev/ttyS0
	json_get_values contents content
	json_select content
	json_get_keys index
	for i in $index
	do
		echo "$i" > /dev/ttyS0
		json_select "$i"
		json_get_var obj "object"
		echo "obj is $obj" > /dev/ttyS0
		json_get_var event "event"
		if [ -z $event ] ; then
			event="invalid"
		fi
		json_get_var timestamp "timestamp"
		json_get_var vid "vid"
		echo "timestamp : $timestamp" > /dev/ttyS0
		json_get_var left "time_left"
		json_get_var date "date"
		json_get_var time "time"
		echo "date : $date; time : $time event : $event" > /dev/ttyS0
		year=${date::4}
		if [ $left -lt 60 ]; then
			/usr/sbin/alarm_action.sh $obj $event $left $year $vid &
			json_select ..
			continue
		fi
		json_get_var date "date"
		json_get_var time "time"
		echo "date : $date; time : $time event : $event" > /dev/ttyS0
		year=${date::4}
		month=${date:4:2}
		day=${date:6:2}
		hour=${time::2}
		minute=${time:3:2}
		second=${time:6:2}
		dow=$(date -d "$year-$month-$day" "+%w")
		json_get_var repeat "repeat"
		echo "repeat $repeat" > /dev/ttyS0
		if [ -z "$repeat" ]; then
			echo "no repeat" > /dev/ttyS0
		else
			case "$repeat" in
				M*)
					dow="*"
					tmp=$(echo repeat | grep "<")
					if [ -n "$tmp" ]; then
						month=$(echo $repeat | sed "s/</-/;s/M//g")
					else
						month=$(echo $repeat | sed "s/M/,/g;s/^,//g")
					fi
					;;
				W*)
					day="*"
					month="*"
					tmp=$(echo $repeat | grep "<")
					if [ -n "$tmp" ]; then
						dow=$(echo $repeat | sed "s/</-/;s/W//g")
					else
						dow=$(echo $repeat | sed "s/W/,/g;s/^,//g")
					fi
					dow=$(echo $dow | sed "s/7/0/g")
					;;
				D*)
					dow="*"
					month="*"
					tmp=$(echo $repeat | grep "<")
					if [ -n "$tmp" ]; then
						day=$(echo $repeat | sed "s/</-/;s/D//g")
					else
						day=$(echo $repeat | sed "s/D/,/g;s/^,//g")
					fi
					;;
				EVERYDAY)
					day="*"
					month="*"
					year="*"
					dow="0-6"
					;;
			esac
		fi
		echo "year :$year; month :$month; day :$day; dow : $dow; hour:$hour; minute : $minute; second :$second, obj : $obj" > /dev/ttyS0
		if [ ! -z "$year" ] && [ ! -z "$month" ] && [ ! -z "$day" ] && [ ! -z "$hour" ] && [ ! -z "$minute" ] && [ ! -z "$second" ] && [ ! -z "$obj" ]; then
			cron="$minute $hour $day $month $dow /usr/sbin/alarm_action.sh $obj $event $second $year"
			cron=$(echo "$cron" | sed "s/^00/0/")
			cron=$(echo "$cron" | sed "s/ 0\([0-9]\)/ \1/g")
			cron="$cron $vid"
			crontab -l | grep -qF "$vid"
			if [ $? -ne 0 ]; then
				echo "$cron" >> /etc/crontabs/admin
				/etc/init.d/cron restart
			else
				echo "$cron has been added" > /dev/ttyS0
			fi
		fi
		json_select ..
	done
}

del_new_alarm()
{
	echo "remove new alarm" > /dev/ttyS0
	json_get_values contents content
	json_select content
	json_get_keys index
	for i in $index
	do
		echo "$i" > /dev/ttyS0
		json_select "$i"
		json_get_var vid "vid"
		if [ ! -z "$vid" ]; then
			sed -i "/$vid/d" /etc/crontabs/admin
			/etc/init.d/cron restart
		fi
		json_select ..
	done
}
