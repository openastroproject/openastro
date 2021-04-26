/*****************************************************************************
 *
 * dummyoacam.c -- main entrypoint for dummy camera
 *
 * Copyright 2019,2020,2021 James Fidell (james@openastroproject.org)
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
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "dummyoacam.h"

#define	NUM_DUMMIES	3

int
oaDummyGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  unsigned int		i, j;
  int							ret;
  oaCameraDevice*	dev[ NUM_DUMMIES ];
  DEVICE_INFO*		_private[ NUM_DUMMIES ];

	// Create three cameras -- one planetary, one astro DSO and one DSLR

	for ( i = 0; i < NUM_DUMMIES; i++ ) {
		if (!( dev[i] = malloc ( sizeof ( oaCameraDevice )))) {
			if ( i ) {
				for ( j = 0; j < i; j++ ) {
					( void ) free (( void* ) dev[j] );
				}
			}
			return -OA_ERR_MEM_ALLOC;
		}
		if (!( _private[i] = malloc ( sizeof ( DEVICE_INFO )))) {
			for ( j = 0; j <= i; j++ ) {
				( void ) free (( void* ) dev[j] );
				if ( j < i ) {
					( void ) free (( void* ) _private[j] );
				}
			}
			return -OA_ERR_MEM_ALLOC;
		}
		oaLogDebug ( OA_LOG_CAMERA, "%s: allocated @ %p for camera device",
				__func__, dev[i] );
		_private[i]->devType = 0;
		_private[i]->devIndex = 0;
		dev[i]->interface = OA_CAM_IF_DUMMY;
		dev[i]->_private = _private[i];
		dev[i]->initCamera = oaDummyInitCamera;
		_oaInitCameraDeviceFunctionPointers ( dev[i] );
  }

	( void ) strncpy ( dev[0]->deviceName, "Planetary cam", OA_MAX_NAME_LEN );
	( void ) strncpy ( dev[1]->deviceName, "DSO cam", OA_MAX_NAME_LEN );
	( void ) strncpy ( dev[2]->deviceName, "DSLR", OA_MAX_NAME_LEN );

	for ( i = 0; i < 2; i++ ) {
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
			for ( j = 0; j < NUM_DUMMIES; j++ ) {
				( void ) free (( void* ) dev[j] );
				( void ) free (( void* ) _private[j] );
				return ret;
			}
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev[i];
  }

  return NUM_DUMMIES;
}
