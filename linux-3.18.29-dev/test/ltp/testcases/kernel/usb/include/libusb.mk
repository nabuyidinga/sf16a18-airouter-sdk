#
#  Copyright (C) 2012  Red Hat, Inc.
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

USB_SRCDIR		:= $(abs_top_srcdir)/testcases/kernel/usb
LIBUSB_SRCDIR		:= $(I2C_SRCDIR)/lib

USB_DIR			:= $(top_builddir)/testcases/kernel/usb
LIBUSB_DIR		:= $(I2C_DIR)/lib
LIBUSB			:= $(LIBI2C_DIR)/libusb.a
FILTER_OUT_DIRS		:= $(LIBUSB_DIR)
CFLAGS			+= -I$(USB_SRCDIR)/include -pthread
LDLIBS			+= $(NUMA_LIBS) -lusb -lltp
LDFLAGS			+= -L$(LIBUSB_DIR)

$(LIBUSB_DIR):
	mkdir -p "$@"

$(LIBUSB): $(LIBI2C_DIR)
	$(MAKE) -C $^ -f "$(LIBUSB_SRCDIR)/Makefile" all

MAKE_DEPS		+= $(LIBUSB)

trunk-clean:: | lib-clean

lib-clean:: $(LIBUSB_DIR)
	$(MAKE) -C $^ -f "$(LIBUSB_SRCDIR)/Makefile" clean

include $(top_srcdir)/testcases/kernel/include/lib.mk
