#!/bin/sh
network_state=0
##114.114.114.114 	114 DNS
##223.5.5.5.5 		Alibaba DNS
##8.8.8.8 			Google DNS
##180.76.76.76 		Baidu DNS
domains="114.114.114.114 223.5.5.5 8.8.8.8 180.76.76.76"
network_check(){
	local ret
	local result=0
	for domain in $domains; do
		ping -c 1 -W 1 -w 1 $domain > /dev/null 2>&1
		ret=$?
		let result=result+ret
	done
	len=$(echo $domains | awk '{print NF}')
	if [ $len -eq $result ]; then
		return 1
	else
		return 0
	fi
}


while [ 1 ] ; do
	sleep 10
	network_check
	if [ $? -eq 0 ]; then
		if [ $network_state -ne 0 ]; then
			echo "network connected" > /dev/ttyS0
			ubus call sf_mm_ubus event_notify "{\"event_id\": 0}"
			network_state=0
		fi
	else
		if [ $network_state -eq 0 ]; then
			echo "network is disconnected" > /dev/ttyS0
			ubus call sf_mm_ubus event_notify "{\"event_id\": 1}"
			network_state=1
		fi
	fi
done
