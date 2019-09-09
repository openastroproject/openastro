/*****************************************************************************
 *
 * QHYconnect.c -- Initialise QHY cameras
 *
 * Copyright 2013,2014,2015,2017,2018,2019
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
#include <openastro/camera.h>
#include <openastro/util.h>

#include <libusb-1.0/libusb.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "QHY.h"
#include "QHYoacam.h"
#include "QHYstate.h"
#include "QHY5.h"
#include "QHY6.h"
#include "QHY5II.h"
#include "QHY5LII.h"
#include "IMG132E.h"


static void _QHYInitFunctionPointers ( oaCamera* );


/**
 * Initialise a given camera device
 */

oaCamera*
oaQHYInitCamera ( oaCameraDevice* device )
{
  oaCamera*				camera;
  int                   		i, matched, ret;
  int					deviceAddr, deviceBus, numUSBDevices;
  libusb_device**			devlist;
  libusb_device*			usbDevice;
  libusb_device_handle*			usbHandle = 0;
  struct libusb_device_descriptor	desc;
  DEVICE_INFO*				devInfo;
  QHY_STATE*				cameraInfo;
  COMMON_INFO*				commonInfo;


  devInfo = device->_private;
  switch ( devInfo->devType ) {
    case CAM_QHY5:
    case CAM_QHY6_ST4:
    case CAM_QHY5II:
    case CAM_QHY5LIIM:
    case CAM_QHY5LIIC:
    case CAM_IMG132E:
      break;
    default:
      fprintf ( stderr, "Unsupported camera %ld: %s\n", devInfo->devType,
          device->deviceName );
      return 0;
      break;
  }

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( QHY_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  _QHYInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  cameraInfo->index = -1;

  // FIX ME -- This is a bit ugly.  Much of it is repeated from the
  // getCameras function.  I should join the two together somehow.

  libusb_init ( &cameraInfo->usbContext );
  // libusb_set_debug ( cameraInfo->usbContext, LIBUSB_LOG_LEVEL_DEBUG );
  numUSBDevices = libusb_get_device_list ( cameraInfo->usbContext, &devlist );
  if ( numUSBDevices < 1 ) {
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( cameraInfo->usbContext );
    if ( numUSBDevices ) {
      fprintf ( stderr, "Can't see any USB devices now (list returns -1)\n" );
      FREE_DATA_STRUCTS;
      return 0;
    }
    fprintf ( stderr, "Can't see any USB devices now\n" );
    FREE_DATA_STRUCTS;
    return 0;
  }

  matched = 0;
  deviceAddr = devInfo->devIndex & 0xff;
  deviceBus = devInfo->devIndex >> 8;
  for ( i = 0; i < numUSBDevices && !matched; i++ ) {
    usbDevice = devlist[i];
    if ( LIBUSB_SUCCESS != libusb_get_device_descriptor ( usbDevice, &desc )) {
      libusb_free_device_list ( devlist, 1 );
      libusb_exit ( cameraInfo->usbContext );
      fprintf ( stderr, "get device descriptor failed\n" );
      FREE_DATA_STRUCTS;
      return 0;
    }
    if ( desc.idVendor == cameraList[ devInfo->misc ].vendorId &&
        desc.idProduct == cameraList[ devInfo->misc ].productId &&
        libusb_get_bus_number ( usbDevice ) == deviceBus &&
        libusb_get_device_address ( usbDevice ) == deviceAddr ) {
      // this looks like the one!
      matched = 1;
      libusb_open ( usbDevice, &usbHandle );
    }
  }
  libusb_free_device_list ( devlist, 1 );
  if ( !matched ) {
    fprintf ( stderr, "No matching USB device found!\n" );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if ( !usbHandle ) {
    fprintf ( stderr, "Unable to open USB device!\n" );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( libusb_kernel_driver_active ( usbHandle, 0 )) {
		// FIX ME -- should reattach this if we detached it
    libusb_detach_kernel_driver( usbHandle, 0 );
  }

  if (( ret = libusb_set_configuration ( usbHandle, 1 ))) {
    fprintf ( stderr, "Can't get configuration for USB device! err = %d\n",
        ret );
    fprintf ( stderr, "Try unplugging and reconnecting the device?\n" );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if ( libusb_claim_interface ( usbHandle, 0 )) {
    fprintf ( stderr, "Unable to claim interface for USB device!\n" );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( CAM_QHY5 == cameraInfo->cameraType ) {
    // may not be required?
    if ( libusb_set_interface_alt_setting ( usbHandle, 0, 0 )) {
      fprintf ( stderr, "Unable to set alternate interface for USB device!\n" );
      libusb_release_interface ( cameraInfo->usbHandle, 0 );
      libusb_exit ( cameraInfo->usbContext );
      FREE_DATA_STRUCTS;
      return 0;
    }
  }

  camera->interface = device->interface;
  cameraInfo->usbHandle = usbHandle;
  cameraInfo->index = devInfo->devIndex;
  cameraInfo->cameraType = devInfo->devType;
  cameraInfo->droppedFrames = 0;
  cameraInfo->isColour = devInfo->colour;

  // This first one probably isn't required any more
  pthread_mutex_init ( &cameraInfo->usbMutex, 0 );
  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  switch ( cameraInfo->cameraType ) {
    case CAM_QHY5:
      ret = _QHY5InitCamera ( camera );
      break;
    case CAM_QHY6_ST4:
      ret = _QHY6InitCamera ( camera );
      break;
    case CAM_QHY5II:
      ret = _QHY5IIInitCamera ( camera );
      break;
    case CAM_QHY5LIIM:
    case CAM_QHY5LIIC:
      ret = _QHY5LIIInitCamera ( camera );
      break;
    case CAM_IMG132E:
      ret = _IMG132EInitCamera ( camera );
      break;
    default:
      fprintf ( stderr, "unsupported camera type\n" );
      ret = -1;
      break;
  }
  if ( ret ) {
    libusb_release_interface ( cameraInfo->usbHandle, 0 );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    camera = 0;
  }

  return camera;
}


static void
_QHYInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaQHYInitCamera;
}
