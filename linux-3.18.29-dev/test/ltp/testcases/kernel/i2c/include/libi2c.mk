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

I2C_SRCDIR		:= $(abs_top_srcdir)/testcases/kernel/i2c
LIBI2C_SRCDIR		:= $(I2C_SRCDIR)/lib

I2C_DIR			:= $(top_builddir)/testcases/kernel/i2c
LIBI2C_DIR		:= $(I2C_DIR)/lib
LIBI2C			:= $(LIBI2C_DIR)/libi2c.a
FILTER_OUT_DIRS		:= $(LIBI2C_DIR)
CFLAGS			+= -I$(I2C_SRCDIR)/include -pthread
LDLIBS			+= $(NUMA_LIBS) -li2c -lltp
LDFLAGS			+= -L$(LIBI2C_DIR)

$(LIBI2C_DIR):
	mkdir -p "$@"

$(LIBI2C): $(LIBI2C_DIR)
	$(MAKE) -C $^ -f "$(LIBI2C_SRCDIR)/Makefile" all

MAKE_DEPS		+= $(LIBI2C)

trunk-clean:: | lib-clean

lib-clean:: $(LIBI2C_DIR)
	$(MAKE) -C $^ -f "$(LIBI2C_SRCDIR)/Makefile" clean

include $(top_srcdir)/testcases/kernel/include/lib.mk
