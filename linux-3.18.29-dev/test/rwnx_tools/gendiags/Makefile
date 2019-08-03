MAKEFLAGS += --no-print-directory

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE:=$(CURDIR)/../../../toolchain/toolchain-mipsel_mips32_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-
endif
CC := $(CROSS_COMPILE)gcc


CFLAGS ?= -O2 -g
CFLAGS += -Wall

OBJS = gendiags.o
ALL = gendiags


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

gendiags:	$(OBJS)
	@$(NQ) ' CC  ' gendiags
	$(Q)$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o gendiags

check:
	$(Q)$(MAKE) all CC="REAL_CC=$(CC) CHECK=\"sparse -Wall\" cgcc"

clean:
	$(Q)rm -f gendiags *.o *~
