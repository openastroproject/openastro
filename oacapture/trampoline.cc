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

#include "demosaicSettings.h"

#include "configuration.h"
#include "trampoline.h"
#include "state.h"
#include "version.h"


int
t_getCurrentGain ( void )
{
  return state.controlWidget->getCurrentGain();
}


int
t_getCurrentExposure ( void )
{
  return state.controlWidget->getCurrentExposure();
}


int
t_getCurrentTargetId ( void )
{
	return state.captureWidget->getCurrentTargetId();
}


QString
t_getCurrentFilterName ( void )
{
  return state.captureWidget->getCurrentFilterName();
}


QString
t_getCurrentProfileName ( void )
{
	return state.captureWidget->getCurrentProfileName();
}


void
t_setFilterSlotCount ( int num )
{
	state.settingsWidget->setSlotCount ( num );
	state.captureWidget->setSlotCount ( num );
}


void
t_reloadFilters ( void )
{
	state.captureWidget->reloadFilters();
}


void
t_updateHistogramLayout ( void )
{
	if ( state.histogramWidget ) {
	  state.histogramWidget->updateLayout();
	}
}


void
t_resetAutorun ( void )
{
	state.captureWidget->resetAutorun();
}


void
t_updateControlCheckbox ( int control, int value )
{
	state.controlWidget->updateCheckbox ( control, value );
}


int
t_getSpinboxMinimum ( int control )
{
	return state.controlWidget->getSpinboxMinimum ( control );
}


int
t_getSpinboxMaximum ( int control )
{
	return state.controlWidget->getSpinboxMaximum ( control );
}


int
t_getSpinboxStep ( int control )
{
	return state.controlWidget->getSpinboxStep ( control );
}


int
t_getSpinboxValue ( int control )
{
	return state.controlWidget->getSpinboxValue ( control );
}


void
t_updateSpinbox ( int control, int value )
{
	state.controlWidget->updateSpinbox ( control, value );
}


QStringList
t_getFrameRates ( void )
{
	state.controlWidget->getFrameRates();
}


int
t_getFrameRateIndex ( void )
{
	state.controlWidget->getFrameRateIndex();
}


void
t_updateFrameRate ( int rate )
{
	state.controlWidget->updateFrameRate ( rate );
}


void
t_setFlipX ( int val )
{
	state.mainWindow->setFlipX ( val );
}


void
t_setFlipY ( int val )
{
	state.mainWindow->setFlipY ( val );
}


void
t_updateForceFrameFormat ( unsigned int oldState, unsigned int format )
{
  state.cameraWidget->updateForceFrameFormat ( oldState, format );
}


void
t_reloadProfiles ( void )
{
	state.captureWidget->reloadProfiles();
}


void
t_resetTemperatureLabel ( void )
{
  state.cameraWidget->resetTemperatureLabel();
}


void
t_setDisplayFPS ( int num )
{
  state.previewWidget->setDisplayFPS ( num );
}


void
t_enableTIFFCapture ( int val )
{ 
  state.captureWidget->enableTIFFCapture ( val );
}


void
t_enableMOVCapture ( int val )
{
  state.captureWidget->enableMOVCapture ( val );
}


void
t_enablePNGCapture ( int val )
{
  state.captureWidget->enablePNGCapture ( val );
}


void
t_setVideoFramePixelFormat ( int format )
{
  state.previewWidget->setVideoFramePixelFormat ( format );
}


void
t_destroyLayout ( QLayout* layout )
{
	state.mainWindow->destroyLayout ( layout );
}


void
t_resetCaptureIndex ( void )
{
	state.captureIndex = 0;
}


int
t_isCameraInitialised ( void )
{
	return state.camera->isInitialised();
}


int
t_videoFramePixelFormat ( void )
{
	return state.camera->videoFramePixelFormat ( &demosaicConf );
}


int
t_isDemosaicEnabled ( void )
{
  return config.demosaic;
}


int
t_isBinningValid ( void )
{
	return state.binningValid;
}


int
t_binModeX ( void )
{
	return state.binModeX;
}


int
t_binModeY ( void )
{
	return state.binModeY;
}


