#
# Makefile.am -- oalive Makefile template
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

#AM_CPPFLAGS = -I$(top_srcdir)/include $(FFMPEG_CFLAGS) $(APP_PATH_CFLAGS) \
#  $(DEFAULT_PATH_CFLAGS) $(LIBWINDIB_CFLAGS) -D__STDC_CONSTANT_MACROS \
#  $(LIBUVC_CFLAGS) $(LIBHIDAPI_CFLAGS) $(LIBUSB_CFLAGS) $(LIBDC1394_CFLAGS) \
#  $(LIBFTDI_CFLAGS)

SUBDIRS = icons desktop # translations

AM_CPPFLAGS = -I$(top_srcdir)/include $(APP_PATH_CFLAGS) \
  $(DEFAULT_PATH_CFLAGS) -D__STDC_CONSTANT_MACROS -DOALIVE \
  -DTRANSLATE_DIR=\"${datadir}/$(bin_PROGRAMS)/\"

MOC_CPPFLAGS = -I$(top_srcdir)/include $(MOC_CFLAGS) $(APP_PATH_CFLAGS) \
  $(DEFAULT_PATH_CFLAGS) -D__STDC_CONSTANT_MACROS -DOALIVE \
  -DTRANSLATE_DIR=\"${datadir}/$(bin_PROGRAMS)/\"

bin_PROGRAMS = oalive
BUILT_SOURCES = 
oalive_SOURCES = advancedSettings.cc camera.cc controlsWidget.cc \
  captureSettings.cc demosaicSettings.cc filterSettings.cc filterWheel.cc \
  fitsSettings.cc focusOverlay.cc generalSettings.cc oalive.cc outputFITS.cc \
  outputHandler.cc outputPNG.cc outputTIFF.cc profileSettings.cc \
  settingsWidget.cc targets.cc mainWindow.cc viewWidget.cc cameraControls.cc \
  stackingControls.cc saveControls.cc \
  moc_advancedSettings.cc moc_camera.cc moc_controlsWidget.cc \
  moc_captureSettings.cc moc_demosaicSettings.cc moc_filterSettings.cc \
  moc_filterWheel.cc moc_fitsSettings.cc moc_focusOverlay.cc \
  moc_generalSettings.cc moc_profileSettings.cc moc_settingsWidget.cc \
  moc_mainWindow.cc moc_viewWidget.cc moc_cameraControls.cc \
  moc_stackingControls.cc moc_saveControls.cc \
  qrc_oalive.cc

oalive_LDADD = \
  ../liboacam/liboacam.la \
  ../liboaimgproc/liboaimgproc.la \
  ../liboademosaic/liboademosaic.la \
  ../liboavideo/liboavideo.la \
  ../liboafilterwheel/liboafilterwheel.la \
  ../liboaPTR/liboaPTR.la \
  ../liboautil/liboautil.la \
  $(LIBUVC_LIBS) \
  $(LIBHIDAPI_LIBS) \
  $(LIBDC1394_LIBS) \
  $(LIBFTDI_LIBS) \
  $(LIBUSB_LIBS) \
  $(LIBASI_LIBS) \
  $(LIBASI2_LIBS) \
  $(OSX_FRAMEWORKS)

WARNINGS = -g -O -Wall -Werror -Wpointer-arith -Wuninitialized -Wsign-compare -Wformat-security -Wno-pointer-sign $(OSX_WARNINGS)

warnings:
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)

verbose-warnings:
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)

moc_%.cc: %.h
	@MOC@ -o$@ $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(MOC_CPPFLAGS) $<

ui-%.h: %.ui
	@UIC@ -o $@ $<

qrc_%.cc: %.qrc
	@RCC@ -o $@ $<

mostlyclean-local:
	-rm -rf moc_*.cc qrc_*.cc
