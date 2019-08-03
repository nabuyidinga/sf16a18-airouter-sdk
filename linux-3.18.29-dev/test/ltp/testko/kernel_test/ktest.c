#include <linux/init.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "ktest.h"
#include "mtd_test.h"

static int dev_major = KTEST_MAJOR;
struct ktest_dev_t  ktest_dev;
static struct class *ktest_class;
static struct device *ktest_device;

int ktest_open(struct inode *inode, struct file *filp) {
	int num = MINOR(inode->i_rdev);
	if (num >= KTEST_NR_DEVS)
	  return -ENODEV;

	return 0;
}

int ktest_release(struct inode *inode, struct file *filp) {

	return 0;
}
long ktest_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int ret = 0;
	int arg_local = 0;
	switch(cmd) {
		case MTD_VARIOUS_TEST:
			ret = get_user(arg_local, (int *)arg);
			printk("get_user %d\n", arg_local);
			mtd_various_test();
			arg_local = 100;
			ret = put_user(arg_local, (int *)arg);
			printk("put_user %d\n", arg_local);
			printk("mtd various test\n");
			break;
		case MTD_SPEED_TEST :
			printk("mtd speed test \n");
				break;
		default:
			break;
	}
	return ret;
}

static struct file_operations ktest_fops = {
	.owner = THIS_MODULE,
	.open = ktest_open,
	.release = ktest_release,
	.unlocked_ioctl = ktest_ioctl,
};

static int ktest_init(void) {
	int result = 0, err =0;
	dev_t dev;

	dev = MKDEV(dev_major, KTEST_MINOR);
	result = register_chrdev_region(dev, KTEST_NR_DEVS, "ktest");
	if(result < 0) {
		result = alloc_chrdev_region(&dev, KTEST_MINOR, KTEST_NR_DEVS,"ktest");
		dev_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", dev_major);
		return result;
	}

	ktest_class = class_create(THIS_MODULE, "ktest");
	ktest_device = device_create(ktest_class, NULL, dev, NULL, "ktest");

	memset(&ktest_dev,0, sizeof(struct ktest_dev_t));
	cdev_init(&ktest_dev.cdev, &ktest_fops);
	ktest_dev.cdev.owner = THIS_MODULE;
	ktest_dev.cdev.ops = &ktest_fops;

	err = cdev_add(&ktest_dev.cdev, dev, KTEST_NR_DEVS);
	if (err){
		printk(KERN_NOTICE "Error %d adding cdev", err);
		return err;
	}
	ktest_dev.dev_num = KTEST_NR_DEVS;
	ktest_dev.dev = dev;
	printk("ktest init done\n");
	return 0;
}

static void ktest_exit(void) {
	cdev_del(&ktest_dev.cdev);
	unregister_chrdev_region(ktest_dev.dev, ktest_dev.dev_num);
	device_unregister(ktest_device);
	class_destroy(ktest_class);
	printk("ktest exit\n");
	return;
}

module_init(ktest_init);
module_exit(ktest_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ps");
