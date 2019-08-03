#!/bin/sh

insmod /lib/modules/3.18.29/syscopyarea.ko
insmod /lib/modules/3.18.29/sysfillrect.ko
insmod /lib/modules/3.18.29/sysimgblt.ko
insmod /lib/modules/3.18.29/fb_sys_fops.ko
insmod /lib/modules/3.18.29/sfax8-fb.ko
