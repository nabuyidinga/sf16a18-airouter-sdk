#!/bin/sh

mic_switch(){
	tmp=$(amixer cget numid=5 | tail -n 1)
	mute=${tmp##*=}
	if [ -n "$mute" ]; then
		if [ "$mute" == "off" ]; then
				ubus call sf_mm_ubus event_notify "{\"event_id\": 2, \"data1\" : \"mute\"}"
				amixer cset numid=5 on > /dev/null
				amixer cset numid=6 on > /dev/null
				amixer cset numid=7 on > /dev/null
				amixer cset numid=8 on > /dev/null
		else
				ubus call sf_mm_ubus event_notify "{\"event_id\": 2, \"data1\" : \"unmute\"}"
				amixer cset numid=5 off > /dev/null
				amixer cset numid=6 off > /dev/null
				amixer cset numid=7 off > /dev/null
				amixer cset numid=8 off > /dev/null
		fi
	fi

}


case "$1" in
	SPRESS)	
		if [ $(ps | grep -c "sf-mm") -eq 1 ]; then
			/etc/init.d/sfmmd restart silence_boot
			exit 0
		fi
		mic_switch
		;;
	ELPRESS)
		##should play a notify tone
		ubus call sf_mm_ubus event_notify "{\"event_id\": 11}"
		sleep 2
		echo "FACTORY RESET" > /dev/console
		jffs2reset -y && reboot &
		;;
esac

return 0
