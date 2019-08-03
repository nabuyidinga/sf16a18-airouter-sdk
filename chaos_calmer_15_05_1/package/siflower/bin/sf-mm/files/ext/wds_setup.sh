#!/bin/ash
. /lib/functions.sh

# band, ssid, encryption, key, bssid, channel
echo ~$1~$2~$3~$4~$5~$6~
shift 1

name=""
delete_sfi() {
	echo !!!!!!! > /dev/ttyS0
	local cfg="$1"
	config_get ifname "$cfg" ifname
	[ -n "$ifname" ] && [ "$ifname" == "sfi0" ] || [ "$ifname" == "sfi1" ] || return 0
	echo !!!!$ifname!!! > /dev/ttyS0
	uci del wireless.$cfg
}
resetfwnet() {
	local cfg="$1"
	config_get fwname "$cfg" name
	[ -n "$fwname" ] && [ "$fwname" == "lan" ] || return 0
	uci set firewall.$cfg.network="lan"
}

#########################check args#############
if [ "$1" == "radio0" ]; then
	name="sfi0"
else
	if [ "$1" == "radio1" ]; then
		name="sfi1"
	else
		echo "wds_setup.sh: disable wds!" > /dev/ttyS0
		uci delete network.wwan
		uci delete network.stabridge
		config_load wireless
		config_foreach delete_sfi wifi-iface
		uci set network.wan.disabled="0"
		uci set dhcp.lan.ignore="0"
		config_load firewall
		config_foreach resetfwnet zone
		uci commit wireless
		uci commit network
		uci commit firewall
		uci commit dhcp
		/etc/init.d/dnsmasq reload
		/etc/init.d/firewall reload
		/etc/init.d/network reload
		wifi reload
		exit 1
	fi
fi

######################add sta interface#########
wcfg=`uci add wireless wifi-iface`
fail_setup() {
	echo "setup failed!" > /dev/ttyS0
	uci del wireless.$wcfg
	exit 2
}

uci set wireless.$wcfg.device=$1
uci set wireless.$wcfg.ifname=$name
if [[ -n $2 && ${#2} -lt 32 ]]; then
	uci set wireless.$wcfg.ssid=$2
else
	fail_setup
fi
if [[ -n $3 && "$3" == "open" ]]; then
	uci set wireless.$wcfg.encryption=$3
	uci set wireless.$wcfg.key="12345678"
else
	if [[ -n $4 && ${#4} -ge 8 && ${#4} -lt 32 ]]; then
		uci set wireless.$wcfg.encryption=$3
		uci set wireless.$wcfg.key=$4
	else
		fail_setup
	fi
fi
if [[ -n $5 && ${#5} -lt 32 ]]; then
	uci set wireless.$wcfg.bssid=$5
else
	fail_setup
fi
uci set wireless.$wcfg.network="wwan"
uci set wireless.$wcfg.mode="sta"

######################setup channel && wwan######
if [[ $6 -lt 200 ]]; then
	uci set wireless.$1.channel=$6
else
	fail_setup
fi
ncfg=`uci add network interface`
uci rename network.$ncfg="wwan"
uci set network.wwan.proto="dhcp"
uci set network.wwan.ifname=$name

#####################setup stabridge##########
scfg=`uci add network interface`
uci rename network.$scfg="stabridge"
uci set network.stabridge.proto="relay"
uci set network.stabridge.network="lan wwan"
uci set network.stabridge.disable_dhcp_parse="1"

#####################need reset when disabled#######
setfwnet() {
	local cfg="$1"
	config_get fwname "$cfg" name
	[ -n "$fwname" ] && [ "$fwname" == "lan" ] || return 0
	uci set firewall.$cfg.network="lan wwan"
}
uci set network.wan.disabled="1"
uci set dhcp.lan.ignore="1"
config_load firewall
config_foreach setfwnet zone


uci commit dhcp
uci commit firewall
uci commit network
uci commit wireless
/etc/init.d/dnsmasq reload
/etc/init.d/firewall reload
/etc/init.d/network reload
wifi reload

#####################setup netclash############
ip=`ifconfig $name | awk -F "inet addr:|Bcast" '{printf "%s", $2}'`
while [ "$ip" == "" ]
do
	sleep 1
	ip=`ifconfig $name | awk -F "inet addr:|Bcast" '{printf "%s", $2}'`
done
netclash $ip "255.255.255.0"

echo "/bin/setup_wds: end" > /dev/ttyS0