int
t_pixelSizeX ( void )
{
	return state.camera->pixelSizeX();
}


int
t_pixelSizeY ( void )
{
	return state.camera->pixelSizeY();
}


int
t_isCropMode ( void )
{
	return state.cropMode;
}


int
t_sensorSizeX ( void )
{
	return state.sensorSizeX;
}


int
t_sensorSizeY ( void )
{
	return state.sensorSizeY;
}


int
t_cropSizeX ( void )
{
	return state.cropSizeX;
}


int
t_cropSizeY ( void )
{
	return state.cropSizeY;
}


int
t_isGPSValid ( void )
{
	return state.gpsValid;
}


double
t_latitude ( void )
{
	return state.latitude;
}


double
t_longitude ( void )
{
	return state.longitude;
}


double
t_altitude ( void )
{
	return state.altitude;
}


int
t_isCameraTempValid ( void )
{
	return state.cameraTempValid;
}


float
t_cameraTemp ( void )
{
	return state.cameraTemp;
}


void
t_setTimerMode ( int mode )
{
	timerConf.timerMode = mode;
}


int
t_isTimerInitialised ( void )
{
	return ( state.timer && state.timer->isInitialised() ? 1 : 0 );
}


int
t_timerHasReset ( void )
{
	return state.timer->hasReset();
}


int
t_timerHasSync ( void )
{
	return state.timer->hasSync();
}


void
t_checkTimerWarnings ( void )
{
	QString		msg;

  if (( CAPTURE_FITS != config.fileTypeOption && CAPTURE_TIFF !=                      config.fileTypeOption ) || !config.limitEnabled ||
      !config.limitType ) {
    msg = QCoreApplication::translate ( "SettingsWidget",
				"\n\nWhen using timer mode the image capture type should "
        "be FITS/TIFF/PNG and a frame-based capture limit should be set." );
    QMessageBox::warning ( state.settingsWidget, APPLICATION_NAME, msg );
  }
  if ( state.camera && state.camera->isInitialised()) {
    if ( state.camera->hasControl ( OA_CAM_CTRL_TRIGGER_ENABLE ) &&
        timerConf.timerMode == OA_TIMER_MODE_TRIGGER &&
        !state.camera->readControl ( OA_CAM_CTRL_TRIGGER_ENABLE )) {
      msg = QCoreApplication::translate ( "SettingsWidget",
					"\n\nThe timer is in trigger mode but the camera is "
          "not.  These two settings should be the same." );
    }
    if ( state.camera->hasControl ( OA_CAM_CTRL_STROBE_ENABLE ) &&
        timerConf.timerMode == OA_TIMER_MODE_STROBE &&
        !state.camera->readControl ( OA_CAM_CTRL_STROBE_ENABLE )) {
      msg = QCoreApplication::translate ( "SettingsWidget",
					"\n\nThe timer is in strobe mode but the camera is "
          "not.  These two settings should be the same." );
    }
    QMessageBox::warning ( state.settingsWidget, APPLICATION_NAME, msg );
  }
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
  .isDemosaicEnabled = &t_isDemosaicEnabled,
  .isBinningValid = &t_isBinningValid,
  .binModeX = &t_binModeX,
  .binModeY = &t_binModeY,
  .pixelSizeX = &t_pixelSizeX,
  .pixelSizeY = &t_pixelSizeY,
  .sensorSizeX = &t_sensorSizeX,
  .sensorSizeY = &t_sensorSizeY,
  .cropSizeX = &t_cropSizeX,
  .cropSizeY = &t_cropSizeY,
  .isCropMode = &t_isCropMode,
  .isGPSValid = &t_isGPSValid,
  .latitude = &t_latitude,
  .longitude = &t_longitude,
  .altitude = &t_altitude,
  .isCameraTempValid = &t_isCameraTempValid,
  .cameraTemp = &t_cameraTemp,
  .setTimerMode = &t_setTimerMode,
  .isTimerInitialised = &t_isTimerInitialised,
  .timerHasReset = &t_timerHasReset,
  .timerHasSync = &t_timerHasSync,
  .checkTimerWarnings = &t_checkTimerWarnings
};
