/*****************************************************************************
 *
 * IIDCoacam.c -- main entrypoint for IEE1394/IIDC Cameras
 *
 * Copyright 2013,2014,2015,2016,2018,2019
 *     James Fidell (james@openastroproject.org)
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

#if HAVE_LIBDC1394

#include <openastro/camera.h>
#include <dc1394/dc1394.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "IIDCoacam.h"
#include "IIDCprivate.h"


/**
 * Cycle through the sys filesystem looking for USBdevices with one
 * of the appropriate vendor ID and product ID
 */

int
oaIIDCGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  dc1394_t*	        iidcContext;
  dc1394error_t	        err;
  dc1394camera_list_t*  devlist;
  dc1394camera_t*       device;
  int                   numFound, ret;
  unsigned int		i;
  uint8_t               unit;
  uint64_t              guid;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;

  if (( ret = _iidcInitLibraryFunctionPointers()) != OA_ERR_NONE ) {
    return ret;
  }

  iidcContext = p_dc1394_new();
  if ( !iidcContext ) {
    fprintf ( stderr, "Can't get IIDC context\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }
  err = p_dc1394_camera_enumerate ( iidcContext, &devlist );
  if ( err < 0 ) {
    p_dc1394_free ( iidcContext );
    fprintf ( stderr, "Can't enumerate IIDC devices\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( !devlist->num ) {
    p_dc1394_camera_free_list ( devlist );
    p_dc1394_free ( iidcContext );
    return 0;
  }

  numFound = 0;
  for ( i = 0; i < devlist->num; i++ ) {
    guid = devlist->ids[i].guid;
    unit = devlist->ids[i].unit;
    device = p_dc1394_camera_new_unit ( iidcContext, guid, unit );

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
      p_dc1394_camera_free_list ( devlist );
      p_dc1394_free ( iidcContext );
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
      p_dc1394_camera_free_list ( devlist );
      p_dc1394_free ( iidcContext );
      return -OA_ERR_MEM_ALLOC;
    }

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_IIDC;
    ( void ) snprintf ( dev->deviceName, OA_MAX_NAME_LEN+1, "%s %s",
        device->vendor, device->model );
    _private->guid = guid;
    _private->unit = unit;
    dev->_private = _private;
    dev->initCamera = oaIIDCInitCamera;
    dev->hasLoadableFirmware = 0;
    p_dc1394_camera_free ( device );
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      free (( void* ) dev );
      free (( void* ) _private );
      p_dc1394_camera_free_list ( devlist );
      p_dc1394_free ( iidcContext );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
    numFound++;
  }

  p_dc1394_camera_free_list ( devlist );
  p_dc1394_free ( iidcContext );
  return numFound;
}

#endif /* HAVE_LIBDC1394 */
