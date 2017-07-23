#
# Makefile.am -- external libraries Makefile template
#
# Copyright 2013,2014,2015,2016 James Fidell (james@openastroproject.org)
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

if INT_LIBUVC_COND
  LIBUVC_DIRS=libuvc
endif
if LIBFFMPEG_COND
  FFMPEG_DIRS=ffmpeg
endif
if LIBDC1394_COND
  LIBDC1394_DIRS=libdc1394
endif
if BUILD_LIBUSB_COND
  LIBUSB_DIRS=libusb
endif
if LIBHIDAPI_COND
  LIBHIDAPI_DIRS=libhidapi
endif

SUBDIRS = $(FFMPEG_DIRS) $(LIBUSB_DIRS) $(LIBHIDAPI_DIRS) $(LIBUVC_DIRS) \
          $(LIBDC1394_DIRS) libwindib
