1.Compile test case:
	1.1: The source code of the test case is "test/gdu_test/gdu_test.c", 
	     it's a user's space program, and we need install linux headers
	     first:
		cd <PRJ_DIR>/linux-3.18.29 && make headers_install
	1.2: Compile the test case, just goes into "test/gdu_test" and make:
		cd <PRJ_DIR>/test/gdu_test && make
	     This will also copy the program to ~/code/linux-3.18.29-dev/root_fs/busybox
	1.3: install.sh is a script used to load the gdu driver, copy it to root_fs/busybox.

2.Compile the linux kernel

3.Load the driver:
	./install.sh

4.If it's necessary to disable the LCD output, run:
	devmem 0x10800000 32 0x400
  re-enable LCD output:
	devmem 0x10800000 32 0x401

5.If needs CPU working, run:
	./gdu_test
  This will generate random size, position, color rectangles on the screen.

