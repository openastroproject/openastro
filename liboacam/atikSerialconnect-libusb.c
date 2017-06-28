/*****************************************************************************
 *
 * atikSerialinit-libusb.c -- Initialise Atik serial cameras via libusb
 *
 * Copyright 2014 James Fidell (james@openastroproject.org)
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

#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <openastro/camera.h>

#include "atikSerialoacam.h"
#include "unimplemented.h"
#include "oacamprivate.h"

#define	cameraState	camera->_atikSerial

static void _atikSerialInitFunctionPointers ( oaCamera* );


/**
 * Initialise a given camera device
 */

oaCamera*
oaAtikSerialInitCamera ( oaCameraDevice* device )
{
  oaCamera*                             camera;
  int                                   i, matched, ret;
  int                                   deviceAddr, deviceBus, numUSBDevices;
  libusb_device**                       devlist;
  libusb_device*                        usbDevice;
  libusb_device_handle*                 usbHandle = 0;
  struct libusb_device_descriptor       desc;
  int					numRead;
  char					buffer[64];
  char					cmd1[4] = { 'C', 'M', 'D', 1 };


  switch ( device->_devType ) {
    case CAM_QHY5:
    case CAM_QHY6_ST4:
    case CAM_QHY5LII:
      break;
    default:
      fprintf ( stderr, "Unsupported camera %ld: %s\n", device->_devType,
          device->deviceName );
      return 0;
      break;
  }

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }

  OA_CLEAR ( cameraState );
  OA_CLEAR ( camera->controls );
  OA_CLEAR ( camera->features );
  _oaInitCameraFunctionPointers ( camera );
  _QHYInitFunctionPointers ( camera );
  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraState.index = -1;

  // FIX ME -- This is a bit ugly.  Much of it is repeated from the
  // getCameras function.  I should join the two together somehow.

  libusb_init ( &cameraState.usbContext );
  // libusb_set_debug ( cameraState.usbContext, LIBUSB_LOG_LEVEL_DEBUG );
  numUSBDevices = libusb_get_device_list ( cameraState.usbContext, &devlist );
  if ( numUSBDevices < 1 ) {
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( cameraState.usbContext );
    if ( numUSBDevices ) {
      fprintf ( stderr, "Can't see any USB devices now (list returns -1)\n" );
      free ( camera );
      return 0;
    }
    fprintf ( stderr, "Can't see any USB devices now\n" );
    free ( camera );
    return 0;
  }

  matched = 0;
  deviceAddr = device->_devIndex & 0xff;
  deviceBus = device->_devIndex >> 8;
  for ( i = 0; i < numUSBDevices && !matched; i++ ) {
    usbDevice = devlist[i];
    if ( LIBUSB_SUCCESS != libusb_get_device_descriptor ( usbDevice, &desc )) {
      libusb_free_device_list ( devlist, 1 );
      libusb_exit ( cameraState.usbContext );
      fprintf ( stderr, "get device descriptor failed\n" );
      free ( camera );
      return 0;
    }
    if ( desc.idVendor == cameraList[ device->_misc ].vendorId &&
        desc.idProduct == cameraList[ device->_misc ].productId &&
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
    libusb_exit ( cameraState.usbContext );
    free ( camera );
    return 0;
  }
  if ( !usbHandle ) {
    fprintf ( stderr, "Unable to open USB device!\n" );
    libusb_exit ( cameraState.usbContext );
    free ( camera );
    return 0;
  }

  if ( libusb_kernel_driver_active ( usbHandle, 0 )) {
      libusb_detach_kernel_driver( usbHandle, 0 );
  }

  if (( ret = libusb_set_configuration ( usbHandle, 1 ))) {
    fprintf ( stderr, "Can't get configuration for USB device! err = %d\n",
        ret );
    fprintf ( stderr, "Try unplugging and reconnecting the device?\n" );
    libusb_exit ( cameraState.usbContext );
    free ( camera );
    return 0;
  }
  if ( libusb_claim_interface ( usbHandle, 0 )) {
    fprintf ( stderr, "Unable to claim interface for USB device!\n" );
    libusb_exit ( cameraState.usbContext );
    free ( camera );
    return 0;
  }

  camera->interface = device->interface;
  cameraState.usbHandle = usbHandle;
  cameraState.index = device->_devIndex;
  cameraState.cameraType = device->_devType;
  cameraState.droppedFrames = 0;

  cameraState.captureThreadStarted = 0;
  cameraState.captureThreadExit = 0;
  pthread_mutex_init ( &cameraState.captureMutex, 0 );
  pthread_mutex_init ( &cameraState.usbMutex, 0 );
  pthread_cond_init ( &cameraState.frameAvailable, 0 );

  if ( _atikSerialCamWrite ( camera, cmd1, 4 )) {
    fprintf ( stderr, "%s: write error on CMD1\n", __FUNCTION__ );
    close ( camDesc );
    return 0;
  }

  if (( numRead = _atikSerialCamRead ( camera, buffer, 2 )) != 2 ) {
    fprintf ( stderr, "%s: read error 1 on CMD1, err = %d\n", __FUNCTION__,
        numRead );
    close ( camDesc );
    return 0;
  }

  fprintf ( stderr, "%s: camera version %d.%d\n", __FUNCTION__, buffer[1],
      buffer[0] );

  if (( numRead = _atikSerialCamReadToZero ( camera, buffer )) < 1 ) {
    fprintf ( stderr, "%s: read error 2 on CMD1\n", __FUNCTION__ );
    close ( camDesc );
    return 0;
  }

  fprintf ( stderr, "%s: camera id '%s'\n", __FUNCTION__, buffer );

  if (( numRead = _atikSerialCamReadToZero ( camDesc, buffer )) < 1 ) {
    fprintf ( stderr, "%s: read error 3 on CMD1\n", __FUNCTION__ );
    close ( camDesc );
    return 0;
  }

  fprintf ( stderr, "%s: manufacturer '%s'\n", __FUNCTION__, buffer );

  if (( numRead = _atikSerialCamRead ( camDesc, buffer, 16 ) != 16 )) {
    fprintf ( stderr, "%s: read error 4 on CMD1\n", __FUNCTION__ );
    close ( camDesc );
    return 0;
  }

  oaAtikSerialCloseCamera ( camera );

  return 0;
}


