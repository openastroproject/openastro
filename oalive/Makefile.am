#
# Makefile.am -- oalive Makefile template
#
# Copyright 2015,2017 James Fidell (james@openastroproject.org)
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

AM_CPPFLAGS = -I$(top_srcdir)/include \
  -I$(top_srcdir)/ext/ffmpeg \
  $(LIBUVC_CFLAGS) $(LIBHIDAPI_CFLAGS) $(LIBUSB_CFLAGS) $(APP_PATH_CFLAGS) \
  $(LIBWINDIB_CFLAGS) $(LIBFTDI_CFLAGS) \
  -D__STDC_CONSTANT_MACROS

MOC_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/ext/ffmpeg \
  $(MOC_CFLAGS) $(APP_PATH_CFLAGS) -D__STDC_CONSTANT_MACROS

bin_PROGRAMS = oalive
BUILT_SOURCES = 
oalive_SOURCES = oalive.cc mainWindow.cc viewWidget.cc focusOverlay.cc \
  controlsWidget.cc cameraControls.cc stackingControls.cc \
  saveControls.cc \
  settingsWidget.cc captureSettings.cc demosaicSettings.cc \
  filterSettings.cc profileSettings.cc generalSettings.cc \
  advancedSettings.cc \
  camera.cc filterWheel.cc targets.cc \
  outputHandler.cc outputTIFF.cc outputFITS.cc \
  moc_mainWindow.cc moc_viewWidget.cc moc_focusOverlay.cc \
  moc_controlsWidget.cc moc_cameraControls.cc moc_stackingControls.cc \
  moc_saveControls.cc \
  moc_settingsWidget.cc moc_captureSettings.cc moc_demosaicSettings.cc \
  moc_filterSettings.cc moc_profileSettings.cc moc_generalSettings.cc \
  moc_advancedSettings.cc \
  qrc_oalive.cc

#autorunSettings.cc camera.cc cameraSettings.cc \
#  cameraWidget.cc captureSettings.cc captureWidget.cc controlWidget.cc \
#  demosaicSettings.cc filterSettings.cc filterWheel.cc generalSettings.cc \
#  histogramSettings.cc histogramWidget.cc imageWidget.cc mainWindow.cc \
#  oalive.cc outputAVI.cc $(OUTPUT_FITS) outputHandler.cc outputSER.cc \
#  outputTIFF.cc previewWidget.cc profileSettings.cc settingsWidget.cc \
#  targets.cc zoomWidget.cc advancedSettings.cc outputFFMPEG.cc \
#  outputMOV.cc outputDIB.cc focusOverlay.cc \
#  moc_autorunSettings.cc moc_camera.cc moc_cameraSettings.cc \
#  moc_cameraWidget.cc moc_captureSettings.cc moc_captureWidget.cc \
#  moc_controlWidget.cc moc_filterSettings.cc \
#  moc_generalSettings.cc moc_imageWidget.cc moc_mainWindow.cc \
#  moc_previewWidget.cc moc_profileSettings.cc moc_settingsWidget.cc \
#  moc_zoomWidget.cc moc_advancedSettings.cc moc_focusOverlay.cc \
#  qrc_oalive.cc

oalive_LDADD = \
  ../liboacam/liboacam.la \
  ../liboaimgproc/liboaimgproc.la \
  ../liboademosaic/liboademosaic.la \
  ../liboavideo/liboavideo.la \
  ../liboafilterwheel/liboafilterwheel.la \
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
