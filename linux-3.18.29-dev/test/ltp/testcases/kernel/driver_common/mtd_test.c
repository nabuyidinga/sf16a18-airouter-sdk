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
#include <mtd/mtd-abi.h>
#include <linux/blkpg.h>
#include "driver_test.h"

struct speed_wrapper{
	int fd;
	unsigned char * tx;
	unsigned char * rx;
	int size;
};
int test_pass = 0;
pthread_t tid[3];
static char device[10];
static char * module_name = "MTD_TEST";
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

static int check_data_correct(unsigned char * tx, unsigned char * rx,int buf_size){
	if(memcmp(tx,rx,buf_size) != 0)
	  return -1;
	else
	  return 0;
}


void * do_speed_test(void * arg){
	int offset = 0, ret = 0;
	struct timeval t1,t2;
	struct speed_wrapper * data = (struct speed_wrapper *)arg;
	double speed_l = 0;
	struct erase_info_user einfo32;

	offset = lseek(data->fd,0, SEEK_SET);
	if(offset < 0){
		print_msg(TFAIL,"seek data fail");
		test_pass = -1;
		return NULL;
	}

	gettimeofday(&t1,NULL);

	einfo32.start = 0;
	einfo32.length= data->size;
	ret = ioctl(data->fd, MEMERASE, &einfo32);
	if (ret == -1){
		test_pass = -1;
		print_msg(TFAIL,"can't earse flash");
		return NULL;
	}
	gettimeofday(&t2,NULL);
	speed_l = calc_speed(&t1,&t2,data->size);
	print_msg(TINFO,"earse data speed %lf KB/S",speed_l);

	gettimeofday(&t1,NULL);
	if(write(data->fd, data->tx, data->size) != data->size){
		print_msg(TFAIL,"speed_test write data fail");
		test_pass = -1;
		return NULL;
	}
	gettimeofday(&t2,NULL);
	speed_l = calc_speed(&t1,&t2,data->size);
	print_msg(TINFO,"write data speed %lf KB/S",speed_l);

	offset = lseek(data->fd,0, SEEK_SET);
	if(offset < 0){
		print_msg(TFAIL,"seek data fail");
		test_pass = -1;
		return NULL;
	}

	gettimeofday(&t1,NULL);
	if(read(data->fd, data->rx, data->size) != data->size){
		print_msg(TFAIL," speed read data fail");
		test_pass = -1;
		return NULL;
	}
	gettimeofday(&t2,NULL);
	speed_l = calc_speed(&t1,&t2,data->size);
	print_msg(TINFO,"read data speed %lf KB/S",speed_l);

	if(check_data_correct(data->tx, data->rx, data->size) != 0){
		test_pass = -1;
		print_msg(TFAIL,"speed check data fail");
		return NULL;
	}
	return NULL;
}

int do_read_write_test(int fd, unsigned char* tx, unsigned char* rx,  int size){
	int offset = 0;

	offset = lseek(fd,0, SEEK_SET);
	if(offset < 0){
		print_msg(TFAIL,"seek data fail 1");
		return -1;
	}

	if(write(fd, tx, size) != size){
		print_msg(TFAIL,"write data fail");
		return -1;
	}
	offset = lseek(fd,0, SEEK_CUR);
	if(offset <= 0){
		print_msg(TFAIL,"seek data fail 2");
		return -1;
	}

	offset = lseek(fd,0, SEEK_SET);
	if(offset < 0){
		print_msg(TFAIL,"seek data fail");
		return -1;
	}

	if(read(fd, rx, size) != size){
		print_msg(TFAIL,"read data fail");
		return -1;
	}
	if(check_data_correct(tx, rx, size) != 0){
		print_msg(TFAIL,"check data fail");
		return -1;
	}
	return 0;
}

