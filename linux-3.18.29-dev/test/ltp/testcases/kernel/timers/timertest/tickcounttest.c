#include <time.h>
#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>
#include "test.h"

// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

}

int main()
{	
	int i;
	struct timespec time1 = { 0, 0 };
	struct  timeval   tv;

	struct  timezone  tz;

	clock_gettime(CLOCK_REALTIME, &time1);
	tst_resm(TINFO,"CLOCK_REALTIME: %d, %d\n", time1.tv_sec, time1.tv_nsec);

	clock_gettime(CLOCK_MONOTONIC, &time1);
	tst_resm(TINFO,"CLOCK_MONOTONIC: %d, %d\n", time1.tv_sec, time1.tv_nsec);

	clock_gettime(CLOCK_MONOTONIC_RAW, &time1);
	tst_resm(TINFO,"CLOCK_MONOTONIC_RAW: %d, %d\n", time1.tv_sec, time1.tv_nsec);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	tst_resm(TINFO,"CLOCK_PROCESS_CPUTIME_ID: %d, %d\n", time1.tv_sec,
			time1.tv_nsec);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time1);
	tst_resm(TINFO,"CLOCK_THREAD_CPUTIME_ID: %d, %d\n", time1.tv_sec,
			time1.tv_nsec);

	tst_resm(TINFO,"\n%d\n", time(NULL));

	tst_resm(TINFO,"tick count in ms: %ul\n", GetTickCount());



	return 0;


}
