top_srcdir       ?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

CPPFLAGS            += -DLINUX
LDLIBS				+= -lpthread
LDFLAGS             += -R/usr/lib

include $(top_srcdir)/include/mk/generic_leaf_target.mk
