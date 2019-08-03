#include "test.h"
#include <asm-generic/errno-base.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>


/*******************************************************************************************
we support two apis in i2c driver
static struct i2c_algorithm i2c_dw_algo = {
	.master_xfer    = i2c_dw_xfer,
	.functionality  = i2c_dw_func,
};

in i2c_dw_func we will return:
.functionality = I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR;

/dev/i2c-X ioctl commands:(in i2c-dev.h)
I2C_RETRIES
I2C_TIMEOUT
I2C_SLAVE		//TODO
I2C_SLAVE_FORCE	//TODO
I2C_TENBIT		//TODO 0 for 7 bit addrs, != 0 for 10 bit
I2C_FUNCS
I2C_RDWR

we can set flag with two features.
struct i2c_msg {
    __u16 flags;
I2C_M_TEN
I2C_M_RD

TODO: we have to do speed test
TODO: both in ioctl and flags we can set 10 bit addrs, we need to test which
will effect finally.

eeprom data:
TODO

*******************************************************************************************/
#define MAX_PRINT_MSG_LEN 1024
// 1 page = 4KB
#define PAGE_NUM	(2)	
#define MSG_SIZE	(1 << 13)
typedef unsigned char u8;
static int test_pass = 1;
u8 *write_data;


static void print_msg(int mode, const char *fmt, ...)
{
	va_list args;
	char msg[MAX_PRINT_MSG_LEN] = {0};
	va_start(args, fmt);
	vprintf(fmt, args);
	puts("");
	vsprintf(msg, fmt, args);
	tst_resm(mode, msg, NULL);
	if (mode == TFAIL)
		test_pass = 0;
	va_end(args);
}
/*
* print the buf message
*/
static void print_buf(unsigned char *buf, int len)
{
	int i,j;
	for(i = 0; i < len / 16; i++)
	{
		for(j = 0; j < 16; j++)
		{
			printf("%.2x", *(buf + i * 16 + j));
			if((j + 1) % 4 == 0)
				printf("\t");
			
		}
		printf("\n");

	}

	if(len % 16)
	{
		for(j = 0; j < (len % 16); j++)
		{
			printf("%.2x", *(buf + (len / 16) * 16 + j));
			if((j + 1) % 4 == 0)
				printf("\t");
			
		}
		printf("\n");
	}

}

int i2c_ioctl_test(int fd)
{
	int ret;
	ret = ioctl(fd, I2C_TIMEOUT, 1); /* 设置超时 */
	if (ret != 0)
		print_msg(TFAIL, "can't set i2c mode TIMEOUT");
	ret = ioctl(fd, I2C_RETRIES, 2); /* 设置重试次数 */
	if (ret != 0)
		print_msg(TFAIL, "can't set i2c mode RETRIES");
	ret = ioctl(fd, I2C_FUNCS, 0);
	if (ret != 0)
		print_msg(TFAIL, "can't set i2c mode FUNCS");
	ret = ioctl(fd, I2C_TENBIT, 0); /* 0 for 7 bit addrs, != 0 for 10 bit */
	if (ret != 0)
		print_msg(TFAIL, "can't set i2c mode TENBIT");
	return 0;
}

void i2c_write_data(int fd)
{
	unsigned int i, j;
	int ret;
	/* e2prom 设备在i2c总线上的地址, codec is 0x1a, eeprom is 0x50*/
	unsigned char device_addr = 0x55;
	struct i2c_rdwr_ioctl_data i2c_data;
	unsigned char buf[(1 << 6)] = {0};
	for(j = 0; j < (1 << 6); j++)
		buf[j] = 0x5a;

	i2c_data.msgs =
			(struct i2c_msg *)malloc(1 * sizeof(struct i2c_msg));
	if (i2c_data.msgs == NULL) {
		perror("malloc error");
		exit(1);
	}
	i2c_data.nmsgs = (PAGE_NUM * 4 * 1024 / (1 << 6));
	for(i = 0; i < i2c_data.nmsgs; i++){	
		i2c_data.msgs[i].len =  (1 << 6);
		i2c_data.msgs[i].addr = device_addr;
		i2c_data.msgs[i].flags = 0; /* write */
		i2c_data.msgs[i].buf = buf;
	}


	ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
	free(i2c_data.msgs);
	if (ret < 0) {
		printf("i2c test trans in speed test fail\n");
		print_msg(TFAIL, "i2c test fail: can't send i2c message in "
				 "speed test");
	}
	printf("Write data finished!\n");
}

