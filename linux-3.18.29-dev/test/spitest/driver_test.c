#include "test.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "driver_test.h"


static char driver_name[DRIVER_INDEX_END][3] = { { "mtd"}};
static char ktest_name[] = "/dev/ktest";

static void  print_msg(int mode, const char* fmt, ...){
	va_list args;
	char msg[MAX_PRINT_MSG_LEN] = {0};
	va_start(args, fmt);
	vprintf(fmt, args);
	puts("");
	vsprintf(msg,fmt,args);
	tst_resm(mode, msg, NULL);
	va_end(args);
}

double calc_speed(struct timeval * t1, struct timeval* t2, int data_size){
double timeuse;
timeuse = t2->tv_sec - t1->tv_sec + (t2->tv_usec - t1->tv_usec)/1000000.0;
return data_size/1024/timeuse;
}

int get_test_module(char * name){
	int i = 0;
	for(; i < DRIVER_INDEX_END; i++){
		if(strcmp(name, driver_name[i]) == 0){
			return i;
		}
	}
	return -1;
}
int main(int argc, char *argv[]) {
	int ret = 0;
	int fd;
	int module = -1;
	int temp;
	print_msg(TINFO,"driver test start");
	if(argc < 3){
		printf("check arg fail\n");
		return -1;
	}

	fd = open(ktest_name, O_RDWR);
	if (fd < 0){
		print_msg(TFAIL,"driver_test fail :can't open device");
		return -1;
	}
	module = get_test_module(argv[1]);
	switch(module){
		case MTD_INDEX:
					if(mtd_test(argv[2], fd) < 0){
						print_msg(TFAIL,"Test fail in mtd test");
						return -1;
					}
			break;
		default :
			break;
	}
// MTD test
	print_msg(TPASS,"Test pass");
	tst_exit();
	return ret;
}
