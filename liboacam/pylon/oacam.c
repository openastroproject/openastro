/*****************************************************************************
 *
 * oacam.c -- main entrypoint for Basler Pylon support
 *
 * Copyright 2020,2021
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

#include <pylonc/PylonC.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/demosaic.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "private.h"
#include "oacam.h"

/**
 * Cycle through the list of cameras returned by the Pylon library
 */

int
oaPylonGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
	PYLON_DEVICE_HANDLE		deviceHandle;
  unsigned int					i, numFound = 0;
	char									buffer[256];
	size_t								size, numCameras;
	oaCameraDevice*				dev;
	DEVICE_INFO*					_private;
	int										ret;

	if (( ret = _pylonInitLibraryFunctionPointers()) != OA_ERR_NONE ) {
		return ret;
	}

	( p_PylonInitialize )();

	if (( p_PylonEnumerateDevices )( &numCameras ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't enumerate Basler devices",
				__func__ );
		( p_PylonTerminate )();
		return -OA_ERR_SYSTEM_ERROR;
	}

  if ( !numCameras ) {
		( p_PylonTerminate )();
    return 0;
  }

  for ( i = 0; i < numCameras; i++ ) {
		if (( p_PylonCreateDeviceByIndex )( i, &deviceHandle ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Error creating Basler device by index",
					__func__ );
			( p_PylonTerminate )();
			return -OA_ERR_SYSTEM_ERROR;
		}

		if (( p_PylonDeviceOpen )( deviceHandle, PYLONC_ACCESS_MODE_CONTROL ) !=
				GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Error opening Basler device %d",
					__func__, i );
			( p_PylonTerminate )();
			return -OA_ERR_SYSTEM_ERROR;
		}

		if (( p_PylonDeviceFeatureIsReadable )( deviceHandle,
				"DeviceModelName" )) {
			size = sizeof ( buffer );
			if (( p_PylonDeviceFeatureToString )( deviceHandle, "DeviceModelName",
					buffer, &size ) != GENAPI_E_OK ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Reading Basler DeviceModelName failed", __func__ );
				( p_PylonDeviceClose )( deviceHandle );
				( p_PylonDestroyDevice )( deviceHandle );
			  ( p_PylonTerminate )();
				return -OA_ERR_SYSTEM_ERROR;
			}
		} else {
			( void ) strcpy ( buffer, "unknown camera" );
		}

		( p_PylonDeviceClose )( deviceHandle );
		( p_PylonDestroyDevice )( deviceHandle );

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
			_oaFreeCameraDeviceList ( deviceList );
			( p_PylonTerminate )();
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
			( p_PylonTerminate )();
      return -OA_ERR_MEM_ALLOC;
    }
		oaLogDebug ( OA_LOG_CAMERA, "%s: allocated @ %p for camera device",
				__func__, dev );

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_PYLON;
    ( void ) strncpy ( dev->deviceName, buffer, OA_MAX_NAME_LEN );
    dev->_private = _private;
    dev->initCamera = oaPylonInitCamera;
		_private->devIndex = i;

		/* FIX ME
		 * Perhaps we should use the info handle entry for the serial number
		 * to identify the camera?
		 * Or use PylonGetDeviceInfoHandle to get the IP address?
		 */

    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
			( p_PylonTerminate )();
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
    numFound++;
  }

	( p_PylonTerminate )();
  return numFound;
}
