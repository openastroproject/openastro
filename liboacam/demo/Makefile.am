#
# Makefile.am -- liboacam demo programs Makefile template
#
# Copyright 2021
#   James Fidell (james@openastroproject.org)
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

#AM_CPPFLAGS = -I$(top_srcdir)/include $(FFMPEG_CFLAGS) $(APP_PATH_CFLAGS) \
#  $(DEFAULT_PATH_CFLAGS) $(LIBWINDIB_CFLAGS) -D__STDC_CONSTANT_MACROS \
#  $(LIBUVC_CFLAGS) $(LIBHIDAPI_CFLAGS) $(LIBUSB_CFLAGS) $(LIBDC1394_CFLAGS) \
#  $(LIBFTDI_CFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/common

noinst_PROGRAMS = showControls showFeatures logging startStreaming \
									updateIntControl updateMenuControl

showControls_SOURCES = showControls.c

showFeatures_SOURCES = showFeatures.c

logging_SOURCES = logging.c

startStreaming_SOURCES = startStreaming.c

updateIntControl_SOURCES = updateIntControl.c

updateMenuControl_SOURCES = updateMenuControl.c

LDADD = \
  ../liboacam.la \
  ../../liboautil/liboautil.la \
  ../../liboavideo/liboavideo.la \
  $(LIBUVC_LIBS) \
  $(LIBDC1394_LIBS) \
  $(LIBFTDI_LIBS) \
  $(LIBUSB_LIBS) \
  $(LIBASI2_LIBS) \
  $(LIBZWOFW_LIBS) \
  $(LIBGPHOTO2_LIBS) \
  $(LIBSVBCAMERASDK_LIBS) \
  $(PYLON_LDFLAGS) $(PYLON_LIBS) \
  $(OSX_FRAMEWORKS) \
  -lpthread

WARNINGS = -g -O -Wall -Werror -Wpointer-arith -Wuninitialized -Wsign-compare -Wformat-security -Wno-pointer-sign $(OSX_WARNINGS)

warnings:
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)

verbose-warnings:
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)
