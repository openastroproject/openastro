/*****************************************************************************
 *
 * trampoline.cc -- redirected function calls
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <oa_common.h>

#include <QtGui>

#include "captureSettings.h"
#include "fitsSettings.h"
#include "trampoline.h"
#include "targets.h"

#include "configuration.h"
#include "state.h"


int
t_getCurrentGain ( void )
{
  return state.cameraControls->getCurrentGain();
}


int
t_getCurrentExposure ( void )
{
  return state.cameraControls->getCurrentExposure();
}


int
t_getCurrentTargetId ( void )
{
	return TGT_UNKNOWN;
}


QString
t_getCurrentFilterName ( void )
{
  return "";
}


QString
t_getCurrentProfileName ( void )
{
	return "";
}


void
t_setFilterSlotCount ( int num )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return;
}


void
t_reloadFilters ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return;
}


void
t_updateHistogramLayout ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return;
}


void
t_resetAutorun ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return;
}


void
t_updateControlCheckbox ( int, int )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return;
}


int
t_getSpinboxMinimum ( int control )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return 0;
}


int
t_getSpinboxMaximum ( int control )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return 0;
}


int
t_getSpinboxStep ( int control )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return 0;
}


int
t_getSpinboxValue ( int control )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return 0;
}


void
t_updateSpinbox ( int control, int value )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


QStringList
t_getFrameRates ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	QStringList l;
  return l;
}


int
t_getFrameRateIndex ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return 0;
}


void
t_updateFrameRate ( int rate )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_setFlipX ( int val )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_setFlipY ( int val )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_updateForceFrameFormat ( unsigned int oldState, unsigned int format )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_reloadProfiles ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return;
}


void
t_resetTemperatureLabel ( void )
{
	state.mainWindow->resetTemperatureLabel();
}


void
t_setDisplayFPS ( int num )
{
	state.viewWidget->setDisplayFPS ( num );
}


void
t_enableTIFFCapture ( int val )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_enableMOVCapture ( int val )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_enablePNGCapture ( int val )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_setVideoFramePixelFormat ( int format )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_destroyLayout ( QLayout* layout )
{
  state.mainWindow->destroyLayout ( layout );
}


void
t_resetCaptureIndex ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


int
t_isCameraInitialised ( void )
{
  return state.camera->isInitialised();
}


int
t_videoFramePixelFormat ( void )
{
  return state.camera->videoFramePixelFormat();
}


int
t_isDemosaicEnabled ( void )
{
	return config.demosaic;
}


trampolineFuncs trampolines {
	.getCurrentGain = &t_getCurrentGain,
	.getCurrentExposure = &t_getCurrentExposure,
	.getCurrentTargetId = &t_getCurrentTargetId,
	.getCurrentFilterName = &t_getCurrentFilterName,
	.getCurrentProfileName = &t_getCurrentProfileName,
	.setFilterSlotCount = &t_setFilterSlotCount,
  .reloadFilters = &t_reloadFilters,
	.updateHistogramLayout = &t_updateHistogramLayout,
  .resetAutorun = &t_resetAutorun,
  .updateControlCheckbox = &t_updateControlCheckbox,
  .getSpinboxMinimum = &t_getSpinboxMinimum,
  .getSpinboxMaximum = &t_getSpinboxMaximum,
  .getSpinboxStep = &t_getSpinboxStep,
  .getSpinboxValue = &t_getSpinboxValue,
	.updateSpinbox = &t_updateSpinbox,
	.getFrameRates = &t_getFrameRates,
	.getFrameRateIndex = &t_getFrameRateIndex,
	.updateFrameRate = &t_updateFrameRate,
	.setFlipX = &t_setFlipX,
	.setFlipY = &t_setFlipY,
	.updateForceFrameFormat = &t_updateForceFrameFormat,
  .reloadProfiles = &t_reloadProfiles,
  .resetTemperatureLabel = &t_resetTemperatureLabel,
  .setDisplayFPS = &t_setDisplayFPS,
  .enableTIFFCapture = &t_enableTIFFCapture,
  .enableMOVCapture = &t_enableMOVCapture,
  .enablePNGCapture = &t_enablePNGCapture,
	.setVideoFramePixelFormat = &t_setVideoFramePixelFormat,
	.destroyLayout = &t_destroyLayout,
  .resetCaptureIndex = &t_resetCaptureIndex,
  .isCameraInitialised = &t_isCameraInitialised,
  .videoFramePixelFormat = &t_videoFramePixelFormat,
  .isDemosaicEnabled = &t_isDemosaicEnabled
};
