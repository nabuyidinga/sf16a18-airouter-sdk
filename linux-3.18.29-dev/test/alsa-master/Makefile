
CC = ./../../toolchain/toolchain-mipsel_mips32_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc

all: playwav record i2sloop 

playwav: audio_alsa.c playwav.c 
	$(CC) -o playwav playwav.c audio_alsa.c 

record: record.c 
	$(CC) -o record record.c -I /home/chong/linux-3.18.29-dev/test/alsa-lib-1.1.0/ipkg-install/usr/include -L /home/chong/linux-3.18.29-dev1/test/alsa-lib-1.1.0/ipkg-install/usr/lib -lasound

i2sloop: i2sloop.c audio_alsa.c
	$(CC) -o i2sloop i2sloop.c audio_alsa.c -I /home/chong/linux-3.18.29-dev1/test/alsa-lib-1.1.0/ipkg-install/usr/include -L /home/chong/linux-3.18.29-dev1/test/alsa-lib-1.1.0/ipkg-install/usr/lib -lasound -lpthread


clean:
	rm -f record playwav i2sloop compare *~
