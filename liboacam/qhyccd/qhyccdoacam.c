/*****************************************************************************
 *
 * qhyccdoacam.c -- main entrypoint for libqhyccd camera support
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
#include <qhyccd/qhyccd.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "qhyccdoacam.h"
#include "qhyccdprivate.h"


/**
 * Cycle through the list of cameras returned by libqhyccd
 */

int
oaQHYCCDGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  unsigned int		numCameras;
  unsigned int		i;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;
  int                   ret;
	char						qhyccdId[ 64 ]; // size is a guess
	char						qhyccdModel[ 64 ]; // size is a guess
#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
	char						firmwarePath[ PATH_MAX+1 ];
#endif

	if (( ret = _qhyccdInitLibraryFunctionPointers()) != OA_ERR_NONE ) {
    return ret;
  }

	if ( p_InitQHYCCDResource() != QHYCCD_SUCCESS ) {
		fprintf ( stderr, "can't init libqhyccd\n" );
		return 0;
	}

#if defined(__APPLE__) && defined(__MACH__) && TARGET_OS_MAC == 1
  if ( installPathRoot ) {
		( void ) strcpy ( firmwarePath, installPathRoot );
  }
	( void ) strcat ( firmwarePath, FIRMWARE_QHY_PATH );
	// because, stupidly, the firmware directory has to be called
	// "firmware"
	// This may in fact have changed in the v4.0.14 release, but it's
	// not entirely clear from the "documentation"
	( void ) strcat ( firmwarePath, "/firmware" );
	p_OSXInitQHYCCDFirmware ( firmwarePath );
	// Don't really know how long this should be, but a short delay appears
	// to be required to allow the camera(s) to reset
	sleep ( 3 );
#endif

  numCameras = ( p_ScanQHYCCD )();
  if ( numCameras < 1 ) {
		p_ReleaseQHYCCDResource();
    return 0;
  }

  for ( i = 0; i < numCameras; i++ ) {

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
			p_ReleaseQHYCCDResource();
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
			p_ReleaseQHYCCDResource();
      return -OA_ERR_MEM_ALLOC;
    }

		if ( p_GetQHYCCDId ( i, qhyccdId ) != QHYCCD_SUCCESS ) {
			p_ReleaseQHYCCDResource();
			fprintf ( stderr, "can't get id for camera %d\n", i );
			return 0;
		} 

		if ( p_GetQHYCCDModel ( qhyccdId, qhyccdModel ) != QHYCCD_SUCCESS ) {
			p_ReleaseQHYCCDResource();
			fprintf ( stderr, "can't get model for camera %d\n", i );
			return 0;
		} 

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_QHYCCD;
    ( void ) strncpy ( dev->deviceName, qhyccdModel, OA_MAX_NAME_LEN+1 );
    _private->devIndex = i;
    ( void ) strcpy ( _private->deviceId, qhyccdId );
    dev->_private = _private;
    dev->initCamera = oaQHYCCDInitCamera;
    dev->hasLoadableFirmware = 0;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
			p_ReleaseQHYCCDResource();
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
  }

	p_ReleaseQHYCCDResource();
  return numCameras;
}
