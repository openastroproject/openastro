/*****************************************************************************
 *
 * filterwheel.cc -- filter wheel interface class
 *
 * Copyright 2014,2018,2019 James Fidell (james@openastroproject.org)
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

#include "commonConfig.h"
#include "filterWheel.h"
#include "captureSettings.h"
#include "fitsSettings.h"


#define wheelFuncs     wheelContext->funcs

FilterWheel::FilterWheel ( trampolineFuncs* funcs ) :
		trampolines ( funcs )
{
  initialised = 0;
}


FilterWheel::~FilterWheel()
{
}


void
FilterWheel::populateControlValue ( oaControlValue* cp, uint32_t c, int64_t v )
{
  cp->valueType = wheelContext->controls[ c ];
  switch ( wheelContext->controls[ c ] ) {
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
        wheelContext->controls[ c ] << " for control " << c;
  }
}


int64_t
FilterWheel::unpackControlValue ( oaControlValue *cp )
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
      qWarning() << "FilterWheel" << __FUNCTION__ <<
        "called with invalid control type " <<
        wheelContext->controls[ cp->valueType ];
      res = -1;
  }

  return res;
}


// FIX ME -- might be nice to make this a tidier type at some point.  vector?
int
FilterWheel::listConnected ( oaFilterWheelDevice*** devs )
{
  return ( oaGetFilterWheels ( devs ));
}


void
FilterWheel::releaseInfo ( oaFilterWheelDevice** devs )
{
  return ( oaReleaseFilterWheels ( devs ));
}


int
FilterWheel::initialise ( oaFilterWheelDevice* device )
{
  disconnect();

  if ( !device ) {
    qWarning() << "device is null!";
    return -1;
  }

  if (( wheelContext = device->initFilterWheel ( device ))) {
    initialised = 1;
    // FIX ME -- this lot should probably be done in the caller
    trampolines->setFilterSlotCount ( wheelContext->numSlots );
    trampolines->reloadFilters();
    return 0;
  }
  return -1;
}


void
FilterWheel::disconnect ( void )
{
  if ( initialised ) {
    wheelFuncs.closeWheel ( wheelContext );
    initialised = 0;
    // FIX ME -- this lot should probably be done in the caller
    trampolines->setFilterSlotCount ( 0 );
    trampolines->reloadFilters();
  }
}


const char*
FilterWheel::name ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return "";
  }
  return wheelContext->deviceName;
}


int
FilterWheel::isInitialised ( void )
{
  return initialised;
}


void
FilterWheel::warmReset ( void )
{
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return;
  }
  if ( !wheelContext->controls [ OA_FW_CTRL_WARM_RESET ]) {
    return;
  }
  populateControlValue ( &v, OA_FW_CTRL_WARM_RESET, 0 );
  wheelFuncs.setControl ( wheelContext, OA_FW_CTRL_WARM_RESET, &v );
  return;
}


void
FilterWheel::coldReset ( void )
{
  oaControlValue v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return;
  }
  if ( !wheelContext->controls [ OA_FW_CTRL_COLD_RESET ]) {
    return;
  }
  populateControlValue ( &v, OA_FW_CTRL_COLD_RESET, 0 );
  wheelFuncs.setControl ( wheelContext, OA_FW_CTRL_COLD_RESET, &v );
  return;
}


int
FilterWheel::numSlots ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return -1;
  }
  return wheelContext->numSlots;
}


int
FilterWheel::selectFilter ( int slotNum )
{
  int			cmd = 0;
  oaControlValue	v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return -1;
  }
  if ( wheelContext->controls [ OA_FW_CTRL_MOVE_ABSOLUTE_SYNC ]) {
    cmd = OA_FW_CTRL_MOVE_ABSOLUTE_SYNC;
  } else {
    if ( wheelContext->controls [ OA_FW_CTRL_MOVE_ABSOLUTE_ASYNC ]) {
      cmd = OA_FW_CTRL_MOVE_ABSOLUTE_ASYNC;
    }
  }
  if ( !cmd ) {
    return -1;
  }

  populateControlValue ( &v, cmd, slotNum );
  return wheelFuncs.setControl ( wheelContext, cmd, &v );
}


int
FilterWheel::hasWarmReset ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return -1;
  }
  return wheelContext->controls [ OA_FW_CTRL_WARM_RESET ] ? 1 : 0;
}


int
FilterWheel::hasColdReset ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return -1;
  }
  return wheelContext->controls [ OA_FW_CTRL_COLD_RESET ] ? 1 : 0;
}


int
FilterWheel::hasSpeedControl ( void )
{
  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return -1;
  }
  return wheelContext->controls [ OA_FW_CTRL_SPEED ] ? 1 : 0;
}


int
FilterWheel::getSpeed ( unsigned int* speed )
{
  oaControlValue	v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return -1;
  }

  if ( !wheelContext->controls [ OA_FW_CTRL_SPEED ]) {
    return 0;
  }
  wheelFuncs.readControl ( wheelContext, OA_FW_CTRL_SPEED, &v );
	*speed = v.int32;
  return OA_ERR_NONE;
}


int
FilterWheel::setSpeed ( unsigned int speed, int nodelay __attribute((unused)))
{
  oaControlValue	v;

  if ( !initialised ) {
    qWarning() << __FUNCTION__ << " called with filter wheel uninitialised";
    return -1;
  }
  if ( !wheelContext->controls [ OA_FW_CTRL_SPEED ]) {
    return -1;
  }

  if ( speed > 100 ) { speed = 100; }

  populateControlValue ( &v, OA_FW_CTRL_SPEED, speed );
  return wheelFuncs.setControl ( wheelContext, OA_FW_CTRL_SPEED, &v );
}


void
FilterWheel::updateSearchFilters ( int interfaceType )
{
  int numIDFilters;

  oaClearFilterWheelIDFilters ( interfaceType );
  numIDFilters = commonConfig.filterWheelConfig[ interfaceType ].count();
  if ( numIDFilters ) {
    for ( int i = 0; i < numIDFilters; i++ ) {
      oaAddFilterWheelIDFilter ( interfaceType,
          &( commonConfig.filterWheelConfig[ interfaceType ][ i ] ));
    }
  }
}


void
FilterWheel::updateAllSearchFilters ( void )
{
  for ( int i = 1; i < OA_FW_IF_COUNT; i++ ) {
    updateSearchFilters ( i );
  }
}
