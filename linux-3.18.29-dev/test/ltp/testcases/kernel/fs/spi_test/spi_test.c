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
#include <pthread.h>
#include <linux/spi/spidev.h>
#include <asm-generic/errno-base.h>

#define MAX_SIZE  3*1024*1024
#define MAX_MSG_SIZE  4*1024
#define MAX_PRINT_MSG_LEN  1024
#define MAX_SPEED 50*1000*1000/4
#define MIN_SPEED 50*1000*1000/(254*256)
static const char *device = "/dev/spidev0.1";
static uint8_t mode;
//bpw 4-16
static uint8_t bits = 16;
//max speed 50mhz clk max /4  min /254*256
static uint32_t speed = 50*1000*1000/4;
static uint16_t delay = 0;

static char *tx = NULL;
static char *rx = NULL;

#define SIZE_TO_DMA 64
int test_pass = 1;
pthread_t tid[3];
static char * module_name = "SPI_TEST";
static void  print_msg(int mode, const char* fmt, ...){
	va_list args;
	char msg[MAX_PRINT_MSG_LEN] = {0};
	printf("%s",module_name);
	va_start(args, fmt);
	vprintf(fmt, args);
	puts("");
	sprintf(msg,"%s",module_name);
	vsprintf(msg + 6,fmt,args);
	tst_resm(mode, msg, NULL);
	va_end(args);
}

static int transfer(int fd, char * buf, int buf_size)
{
	int ret;
	int trans_size = 0;
	char* tx_send = buf;
	char* rx_send = rx;
	memset(rx,0,buf_size);
	do {
		buf_size -= MAX_MSG_SIZE;
		if(buf_size > 0)
		  trans_size = MAX_MSG_SIZE;
		else
		  trans_size = buf_size + MAX_MSG_SIZE;

		struct spi_ioc_transfer tr = {
			.tx_buf = (unsigned long)tx_send,
			.rx_buf = (unsigned long)rx_send,
			.len = trans_size,
			.delay_usecs = delay,
			.speed_hz = speed,
			.bits_per_word = bits,
		};
// 1000 transfor in one message
		ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
		if (ret < 1){
			printf(" spi test trans fail\n");
			print_msg(TFAIL,"spi_test fail :can't send spi message");
			return -1;
		}
		tx_send  += trans_size;
		rx_send  += trans_size;
	} while(buf_size > 0);
	return 0;
}

static int check_data_correct(int buf_size){
	if(memcmp(tx,rx,buf_size) != 0)
	  return -1;
	else
	  return 0;
}

int do_various_test(int fd){
	uint8_t bits_r = 0;
	uint32_t speed_r = 0;
	uint32_t i  = 0;
	int ret = 0;
	for(bits = 4; bits <= 16; bits++){
		ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
		if (ret == -1)
		  print_msg(TFAIL, "can't set bits per word");
		ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits_r);
		if (ret == -1)
		  print_msg(TFAIL, "can't set bits per word");
		if(bits != bits_r)
		  print_msg(TFAIL,"bits cmp fail bits %d bits_r %d",bits, bits_r);

		for(speed = 50*1000*1000/254*2; speed < MAX_SPEED; speed += 50*1000*1000/254*2){

			ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
			if (ret == -1)
			  print_msg(TFAIL, "can't set max speed hz");

			ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed_r);
			if (ret == -1)
			  print_msg(TFAIL,"can't get max speed hz");

			if(speed!= speed_r)
			  print_msg(TFAIL,"speed cmp fail speed %d speed_r %d", speed, speed_r);

			printf("spi mode: 0x%x\n", mode);
			printf("bits per word: %d\n", bits);
			printf("speed: %d Hz (%d KHz)\n", speed, speed/1000);

			if(transfer(fd, tx, 32) < 0){
				print_msg(TFAIL,"spi _test trans 32 bytes fail");
				return -1;
			}
			if(check_data_correct(32) < 0){
			  print_msg(TFAIL,"spi _test check data 32 bytes fail");
				return -1;
			}

			if(transfer(fd, tx, 17) < 0){
				print_msg(TFAIL,"spi _test trans 17 bytes fail");
				return -1;
			}
			if(check_data_correct(17) < 0){
				print_msg(TFAIL,"spi _test check data 17 bytes fail");
				return -1;
			}

			if(transfer(fd, tx, 128) < 0){
				print_msg(TFAIL,"spi _test trans 128 bytes fail");
				return -1;
			}
			if(check_data_correct(128) < 0){
			  print_msg(TFAIL,"spi _test check data 128 bytes fail");
			  return -1;
			}

			if(transfer(fd, tx, 1111) < 0){
				print_msg(TFAIL,"spi _test trans 1111 bytes fail");
				return -1;
			}
			if(check_data_correct(1111) < 0){
			  print_msg(TFAIL,"spi _test check data 1111 bytes fail");
			  return -1;
			}

			i = random();
			if(transfer(fd, tx,  i% (4*1024)) < 0){
				print_msg(TFAIL,"spi _test trans %d bytes fail",  i% (4*1024));
				return -1;
			}

			if(check_data_correct(i) < 0){
				print_msg(TFAIL,"spi _test check data %d bytes fail",  i% (4*1024));
				return -1;
			}

			if(transfer(fd, tx, 4*1024) < 0){
				print_msg(TFAIL,"spi _test trans %d bytes fail",  4*1024);
				return -1;
			}
			if(check_data_correct(4*1024) < 0){
				print_msg(TFAIL,"spi _test check data 4096 bytes fail");
				return -1;
			}
		}
	}
	return 0;
}

