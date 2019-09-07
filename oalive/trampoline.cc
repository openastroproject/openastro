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

#include "commonConfig.h"
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
t_setFilterSlotCount ( int num __attribute((unused)))
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
t_updateCameraControlCheckbox ( int c __attribute__((unused)),
		int v __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return;
}


int
t_getCameraSpinboxMinimum ( int control __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return 0;
}


int
t_getCameraSpinboxMaximum ( int control __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return 0;
}


int
t_getCameraSpinboxStep ( int control __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return 0;
}


int
t_getCameraSpinboxValue ( int control __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
  return 0;
}


void
t_updateCameraSpinbox ( int control __attribute__((unused)),
		int value __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


QStringList
t_getCameraFrameRates ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	QStringList l;
  return l;
}


int
t_getCameraFrameRateIndex ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return 0;
}


void
t_updateCameraFrameRate ( int rate __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_setCameraFlipX ( int val __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_setCameraFlipY ( int val __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_updateForceFrameFormat ( unsigned int oldState __attribute__((unused)),
		unsigned int format __attribute__((unused)))
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
t_enableTIFFCapture ( int val __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_enableMOVCapture ( int val __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_enablePNGCapture ( int val __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_enableNamedPipeCapture ( int val __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_setVideoFramePixelFormat ( int format __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_destroyLayout ( QLayout* layout )
{
  state.mainWindow->destroyLayout ( layout );
}


void
t_setTimerMode ( int mode __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


void
t_checkTimerWarnings ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
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
t_enableTimerExternalLED ( int state __attribute__((unused)))
{
	qWarning() << __FUNCTION__ << "doing nothing";
}


int
t_getTimerExternalLED ( void )
{
	qWarning() << __FUNCTION__ << "doing nothing";
	return 0;
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
