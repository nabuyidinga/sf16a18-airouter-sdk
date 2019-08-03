Test Steps:
1.Copy storage_test.sh to root_fs/busybox/

2.Compile the linux image

3.Load USB driver:
	insmod lib/modules/3.18.29/phy-sfax8-usb.ko
	insmod lib/modules/3.18.29/dwc2.ko

4.When transfering data through USB, follow these steps:
	4.1 Plug a U-disk to the board
	4.2 mount USB:
		mount /dev/sda /mnt
	4.3 run test:
		./storage_test.sh /mnt
