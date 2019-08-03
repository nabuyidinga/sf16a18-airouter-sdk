#!/bin/sh

source /router/common.sh

RANDOM_NAME=/etc/autotest/random_name
LOG_FILE=/tmp/storage_test.log

rand(){  
    min=$1  
    max=$(($2-$min+1))
    num=$(cat /proc/sys/kernel/random/uuid | cksum | awk -F ' ' '{print $1}')
    echo $(($num%$max+$min))
}

#generate a random path and creat it, $1 is the path's prefix
#max path deepth is 20
gen_random_path(){
	path=$1
	path_depth=$(rand 0 20)
	i=0

	while [ $((i)) -lt $((path_depth)) ]; do
		path=${path}/$($RANDOM_NAME)
		i=$(($i+1))
	done
	if [ ! -d $path ]; then
		mkdir -p $path
	fi
	echo $path
}

do_float_div(){
	awk -v a=$1 -v b=$2 'BEGIN {print a/b}'
}

do_one_test(){
	file=$($RANDOM_NAME)
	src=$(gen_random_path $1)/$file
	dst=$(gen_random_path $2)/$file
	TMP=/tmp/tmp-${$}
	file_len_MB=$(rand 0 $(($3-1)))
	file_len_B=$(rand 1 1048576)
	CMD=cp

	dd if=/dev/urandom of=${src} bs=1M count=${file_len_MB} > /dev/null 2>&1
	dd if=/dev/urandom of=${TMP} bs=${file_len_B} count=1 > /dev/null 2>&1
	dd if=${TMP} of=${src} seek=${file_len_MB} obs=1M > /dev/null 2>&1
	rm ${TMP}
	echo "$(date "+%H:%M:%S")  $CMD $((file_len_MB*1048576+file_len_B)) bytes data..." >> $LOG_FILE

	origin_md5=$(md5sum $src | awk '{print $1}')
	$CMD $src $dst >> $LOG_FILE
	sync
	new_md5=$(md5sum $dst | awk '{print $1}')
	if [ "$new_md5" != "$origin_md5" ]; then
		echo "transmission error:" >> $LOG_FILE
		echo "origin_md5=$origin_md5" >> $LOG_FILE
		echo "new_md5=$new_md5" >> $LOG_FILE
		echo "cmp: $(cmp $src $dst)" >> $LOG_FILE
		return 1
	fi
	echo "$(date "+%H:%M:%S")  success" >> $LOG_FILE
	rm -rf ${1}/* ${2}/*
	return 0
}

get_current_time(){
	echo $(cat /proc/uptime | awk '{print $1}')
}

do_speed_test(){
	start_time=$(get_current_time)
	dd if=$1 of=$2 count=200 bs=1M $3
	end_time=$(get_current_time)
	awk -v a=$start_time -v b=$end_time 'BEGIN {print 200/(b-a)}'
}

SPEED_LOG=/storate_speed.log

speed_test(){
	write_speed=$(do_speed_test /dev/zero ${1}/test.usb conv=fsync)
	echo "write speed = $write_speed MB/s" >> $SPEED_LOG
	read_speed=$(do_speed_test ${1}/200M.bin /dev/null)
	echo "read speed = $read_speed MB/s" >> $SPEED_LOG
	rm ${1}/test.usb
	return 0
}

keep_transfering(){
	if [ ! -d $1 ]; then
		mkdir -p $1
	fi
	free_space=$(df -m $1 | awk 'NR==2 {print $4}')
	if [ $free_space -gt 200 ]; then
		free_space=200
	fi
	start_time=$(date +%s)
	now=$(date +%s)
	while [ -z $3 ] || [ $((${now}-${start_time})) -lt $3 ]
	do
		dd if=/dev/zero of=$1/test ${2}.bin bs=1M count=$free_space conv=fsync > /dev/null 2>&1
		dd if=$1/test${2}.bin of=/dev/null bs=1M count=$free_space > /dev/null 2>&1
		now=$(date +%s)
	done
	rm $1/test${2}.bin
}

# $1: origin total bytes transfered
# $2: total bytes transfered after $3 seconds
# $3: delta seconds
# $4: total time
present_speed(){
	bytes=$((${2}-${1}))
	# calculate speed (Bytes/s)
	speed=$((${bytes}/${3}))

	if [ $speed -gt 1048576 ]
	then
		speed="$(do_float_div ${speed} 1048576) MB/s"
	elif [ $speed -gt 1024 ]; then
		speed="$(do_float_div ${speed} 1024) KB/s"
	else
		speed="$speed B/s"
	fi
	echo "$((${4}-1)) - $4 sec: ${speed}"
}

#$1: spi-flash or USB
#$2: specifies the amount of time in seconds between each report
#$3: determines the number of reports generated at interval seconds apart,
#    infinite if not specified.
show_speed(){
	#echo $(get_current_time)
	i=0
	case $1 in
	spi)
		statistic=/sys/devices/10000000.palmbus/18200000.spi/bytes
		;;
	usb)
		statistic=/sys/kernel/debug/17400000.usb/statistic
		;;
	*)
		echo "unknown device $1"
		exit
		;;
	esac
	while [ "$3" == "" ] || [ $i -lt $3 ]
	do
		start=$(cat ${statistic})
		sleep $2
		end=$(cat ${statistic})
		i=$((${i}+1))
		present_speed start end $2 $i &
	done
	#echo $(get_current_time)
}

#$1: The directory of the device
#$2: Total transfer time in seconds, none for infinite
keep_transfering $1 1 $2 &
keep_transfering $1 2 $2 &
keep_transfering $1 3 $2 &
keep_transfering $1 4 $2 &

