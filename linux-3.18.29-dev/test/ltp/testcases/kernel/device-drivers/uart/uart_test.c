/*
 * =====================================================================================
 *
 *       Filename:  uart_test.c
 *
 *    Description:  test uart driver
 *
 *        Version:  1.0
 *        Created:  2016年10月11日 15时22分38秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  robert , robert.chang@siflower.com.cn
 *        Company:  Siflower
 *	     refs:  http://blog.chinaunix.net/uid-21411227-id-1826767.html
 *	     refs:  linux-3.18.29-dev/linux-3.18.29/Documentation/serial/driver
 *
 * =====================================================================================
 */
//#include "test.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/types.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define FALSE -1
#define TRUE 0

/*
 * 测试须知：
 * 在该测试中，uart0用来接串口线，在pc上输出日志；
 * uart1与uart3在接线的时候比较方便，所以选用uart1与uart3相连来测试
 * uart1 tx与uart3 rx相连，uart1 rx与uart3 tx相连，
 * uart1 cts与uart3 rts相连，uart3rts与uart1 cts相连，所有测试均是测量这两个uart
 * 如果需要测试其他uart，只需修改TTY_DEV0，TTY_DEV1，TTY_DEV3的值即可
 *
 * 测试覆盖的内容：
 * 波特率（115200, 576000, 38400, 19200, 9600, 4800, 2400, 1200, 300, 38400,
 * 19200, 9600, 4800, 2400, 1200, 300,）
 * 数据位数（5,6,7,8,）
 * 奇偶校验（奇校验，偶校验）
 * 停止位（1位，2位）
*/

typedef struct {
	char *option;
	int  *flag;
	char **arg;
} option_t;

struct uart_fd {
	int fd1;
	int fd2;
};

char *TTY_DEV0 = "/dev/ttyS0";
char *TTY_DEV1 = "/dev/ttyS1";
char *TTY_DEV2 = "/dev/ttyS3";

static int dflag;
static const option_t options[] = {
		{"d:", &dflag, &TTY_DEV1}, {NULL, NULL, NULL}};
pthread_t tid[3];
static int testret = 0;



static void help(void)
{
	printf("-d x uart device node, default is %s\n", TTY_DEV1);
}

int speed_arr[] = {B115200, B576000, B38400, B19200, B9600, B4800, B2400, B1200,
		B300};

int name_arr[] = {115200, 576000, 38400, 19200, 9600, 4800, 2400, 1200, 300};

int data_bits[] = {7, 8};
//int data_bits[] = {5, 6, 7, 8};

int stop_bits[] = {1, 2};

char parity_bits[] = {'n', 'o', 'e', 's'};

void set_speed(int fd, int speed)
{
	unsigned int i;
	int status;
	struct termios opt;
	tcgetattr(fd, &opt);
	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {
			//清空终端未完成的输入/输出请求及数据。
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&opt, speed_arr[i]);
			cfsetospeed(&opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &opt);
			tcgetattr(fd, &opt);
//			printf("get c_cflag = 0x%x\n", opt.c_cflag);
//			printf("get c_iflag = 0x%x\n", opt.c_iflag);
//			printf("get c_oflag = 0x%x\n", opt.c_oflag);
			if (status != 0) {
				perror("tcsetattr fd");
				return;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
}

/* in linux-3.18.29-dev/linux-3.18.29/Documentation/serial/driver

   set_termios(port,termios,oldtermios)
	Change the port parameters, including word length, parity, stop
	bits.  Update read_status_mask and ignore_status_mask to indicate
	the types of events we are interested in receiving.  Relevant
	termios->c_cflag bits are:
		CSIZE	- word size
		CSTOPB	- 2 stop bits
		PARENB	- parity enable
		PARODD	- odd parity (when PARENB is in force)
		CREAD	- enable reception of characters (if not set,
			  still receive characters from the port, but
			  throw them away.
		CRTSCTS	- if set, enable CTS status change reporting
		CLOCAL	- if not set, enable modem status change
			  reporting.
	Relevant termios->c_iflag bits are:
		INPCK	- enable frame and parity error events to be
			  passed to the TTY layer.
		BRKINT
		PARMRK	- both of these enable break events to be
			  passed to the TTY layer.

		IGNPAR	- ignore parity and framing errors
		IGNBRK	- ignore break errors,  If IGNPAR is also
			  set, ignore overrun errors as well.
	The interaction of the iflag bits is as follows (parity error
	given as an example):
	Parity error	INPCK	IGNPAR
	n/a		0	n/a	character received, marked as
					TTY_NORMAL
	None		1	n/a	character received, marked as
					TTY_NORMAL
	Yes		1	0	character received, marked as
					TTY_PARITY
	Yes		1	1	character discarded

	Other flags may be used (eg, xon/xoff characters) if your
	hardware supports hardware "soft" flow control.

	Locking: none.
	Interrupts: caller dependent.
	This call must not sleep
*/

/* *
 *
 * *@brief 设置串口数据位，停止位和效验位
 * *@param fd 类型 int 打开的串口文件句柄
 * *@param databits 类型 int 数据位 取值 5/6/7/8
 * *@param stopbits 类型 int 停止位 取值为 1/2
 * *@param parity 类型 int 效验类型 取值为N,E,O,S
 * */

int set_parity(int fd, int databits, int stopbits, char parity)
{
	struct termios options;
	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return (FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) {
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data sizen");
		return (FALSE);
	}
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB; /* Clear parity enable */
		options.c_iflag &= ~INPCK;  /* Disable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验 */
		options.c_iflag |= INPCK; /* Enable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;  /* Enable parity */
		options.c_cflag &= ~PARODD; /* 转换为偶效验 */
		options.c_iflag |= INPCK;   /* Enable parity checking */
		break;
	case 'S':
	case 's':			    /* as no parity */
		options.c_cflag &= ~PARENB; /* Clear parity enable */
		options.c_iflag |= INPCK;   /* Enable parity checking */
		break;
	default:
		fprintf(stderr, "Unsupported parityn");
		return (FALSE);
	}
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bitsn");
		return (FALSE);
	}
	tcflush(fd, TCIFLUSH);
	options.c_cc[VTIME] = 150; /* 设置超时15 seconds */
	options.c_cc[VMIN] = 0;    /* Update the options and do it NOW */
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}

