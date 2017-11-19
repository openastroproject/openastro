#
# Makefile.am -- liboacam Makefile template
#
# Copyright 2013,2014,2015,2016,2017 James Fidell (james@openastroproject.org)
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
  $(LIBASI_CFLAGS) $(LIBASI2_CFLAGS) $(LIBUSB_CFLAGS) $(FXLOAD_CFLAGS) \
  $(LIBUVC_CFLAGS) $(LIBDC1394_CFLAGS) $(LIBTOUPCAM_CFLAGS) \
  $(DYNLIB_EXTN_CFLAGS)

lib_LTLIBRARIES = liboacam.la

if LIBASI_COND
ASISRC = ZWASIoacam.c ZWASIconnect.c ZWASIcontroller.c ZWASIgetState.c \
  ZWASIcontrol.c
endif
if LIBASI2_COND
ASI2SRC = ZWASI2oacam.c ZWASI2connect.c ZWASI2controller.c ZWASI2getState.c \
  ZWASI2control.c
endif
if LIBASI_COMMON_COND
ASICOMMON = ZWASIcommonControl.c ZWASIcallback.c ZWASIroi.c ZWASIcommon.c
endif

if LIBV4L2_COND
V4L2SRC = V4L2oacam.c V4L2connect.c V4L2ioctl.c V4L2cameras.c \
  V4L2getState.c V4L2controller.c V4L2callback.c V4L2control.c
endif

UVCSRC = UVCoacam.c UVCconnect.c UVCgetState.c UVCcontrol.c \
  UVCcontroller.c UVCcallback.c UVCExtnUnits.c TISExtensionUnits.c
IIDCSRC = IIDCoacam.c IIDCconnect.c IIDCgetState.c IIDCcontrol.c \
  IIDCcontroller.c IIDCcallback.c

if LIBFLYCAPTURE2_COND
PGESRC = PGEoacam.c PGEconnect.c PGEgetState.c PGEcontrol.c \
  PGEcontroller.c PGEcallback.c PGEroi.c
endif

if LIBUDEV_COND
ATIKSERIALUDEVSRC = atikSerialoacam-udev.c atikSerialconnect-udev.c \
  atikSerialIO-udev.c
endif

if LIBFTDI_COND
ATIKSERIALFTDISRC = atikSerialoacam-ftdi.c atikSerialconnect-ftdi.c \
  atikSerialIO-ftdi.c
endif

if ATIK_COND
ATIKSRC = atikSerialgetState.c atikSerialcallback.c atikSerialcontrol.c \
  atikSerialcontroller.c
endif

if LIBTOUPCAM_COND
TOUPCAMSRC = Touptekoacam.c Touptekconnect.c Touptekcallback.c \
  Touptekcontrol.c Touptekcontroller.c TouptekgetState.c Touptekroi.c
endif

if LIBMALLINCAM_COND
MALLINCAMSRC = Mallincamoacam.c Mallincamconnect.c Mallincamcallback.c \
  Mallincamcontrol.c Mallincamcontroller.c MallincamgetState.c Mallincamroi.c
endif

if LIBALTAIRCAM_COND
ALTAIRSRC = Altairoacam.c Altairconnect.c Altaircallback.c \
  Altaircontrol.c Altaircontroller.c AltairgetState.c Altairroi.c
endif

liboacam_la_SOURCES = \
  control.c debug.c oacam.c unimplemented.c utils.c \
  $(V4L2SRC) $(ASISRC) $(ASI2SRC) $(ASICOMMON) $(UVCSRC) $(IIDCSRC) \
  $(ATIKSRC) $(ATIKSERIALUDEVSRC) $(ATIKSERIALFTDISRC) $(PGESRC) \
  $(TOUPCAMSRC) $(MALLINCAMSRC) $(ALTAIRSRC) \
  PWCgetState.c PWCconnect.c PWCoacam.c \
  QHYoacam.c QHYconnect.c QHYfirmware.c QHYcontrol.c QHYcallback.c \
  QHYgetState.c QHYusb.c \
  QHY5.c QHY5controller.c \
  QHY5II.c QHY5IIcontroller.c \
  QHY5LII.c QHY5LIIcontroller.c \
  QHY6.c QHY6controller.c \
  SXoacam.c SXconnect.c SXcallback.c SXcontroller.c SXcontrol.c \
  SXgetState.c \
  EUVCoacam.c EUVCconnect.c EUVCcontrol.c EUVCcallback.c EUVCgetState.c \
  EUVCcontroller.c EUVCusb.c EUVCroi.c

WARNINGS = -g -O -Wall -Werror -Wpointer-arith -Wuninitialized -Wsign-compare -Wformat-security -Wno-pointer-sign $(OSX_WARNINGS)

warnings:
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)

verbose-warnings:
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)
