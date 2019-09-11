/*****************************************************************************
 *
 * ZWASI2oacam.c -- main entrypoint for ZW ASI Cameras, API v2
 *
 * Copyright 2013,2014,2015,2016,2017,2019
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
#include <openastro/camera.h>

#include <ASICamera2.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "ZWASI.h"
#include "ZWASI2oacam.h"
#include "ZWASI2private.h"


static const char *cameraNames[ ZWO_NUM_CAMERAS ] = {
  "ZWO ASI030MC", "ZWO ASI031MC", "ZWO ASI031MM", "ZWO ASI034MC",
  "ZWO ASI035MC", "ZWO ASI035MM", "ZWO ASI071MC-Cool", "ZWO ASI071MC Pro",
  "ZWO ASI094MC-Cool", "ZWO ASI094MC Pro", "ZWO ASI120MC", "ZWO ASI120MC-S",
  "ZWO ASI120MC-SC", "ZWO ASI120MM", "ZWO ASI120MM Mini", "ZWO ASI120MM-S",
  "ZWO ASI120MM-SC", "ZWO ASI128MC-Cool", "ZWO ASI128MC Pro", "ZWO ASI130MM",
  "ZWO ASI136MC", "ZWO ASI174MC", "ZWO ASI174MC-Cool", "ZWO ASI174MM",
  "ZWO ASI174MM-Cool", "ZWO ASI174MM Mini", "ZWO ASI178MC",
  "ZWO ASI178MC-Cool", "ZWO ASI178MC-Pro", "ZWO ASI178MM", "ZWO ASI178MM-Cool",
  "ZWO ASI178MM-Pro", "ZWO ASI183MC", "ZWO ASI183MC-Cool", "ZWO ASI183MC Pro",
  "ZWO ASI183MM", "ZWO ASI183MM Pro", "ZWO ASI185MC", "ZWO ASI185MC-Cool",
  "ZWO ASI224MC", "ZWO ASI224MC-Cool", "ZWO ASI226MC", "ZWO ASI2400MC Pro",
  "ZWO ASI2400MM Pro", "ZWO ASI252MC", "ZWO ASI252MM", "ZWO ASI290MC",
  "ZWO ASI290MC-Cool", "ZWO ASI290MM", "ZWO ASI290MM-Cool",
  "ZWO ASI290MM Mini", "ZWO ASI294MC", "ZWO ASI294MC-Cool", "ZWO ASI294MC Pro",
  "ZWO ASI385MC", "ZWO ASI385MC-Cool", "ZWO ASI1600GT", "ZWO ASI1600MC",
  "ZWO ASI1600MC-Cool", "ZWO ASI1600MC Pro", "ZWO ASI1600MM",
  "ZWO ASI1600MM-Cool", "ZWO ASI1600MM Pro"
};

/**
 * Cycle through the cameras reported by the ASI library
 */

int
oaZWASI2GetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  unsigned int		numFound = 0, i;
  int			ret;
  const char*		currName;
  oaCameraDevice*	dev;
  DEVICE_INFO*		_private;
  ASI_CAMERA_INFO	camInfo;
  unsigned int		typesFound[ ZWO_NUM_CAMERAS + 1 ];
  int			j, cameraType, found;

	if (( ret = _asiInitLibraryFunctionPointers()) != OA_ERR_NONE ) {
		return ret;
	}

  if (( numFound = p_ASIGetNumOfConnectedCameras()) < 1 ) {
    return 0;
  }

  for ( i = 0; i <= ZWO_NUM_CAMERAS; i++ ) {
    typesFound[i] = 0;
  }

  for ( i = 0; i < numFound; i++ ) {
    p_ASIGetCameraProperty ( &camInfo, i );
    currName = camInfo.Name;
    found = 0;
    for ( j = 0; !found && j < ZWO_NUM_CAMERAS; j++ ) {
      if ( !strcmp ( currName, cameraNames[j] )) {
        found = 1;
        cameraType = j;
      }
    }
    if ( !found ) {
      fprintf ( stderr, "Unrecognised camera '%s'\n", currName );
      cameraType = ZWOCAM_UNKNOWN;
    }

    // +1 is so ZWOCAM_UNKNOWN becomes entry 0
    typesFound[ cameraType+1 ]++;

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
      return -OA_ERR_MEM_ALLOC;
    }
    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
      return -OA_ERR_MEM_ALLOC;
    }
    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_ZWASI2;
    if ( typesFound[ cameraType+1 ] == 1 ) {
      ( void ) strncpy ( dev->deviceName, currName, OA_MAX_NAME_LEN );
    } else {
      snprintf ( dev->deviceName, OA_MAX_NAME_LEN, "%s #%d", currName,
          typesFound[ cameraType+1 ] );
    }
    _private->devType = cameraType;
    _private->devIndex = i;
    dev->_private = _private;
    dev->initCamera = oaZWASI2InitCamera;
    dev->hasLoadableFirmware = 0;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
  }

  return numFound;
}
