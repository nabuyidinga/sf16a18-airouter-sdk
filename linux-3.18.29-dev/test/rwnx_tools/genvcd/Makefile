MAKEFLAGS += --no-print-directory

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE:=$(CURDIR)/../../../toolchain/toolchain-mipsel_mips32_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-
endif
CC := $(CROSS_COMPILE)gcc

CFLAGS ?= -O2 -g
CFLAGS += -Wall

OBJS = genvcd.o
ALL = genvcd


#LIBS += $(shell pkg-config --libs $(NLLIBNAME))
#CFLAGS += $(shell pkg-config --cflags $(NLLIBNAME))

ifeq ($(V),1)
Q=
NQ=true
else
Q=@
NQ=echo
endif

all: $(ALL)

%.o: %.c
	@$(NQ) ' CC  ' $@
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

genvcd:	$(OBJS)
	@$(NQ) ' CC  ' genvcd
	$(Q)$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o genvcd

check:
	$(Q)$(MAKE) all CC="REAL_CC=$(CC) CHECK=\"sparse -Wall\" cgcc"

clean:
	$(Q)rm -f genvcd *.o *~
