/*****************************************************************************
 *
 * oacam.c -- main entrypoint for Touptek cameras
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

#include <openastro/camera.h>

#include "touptek-conf.h"
#include "oacamprivate.h"
#include "unimplemented.h"
#include "touptekoacam.h"
#include "touptekprivate.h"


/**
 * Cycle through the list of cameras returned by the touptek library
 */

int
TT_FUNC( oa, GetCameras )( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  TT_VAR_TYPE( InstV2 )		devList[ TT_DEFINE( MAX )];
  unsigned int		numCameras;
  unsigned int		i;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;
  int                   ret;

	if (( ret = TT_FUNC( _, InitLibraryFunctionPointers )()) != OA_ERR_NONE ) {
		return ret;
	}

  numCameras = ( TT_LIB_PTR( EnumV2 ))( devList );
  if ( numCameras < 1 ) {
    return 0;
  }

  for ( i = 0; i < numCameras; i++ ) {

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
      return -OA_ERR_MEM_ALLOC;
    }

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = TT_INTERFACE;
    ( void ) strncpy ( dev->deviceName, devList[i].displayname,
        OA_MAX_NAME_LEN+1 );
    _private->devIndex = i;
    ( void ) strcpy ( _private->deviceId, devList[i].id );
    dev->_private = _private;
    dev->initCamera = TT_FUNC( oa, InitCamera );
    dev->hasLoadableFirmware = 0;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
  }

  return numCameras;
}