int do_mode_test(int fd, uint8_t mode){
	int ret = 0;
	uint8_t mode_r = 0;
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1){
		print_msg(TFAIL, "can't set spi mode");
		return -1;
	}

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode_r);
	if (ret == -1){
		print_msg(TFAIL,"can't get spi mode");
		return ret;
	}

	if(mode != mode_r){
		print_msg(TFAIL,"mode cmp fail mode %d mode_r %d", mode, mode_r);
		return -1;
	}

	return 0;
}

double calc_speed(struct timeval *t1, struct timeval *t2, int data_size){
	double timeuse;
	timeuse = t2->tv_sec - t1->tv_sec + (t2->tv_usec - t1->tv_usec)/1000000.0;
	return data_size/1024/timeuse;
}

void * do_speed_test(void * arg){

	struct timeval t1,t2;
	double speed_l = 0;
	struct spi_ioc_transfer tr[1024];
	int i = 0,ret = 0;
	int fd =(int) arg;

	for(;i < 1024; i++){
		tr[i].tx_buf = (unsigned long)(tx + 4 * 1024 * i);
		tr[i].rx_buf = (unsigned long)(rx + 4 * 1024 * i);
		tr[i].len = 4*1024;
		tr[i].delay_usecs = delay;
		tr[i].speed_hz = speed;
		tr[i].bits_per_word = bits;
	}

	printf("transfor start in speed test tid %d\n",(int)pthread_self());
	gettimeofday(&t1,NULL);
	// 1000 transfor in one message
	ret = ioctl(fd, SPI_IOC_MESSAGE(1024), &tr);
	if (ret < 1){
		print_msg(TFAIL,"spi_test fail :can't send spi message in speed test");
		test_pass = -1;
		return NULL;
	}
	gettimeofday(&t2,NULL);
	printf("transfor end in speed test tid %d\n",(int)pthread_self());
	if(check_data_correct(4*1024 * 1024) < 0)
	  print_msg(TFAIL,"spi_test check data 4096 KB fail");

	speed_l = calc_speed(&t1, &t2, 4*1024*1024);
	print_msg(TINFO,"spi _test data speed %lf KB/S",speed_l);
	return 0;
}

void * do_open_close_test(void * arg){
	int i = 0;
	for(;i < 1000; i++){
		int fd;
		fd = open(device, O_RDWR);
		if(transfer(fd, tx, 255) < 0){
			test_pass = -2;
			return NULL;
		}
		close(fd);
	}
	print_msg(TINFO,"open close test pass");
	return NULL;
}

int main(int argc, char *argv[]) {
	int ret = 0, err=0;
	int fd;
	int i = 0;

	print_msg(TINFO,"spi_test start");

	tx = malloc(MAX_SIZE);
	rx = malloc(MAX_SIZE);
	memset(tx,0,MAX_SIZE);
	memset(rx,0,MAX_SIZE);

	for(i = 0; i < MAX_SIZE; i++){
		tx[i] = i%128;
	}

	fd = open(device, O_RDWR);
	if (fd < 0){
		print_msg(TFAIL,"spi_test fail :can't open device");
		return -1;
	}

	/* * spi mode */

	mode = SPI_MODE_0;
	mode |= SPI_LOOP;
	if(do_mode_test(fd, mode) < 0)
	  return -1;

	if(do_various_test(fd) < 0)
	  return -1;

	mode = SPI_MODE_1;
	mode |= SPI_LOOP;
	if(do_mode_test(fd, mode) < 0)
	  return -1;

	if(do_various_test(fd) < 0)
	  return -1;

	mode = SPI_MODE_2;
	mode |= SPI_LOOP;
	if(do_mode_test(fd, mode) < 0)
	  return -1;

	if(do_various_test(fd) < 0)
	  return -1;

	mode = SPI_MODE_3;
	mode |= SPI_LOOP;
	if(do_mode_test(fd, mode) < 0)
	  return -1;

	if(do_various_test(fd) < 0)
	  return -1;


	speed = MAX_SPEED;
	bits = 16;

	err = pthread_create(&(tid[0]), NULL, &do_open_close_test, (void *)fd);
	if (err != 0)
	  printf("\ncan't create thread :[%s]", strerror(err));
	else
	  printf("\n Thread %d created successfully\n", (int)tid[0]);

	err = pthread_create(&(tid[1]), NULL, &do_speed_test, (void *)fd);
	if (err != 0)
	  printf("\ncan't create thread :[%s]", strerror(err));
	else
	  printf("\n Thread %d created successfully\n",(int)tid[1]);

	err = pthread_create(&(tid[2]), NULL, &do_speed_test, (void *)fd);
	if (err != 0)
	  printf("\ncan't create thread :[%s]", strerror(err));
	else
	  printf("\n Thread %d created successfully\n", (int)tid[2]);

	while(1){
		int kill_0 = pthread_kill(tid[0],0);
		int kill_1 = pthread_kill(tid[1],0);
		int kill_2 = pthread_kill(tid[2],0);

		if(kill_0 == EINVAL)
		  printf("spi_test the specified thread %d is alive \n", (int)tid[0]);

		if(kill_1 == EINVAL)
		  printf("spi_test the specified thread %d is alive\n", (int)tid[1]);

		if(kill_2 == EINVAL)
		  printf("spi_test the specified thread %d is alive\n", (int)tid[2]);

		if((kill_0 == ESRCH) && (kill_1 == ESRCH) && (kill_2 == ESRCH))
			break;

		sleep(5);
	}

	close(fd);
	if(test_pass >= 0)
	  print_msg(TPASS,"=============Spi Test pass");
	else
	  print_msg(TFAIL,"=============Spi Test fail");

	tst_exit();
	return ret;
}
