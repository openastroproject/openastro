#
# Makefile.am -- liboafilterwheel ZWO Makefile template
#
# Copyright 2018 James Fidell (james@openastroproject.org)
#
# License:
#
# This file is part of the Open Astro Project.
#
# The Open Astro Project is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Open Astro Project is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the Open Astro Project.  If not, see
# <http://www.gnu.org/licenses/>.
#

AM_CPPFLAGS = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/ext/libEFWFilter/include \
  -I$(top_srcdir)/liboafilterwheel \
  $(LIBHIDAPI_CFLAGS) \
  $(LIBUSB_CFLAGS) \
  $(LIBZWOFW_CFLAGS) \
  $(DYNLIB_EXTN_CFLAGS)

noinst_LTLIBRARIES = libzwo.la

libzwo_la_SOURCES = \
	zwofw.c zwoConnect.c zwoController.c zwoConfig.c zwoDynloader.c

WARNINGS = -g -O -Wall -Werror -Wpointer-arith -Wuninitialized -Wsign-compare -Wformat-security -Wno-pointer-sign $(OSX_WARNINGS)

warnings:
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)

verbose-warnings:
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)
