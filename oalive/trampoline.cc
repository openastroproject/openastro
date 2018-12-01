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
t_isCropMode ( void )
{
  return state.cropMode;
}


unsigned int
t_sensorSizeX ( void )
{
  return state.sensorSizeX;
}


unsigned int
t_sensorSizeY ( void )
{
  return state.sensorSizeY;
}


unsigned int
t_cropSizeX ( void )
{
  return state.cropSizeX;
}


unsigned int
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
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_checkTimerWarnings ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


int
t_binning2x2 ( void )
{
	return config.binning2x2;
}

int
t_colourise ( void )
{
	return config.colourise;
}

int
t_useROI ( void )
{
	return config.useROI;
}


unsigned int
t_imageSizeX ( void )
{
	return config.imageSizeX;
}


unsigned int
t_imageSizeY ( void )
{
	return config.imageSizeY;
}


int
t_frameRateNumerator ( void )
{
	return config.frameRateNumerator;
}


int
t_frameRateDenominator ( void )
{
	return config.frameRateDenominator;
}


int
t_filterOption ( void )
{
	return config.filterOption;
}


int
t_numFilters ( void )
{
	return config.numFilters;
}


QString
t_filterName ( int n )
{
	return config.filters[n].filterName;
}


int64_t
t_cameraControlValue ( int m, int c )
{
	return cameraConf.controlValues[m][c];
}


int
t_fileTypeOption ( void )
{
	return config.fileTypeOption;
}


QString
t_frameFileNameTemplate ( void )
{
	return config.frameFileNameTemplate;
}


QString
t_fileNameTemplate ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return "";
}


int
t_limitEnabled ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return 0;
}


int
t_framesLimitValue ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return 0;
}


int
t_secondsLimitValue ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return 0;
}


QString
t_captureDirectory ( void )
{
	return config.captureDirectory;
}


QString
t_currentDirectory ( void )
{
	return state.currentDirectory;
}


QList<userDeviceConfig>
t_filterWheelDeviceConfig ( int interfaceType )
{
  return config.filterWheelConfig[ interfaceType ];
}


QList<userDeviceConfig>
t_timerDeviceConfig ( int interfaceType )
{
  return config.timerConfig[ interfaceType ];
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
  .isDemosaicEnabled = &t_isDemosaicEnabled,
  .isBinningValid = &t_isBinningValid,
  .binModeX = &t_binModeX,
  .binModeY = &t_binModeY,
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
	.checkTimerWarnings = &t_checkTimerWarnings,
  .binning2x2 = &t_binning2x2,
  .colourise = &t_colourise,
  .useROI = &t_useROI,
  .imageSizeX = &t_imageSizeX,
  .imageSizeY = &t_imageSizeY,
  .frameRateNumerator = &t_frameRateNumerator,
  .frameRateDenominator = &t_frameRateDenominator,
  .filterOption = &t_filterOption,
  .numFilters = &t_numFilters,
  .filterName = &t_filterName,
  .cameraControlValue = &t_cameraControlValue,
  .fileTypeOption = &t_fileTypeOption,
  .frameFileNameTemplate = &t_frameFileNameTemplate,
  .fileNameTemplate = &t_fileNameTemplate,
  .limitEnabled = &t_limitEnabled,
  .framesLimitValue = &t_framesLimitValue,
  .secondsLimitValue = &t_secondsLimitValue,
  .captureDirectory = &t_captureDirectory,
  .currentDirectory = &t_currentDirectory,
	.filterWheelDeviceConfig = &t_filterWheelDeviceConfig,
	.timerDeviceConfig = &t_timerDeviceConfig,
	.updateConfig = &t_updateConfig,
	.showStatusMessage = &t_showStatusMessage,
	.propagateNewSlotName = &t_propagateNewSlotName,
	.slotFilterName = &t_slotFilterName
};
