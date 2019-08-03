#!/bin/sh

get_network_speed() {
	#speed=$(netdetect -d | grep "downbandwidth")
	#if [ "$speed" == "" ]; then
	#	str="当前无法完成测速操作，请稍后再试"
	#else
	#	speed=$(echo $speed | awk -F '=' '{print $2}')
	#	speed=$((speed))
	#	if [ $speed -le 0 ]; then
	#		str="当前无法完成测速操作，请稍后再试"
	#	else
	#		speed=$(echo "" | awk -v sp=$speed '{printf"%0.1f", sp*8.0/1000}')
	#		speed=${speed%\.0}
	#		str="当前网速为${speed}兆每秒"
	#	fi
	#fi
	ping -c 1 -w 1 baidu.com > /dev/null
	if [ $? -eq 0 ]; then
		str="当前网速良好"
	else
		str="主人，你的网络好像有点问题哦"
	fi

    common_json_output_status "ok" "$str"
}

get_network() {
	json_get_vars option
	common_echo_debug "get_network option : $option"
	case "$option" in
		"speed" )
		get_network_speed	
		;;
	esac
}

network_connect() {
	local radio="radio0"
	local ssid=""
	local encrypt="open"
	local passwd="1234"
	local mac=""
	local channel=""
	local index=0
	local signal=
	local filter=0
	local info=""
	iwinfo wlan0 scan  | grep -E "(Address|ESSID|Mode|Signal|Encryption)" > /tmp/.scan
	while read line
	do
		if [ $(echo "$line" | grep -c "Address") -ne 0 ]; then
			if [ $index -eq 4 ] && [ $filter -eq 0 ]; then
				echo "$signal $radio $ssid $encrypt $passwd $mac $channel" >> /tmp/.scan_open
			fi

			index=0
			channel=""
			ssid=""
			filter=0
			mac=$(echo $line | awk -F ': ' '{print $2}')
			let index=index+1
			continue
		fi
		if [ $(echo "$line" | grep -c "ESSID") -ne 0 ]; then
			ssid=$(echo "$line" | awk -F ': ' '{print $2}')
			ssid=$(echo $ssid | sed "s/.*\"\(.*\)\".*/\1/")
			if [ "$ssid" == "unkown" ];then
				filter=1
			fi
			let index=index+1
			continue
		fi

		if [ $(echo "$line" | grep -c "Channel") -ne 0 ]; then
			channel=$(echo "$line" | awk -F ': ' '{print $3}')	
			let index=index+1
			continue
		fi

		if [ $(echo "$line" | grep -c "Signal") -ne 0 ]; then
			signal=$(echo "$line" | grep -oE "\-[0-9]{1,4}")
			let index=index+1
			continue
		fi

		if [ $(echo "$line" | grep -c "Encryption") -ne 0 ]; then
			tmp=$(echo "$line" | awk -F ': ' '{print $2}')
			if [ "$tmp" != "none" ]; then
				filter=1
			fi
		fi

	done < /tmp/.scan

	if [ $index -eq 4 ] && [ $filter -eq 0 ]; then
		echo "$signal $radio $ssid $encrypt $passwd $mac $channel" >> /tmp/.scan_open
	fi

	info=$(cat /tmp/.scan_open | sort -r -n | head -n 1)
	if [ "$info" != "" ]; then
		wds_setup.sh $info &
	fi
	if [ -z "/tmp/.scan_open" ]; then
		rm /tmp/.scan_open
	fi
	if [ -z "/tmp/.scan" ]; then
		rm /tmp/.scan
	fi
}

network_recovery() {
	/etc/init.d/network restart
    common_json_output_status "ok" "网络恢复成功"
}

set_network() {
	json_get_vars option
	common_echo_debug "set_network option : $option"
	case "$option" in
		"connect" )
		network_connect
		;;
		"recovery" )
		network_recovery
		;;
	esac
}
