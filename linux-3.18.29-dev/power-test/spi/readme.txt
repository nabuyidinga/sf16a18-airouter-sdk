Test Steps:
1.Make sure CONFIG_MTD_TESTS=m in defconfig

2.Compile the linux image

3.Load spi and mtd driver:
	insmod lib/modules/3.18.29/spi-sfax8.ko
	(Only do this when CONFIG_MTD_M25P80=m:
		insmod lib/modules/3.18.29/m25p80.ko)

4.When transfering data through spi, run the following command:
	insmod mtd_torturetest.ko dev=5 check=0
