#!/bin/sh

##### json output #####
common_json_output_status() {
	local status="$1"
    local report_text="$2"

	json_init
	json_add_string "status" "$status"
    [ -n "$report_text" ] && json_add_string "report_text" "$report_text"
	json_close_object
	local msg=`json_dump`
	echo "$msg"
}

common_echo_debug()
{
        echo $1 > /dev/ttyS0
}
