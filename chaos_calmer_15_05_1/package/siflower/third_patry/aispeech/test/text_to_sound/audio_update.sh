#!/bin/sh
set -x
if [ ! -d "/tmp/audio" ]; then
	mkdir /tmp/audio
else
	rm /tmp/audio/* -rf
fi

index=0
dir=""
while read line
do
	if [ "$line" != "" ]; then
		echo "$line" | grep -q "^#"
		if [ $? -eq 0 ]; then
			index=0
			dir=${line#\#}
			mkdir /tmp/audio/$dir
		else
			url=$(aispeech_test "$line" | grep "speakUrl" | awk -F ': ' '{print $2}')
			let index=index+1
			wget -O "/tmp/audio/$dir/${index}.wav" "$url"
		fi
	fi
done < $1
