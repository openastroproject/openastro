/*****************************************************************************
 *
 * trampoline.cc -- redirected function calls
 *
 * Copyright 2018,2019 James Fidell (james@openastroproject.org)
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
#include "commonState.h"
#include "commonConfig.h"

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
	if ( state.settingsWidget ) {
		state.settingsWidget->setSlotCount ( num );
	}
	if ( state.captureWidget ) {
		state.captureWidget->setSlotCount ( num );
	}
}


void
t_reloadFilters ( void )
{
	if ( state.captureWidget ) {
	  state.captureWidget->reloadFilters();
	}
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
t_updateCameraControlCheckbox ( int control, int value )
{
	state.controlWidget->updateCheckbox ( control, value );
}


int
t_getCameraSpinboxMinimum ( int control )
{
	return state.controlWidget->getSpinboxMinimum ( control );
}


int
t_getCameraSpinboxMaximum ( int control )
{
	return state.controlWidget->getSpinboxMaximum ( control );
}


int
t_getCameraSpinboxStep ( int control )
{
	return state.controlWidget->getSpinboxStep ( control );
}


int
t_getCameraSpinboxValue ( int control )
{
	return state.controlWidget->getSpinboxValue ( control );
}


void
t_updateCameraSpinbox ( int control, int value )
{
	state.controlWidget->updateSpinbox ( control, value );
}


QStringList
t_getCameraFrameRates ( void )
{
	return state.controlWidget->getFrameRates();
}


int
t_getCameraFrameRateIndex ( void )
{
	return state.controlWidget->getFrameRateIndex();
}


void
t_updateCameraFrameRate ( int rate )
{
	state.controlWidget->updateFrameRate ( rate );
}


void
t_setCameraFlipX ( int val )
{
	state.mainWindow->setFlipX ( val );
}


void
t_setCameraFlipY ( int val )
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
t_enableNamedPipeCapture ( int val )
{
  state.captureWidget->enableNamedPipeCapture ( val );
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
t_checkTimerWarnings ( void )
{
	QString		msg;

  if (( CAPTURE_FITS != commonConfig.fileTypeOption && CAPTURE_TIFF !=
			commonConfig.fileTypeOption ) || !commonConfig.limitEnabled ||
      !commonConfig.limitType ) {
    msg = QCoreApplication::translate ( "SettingsWidget",
				"\n\nWhen using timer mode the image capture type should "
        "be FITS/TIFF/PNG and a frame-based capture limit should be set." );
    QMessageBox::warning ( state.settingsWidget, APPLICATION_NAME, msg );
  }
  if ( commonState.camera && commonState.camera->isInitialised()) {
    if ( commonState.camera->hasControl ( OA_CAM_CTRL_TRIGGER_ENABLE ) &&
        timerConf.timerMode == OA_TIMER_MODE_TRIGGER &&
        !commonState.camera->readControl ( OA_CAM_CTRL_TRIGGER_ENABLE )) {
      msg = QCoreApplication::translate ( "SettingsWidget",
					"\n\nThe timer is in trigger mode but the camera is "
          "not.  These two settings should be the same." );
    }
    if ( commonState.camera->hasControl ( OA_CAM_CTRL_STROBE_ENABLE ) &&
        timerConf.timerMode == OA_TIMER_MODE_STROBE &&
        !commonState.camera->readControl ( OA_CAM_CTRL_STROBE_ENABLE )) {
      msg = QCoreApplication::translate ( "SettingsWidget",
					"\n\nThe timer is in strobe mode but the camera is "
          "not.  These two settings should be the same." );
    }
    QMessageBox::warning ( state.settingsWidget, APPLICATION_NAME, msg );
  }
}


void
t_updateConfig ( void )
{
  state.mainWindow->updateConfig();
}


void
t_showStatusMessage ( QString msg )
{
  state.mainWindow->showStatusMessage ( msg );
}


void
t_propagateNewSlotName ( int slotIndex, QString filterName )
{
  state.settingsWidget->propagateNewSlotName ( slotIndex, filterName );
}


QString
t_slotFilterName ( int slot )
{
	return state.settingsWidget->getSlotFilterName ( slot );
}


void
t_enableTimerExternalLED ( int value )
{
	state.mainWindow->enableTimerExternalLED ( value );
}


int
t_getTimerExternalLED ( void )
{
	return state.mainWindow->getTimerExternalLEDState();
}


trampolineFuncs trampolines = {
  t_getCurrentGain,
  t_getCurrentExposure,
  t_getCurrentTargetId,
  t_getCurrentFilterName,
  t_getCurrentProfileName,
  t_setFilterSlotCount,
  t_reloadFilters,
  t_updateHistogramLayout,
  t_resetAutorun,
  t_updateCameraControlCheckbox,
  t_getCameraSpinboxMinimum,
  t_getCameraSpinboxMaximum,
  t_getCameraSpinboxStep,
  t_getCameraSpinboxValue,
  t_updateCameraSpinbox,
  t_getCameraFrameRates,
  t_getCameraFrameRateIndex,
  t_updateCameraFrameRate,
  t_setCameraFlipX,
  t_setCameraFlipY,
  t_updateForceFrameFormat,
  t_reloadProfiles,
  t_resetTemperatureLabel,
  t_setDisplayFPS,
  t_enableTIFFCapture,
  t_enableMOVCapture,
  t_enablePNGCapture,
  t_enableNamedPipeCapture,
  t_setVideoFramePixelFormat,
  t_destroyLayout,
  t_checkTimerWarnings,
  t_updateConfig,
  t_showStatusMessage,
  t_propagateNewSlotName,
  t_slotFilterName,
	t_enableTimerExternalLED,
	t_getTimerExternalLED
};
