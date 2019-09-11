/*****************************************************************************
 *
 * atikSerialoacam-ftdi.c -- main entrypoint for Atik serial support via udev
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
#include <openastro/camera.h>
#include <openastro/util.h>

#ifdef HAVE_FTDI_H
#include <ftdi.h>
#endif
#ifdef HAVE_LIBFTDI1_FTDI_H
#include <libftdi1/ftdi.h>
#endif
#ifdef HAVE_LIBFTDI_FTDI_H
#include <libftdi/ftdi.h>
#endif
#include <libusb-1.0/libusb.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "atikSerialoacam.h"
#include "atikSerial.h"

struct atikSerialCam atikCameraList[] =
{
  // ?
  { ATIK_FTDI_VENDOR_ID, 0xdf28, "Artemis", CAM_ARTEMIS },
  // 782x582, 8.3um pixels, USB1, 16-bit mono
  { ATIK_FTDI_VENDOR_ID, 0xdf2c, "Atik ATK-16IC-S", CAM_ATK16ICS },
  // 782x582, 8.3um pixels, USB1, 16-bit colour
  { ATIK_FTDI_VENDOR_ID, 0xdf2d, "Atik ATK-16IC-S-C", CAM_ATK16ICSC },
  // 782x582, 8.6x8.3um pixels, 16-bit, mono
  { ATIK_FTDI_VENDOR_ID, 0xdf30, "Atik ATK-16", CAM_ATK16 },
  { ATIK_FTDI_VENDOR_ID, 0xdf31, "Atik ATK-16HR", CAM_ATK16HR },
  { ATIK_FTDI_VENDOR_ID, 0xdf32, "Atik ATK-16C", CAM_ATK16HR },
  { ATIK_FTDI_VENDOR_ID, 0xdf33, "Atik ATK-16HRC", CAM_ATK16HRC },
  { ATIK_FTDI_VENDOR_ID, 0xdf34, "Mini Artemis", CAM_MINI_ARTEMIS },
  // 659x494, 7.4um pixels, USB1, mono
  { ATIK_FTDI_VENDOR_ID, 0xdf35, "Atik ATK-16IC", CAM_ATK16IC },
  // 659x494, 7.4um pixels, USB1, colour
  { ATIK_FTDI_VENDOR_ID, 0xdf36, "Atik ATK-16IC-C", CAM_ATK16ICC }
};


int
oaAtikSerialGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  int numFound = 0, numUSBDevices, i, j, ret;
  int matchedVidPid, numCameras;
  libusb_context*                       ctx = 0;
  libusb_device**                       devlist;
  libusb_device*                        device;
  struct libusb_device_descriptor       desc;
  oaCameraDevice*                       camera;
  unsigned short                        busNum, addr;
  struct ftdi_context*                  ftdiCtx;
  DEVICE_INFO*				_private;

  numCameras = sizeof ( atikCameraList ) / sizeof ( struct atikSerialCam );

  libusb_init ( &ctx );
  // libusb_set_debug ( ctx, LIBUSB_LOG_LEVEL_DEBUG );
  numUSBDevices = libusb_get_device_list ( ctx, &devlist );
  if ( numUSBDevices < 1 ) {
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( ctx );
    if ( numUSBDevices ) {
      return -OA_ERR_SYSTEM_ERROR;
    }
    return 0;
  }

  if (!( ftdiCtx = ftdi_new())) {
    fprintf ( stderr, "can't connect to ftdi\n" );
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( ctx );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( ret = ftdi_init ( ftdiCtx ))) {
    fprintf ( stderr, "can't initialise ftdi context, err = %d\n", ret );
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( ctx );
    return -OA_ERR_SYSTEM_ERROR;
  }

  ftdi_set_interface ( ftdiCtx, INTERFACE_ANY );

  for ( i = 0; i < numUSBDevices; i++ ) {
    device = devlist[i];
    if ( LIBUSB_SUCCESS != libusb_get_device_descriptor ( device, &desc )) {
      libusb_free_device_list ( devlist, 1 );
      libusb_exit ( ctx );
      return -OA_ERR_SYSTEM_ERROR;
    }
    busNum = libusb_get_bus_number ( device );
    addr = libusb_get_device_address ( device );
    matchedVidPid = -1;
    for ( j = 0; j < numCameras && matchedVidPid == -1; j++ ) {
      if ( desc.idVendor == atikCameraList[j].vendorId &&
          desc.idProduct == atikCameraList[j].productId ) {
        matchedVidPid = j;
      }
    }
    if ( matchedVidPid >= 0 ) {

      if (!( camera = malloc ( sizeof ( oaCameraDevice )))) {
        libusb_free_device_list ( devlist, 1 );
        libusb_exit ( ctx );
        ftdi_free ( ftdiCtx );
        return -OA_ERR_MEM_ALLOC;
      }
      if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
        free ( camera );
        _oaFreeCameraDeviceList ( deviceList );
        return -OA_ERR_MEM_ALLOC;
      }

      _oaInitCameraDeviceFunctionPointers ( camera );
      camera->initCamera = oaAtikSerialInitCamera;
      camera->interface = OA_CAM_IF_ATIK_SERIAL;
      camera->_private = _private;
      _private->devType = atikCameraList[ matchedVidPid ].devType;
      ( void ) strcpy ( camera->deviceName,
          atikCameraList[ matchedVidPid ].name );
      _private->devIndex = ( busNum << 8 ) | addr;
      _private->vendorId = atikCameraList[ matchedVidPid ].vendorId;
      _private->productId = atikCameraList[ matchedVidPid ].productId;
      camera->hasLoadableFirmware = 0;
      if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
        free ( camera );
        free ( _private );
        _oaFreeCameraDeviceList ( deviceList );
        return ret;
      }
      deviceList->cameraList[ deviceList->numCameras++ ] = camera;
      numFound++;
    }
  }

  libusb_free_device_list ( devlist, 1 );
  libusb_exit ( ctx );

  return numFound;
}
