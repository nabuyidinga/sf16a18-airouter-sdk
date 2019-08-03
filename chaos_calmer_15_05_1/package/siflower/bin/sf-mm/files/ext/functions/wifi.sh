#!/bin/sh

check_wifi_iface() {
	local wdev
	config_get wdev "$1" device
	config_get wdisabled "$1" disabled
	config_get wifname "$1" ifname


	if [ -n "$wdev" ]; then
		if [ "$wdev" == "radio${phy}" -o "$phy" == "-1" ]; then
			if [ "$wdisabled" != "1" ]; then
				ubus call hostapd.$wifname update_params "{\"hidden\": $hidden}"
			fi
		fi
	fi
	iface_index=$(($iface_index + 1))
}

set_wifi() {
	json_get_vars hidden phy

    echo "phy  is  $phy" > /dev/ttyS0
    echo "hidden is $hidden" > /dev/ttyS0

	config_load wireless
	iface_index=0
	config_foreach check_wifi_iface wifi-iface

    common_json_output_status "ok"
}
