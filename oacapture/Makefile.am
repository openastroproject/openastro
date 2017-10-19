#
# Makefile.am -- oacapture Makefile template
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

SUBDIRS = icons desktop translations

AM_CPPFLAGS = -I$(top_srcdir)/include $(FFMPEG_CFLAGS) $(APP_PATH_CFLAGS) \
  $(DEFAULT_PATH_CFLAGS) $(LIBWINDIB_CFLAGS) -D__STDC_CONSTANT_MACROS \
  -DTRANSLATE_DIR=\"${datadir}/$(bin_PROGRAMS)/\"

MOC_CPPFLAGS = -I$(top_srcdir)/include $(MOC_CFLAGS) $(APP_PATH_CFLAGS) \
  $(DEFAULT_PATH_CFLAGS) -D__STDC_CONSTANT_MACROS \
  -DTRANSLATE_DIR=\"${datadir}/$(bin_PROGRAMS)/\"

bin_PROGRAMS = oacapture
BUILT_SOURCES = 
oacapture_SOURCES = autorunSettings.cc camera.cc cameraSettings.cc \
  cameraWidget.cc captureSettings.cc captureWidget.cc controlWidget.cc \
  demosaicSettings.cc filterSettings.cc filterWheel.cc generalSettings.cc \
  histogramSettings.cc histogramWidget.cc imageWidget.cc mainWindow.cc \
  oacapture.cc outputAVI.cc outputFITS.cc outputHandler.cc outputSER.cc \
  outputTIFF.cc outputPNG.cc previewWidget.cc profileSettings.cc \
  settingsWidget.cc targets.cc zoomWidget.cc advancedSettings.cc \
  outputFFMPEG.cc outputMOV.cc outputDIB.cc focusOverlay.cc timer.cc \
  fitsSettings.cc timerSettings.cc outputRAW.cc \
  moc_autorunSettings.cc moc_camera.cc moc_cameraSettings.cc \
  moc_cameraWidget.cc moc_captureSettings.cc moc_captureWidget.cc \
  moc_controlWidget.cc moc_filterSettings.cc \
  moc_generalSettings.cc moc_imageWidget.cc moc_mainWindow.cc \
  moc_previewWidget.cc moc_profileSettings.cc moc_settingsWidget.cc \
  moc_zoomWidget.cc moc_advancedSettings.cc moc_focusOverlay.cc \
  moc_fitsSettings.cc moc_timerSettings.cc moc_demosaicSettings.cc \
  moc_histogramSettings.cc \
  qrc_oacapture.cc

oacapture_LDADD = \
  ../liboacam/liboacam.la \
  ../liboaimgproc/liboaimgproc.la \
  ../liboademosaic/liboademosaic.la \
  ../liboaSER/liboaSER.la \
  ../liboavideo/liboavideo.la \
  ../liboafilterwheel/liboafilterwheel.la \
  ../liboaPTR/liboaPTR.la \
  ../liboautil/liboautil.la \
  $(FFMPEG_LIBS) \
  $(LIBWINDIB_LIBS) \
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
