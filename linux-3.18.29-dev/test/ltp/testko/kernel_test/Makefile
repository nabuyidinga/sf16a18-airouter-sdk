PWD := $(shell pwd)
KDIR=$(PWD)/../../../..
LINUX_DIR=$(KDIR)/linux-3.18.29
ARCH:=mips
CC:=$(CROSS_COMPILE)gcc
LD:=$(CROSS_COMPILE)ld
STAGING_DIR:=$(CURDIR)/include
CROSS_COMPILE:=$(CURDIR)/../../../../toolchain/toolchain-mipsel_mips32_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-
LINUX_VERSION:=3.18.29
export CROSS_COMPILE ARCH STAGING_DIR KDIR LINUX_VERSION

obj-m := kernel_test.o
kernel_test-objs = ktest.o  mtd_test.o

default:
	    make -C $(LINUX_DIR) M=$(PWD) modules
clean:
	    rm -f *.ko *.o *.mod.c *.symvers
