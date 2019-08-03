GCC_VENDOR=mipsel-openwrt-linux-
CC=$(GCC_VENDOR)gcc
LD=$(GCC_VENDOR)ld
OD=$(GCC_VENDOR)objdump
OC=$(GCC_VENDOR)objcopy
SZ=$(GCC_VENDOR)size
AR=$(GCC_VENDOR)ar

INCLUDE=-I/home/zq/code/linux-3.18.29-dev/linux-3.18.29/usr/include

APP_SOURCE_DIR=.

all: app

.PHONY: all

APP_CSOURCES = \
	$(APP_SOURCE_DIR)/gdu_test.c \

LINKER_FLAGS = -O3

APP_TARGET = gdu_test

APP_COBJECTS=${patsubst %.c, %.o, $(APP_CSOURCES)}

$(APP_COBJECTS):%o:%c
	$(CC) $(INCLUDE) $(APP_CFLAGS) -o $@ -c $<

$(APP_TARGET): $(APP_COBJECTS)
	$(CC) $(LINKER_FLAGS) $(APP_COBJECTS) $(LIBS) -o $(APP_TARGET)
	cp $(APP_TARGET) ~/code/linux-3.18.29-dev/root_fs/busybox/

app: $(APP_TARGET)

clean:
	rm -rf $(APP_COBJECTS)
	rm -rf $(APP_TARGET)
