#
# Makefile.am -- liboacam Makefile template
#
# Copyright 2013,2014,2015,2016,2017,2018,2019
#     James Fidell (james@openastroproject.org)
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
  -I$(top_srcdir)/ext/libusb \
  -I$(top_srcdir)/ext/libASICamera/include \
  -I$(top_srcdir)/ext/libtoupcam/include \
  -I$(top_srcdir)/ext/libqhyccd/include \
  $(LIBUSB_CFLAGS) $(LIBUVC_CFLAGS) \
  $(LIBALTAIRCAM_CFLAGS) $(LIBALTAIRCAM_LEGACY_CFLAGS) $(LIBTOUPCAM_CFLAGS) \
	$(LIBQHYCCD_CFLAGS) \
  $(DYNLIB_EXTN_CFLAGS)

lib_LTLIBRARIES = liboacam.la

if LIBASI2_COND
ZWODIR	= zwo
ZWOLIB	= $(ZWODIR)/libzwo.la
endif

if LIBV4L2_COND
V4L2DIR	= v4l2
V4L2LIB	= $(V4L2DIR)/libvideo4linux.la
endif

if LIBFLYCAPTURE2_COND
FC2DIR	= flycap2
FC2LIB	= $(FC2DIR)/libflycap2.la
endif

if LIBSPINNAKER_COND
SPINDIR	= spinnaker
SPINLIB	= $(SPINDIR)/libspinnaker.la
endif

if ATIK_COND
ATIKDIR	= atik
ATIKLIB	= $(ATIKDIR)/libatik.la
endif

if LIBTOUPCAM_COND
TOUPCAMDIR = toupcam
TOUPCAMLIB = $(TOUPCAMDIR)/libtoupcam.la
endif

if LIBMALLINCAM_COND
MALLINCAMDIR	= mallincam
MALLINCAMLIB	= $(MALLINCAMDIR)/libmallincam.la
endif

if LIBALTAIRCAM_COND
ALTAIRDIR= altair
ALTAIRLIB= $(ALTAIRDIR)/libaltair.la
endif

if LIBSTARSHOOTG_COND
STARSHOOTGDIR= starshootg
STARSHOOTGLIB= $(STARSHOOTGDIR)/libstarshootg.la
endif

if LIBNNCAM_COND
RISINGCAMDIR= risingcam
RISINGCAMLIB= $(RISINGCAMDIR)/librisingcam.la
endif

if LIBALTAIRCAM_LEGACY_COND
ALTAIRLEGACYDIR= altair-legacy
ALTAIRLEGACYLIB= $(ALTAIRLEGACYDIR)/libaltairlegacy.la
endif

if LIBQHYCCD_COND
QHYCCDDIR = qhyccd
QHYCCDLIB = $(QHYCCDDIR)/libqhyccd.la
endif

if LIBGPHOTO2_COND
GPHOTO2DIR = gphoto2
GPHOTO2LIB = $(GPHOTO2DIR)/libgphoto2.la
endif

SUBDIRS	= euvc iidc pwc qhy sx uvc dummy $(ALTAIRDIR) $(ATIKDIR) \
	$(MALLINCAMDIR) $(FC2DIR) $(SPINDIR) $(TOUPCAMDIR) $(V4L2DIR) $(ZWODIR) \
	$(ALTAIRLEGACYDIR) $(QHYCCDDIR) $(GPHOTO2DIR) $(STARSHOOTGDIR) \
	$(RISINGCAMDIR) .

liboacam_la_SOURCES = \
  control.c debug.c oacam.c unimplemented.c utils.c

liboacam_la_LIBADD = euvc/libeuvc.la iidc/libiidc.la pwc/libpwc.la \
  qhy/libqhy.la sx/libsx.la uvc/libuvc.la dummy/libdummy.la $(ALTAIRLIB) \
	$(ATIKLIB) $(MALLINCAMLIB) $(FC2LIB) $(SPINLIB) $(TOUPCAMLIB) $(V4L2LIB) \
	$(ZWOLIB) $(ALTAIRLEGACYLIB) $(QHYCCDLIB) $(GPHOTO2LIB) $(STARSHOOTGLIB) \
	$(RISINGCAMLIB)

WARNINGS = -g -O -Wall -Werror -Wpointer-arith -Wuninitialized -Wsign-compare -Wformat-security -Wno-pointer-sign $(OSX_WARNINGS)

warnings:
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)

verbose-warnings:
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)
