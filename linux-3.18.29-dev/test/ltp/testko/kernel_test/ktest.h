#ifndef _KTEST_H_
#define _KTEST_H_

#ifndef KTEST_MAJOR
#define KTEST_MAJOR 251
#endif

#ifndef KTEST_MINOR
#define KTEST_MINOR 0
#endif

#ifndef KTEST_NR_DEVS
#define KTEST_NR_DEVS 1
#endif

#define KTEST_IOC_MAGIC  'k'

#define MTD_VARIOUS_TEST  _IOWR(KTEST_IOC_MAGIC,1,int)
#define MTD_SPEED_TEST    _IOWR(KTEST_IOC_MAGIC,2,int)

struct ktest_dev_t {
	unsigned int dev_num;
	struct cdev cdev;
	dev_t dev;
};
#endif
