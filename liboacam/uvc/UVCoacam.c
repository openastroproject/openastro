/*****************************************************************************
 *
 * UVCoacam.c -- main entrypoint for UVC Cameras
 *
 * Copyright 2013,2014,2015,2016 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBUVC

#include <openastro/camera.h>
#include <libuvc/libuvc.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "UVCoacam.h"


int
oaUVCGetCameras ( CAMERA_LIST* deviceList, int flags )
{
  unsigned int			numFound = 0, numUVCDevices = 0, i;
  unsigned int  	        index;
  unsigned short		busNum, addr;
  int				ret;
  uvc_context_t*		ctx;
  uvc_device_t**      	 	devlist;
  uvc_device_t*      	 	device;
  uvc_device_descriptor_t*	desc;
  oaCameraDevice*		dev;
  DEVICE_INFO*			_private;

  if ( uvc_init ( &ctx, 0 ) != UVC_SUCCESS ) {
    fprintf ( stderr, "uvc_init failed\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( uvc_get_device_list ( ctx, &devlist ) != UVC_SUCCESS ) {
    fprintf ( stderr, "uvc_get_device_list failed\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }
  while ( devlist[numUVCDevices] ) { numUVCDevices++; }
  if ( numUVCDevices < 1 ) {
    uvc_free_device_list ( devlist, 1 );
    uvc_exit ( ctx );
    return 0;
  }

  for ( i = 0; i < numUVCDevices; i++ ) {
    device = devlist[i];
    if ( uvc_get_device_descriptor ( device, &desc )) {
      uvc_free_device_list ( devlist, 1 );
      uvc_exit ( ctx );
      return -OA_ERR_SYSTEM_ERROR;
    }
    busNum = uvc_get_bus_number ( device );
    addr = uvc_get_device_address ( device );
    index = ( busNum << 8 ) | addr;

    // fprintf ( stderr, "found %s %s camera at %d, %d\n", desc->manufacturer ? desc->manufacturer : "unknown", desc->product ? desc->product: "unknown", busNum, addr );
    // now we can drop the data into the list
    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
      uvc_free_device_descriptor ( desc );
      uvc_free_device_list ( devlist, 1 );
      uvc_exit ( ctx );
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      free ( dev );
      uvc_free_device_descriptor ( desc );
      uvc_free_device_list ( devlist, 1 );
      uvc_exit ( ctx );
      _oaFreeCameraDeviceList ( deviceList );
      return -OA_ERR_MEM_ALLOC;
    }

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_UVC;
    _private->devIndex = index;
    dev->_private = _private;

    if ( desc->product ) {
      ( void ) strcpy ( dev->deviceName, desc->product );
    } else {
      ( void ) snprintf ( dev->deviceName, OA_MAX_NAME_LEN+1,
          "UVC camera @ USB %d:%d", busNum, addr );
    }
    // dev->vendorId = desc->vendorId;
    // dev->productId = desc->productId;
    dev->initCamera = oaUVCInitCamera;
    dev->hasLoadableFirmware = 0;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      free ( dev );
      free ( _private );
      uvc_free_device_descriptor ( desc );
      uvc_free_device_list ( devlist, 1 );
      uvc_exit ( ctx );
      _oaFreeCameraDeviceList ( deviceList );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
    numFound++;

    uvc_free_device_descriptor ( desc );
  }

  uvc_free_device_list ( devlist, 1 );
  uvc_exit ( ctx );
  return numFound;
}

#endif	/* HAVE_LIBUVC */
