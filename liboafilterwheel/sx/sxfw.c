/*****************************************************************************
 *
 * sxfw.c -- Control Starlight Xpress filter wheels
 *
 * Copyright 2014,2015,2016,2018 James Fidell (james@openastroproject.org)
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
#include <hidapi.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "sxfw.h"

int
oaSXGetFilterWheels ( FILTERWHEEL_LIST* deviceList )
{
  int                                   numFound = 0, i;
  int					ret;
  struct hid_device_info*               devlist;
  struct hid_device_info*               device;
  oaFilterWheelDevice*			wheel;
  DEVICE_INFO*				_private;

  if ( hid_init()) {
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (!( devlist = hid_enumerate ( STARLIGHT_XPRESS_VID,
      STARLIGHT_XPRESS_FILTERWHEEL_PID ))) {
    return OA_ERR_NONE;
  }

  device = devlist;
  i = 0;
  while ( device ) {

    if (!( wheel = malloc ( sizeof ( oaFilterWheelDevice )))) {
      hid_free_enumeration ( devlist );
      hid_exit();
      return -OA_ERR_MEM_ALLOC;
    }
    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) wheel );
      hid_free_enumeration ( devlist );
      hid_exit();
      return -OA_ERR_MEM_ALLOC;
    }
    _oaInitFilterWheelDeviceFunctionPointers ( wheel );
    wheel->interface = OA_FW_IF_SX;
    _private->devIndex = i;

    // Don't know of any other types at the moment
    ( void ) strcpy ( wheel->deviceName,
        "Starlight Xpress Universal Wheel" );
    _private->devType = SX_UNIVERSAL;
    _private->vendorId = STARLIGHT_XPRESS_VID;
    _private->productId = STARLIGHT_XPRESS_FILTERWHEEL_PID;
    ( void ) strncpy ( _private->sysPath, device->path, PATH_MAX );
    wheel->_private = _private;
    wheel->initFilterWheel = oaSXInitFilterWheel;
    if (( ret = _oaCheckFilterWheelArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) wheel );
      ( void ) free (( void* ) _private );
      hid_free_enumeration ( devlist );
      hid_exit();
      return ret;
    }
    deviceList->wheelList[ deviceList->numFilterWheels++ ] = wheel;
    numFound++;

    device = device->next;
  }

  hid_free_enumeration ( devlist );
  hid_exit();

  return numFound;
}


const char*
oaSXWheelGetName ( oaFilterWheel* wheel )
{
  return wheel->deviceName;
}
