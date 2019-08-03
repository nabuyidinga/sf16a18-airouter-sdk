EXEC = ate_cmd
SRCS = main.c
LINUX_DIR=

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE:=$(CURDIR)/../../toolchain/toolchain-mipsel_mips32_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-
endif
CC := $(CROSS_COMPILE)gcc

all: $(EXEC)

$(EXEC):
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(LDLIBS)
clean:
	-rm -f $(EXEC) *.o
