/*****************************************************************************
 *
 * dummyoacam.c -- main entrypoint for dummy camera
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

#include "unimplemented.h"
#include "oacamprivate.h"
#include "dummyoacam.h"


int
oaDummyGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  unsigned int		i;
  int							ret;
  oaCameraDevice*	dev[2];
  DEVICE_INFO*		_private[2];

	// Create two cameras -- one for planetary and one for DSO

	if (!( dev[0] = malloc ( sizeof ( oaCameraDevice )))) {
    return -OA_ERR_MEM_ALLOC;
  }
	if (!( dev[1] = malloc ( sizeof ( oaCameraDevice )))) {
    ( void ) free (( void* ) dev[0] );
    return -OA_ERR_MEM_ALLOC;
  }

  if (!( _private[0] = malloc ( sizeof ( DEVICE_INFO )))) {
    ( void ) free (( void* ) dev[0] );
    ( void ) free (( void* ) dev[1] );
    return -OA_ERR_MEM_ALLOC;
  }
  if (!( _private[1] = malloc ( sizeof ( DEVICE_INFO )))) {
    ( void ) free (( void* ) dev[0] );
    ( void ) free (( void* ) dev[1] );
    ( void ) free (( void* ) _private[0] );
    return -OA_ERR_MEM_ALLOC;
  }

  _oaInitCameraDeviceFunctionPointers ( dev[0] );
  _oaInitCameraDeviceFunctionPointers ( dev[1] );

  dev[0]->interface = dev[1]->interface = OA_CAM_IF_DUMMY;
  _private[0]->devType = _private[1]->devType = 0;
  _private[0]->devIndex = _private[1]->devIndex = 0;
	( void ) strncpy ( dev[0]->deviceName, "Planetary cam", OA_MAX_NAME_LEN );
	( void ) strncpy ( dev[1]->deviceName, "DSO cam", OA_MAX_NAME_LEN );
  dev[0]->_private = _private[0];
  dev[1]->_private = _private[1];
  dev[0]->initCamera = dev[1]->initCamera = oaDummyInitCamera;
  dev[0]->hasLoadableFirmware = dev[1]->hasLoadableFirmware = 0;

	for ( i = 0; i < 2; i++ ) {
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) dev[0] );
      ( void ) free (( void* ) dev[1] );
      ( void ) free (( void* ) _private[0] );
      ( void ) free (( void* ) _private[1] );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev[i];
  }

  return 2;
}