int opendev(char *Dev)
{
	int fd = open(Dev, O_RDWR);
	if (-1 == fd) {
		perror("Can't Open Serial Port");
		return -1;
	} else
		return fd;
}

static const char *TEST_BUFFER = "abcdefghijklmnopqrstuvwxyz 1234567890\n";

static ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	n = read(fd, buf, count);

	return n;
}

ssize_t full_read(int fd, void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_read(fd, buf, len);
		if (cc < 0) {
			if (total) {
				return total;
			}
			return cc;
		}
		if (cc == 0)
			break;
		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
		if (len <= 0)
			break;
	}

	return total;
}

static ssize_t safe_write(int fd, const void *buf, size_t count)
{
	ssize_t n;

	do {
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);
	if (n < 0)
		printf("get error %s \n", strerror(errno));
	return n;
}

ssize_t full_write(int fd, const void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_write(fd, buf, len);
		if (cc < 0) {
			if (total) {
				return total;
			}
			return cc;
		}

		total += cc;
		buf = ((const char *)buf) + cc;
		len -= cc;
	}
	return total;
}

void uart_stress_test(void *arg)
{
	int i = 0;
	int nread;
	struct uart_fd *fd = (struct uart_fd *)arg;
	int length = strlen(TEST_BUFFER);
	char buff[512] = {0};

	set_speed(fd->fd1, name_arr[0]);
	set_speed(fd->fd2, name_arr[0]);
	if ((set_parity(fd->fd1, data_bits[0], stop_bits[0], parity_bits[0]) == FALSE) ||
			(set_parity(fd->fd2, data_bits[0], stop_bits[0], parity_bits[0]) == FALSE)) {
		printf("Set Parity Error\n");
		testret = -1;
	}
	for (; i < 10; i++) {
		// uart1 tx is connected with uart3 rx
		full_write(fd->fd1, TEST_BUFFER, length);
		nread = full_read(fd->fd2, buff, length);
		if (nread > 0) {
			printf("read cnt = %d\n", nread);
			printf("buff = %s\n", buff);
		} else {
			testret = -1;
			printf("testret = %d\n", testret);
		}
	}
}

static int lock = 0;

int __uart_rw_test(struct uart_fd *fd)
{
	int nread;
	int length = strlen(TEST_BUFFER);
	char buff[512] = {0};
	while(1) {
		if (lock)
			continue;
		lock = 1;
		memset(buff, 0, sizeof(buff));
		full_write(fd->fd1, TEST_BUFFER, length);
		nread = full_read(fd->fd2, buff, length);
		if (nread != length || memcmp(TEST_BUFFER, buff, length)){
			lock = 0;
			return -1;
		}
		lock = 0;
		break;
	}
	return 0;
}