void i2c_cmp_data(void)
{
	char *mem_start;
	int i, result = 0, fd;
	time_t   now;  
	struct   tm     *timenow;
	fd = open("/dev/i2c-test",O_RDWR);
	if((mem_start = mmap(NULL, PAGE_NUM * 4 * 1024, PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0)) == MAP_FAILED){
		 printf("mmap failed.\n");
		 exit(0);
	}
	for(i = 0; i < PAGE_NUM * 4 * 1024; i++){
		if(*(mem_start + i) != 0x5a && !result){
			time(&now);
			timenow = localtime(&now);
			printf("%s :data cmp different, %d's byte different\n",asctime(timenow), i);
			result = 1;
		}
	}
		
	if(!result){
	
		time(&now);
		timenow = localtime(&now);
		printf("%s: compare ok ! no data different!\n", asctime(timenow));
	}
	close(fd);
}

int data_cmp(unsigned char *msg1, unsigned char *msg2, int len)
{
	int i;
	for(i = 0; i < len; i++)
		if(msg1[i] !=msg2[i])
		{
			printf("data cmp diff: i = %d, msg1[i] = %x, msg2[i] = %x\n", i, msg1[i], msg2[i]);
			return -1;
		}
	return 0;
}

/*init write dat*/
int write_data_init(int ret)
{
	if(!ret)
		return ret;
	else {
		return -1;
	}
}
/* 
**i2c transmit test function
** fd : i2c adapter fd
** msg_num : the num of msg in one io.
** msg_len: the buf len of each msg
*/

int i2c_transmit_test(int fd, int msg_num, int msg_len)
{
	int i, ret;
	unsigned short device_addr = 0x155;
	unsigned char *rw_buf;
	unsigned char *data_buf;
	struct timeval t1;
	struct i2c_rdwr_ioctl_data i2c_data;
	int fd_device;
	int state = 0;
	fd_device = open("/dev/i2c-dev",O_RDWR);
	if(fd_device < 0){
		printf("Can't open /dev/i2c-dev, please mknod /dev/shm first.\n");
		return -1;
	}
	printf("i2c transmit test start, msg_num is %d, msg_len is %d, transmit data size is %d\n", msg_num, msg_len, msg_num * msg_len);
	i2c_data.msgs =
			(struct i2c_msg *)malloc(msg_num * sizeof(struct i2c_msg));
	if (i2c_data.msgs == NULL) {
		perror("malloc error");
		exit(1);
	}

	i2c_data.nmsgs = msg_num;

	rw_buf = (unsigned char *)malloc(msg_num * msg_len * sizeof(char));
	data_buf = (unsigned char *)malloc(msg_num * msg_len * sizeof(char));
	if(rw_buf == NULL  || data_buf == NULL){
		perror("malloc error");
		exit(1);
	}	


	memset(data_buf, 0 , msg_num * msg_len);
	memset(rw_buf, 0, msg_len * msg_num);
	for (i = 0; i < msg_num; i++)
	{
		int j;
		i2c_data.msgs[i].len = msg_len;
		i2c_data.msgs[i].addr = device_addr;
		i2c_data.msgs[i].flags = I2C_M_TEN; 	/* write */


		i2c_data.msgs[i].buf = (unsigned char *)malloc(msg_len * sizeof(char));
		if(i2c_data.msgs[i].buf == NULL)
			printf("alloc memory fail!\n");

		memset(i2c_data.msgs[i].buf, 0, msg_len);
		for(j = 0; j < msg_len; j++)
		{
			gettimeofday(&t1, NULL);
			i2c_data.msgs[i].buf[j] = (u8)t1.tv_usec;		
		}
		memcpy(data_buf + i * msg_len, i2c_data.msgs[i].buf, msg_len);
		
	}
	ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
	if (ret < 0) {
		perror("write or read data error");	
		state = -1;
	}else {
		if(!read(fd_device,rw_buf, msg_len * msg_num)){	
			if(data_cmp(rw_buf, data_buf, msg_num * msg_len)){
				printf("transmit message differs from read back message.\n");
				printf("The read bcak message:\n");
				print_buf(rw_buf, msg_num * msg_len);
				printf("The transmit   message:\n");
				print_buf(data_buf ,msg_num * msg_len);
				state = -2;
			} else {
					printf("data compare ok!\n");
					state = 0;
					memcpy(write_data, data_buf, msg_num * msg_len);
				}	
		} else {	
			printf("Can't read msg from i2c device.\n");		
			state = -1;
		}
	}
	for(i = 0 ; i < msg_num; i++)
		free(i2c_data.msgs[i].buf);
	free(i2c_data.msgs);
	free(rw_buf);
	free(data_buf);
	close(fd_device);
	return state;
}

