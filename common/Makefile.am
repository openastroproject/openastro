#
# Makefile.am -- oacapture Makefile template
#
# Copyright 2013,2014,2015,2016,2017,2018,2019
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

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/common $(FFMPEG_CFLAGS) \
	$(APP_PATH_CFLAGS) $(DEFAULT_PATH_CFLAGS) $(LIBWINDIB_CFLAGS) \
	-D__STDC_CONSTANT_MACROS -DTRANSLATE_DIR=\"${datadir}/$(bin_PROGRAMS)/\"

MOC_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/common $(MOC_CFLAGS) \
	$(APP_PATH_CFLAGS) $(DEFAULT_PATH_CFLAGS) -D__STDC_CONSTANT_MACROS \
  -DTRANSLATE_DIR=\"${datadir}/$(bin_PROGRAMS)/\"

lib_LTLIBRARIES = liboacommon.la
liboacommon_la_SOURCES = camera.cc filterWheel.cc timer.cc targets.cc \
	commonConfig.cc commonState.cc \
	advancedSettings.cc autorunSettings.cc cameraSettings.cc captureSettings.cc \
	demosaicSettings.cc filterSettings.cc fitsSettings.cc generalSettings.cc \
	histogramSettings.cc profileSettings.cc timerSettings.cc settingsWidget.cc \
	focusOverlay.cc histogramWidget.cc \
	outputAVI.cc outputDIB.cc outputFFMPEG.cc outputFITS.cc outputMOV.cc \
	outputPNG.cc outputSER.cc outputTIFF.cc outputHandler.cc \
	outputNamedPipe.cc \
	moc_camera.cc \
	moc_focusOverlay.cc moc_settingsWidget.cc moc_histogramWidget.cc \
	moc_advancedSettings.cc moc_autorunSettings.cc moc_cameraSettings.cc \
	moc_captureSettings.cc moc_demosaicSettings.cc moc_filterSettings.cc \
	moc_fitsSettings.cc moc_generalSettings.cc moc_histogramSettings.cc \
	moc_profileSettings.cc moc_timerSettings.cc

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
	$(LN_S) -f $(top_srcdir)/qt-icons .
	@RCC@ -o $@ $<

mostlyclean-local:
	-rm -rf moc_*.cc qrc_*.cc version.h

version.h: version.h.in $(top_srcdir)/version
	-rm -f $@.tmp
	maj=`cat $(top_srcdir)/version | cut -d. -f1`; \
	min=`cat $(top_srcdir)/version | cut -d. -f2`; \
	rev=`cat $(top_srcdir)/version | cut -d. -f3`; \
	sed -e "s/##MAJOR_VERSION##/$$maj/g" -e "s/##MINOR_VERSION##/$$min/g" \
		-e "s/##REVISION##/$$rev/g" >$@.tmp $<
	cmp -s $@.tmp $@ || ( rm -f $@ && mv $@.tmp $@ )
