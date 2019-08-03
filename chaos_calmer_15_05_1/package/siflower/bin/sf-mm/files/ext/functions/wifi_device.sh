#!/bin/sh
. /usr/share/sf-mm/functions/wifi_acl.sh

prepare_device_config()
{
        if [ $mode == "limited" ]; then
                limitdown=100
                timeset='22:00 18:00'
        elif [ $mode == "normal" ]; then
                imitdown=0
                timeset=''
        else
                imitdown=0
                timeset=''
        fi
}

add_device_config()
{
        prepare_device_config
        cat <<EOF
config device '$1'
        option mac '$1'
        option internet '${allow}'
        option lan '1'
        option limitdown '${limitdown}'
        option timelist '${timeset}'
        option change '0'
        option restrictenable '0'
        option game '1'
        option video '1'
        option social '1'
        option online '1'
        option hostname 'unknown'
        option associate_time '0'

EOF
}


check_device_config()
{
        local config="$1"
        config_get user_mac "$config" mac
        if [ $cmp_mac == $user_mac ]; then
                found=1
                prepare_device_config
                uci set wldevlist.$cmp_mac.internet="$allow"
                uci set wldevlist.$cmp_mac.limitdown="$limitdown"
                uci set wldevlist.$cmp_mac.timelist="$timeset"
        fi
}

update_wifi_list()
{
        local wifi_macs wifi_mac
        json_get_values wifi_macs mac
        json_select mac
        config_load wldevlist
        for wifi_mac in $wifi_macs; do
                found=0
                cmp_mac=${wifi_mac//:/_}
                cmp_mac=$(echo $cmp_mac | tr '[a-z]' '[A-Z]')
                config_foreach check_device_config device
                if [ $found == 0 ]; then
                        #insert new one
                        add_device_config $cmp_mac >> /etc/config/wldevlist
                fi
        done
        json_select ..
        uci commit wldevlist
}

set_wifi_test()
{
        echo_debug "$(cat 1.json)"
        json_load "$(cat 1.json)"
        json_get_vars allow mode
        echo_debug "allow=$allow mode=$mode"
        update_wifi_list
        /etc/init.d/functl restart
}

check_wifi_iface_per()
{
    config_get wdev "$1" device
    config_get wdisabled "$1" disabled
    config_get wifname "$1" ifname

    if [ -n "$wdev" ]; then
        if [ "$wdisabled" != "1" ]; then
                local wifi_macs wifi_mac
                json_get_values wifi_macs mac
				json_set_namespace nmnew nmold
                json_init
                json_add_object sta_macaddr_list
                mac_index=0
                for wifi_mac in $wifi_macs; do
                        if [ "$allow" == "0" ]; then
                                if iw dev $wifname station dump | grep "$wifi_mac" > /dev/null; then
                                        iw dev $wifname station del $wifi_mac
                                fi
                        fi
                        json_add_string sta$mac_index $wifi_mac
                        mac_index=$(($mac_index + 1))
                done
                if [ "$allow" == "0" ]; then
                        white_func=del_sta_whitelist
                else
                        white_func=add_sta_whitelist
                fi
                ubus call hostapd.$wifname $white_func "$(json_dump)" > /dev/null
                echo "ubus call hostapd.$wifname $white_func $(json_dump)" > /dev/ttyS0
                json_close_object
				json_set_namespace $nmold
        fi
    fi

}

check_all_wifi_device()
{
        config_load wireless
        config_foreach check_wifi_iface_per wifi-iface
}


set_wifi_device()
{
        json_get_vars allow mode
        common_echo_debug "allow=$allow mode=$mode"
        update_wifi_list
        update_pctl
        reload_acl wldevlist
        check_all_wifi_device
}

get_wifi_dev() {
    config_get type "$1" network
	common_echo_debug "get_wifi_dev type : $type"
	if [ "$type" == "lan" ]; then
		config_get ifname "$1" ifname
		common_echo_debug "get_wifi_dev ifname : $ifname"
		echo $ifname >> "/tmp/wifi_dev_list"
	fi

}

get_wifi_device_num() {
	device_num=0
	while read line
	do
		common_echo_debug "get_wifi_device_num line: $line"
		tmp=$(iw dev $line station dump | grep "Station" | wc -l)
		let device_num=device_num+tmp
	done < "/tmp/wifi_dev_list"

	if [ $device_num -eq 0 ]; then
		str="当前还没有设备连接哦"
	else
		str="当前有${device_num}个无线设备接入"
	fi
    common_json_output_status "ok" "$str"
}

get_wifi_device() {
	json_get_vars option
	common_echo_debug "get_wifi option : $option"
	case "$option" in
		"device_num" )
		touch "/tmp/wifi_dev_list"
        config_load wireless
        config_foreach get_wifi_dev wifi-iface
		get_wifi_device_num
		rm "/tmp/wifi_dev_list"
		;;
	esac
}
