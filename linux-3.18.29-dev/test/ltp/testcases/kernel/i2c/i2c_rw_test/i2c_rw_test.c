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
static int test_pass = 1;

double calc_speed(struct timeval t1, struct timeval t2, int data_size)
{
	double timeuse;
	timeuse = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;
	return data_size / 1024 / timeuse;
}

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

void i2c_write_speed_test(void *arg)
{
	int i, ret;
	int fd = (int)arg;
	unsigned char rdwr_addr = 0x34; /* e2prom 内部寄存器读写地址, */
	/* e2prom 设备在i2c总线上的地址, codec is 0x1a, eeprom is 0x50*/
	unsigned char device_addr = 0x1a;
	unsigned char data = 128; /* 向e2prom写的数据 */
	struct i2c_rdwr_ioctl_data e2prom_data;
	unsigned short len = 2;
	struct timeval t1, t2;
	double speed_l = 0;

	e2prom_data.msgs =
			(struct i2c_msg *)malloc(1024 * sizeof(struct i2c_msg));
	if (e2prom_data.msgs == NULL) {
		perror("malloc error");
		exit(1);
	}
	e2prom_data.nmsgs = 500;

	for (i = 0; i < 500; i++) {
		e2prom_data.msgs[i].len = len;
		e2prom_data.msgs[i].addr = device_addr;
		e2prom_data.msgs[i].flags = 0; /* write */

		e2prom_data.msgs[i].buf =
				(unsigned char *)malloc(2 * sizeof(char));
		e2prom_data.msgs[i].buf[0] = rdwr_addr; /* write address */
		e2prom_data.msgs[i].buf[1] = data;      /* write data */
	}
	gettimeofday(&t1, NULL);
	ret = ioctl(fd, I2C_RDWR, (unsigned long)&e2prom_data);
	if (ret < 1) {
		printf("i2c test trans in speed test fail\n");
		print_msg(TFAIL, "i2c test fail: can't send i2c message in "
				 "speed test");
	}
	gettimeofday(&t2, NULL);
	speed_l = calc_speed(t1, t2, len * i);
	print_msg(TINFO, "i2c test data speed %lf KB/s", speed_l);
}

void i2c_rw_test(int fd)
{
	int i, ret;
	unsigned char rdwr_addr = 0x34; /* e2prom 内部寄存器读写地址, */
	/* e2prom 设备在i2c总线上的地址, codec is 0x1a, eeprom is 0x50*/
	unsigned char device_addr = 0x1a;
	unsigned char data = 128; /* 向e2prom写的数据 */
	struct i2c_rdwr_ioctl_data e2prom_data;
	e2prom_data.msgs =
			(struct i2c_msg *)malloc(100 * sizeof(struct i2c_msg));
	if (e2prom_data.msgs == NULL) {
		perror("malloc error");
		exit(1);
	}

	/* i = 0 for 7 bit addrs, i = 1 for 10 bit addrs */
	for (i = 0; i <= 1; i++)
	{
		/*向e2prom的rdwr_addr地址写入数据data*/
		e2prom_data.nmsgs = 1;
		e2prom_data.msgs[0].len = 2;
		e2prom_data.msgs[0].addr = device_addr;
		e2prom_data.msgs[0].flags = 0; /* write */

		e2prom_data.msgs[0].buf =
				(unsigned char *)malloc(2 * sizeof(char));
		e2prom_data.msgs[0].buf[0] = rdwr_addr; /* write address */
		e2prom_data.msgs[0].buf[1] = data;      /* write data */

		ret = ioctl(fd, I2C_RDWR, (unsigned long)&e2prom_data);
		if (ret < 0) {
			perror("write data error");
			exit(1);
		}
		printf("write data: %d to address: %#x\n", data, rdwr_addr);
		data = 0; /* be zero*/

		/*从e2prom的rdwr_addr地址读取数据存入buf*/
		e2prom_data.nmsgs = 2;
		e2prom_data.msgs[0].len = 1;
		e2prom_data.msgs[0].addr = device_addr;
		e2prom_data.msgs[0].flags = (i ? I2C_M_TEN : 0); /* write */
		e2prom_data.msgs[0].buf = &rdwr_addr;

		e2prom_data.msgs[1].len = 1;
		e2prom_data.msgs[1].addr = device_addr;
		e2prom_data.msgs[1].flags =
				I2C_M_RD | (i ? I2C_M_TEN : 0); /* read */
		ret = ioctl(fd, I2C_RDWR, (unsigned long)&e2prom_data);
		if (ret < 0) {
			perror("read error");
			exit(1);
		}
		printf("read  data: %d from address: %#x\n", data, rdwr_addr);
	}
}

int main(int argc, char *argv[])
{
	int fd, err;
	int kill_0, kill_1, kill_2;
	char *device;
	pthread_t tid[3];

//	device = getenv ("DEVICE");
//	printf("device = %s\n", device);

	fd = open("/dev/i2c-1", O_RDWR);
	if (fd < 0) {
		perror("open error");
		exit(1);
	}

	i2c_ioctl_test(fd);
	i2c_rw_test(fd);

	// we test write operation and multithreading here.
	err = pthread_create(&(tid[0]), NULL, (void *)i2c_write_speed_test, (void *)fd);
	if (err != 0)
		printf("can't create thread :[%s]\n", strerror(err));
	else
		printf("Thread %d created successfully\n", (int)tid[0]);

	err = pthread_create(&(tid[1]), NULL, (void *)i2c_write_speed_test, (void *)fd);
	if (err != 0)
		printf("can't create thread :[%s]\n", strerror(err));
	else
		printf("Thread %d created successfully\n", (int)tid[1]);

	err = pthread_create(&(tid[2]), NULL, (void *)i2c_write_speed_test, (void *)fd);
	if (err != 0)
		printf("can't create thread :[%s]\n", strerror(err));
	else
		printf("Thread %d created successfully\n", (int)tid[2]);

	while (1) {
		kill_0 = pthread_kill(tid[0], 0);
		kill_1 = pthread_kill(tid[1], 0);
		kill_2 = pthread_kill(tid[2], 0);

		if (kill_0 == EINVAL)
			printf("i2c test the specified thread %d is alive\n", (int)tid[0]);

		if (kill_1 == EINVAL)
			printf("i2c test the specified thread %d is alive\n", (int)tid[1]);

		if (kill_2 == EINVAL)
			printf("i2c test the specified thread %d is alive\n", (int)tid[2]);

		if ((kill_0 == ESRCH) && (kill_1 == ESRCH) && (kill_2 == ESRCH))
			break;

		sleep(5);
	}

	close(fd);
	if (test_pass)
		print_msg(TPASS, "=============i2c test pass\n");
	else
		print_msg(TFAIL, "=============i2c test fail\n");

	return 0;
}
