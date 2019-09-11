/*****************************************************************************
 *
 * LegacyAltairoacam.c -- main entrypoint for Altair cameras
 *
 * Copyright 2016,2017,2018,2019 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#endif
#include <openastro/camera.h>
#include <altaircamlegacy.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "LegacyAltairoacam.h"
#include "LegacyAltairprivate.h"


#define ALTAIR_PREFIX	1
#define TOUPCAM_PREFIX	2

/**
 * Cycle through the list of cameras returned by the altaircam library
 */

int
oaAltairLegacyGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  ToupcamInst		devList[ TOUPCAM_MAX ];
  unsigned int		numCameras;
  unsigned int		i;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;
  int             ret;

	if (( ret = _altairLegacyInitLibraryFunctionPointers()) != OA_ERR_NONE ) {
		return ret;
	}

  numCameras = ( p_legacyAltaircam_Enum )( devList );
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
    dev->interface = OA_CAM_IF_ALTAIRCAM_LEGACY;
    ( void ) strncpy ( dev->deviceName, devList[i].displayname,
        OA_MAX_NAME_LEN+1 );
    _private->devIndex = i;
    ( void ) strcpy ( _private->deviceId, devList[i].id );
    dev->_private = _private;
    dev->initCamera = oaAltairLegacyInitCamera;
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