int  mtd_test(char* arg, int fd_in){
	int fd, region_count = 0, ret = 0, err = 0;
	struct mtd_info_user info;
	struct erase_info_user einfo32;
	struct blkpg_ioctl_arg  blk_info;
	struct blkpg_partition  blk_prt;
	struct speed_wrapper  data;
	int i = 0;

	print_msg(TINFO,"mtd_test start %s",arg);
	sprintf(device,"/dev/mtd%s",arg);
	fd = open(device, O_RDWR);
	if (fd < -1){
		print_msg(TFAIL,"mtd_test fail :can't open device");
		return -1;
	}

	ret = ioctl(fd, MEMGETREGIONCOUNT, &region_count);
	if (ret == -1){
		print_msg(TFAIL,"can't get region_count");
		return ret;
	}

	print_msg(TINFO,"get region_count = %d",region_count);

	ret = ioctl(fd, MEMGETINFO, &info);
	if (ret == -1){
		print_msg(TFAIL,"can't get mtd info");
		return ret;
	}

	print_msg(TINFO,"get mtd type = %d", info.type);
	print_msg(TINFO,"get mtd flags = 0x%x", info.flags);
	print_msg(TINFO,"get mtd size = %d",info.size);
	print_msg(TINFO,"get mtd earse size = %d",info.erasesize);
	print_msg(TINFO,"get mtd write size = %d",info.writesize);
	print_msg(TINFO,"get mtd oob size = %d",info.oobsize);
	unsigned char * tx = malloc(info.size);
	unsigned char * rx = malloc(info.size);
	memset(tx,'a',info.size);
	memset(rx, 0,info.size);

	einfo32.start = 0;
	einfo32.length= info.size;
	ret = ioctl(fd, MEMERASE, &einfo32);
	if (ret == -1){
		print_msg(TFAIL,"can't earse flash");
		return ret;
	}

	if(read(fd, rx, 64) != 64){
		print_msg(TFAIL,"read data fail");
		return -1;
	}
    for(i = 0; i < 64; i++){
		if(rx[i] != 0xFF){
			print_msg(TFAIL," check earse flash fail");
			return -1;
		}
	}

#if 0
	memcpy(blk_prt.devname,module_name,6);
	blk_prt.start = 0;
	blk_prt.length = info.size/2;

	blk_info.op = BLKPG_ADD_PARTITION;
	blk_info.data = &blk_prt;

	ret = ioctl(fd, BLKPG, &blk_info);
	if (ret == -1){
		print_msg(TFAIL,"can't add partation ");
		return ret;
	}

	blk_prt.pno = 0;
	blk_info.op = BLKPG_DEL_PARTITION;
	blk_info.data = &blk_prt;
	ret = ioctl(fd, BLKPG, &blk_info);
	if (ret == -1){
		print_msg(TFAIL,"can't del partation ");
		return ret;
	}
#endif
	if(do_read_write_test(fd,tx,rx,64) < 0 ){
		print_msg(TFAIL,"test 64 byte read write fail");
		return -1;
	}
	if(do_read_write_test(fd,tx,rx,117) < 0 ){
		print_msg(TFAIL,"test 117 byte read write fail");
		return -1;
	}
	if(do_read_write_test(fd,tx,rx, 311) < 0 ){
		print_msg(TFAIL,"test 311 byte read write fail");
		return -1;
	}
	if(do_read_write_test(fd,tx,rx,info.size/2) < 0 ){
		print_msg(TFAIL,"test %d byte read write fail", info.size/2);
		return -1;
	}
	data.fd = fd;
	data.tx = tx;
	data.rx = rx;
	data.size = info.size;
	err = pthread_create(&(tid[0]), NULL, &do_speed_test, (void *)&data);
	if (err != 0)
	  printf("\ncan't create thread :[%s]", strerror(err));
	else
	  printf("\n Thread %d created successfully\n", (int)tid[0]);


	while(1){
		int kill_0 = pthread_kill(tid[0],0);

		if(kill_0 == EINVAL)
		  printf("spi_test the specified thread %d is alive \n", (int)tid[0]);

		if(kill_0 == ESRCH )
			break;

		sleep(5);
	}

	close(fd);

	if(test_pass >= 0)
	  print_msg(TPASS,"=============mtd Test pass");
	else
	  print_msg(TFAIL,"=============mtd Test fail");


	/* not support now
	   ret = ioctl(fd, MEMLOCK, &einfo32);
	   if (ret == -1){
	   print_msg(TFAIL,"can't earse flash");
	   return ret;
	   }

	   ret = ioctl(fd, MEMUNLOCK, &einfo32);
	   if (ret == -1){
	   print_msg(TFAIL,"can't earse flash");
	   return ret;
	   }
	*/
	return 0;
}
