#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>

//we must run a eeprom driver first
int main(int argc, char **argv)
{
	int ret, fd, i;
	char read_data[256];
	char write_data[256];
	char offset;

	fd = open("/sys/devices/platform/s3c2440-i2c/i2c-0/0-0050/eeprom", O_RDWR);
	if(fd < 0){
		printf("Open at24c02 fail\n");
		return -1;
	}

	ret = read(fd, &offset, 1);
	if(ret < 0){
		printf("Read error\n");
		return -1;
	}else if(ret < 1){
		perror("Incomplete read\n");
		printf("ret = %d\n", ret);
		return -1;
	}

	for(i = 0; i < 256; i++)
		write_data[i] = offset + 1 + i;

	lseek(fd, 0, SEEK_SET);		//It's a must, or something wierd will happen

	ret = write(fd, write_data, 256);
	if(ret < 0){
		printf("Write error\n");
		return -1;
	}

	lseek(fd, 0, SEEK_SET);	 //It's a must, or something wierd will happen

	ret = read(fd, read_data, 256);
	if(ret < 0){
		printf("Read error\n");
		return -1;
	}else if(ret < 256){
		perror("Incomplete read\n");
		printf("ret = %d\n", ret);
		return -1;
	}

	printf("read_data_value = :\n");
	for(i = 0; i < 256; i++){
		if(i %16 == 0)
			printf("\n");
		printf(" %03d ", read_data[i]);
	}
	printf("\n");
	return 0;
}
