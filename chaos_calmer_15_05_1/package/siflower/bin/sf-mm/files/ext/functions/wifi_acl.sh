#!/bin/sh

WUCIG="uci -q get wldevlist"

reload_acl()
{
        index=0
        while true
        do
        mac=$(uci get $1.@device[$index].mac)
        if [ "x$mac" != "x" ];then
		echo "reload_acl $mac" > /dev/ttyS0
            macaddr=$(echo $mac | sed -e 's/_/:/g')
            lan=$(uci get $1.@device[$index].lan)
            internet=$(uci get $1.@device[$index].internet)
            ipset del inputvar $macaddr -q
            ipset del forwardvar $macaddr -q
            if [ "x$lan" = "x0" ];then
                ipset add inputvar $macaddr
            fi
            if [ "x$internet" = "x0" ];then
                ipset add forwardvar $macaddr
            fi
        else
            break
        fi
        index=$((index+1))
        done
}

start_func(){
	local mac=$1
	local fmac=${mac//:/_}
	local speed=$(${WUCIG}.$fmac.limitdown)
	local time=$(${WUCIG}.$fmac.timelist)
	local flow=$(${WUCIG}.$fmac.change)
	local svg=$(${WUCIG}.$fmac.restrictenable)
	local internet=$(${WUCIG}.$fmac.internet)

	if [ "x$speed" != "x0" ]; then
		pctl speed del $mac
		pctl speed add $mac $speed $speed
	else
		pctl speed del $mac
	fi

	if [ "x$time" != "x" ]; then
		pctl time update $mac time $time 8
	else
		if [ "x$internet" != "x0" ]; then
			pctl time del $mac
		fi
	fi

	if [ "x$flow" != "x0" -a "x$flow" != "x" ]; then
		tscript flow_en $mac $flow
	else
		tscript flow_dis $mac
	fi

	if [ "x$svg" != "x0" -a "x$flow" != "x" ]; then
		local game=$(${WUCIG}.$fmac.game)
		local video=$(${WUCIG}.$fmac.video)
		local social=$(${WUCIG}.$fmac.social)
		tscript type_flow $mac 1 $game $video $social
	else
		tscript type_flow $mac 0 0 0 0
	fi
}

start(){
	local mac
	for m in $*
	do
		mac=$(echo $m | grep -E ^\([0-9a-fA-F]{2}:\){5}[0-9a-fA-F]{2}\$)
		if [ "x$mac" = "x" ]; then
			echo "error mac address $mac"
		else
			start_func $mac
		fi
	done
}

update_pctl()
{
	local mac
	local i=0
	while true
	do
		mac=$(${WUCIG}.@device[$i].mac)
		if [ "x$mac" != "x" ];then
			start_func ${mac//_/:}
			i=$((i+1))
		else
			break
		fi
	done
}
