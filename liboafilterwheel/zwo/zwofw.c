/*****************************************************************************
 *
 * zwofw.c -- Control ZWO filter wheels
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

#include <errno.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include <EFW_filter.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "zwofw.h"
#include "zwofwprivate.h"

int
oaZWOGetFilterWheels ( FILTERWHEEL_LIST* deviceList )
{
  int                   numFound = 0, i;
  int                   ret;
  oaFilterWheelDevice*  wheel;
  DEVICE_INFO*          _private;
  EFW_INFO              wheelInfo;
  EFW_ERROR_CODE        err;

	if (( ret = _zwofwInitLibraryFunctionPointers()) != OA_ERR_NONE ) {
		return ret;
	}

  if (( numFound = p_EFWGetNum()) < 1 ) {
    return 0;
  }

  for ( i = 0; i < numFound; i++ ) {
    if (( err = p_EFWGetID ( i, &wheelInfo.ID )) != EFW_SUCCESS ) {
      fprintf ( stderr, "%s: EFWGetID returns error %d\n", __FUNCTION__, err );
      return 0;
    }
    if (( err = p_EFWOpen ( wheelInfo.ID )) != EFW_SUCCESS ) {
      fprintf ( stderr, "%s: EFWOpen returns error %d\n", __FUNCTION__, err );
      return 0;
    }
    if (( err = p_EFWGetProperty ( wheelInfo.ID, &wheelInfo )) !=
				EFW_SUCCESS ) {
      fprintf ( stderr, "%s: EFWGetProperty returns error %d\n",
          __FUNCTION__, err );
      return 0;
    }
    p_EFWClose ( wheelInfo.ID );

    if (!( wheel = malloc ( sizeof ( oaFilterWheelDevice )))) {
      return -OA_ERR_MEM_ALLOC;
    }
    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) wheel );
      return -OA_ERR_MEM_ALLOC;
    }
    _oaInitFilterWheelDeviceFunctionPointers ( wheel );
    wheel->interface = OA_FW_IF_ZWO;
    ( void ) strcpy ( wheel->deviceName, wheelInfo.Name );
    _private->devIndex = i;
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