static void
_atikSerialInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaAtikSerialInitCamera;
  camera->funcs.closeCamera = oaAtikSerialCloseCamera;
/*
  camera->funcs.resetCamera = oaAtikSerialCameraReset;
  // camera->funcs.startCapture = oaAtikSerialCameraStart;
  camera->funcs.startCaptureExtended = oaAtikSerialCameraStart;
  camera->funcs.stopCapture = oaAtikSerialCameraStop;
  camera->funcs.getControlRange = oaAtikSerialCameraGetControlRange;
  camera->funcs.readControl = oaAtikSerialCameraReadControl;
  camera->funcs.setControl = oaAtikSerialCameraSetControl;
  camera->funcs.testControl = oaAtikSerialCameraTestControl;
  // camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;
  camera->funcs.enumerateFrameSizes = oaAtikSerialCameraGetFrameSizes;
  camera->funcs.enumerateFrameRates = oaAtikSerialCameraGetFrameRates;
  camera->funcs.getFramePixelFormat = oaAtikSerialCameraGetFramePixelFormat;
  // camera->funcs.testROISize = oaAtikSerialCameraTestROISize;
  camera->funcs.getMenuString = oaAtikSerialCameraGetMenuString;
  // camera->funcs.getAutoWBManualSetting = oaAtikSerialCameraGetAutoWBManualSetting;
*/
}


int
oaAtikSerialCloseCamera ( oaCamera* camera )
{
  libusb_release_interface ( cameraState.usbHandle, 0 );
  libusb_close ( cameraState.usbHandle );
  libusb_exit ( cameraState.usbContext );
  return OA_ERR_NONE;
}