void uart_rw_test(void *arg)
{
	int i, j, k, m;
	int nread;
	struct uart_fd *fd = (struct uart_fd *)arg;
	int icount = sizeof(name_arr) / sizeof(int);
	int jcount = sizeof(stop_bits) / sizeof(int);
	int kcount = sizeof(data_bits) / sizeof(int);
	int mcount = sizeof(parity_bits) / sizeof(char);
	printf("icount = %d, jcount = %d, kcount = %d, mcount = %d\n", icount, jcount, kcount, mcount);
	for (i = 0; i < icount; i++) {
//		set_speed(fd->fd1, name_arr[i]);
//		set_speed(fd->fd2, name_arr[i]);
		printf("test uart speed %d\n", name_arr[i]);
		for (j = 0; j < jcount; j++) {
			for (k = 0; k < kcount; k++) {
				for (m = 0; m < mcount; m++) {
					// set parity/data/stop bits
//					if ((set_parity(fd->fd1, data_bits[k], stop_bits[j], parity_bits[m]) == FALSE) ||
//						(set_parity(fd->fd2, data_bits[k], stop_bits[j], parity_bits[m]) == FALSE)) {
//						printf("Set Parity Error\n");
//						testret = -1;
//					}
					// uart1 tx is connected with uart3 rx
#if 0
					full_write(fd->fd1, TEST_BUFFER, length);
					printf("i = %d, j = %d, k = %d, m = %d\n",
							i, j, k, m);
					memset(buff, 0, sizeof(buff));
					nread = full_read(fd->fd2, buff, length);
					if (nread != length || memcmp(TEST_BUFFER, buff, length)){
						testret--;
						printf("nread = %d, testret = %d\n",nread, testret);
						// TODO:maybe we should do sth
						// more when it's wrong
					}
#endif
					if (__uart_rw_test(fd) == -1){
						testret--;
						printf("testret = %d\n", testret);
					}
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	struct uart_fd fd;
	int err;
	int ret;
	struct termios save_opt1, save_opt2;
	int kill_0, kill_1, kill_2;
	int length = strlen(TEST_BUFFER);
	int nread;
	int i;
	char buff[512] = {0};
	char *TEST_BUFFER1 = "abcdefghijklmnopqrstuvwxyz 1234567890";

	printf("uart test start!\n");
	printf("uart test open device\n");

	ret = 0;
	fd.fd1 = opendev(TTY_DEV1);
	if (fd.fd1 < 0) {
		printf("open %s fail\n", TTY_DEV1);
		ret = -1;
		goto EXIT;
	}
	fd.fd2 = opendev(TTY_DEV2);
	if (fd.fd2 < 0) {
		printf("open %s fail\n", TTY_DEV2);
		ret = -1;
		goto EXIT;
	}

	// TODO: test uart when it is using dma
	//	test_default_speed();
	// save opts first
	tcgetattr(fd.fd1, &save_opt1);
	tcgetattr(fd.fd2, &save_opt2);
	printf("open end\n");
#if 1
		while(1) {
			if (lock)
				continue;
			lock = 1;
			memset(buff, 0, sizeof(buff));
			full_write(fd.fd1, TEST_BUFFER, length);
			nread = full_read(fd.fd2, buff, length);
			printf("read cnt = %d, buff = %s, TEST_BUFFER = %s\n", nread, buff, TEST_BUFFER);
			char* buff1 = buff;
			buff1++;
			if (memcmp(TEST_BUFFER1, buff, length-1) == 0){
				printf("memcmp success\n");
				i = 0;
			} else if (memcmp(TEST_BUFFER1, buff1, length-1) == 0){
				printf("memcmp success\n");
				i = 0;
			} else {
				printf("memcmp fail\n");
				i = 1;
			}
			lock = 0;
			break;
		}
#else
	//uart_stress_test((void *)&fd);
	printf("start rw test\n");
	uart_rw_test((void *)&fd);

	printf("thread test start, testret = %d\n", testret);
#if 0
	err = pthread_create(&(tid[0]), NULL, (void *)uart_rw_test, (void *)&fd);
	if (err != 0)
		printf("can't create thread :[%s]\n", strerror(err));
	else
		printf("Thread %d created successfully\n", (int)tid[0]);

	err = pthread_create(&(tid[1]), NULL, (void *)uart_rw_test, (void *)&fd);
	if (err != 0)
		printf("can't create thread :[%s]\n", strerror(err));
	else
		printf("Thread %d created successfully\n", (int)tid[1]);

	err = pthread_create(&(tid[2]), NULL, (void *)uart_rw_test, (void *)&fd);
	if (err != 0)
		printf("can't create thread :[%s]\n", strerror(err));
	else
		printf("Thread %d created successfully\n", (int)tid[2]);

	while (1) {
		kill_0 = pthread_kill(tid[0], 0);
		kill_1 = pthread_kill(tid[1], 0);
		kill_2 = pthread_kill(tid[2], 0);

		if (kill_0 == EINVAL)
			printf("uart test the specified thread %d is alive\n", (int)tid[0]);

		if (kill_1 == EINVAL)
			printf("uart test the specified thread %d is alive\n", (int)tid[1]);

		if (kill_2 == EINVAL)
			printf("uart test the specified thread %d is alive\n", (int)tid[2]);

		if ((kill_0 == ESRCH) && (kill_1 == ESRCH) && (kill_2 == ESRCH))
			break;

		sleep(5);
	}
#endif
#endif

	close(fd.fd1);
	close(fd.fd2);
	return i;
EXIT:
	if (fd.fd1 > 0) {
		tcsetattr(fd.fd1, TCSANOW, &save_opt1);
		close(fd.fd1);
	}
	if (fd.fd2 > 0) {
		tcsetattr(fd.fd1, TCSANOW, &save_opt2);
		close(fd.fd2);
	}
	if (ret < 0)
		printf("uart test fail\n");
	printf("UART Tests Done!\n");
	exit(0);
}
