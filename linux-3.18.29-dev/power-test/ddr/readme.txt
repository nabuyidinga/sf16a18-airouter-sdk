Test Steps:
1.Copy "memtester" and "memtester-multi.sh" to root_fs/busybox/

2.Compile linux image:
	make busybox=1 -B -j8

3.When needs read/write DDR, run the following command on the board:
	./memtester-multi.sh -c 4 -m 200 -t 3h
