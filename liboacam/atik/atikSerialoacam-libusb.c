/*****************************************************************************
 *
 * atikSerialoacam-libusb.c -- main entrypoint for Atik serial support via
 * libusb
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

#include <libusb-1.0/libusb.h>

#include "atikSerialoacam.h"
#include "unimplemented.h"

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

#define	cameraState	camera->_atikSerial


/**
 * Use udev to traverse the filesystem to see what we can find from the
 * list of cameras above
 */

int
oaAtikSerialGetCameras ( oaCameraDevice** deviceList,
		unsigned long featureFlags, int flags )
{
  unsigned int                          numFound = 0, current = 0;
  unsigned int                          numUSBDevices, i, j;
  unsigned int                          index;
  libusb_context*                       ctx = 0;
  libusb_device**                       devlist;
  libusb_device*                        device;
  libusb_device_handle*                 handle;
  struct libusb_device_descriptor       desc;
  oaCameraDevice*                       dev;
  unsigned int                          matched;
  unsigned int                          numCameras;
  unsigned short                        busNum, addr;

  numCameras = sizeof ( atikCameraList ) / sizeof ( struct atikSerialCam );

  while ( deviceList[ current ] ) {
    current++;
  }
  if ( current >= OA_MAX_DEVICES ) {
    return 0;
  }

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

  for ( i = 0; i < numUSBDevices; i++ ) {
    device = devlist[i];
    if ( LIBUSB_SUCCESS != libusb_get_device_descriptor ( device, &desc )) {
      libusb_free_device_list ( devlist, 1 );
      libusb_exit ( ctx );
      return -OA_ERR_SYSTEM_ERROR;
    }
    busNum = libusb_get_bus_number ( device );
    addr = libusb_get_device_address ( device );
    index = ( busNum << 8 ) | addr;
    for ( j = 0, matched = 0; j < numCameras && !matched; j++ ) {
      if ( desc.idVendor == atikCameraList[j].vendorId &&
          desc.idProduct == atikCameraList[j].productId ) {

        // now we can drop the data into the list
        if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_MEM_ALLOC;
        }
        _oaInitCameraDeviceFunctionPointers ( dev );
        dev->interface = OA_CAM_IF_ATIK_SERIAL;
        dev->_devIndex = index;
        ( void ) strcpy ( dev->deviceName, atikCameraList[j].name );
        dev->_devType = atikCameraList[j].devType;
        dev->_vendorId = atikCameraList[j].vendorId;
        dev->_productId = atikCameraList[j].productId;
        dev->initCamera = oaAtikSerialInitCamera;
        dev->hasLoadableFirmware = oaAtikSerialCameraDeviceHasLoadableFirmware;
        // store the device data here so we can use it later
        dev->_misc = j;
        deviceList[ current++ ] = dev;
        numFound++;
        matched = 1;
      }
    }
  }

  libusb_free_device_list ( devlist, 1 );
  libusb_exit ( ctx );

  return numFound;
}
