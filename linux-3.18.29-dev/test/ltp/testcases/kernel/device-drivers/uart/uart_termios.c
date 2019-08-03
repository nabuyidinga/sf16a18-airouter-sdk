/*
 * =====================================================================================
 *
 *       Filename:  uart_termios.c
 *
 *    Description:  set uart parameter
 *
 *        Version:  1.0
 *        Created:  2016年11月11日 11时09分38秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  austin, austin.xu@siflower.com.cn
 *        Company:  Siflower
 *	     refs:  http://blog.chinaunix.net/uid-21411227-id-1826767.html
 *	     refs:  linux-3.18.29-dev/linux-3.18.29/Documentation/serial/driver
 *
 * =====================================================================================
 */
#include "test.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/types.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define FALSE -1
#define TRUE 0

/*
 * 测试须知：
 * 该测试通过手动传参数来设置uart的参数
 * ./uart_termios -D -d -b -s -p
 * 测试覆盖的内容：
 * -D 0/1/2/3 对应uart0/1/2/3(默认为uart0)
 * -d 5/6/7/8 对应数据位数5/6/7/8(默认为8)
 * -b 115200/576000/38400/.../300 对应相应波特率(默认为不修改)
 * -s 1/2 对应停止位1位/2位(默认为1位)
 * -p n/o/e/s 对应无奇偶校验/奇校验/偶校验/无奇偶校验(默认为无奇偶校验)
*/

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

char *TTY_DEV0 = "/dev/ttyS0";
char *TTY_DEV1 = "/dev/ttyS1";
char *TTY_DEV2 = "/dev/ttyS2";
char *TTY_DEV3 = "/dev/ttyS3";

int speed_arr[] = {B115200, B576000, B38400, B19200, B9600, B4800, B2400, B1200,
		B300};

int name_arr[] = {115200, 576000, 38400, 19200, 9600, 4800, 2400, 1200, 300};

int data_bits[] = {5, 6, 7, 8};
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
			if (status != 0) {
				perror("tcsetattr fd");
				return;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
}

static int parse_num(unsigned *num, const char *str)
{
	unsigned long val;
	char *end;

	errno = 0;
	val = strtoul(str, &end, 0);
	if (errno || *end || val > UINT_MAX)
		return -1;
	*num = val;
	return 0;
}

static int set_parity(int fd, int databits, int stopbits, int parity)
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
	default:
		options.c_cflag |= CS8;
		break;
	}
#if 1
	switch (parity) {
	case 0:
		options.c_cflag &= ~PARENB; /*  Clear parity enable */
		options.c_iflag &= ~INPCK;  /*  Disable parity checking */
		break;
	case 1:
		options.c_cflag |= (PARODD | PARENB); /*  设置为奇效验 */
		options.c_iflag |= INPCK; /*  Enable parity checking */
		break;
	case 2:
		options.c_cflag |= PARENB;  /*  Enable parity */
		options.c_cflag &= ~PARODD; /*  设置为偶效验 */
		options.c_iflag |= INPCK;   /*  Enable parity checking */
		break;
	default:
		break;
	}
#else
	printf("parity = %d\n",parity);
	switch (parity) {
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB); /*  设置为奇效验 */
		options.c_iflag |= INPCK; /*  Enable parity checking */
		break;
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB; /*  Clear parity enable */
		options.c_iflag &= ~INPCK;  /*  Disable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;  /*  Enable parity */
		options.c_cflag &= ~PARODD; /*  转换为偶效验 */
		options.c_iflag |= INPCK;   /*  Enable parity checking */
		break;
	case 'S':
	case 's':			    /* as no parity*/
		options.c_cflag &= ~PARENB; /*  Clear parity enable */
		options.c_iflag |= INPCK;   /*  Enable parity checking */
		break;
	default:
		break;
	}
#endif
	switch (stopbits) {
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	case 1:
	default:
		options.c_cflag &= ~CSTOPB;
		break;
	}
	tcflush(fd, TCIFLUSH);
	options.c_cc[VTIME] = 150; /*  设置超时15 seconds*/
	options.c_cc[VMIN] = 0;    /*  Update the options and do it NOW */
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}

int opendev(char *dev)
{
	int fd = open(dev, O_RDWR);
	if (-1 == fd) {
		perror("Can't Open Serial Port");
		return -1;
	} else
		return fd;
}

static void get_parity(int fd)
{
	unsigned int i;
	struct termios options;
	int databits, stopbits, parity, baud;

	tcgetattr(fd, &options);
	switch (options.c_cflag & CSIZE) {
	case CS5:
		databits = 5;
		break;
	case CS6:
		databits = 6;
		break;
	case CS7:
		databits = 7;
		break;
	case CS8:
		databits = 8;
		break;
	default:
		databits = -1;
		break;
	}

	switch (options.c_cflag & CSTOPB) {
	case CSTOPB:
		stopbits = 2;
		break;
	case 0:
		stopbits = 1;
		break;
	default:
		stopbits = -1;
		break;
	}

	if (options.c_cflag & PARENB) {
		if (options.c_cflag & PARODD)
			parity = 1;
		else
			parity = 2;
	} else
		parity = 0;

	baud = options.c_cflag & CBAUD;
	for (i = 0; i < ARRAY_SIZE(speed_arr); i++) {
		if (baud == speed_arr[i]) {
			baud = name_arr[i];
			break;
		}
	}

	printf("databits = %d, stopbits = %d, parity = %d, baud = %d\n",
			databits, stopbits, parity, baud);
}

int main(int argc, char **argv)
{
	int fd;
	char *dev;
	unsigned int device_num = 0;
	int c;
	unsigned int databits, stopbits, parity, baud = 0;

	while ((c = getopt(argc, argv, "D:d:s:p:b:")) != EOF)
		switch (c) {
		case 'D': /* device */
			parse_num(&device_num, optarg);
			break;
		case 'd': /* databits */
			parse_num(&databits, optarg);
			break;
		case 's': /* stopbits */
			parse_num(&stopbits, optarg);
			break;
		case 'p': /* parity */
			parse_num(&parity, optarg);
			break;
		case 'b': /* baud */
			parse_num(&baud, optarg);
			break;
		case '?':
		default:
			break;
		}
	switch (device_num) {
		break;
	case 1:
		dev = TTY_DEV1;
		break;
	case 2:
		dev = TTY_DEV2;
		break;
	case 3:
		dev = TTY_DEV3;
		break;
	case 0:
	default:
		dev = TTY_DEV0;
		break;
	}
	fd = opendev(dev);
	printf("before uart%d set, ", device_num);
	get_parity(fd);
	set_parity(fd, databits, stopbits, parity);
	if (baud)
		set_speed(fd, baud);
	printf("after uart%d set, ", device_num);
	get_parity(fd);
	return 0;
}
