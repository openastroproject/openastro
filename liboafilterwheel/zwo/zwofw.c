/*****************************************************************************
 *
 * zwofw.c -- Control ZWO filter wheels
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

#include <errno.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

// This bit of nastiness is because the ZWO SDK uses this type without
// it being defined
typedef uint8_t bool;
#include <EFW_filter.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "zwofw.h"

int
oaZWOGetFilterWheels ( FILTERWHEEL_LIST* deviceList )
{
  int                   numFound = 0, i;
  int                   ret;
  oaFilterWheelDevice*  wheel;
  DEVICE_INFO*          _private;
  EFW_INFO              wheelInfo;

  if (( numFound = EFWGetNum()) < 1 ) {
    return 0;
  }

  for ( i = 0; i < numFound; i++ ) {
    EFWGetID ( i, &wheelInfo.ID );
    EFWGetProperty ( wheelInfo.ID, &wheelInfo );

    if (!( wheel = malloc ( sizeof ( oaFilterWheelDevice )))) {
      return -OA_ERR_MEM_ALLOC;
    }
    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) wheel );
      return -OA_ERR_MEM_ALLOC;
    }
    _oaInitFilterWheelDeviceFunctionPointers ( wheel );
    wheel->interface = OA_FW_IF_ZWO;
    _private->devIndex = wheelInfo.ID;
    ( void ) strcpy ( wheel->deviceName, wheelInfo.Name );
    wheel->_private = _private;
    wheel->initFilterWheel = oaZWOInitFilterWheel;
    if (( ret = _oaCheckFilterWheelArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) wheel );
      ( void ) free (( void* ) _private );
      return ret;
    }
    deviceList->wheelList[ deviceList->numFilterWheels++ ] = wheel;
  }

  return numFound;
}


const char*
oaZWOWheelGetName ( oaFilterWheel* wheel )
{
  return wheel->deviceName;
}
