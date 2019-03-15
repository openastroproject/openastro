/*****************************************************************************
 *
 * timer.cc -- timer device interface class
 *
 * Copyright 2015,2016,2017,2018,2019
 *   James Fidell (james@openastroproject.org)
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

extern "C" {
#include <openastro/timer.h>
}

#include "commonConfig.h"
#include "timer.h"
#include "timerSettings.h"


#define timerFuncs	timerContext->funcs
#define timerControls	timerContext->controls

Timer::Timer ( trampolineFuncs* redirs ) :
		trampolines ( redirs )
{
  initialised = 0;
}


Timer::~Timer()
{
}


void
Timer::populateControlValue ( oaControlValue* cp, uint32_t c, int64_t v )
{
  cp->valueType = timerContext->controls[ c ];
  switch ( timerContext->controls[ c ] ) {
    case OA_CTRL_TYPE_INT32:
      cp->int32 = v & 0xffffffff;
      break;
    case OA_CTRL_TYPE_BOOLEAN:
      cp->boolean = v ? 1 : 0;
      break;
    case OA_CTRL_TYPE_MENU:
      cp->menu = v & 0xffffffff;
      break;
    case OA_CTRL_TYPE_BUTTON:
      break;
    case OA_CTRL_TYPE_INT64:
      cp->int64 = v;
      break;
    case OA_CTRL_TYPE_DISCRETE:
      cp->discrete = v & 0xffffffff;
      break;
    default:
      qWarning() << __FUNCTION__ << " called with invalid control type " <<
        timerContext->controls[ c ] << " for control " << c;
  }
}


int64_t
Timer::unpackControlValue ( oaControlValue *cp )
{
  int64_t res;

  switch ( cp->valueType ) {
    case OA_CTRL_TYPE_INT32:
      res = cp->int32;
      break;
    case OA_CTRL_TYPE_BOOLEAN:
      res = cp->boolean;
      break;
    case OA_CTRL_TYPE_MENU:
      res = cp->menu;
      break;
    case OA_CTRL_TYPE_READONLY:
      res = cp->readonly;
      break;
    case OA_CTRL_TYPE_INT64:
      res = cp->int64;
      // FIX ME -- because at the moment Qt controls can only handle 32-bit
      // values
      res &= 0xffffffff;
      break;
    case OA_CTRL_TYPE_DISCRETE:
      res = cp->discrete;
      break;
    default:
      qWarning() << "Timer" << __FUNCTION__ <<
        "called with invalid control type " <<
        timerContext->controls[ cp->valueType ];
      res = -1;
  }

  return res;
}


// FIX ME -- might be nice to make this a tidier type at some point.  vector?
int
Timer::listConnected ( oaTimerDevice*** devs )
{
  return ( oaGetPTRDevices ( devs ));
}


void
Timer::releaseInfo ( oaTimerDevice** devs )
{
  return ( oaReleasePTRDevices ( devs ));
}


int
Timer::initialise ( oaTimerDevice* device )
{
  disconnect();

  if ( !device ) {
    qWarning() << "device is null!";
    return -1;
  }

  if (( timerContext = device->init ( device ))) {
    initialised = 1;
    return 0;
  }
  return -1;
}


void
Timer::disconnect ( void )
{
  if ( initialised ) {
    timerFuncs.close ( timerContext );
    initialised = 0;
  }
}


const char*
Timer::name ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return "";
  }
  return timerContext->deviceName;
}


int
Timer::isInitialised ( void )
{
  return initialised;
}


int
Timer::isRunning ( void )
{
  if ( initialised ) {
    return timerFuncs.isRunning ( timerContext );
  }
  return 0;
}


void
Timer::reset ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return;
  }
  if ( !timerFuncs.reset ) {
    qWarning() << __FUNCTION__ << " called with reset uninitialised";
    return;
  }
  timerFuncs.reset ( timerContext );
  return;
}


int
Timer::hasReset ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return -1;
  }
  return ( timerFuncs.reset ? 1 : 0 );
}


int
Timer::hasSync ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return -1;
  }
  return timerContext->controls [ OA_TIMER_CTRL_SYNC ] ? 1 : 0;
}


void
Timer::updateSearchFilters ( int interfaceType )
{
  int numIDFilters;

  oaClearPTRIDFilters();
  numIDFilters = commonConfig.timerConfig[ interfaceType ].count();
  if ( numIDFilters ) {
    for ( int i = 0; i < numIDFilters; i++ ) {
      oaAddPTRIDFilter (
					&( commonConfig.timerConfig[ interfaceType ][ i ] ));
    }
  }
}


void
Timer::updateAllSearchFilters ( void )
{
  for ( int i = 1; i < OA_TIMER_IF_COUNT; i++ ) {
    updateSearchFilters ( i );
  }
}


int
Timer::hasControl ( int control )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return 0;
  }

  if ( control > 0 && control < OA_TIMER_CTRL_LAST_P1 ) {
    return timerControls[ control ];
  }
  qWarning() << __FUNCTION__ << " unrecognised control" << control;
  return 0;
}


int
Timer::setControl ( int control, int64_t value )
{
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return -1;
  }

  populateControlValue ( &v, control, value );
  return timerFuncs.setControl ( timerContext, control, &v );
}


int
Timer::readControl ( int control )
{
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return 0;
  }

  if ( timerFuncs.readControl ( timerContext, control, &v ) == OA_ERR_NONE ) {
    return unpackControlValue ( &v );
  }
  qWarning() << "error trying to timer control" << control;
  return 0;
}


int
Timer::start ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return -1;
  }

  return timerFuncs.start ( timerContext );
}


oaTimerStamp*
Timer::readTimestamp ( void )
{
  // FIX ME -- this is quite an ugly way to do this
  static oaTimerStamp ts;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return 0;
  }

  if ( timerFuncs.readTimestamp ( timerContext, timerConf.timestampDelay,
      &ts ) != OA_ERR_NONE ) {
    ts.timestamp[0] = 0;
    ts.index = 0;
  }

  return &ts;
}


int
Timer::readGPS ( double* lat, double* lng, double* alt, int useCache )
{
  double	data[3];
  int		ret;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with timer uninitialised";
    return 0;
  }

  if ( useCache ) {
    if (( ret = timerFuncs.readCachedGPS ( timerContext, data )) ==
        OA_ERR_NONE ) {
      *lat = data[0];
      *lng = data[1];
      *alt = data[2];
    }
  } else {
    if (( ret = timerFuncs.readGPS ( timerContext, data )) == OA_ERR_NONE ) {
      *lat = data[0];
      *lng = data[1];
      *alt = data[2];
    }
  }

  return ret;
}


int
Timer::hasGPS ( void )
{
  if ( !initialised ) {
    return 0;
  }
  return timerContext->features.gps;
}
