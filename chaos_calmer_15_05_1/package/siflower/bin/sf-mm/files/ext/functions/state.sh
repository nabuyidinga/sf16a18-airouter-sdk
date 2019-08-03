#!/bin/sh

set_state_sleep() {
	if [ $(ps | grep -c "sf-mm") -eq 2 ]; then
		/etc/init.d/sfmmd stop > /dev/null 2>&1
		echo "0 4" > /sys/devices/sf-mm-led/sf_mm_led
	fi
}

set_state_student() {
	uci set basic_setting.student_mode.enable=1
	uci commit basic_setting
    common_json_output_status "ok"
}

set_state() {
	json_get_vars option
	common_echo_debug "set_state option : $option"
	case "$option" in
		"sleep" )
			set_state_sleep
			;;
		"student" )
			set_state_student	
			;;
	esac
}
