/*****************************************************************************
 *
 * dummyfw.c -- Control dummy filter wheels
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#include "oafwprivate.h"
#include "unimplemented.h"
#include "dummyfw.h"


int
oaDummyGetFilterWheels ( FILTERWHEEL_LIST* deviceList )
{
  int                   ret;
  oaFilterWheelDevice*  wheel;
  DEVICE_INFO*          _private;

  if (!( wheel = malloc ( sizeof ( oaFilterWheelDevice )))) {
    return -OA_ERR_MEM_ALLOC;
  }
  if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
    ( void ) free (( void* ) wheel );
    return -OA_ERR_MEM_ALLOC;
  }
  _oaInitFilterWheelDeviceFunctionPointers ( wheel );
  wheel->interface = OA_FW_IF_DUMMY;
  ( void ) strcpy ( wheel->deviceName, "Dummy 7-slot filter wheel" );
  wheel->_private = _private;
  wheel->initFilterWheel = oaDummyInitFilterWheel;
  if (( ret = _oaCheckFilterWheelArraySize ( deviceList )) < 0 ) {
    ( void ) free (( void* ) wheel );
    ( void ) free (( void* ) _private );
    return ret;
  }
  deviceList->wheelList[ deviceList->numFilterWheels++ ] = wheel;
  return 1;
}


const char*
oaDummyWheelGetName ( oaFilterWheel* wheel )
{
  return wheel->deviceName;
}
