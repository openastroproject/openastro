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
t_setTimerMode ( int mode )
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


trampolineFuncs trampolines {
	t_getCurrentGain,
	t_getCurrentExposure,
	t_getCurrentTargetId,
	t_getCurrentFilterName,
	t_getCurrentProfileName,
	t_setFilterSlotCount,
	t_reloadFilters,
	t_updateHistogramLayout,
	t_resetAutorun,
	t_updateControlCheckbox,
	t_getSpinboxMinimum,
	t_getSpinboxMaximum,
	t_getSpinboxStep,
	t_getSpinboxValue,
	t_updateSpinbox,
	t_getFrameRates,
	t_getFrameRateIndex,
	t_updateFrameRate,
	t_setFlipX,
	t_setFlipY,
	t_updateForceFrameFormat,
	t_reloadProfiles,
	t_resetTemperatureLabel,
	t_setDisplayFPS,
	t_enableTIFFCapture,
	t_enableMOVCapture,
	t_enablePNGCapture,
	t_setVideoFramePixelFormat,
	t_destroyLayout,
	t_checkTimerWarnings,
	t_updateConfig,
	t_showStatusMessage,
	t_propagateNewSlotName,
	t_slotFilterName
};
