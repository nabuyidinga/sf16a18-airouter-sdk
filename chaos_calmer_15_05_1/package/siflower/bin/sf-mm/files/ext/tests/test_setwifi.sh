#!/bin/sh
. /lib/functions.sh
. /usr/share/libubox/jshn.sh
. /usr/share/sf-mm/functions/wifi_device.sh

echo_debug "$(cat 1.json)"
json_load "$(cat 1.json)"
json_get_vars allow mode
set_wifi_device