int i2c_recieve_test(int fd, int msg_num, int msg_len)
{
	int i, ret;
	unsigned short device_addr = 0x155;
	unsigned char *rw_buf;
	unsigned char *data_buf;
	struct i2c_rdwr_ioctl_data i2c_data;
	int fd_device;
	int state = 0;
	fd_device = open("/dev/i2c-dev",O_RDWR);
	if(fd_device < 0){
		printf("Can't open /dev/i2c-dev, please mknod /dev/i2c-dev first.\n");
		return -1;
	}
	printf("i2c  receive test start, msg_num is %d, msg_len is %d, transmit data size is %d\n", msg_num, msg_len, msg_num * msg_len);
	i2c_data.msgs =
			(struct i2c_msg *)malloc(msg_num * sizeof(struct i2c_msg));
	if (i2c_data.msgs == NULL) {
		perror("malloc error");
		exit(1);
	}

	i2c_data.nmsgs = msg_num;

	rw_buf = (unsigned char *)malloc(msg_num * msg_len * sizeof(char));
	data_buf = (unsigned char *)malloc(msg_num * msg_len * sizeof(char));
	if(rw_buf == NULL  || data_buf == NULL){
		perror("malloc error");
		exit(1);
	}	


	memset(data_buf, 0 , msg_num * msg_len);
	memset(rw_buf, 0, msg_len * msg_num);
	for (i = 0; i < msg_num; i++)
	{
		i2c_data.msgs[i].len = msg_len;
		i2c_data.msgs[i].addr = device_addr;
		i2c_data.msgs[i].flags = I2C_M_TEN | I2C_M_RD; 	/* read */


		i2c_data.msgs[i].buf = (unsigned char *)malloc(msg_len * sizeof(char));
		if(i2c_data.msgs[i].buf == NULL)
			printf("alloc memory fail!\n");

		memset(i2c_data.msgs[i].buf, 0, msg_len);
/*
		for(j = 0; j < msg_len; j++)
		{
			gettimeofday(&t1, NULL);
			i2c_data.msgs[i].buf[j] = (u8)t1.tv_usec;		
		}
		memcpy(data_buf + i * msg_len, i2c_data.msgs[i].buf, msg_len);
*/		
	}
	ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
	if (ret < 0) {
		perror("write or read data error");	
		state = -1;
	}else {
			for( i = 0; i < msg_num; i++)
			{

				memcpy(rw_buf + i * msg_len, i2c_data.msgs[i].buf, msg_len);
				free(i2c_data.msgs[i].buf);
			}

			if(data_cmp(rw_buf, write_data, msg_num * msg_len)){
				printf("transmit message differs from read back message.\n");
				printf("The read bcak message:\n");
				print_buf(rw_buf, msg_num * msg_len);
				printf("The transmit   message:\n");
				print_buf(write_data ,msg_num * msg_len);
				state = -2;
			} else {
					printf("data compare ok!\n");
					state = 0;
			}	
	}	
	free(i2c_data.msgs);
	free(rw_buf);
	free(data_buf);
	close(fd_device);
	return state;
}

int main(int argc, char *argv[])
{
	int fd, ret;
	int msg_num , msg_len;

	if(argc == 1){
		msg_num = 5;
		msg_len = 10;
	}else{
		if((argc == 3) && (atoi(argv[1]) >= 0) && (atoi(argv[2]) >= 0)){
			msg_num = atoi(argv[1]);
			msg_len = atoi(argv[2]);
		}else {
			printf("Usage error! please retry!\n");
			printf("usage: i2c_test msg_num msg_len\n");
			return 0;
		}
	}

	write_data = (u8 *)malloc(msg_len * msg_num * sizeof(char));
	fd = open("/dev/i2c-1", O_RDWR);
	if (fd < 0) {
		perror("open error");
		exit(1);
	}else
		printf("open i2c-1 success!\n");
	ret = i2c_transmit_test(fd,  msg_num, msg_len);	
	/*init write data to device, then i2c apater(/dev/i2c-1) will read data from device(/dev/i2c-device)*/
	ret =  write_data_init(ret);
	if(!ret)
		ret = i2c_recieve_test(fd, msg_num, msg_len);
	else 
		printf("Can't init write data, stop i2c recieve test!\n");
	close(fd);
	free(write_data);
	return ret;
}
