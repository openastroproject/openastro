/*****************************************************************************
 *
 * atikSerialoacam-udev.c -- main entrypoint for Atik serial support via udev
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

#include <libudev.h>
#include <libusb-1.0/libusb.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "atikSerial.h"
#include "atikSerialoacam.h"


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


/**
 * Use udev to traverse the filesystem to see what we can find from the
 * list of cameras above
 */

int
oaAtikSerialGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  struct udev*			udev;
  struct udev_enumerate*	enumerate;
  struct udev_list_entry*	devices;
  struct udev_list_entry*	dev_list_entry;
  struct udev_device*		dev;
  struct udev_device*		parentDev;
  const char*			vidStr;
  const char*			pidStr;
  const char*			path;
  const char*			deviceNode;
  unsigned int			vid, pid;
  unsigned int			numCameras, i;
  int				match, found, ret, numFound = 0;
  oaCameraDevice*		cam;
  DEVICE_INFO*			_private;

  numCameras = sizeof ( atikCameraList ) / sizeof ( struct atikSerialCam );

  if (!( udev = udev_new())) {
    fprintf ( stderr, "can't get connection to udev\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  enumerate = udev_enumerate_new ( udev );
  udev_enumerate_scan_devices ( enumerate );
  devices = udev_enumerate_get_list_entry ( enumerate );
  udev_list_entry_foreach ( dev_list_entry, devices ) {

    path = udev_list_entry_get_name ( dev_list_entry );
    dev = udev_device_new_from_syspath ( udev, path );
    deviceNode = udev_device_get_devnode ( dev );

    vidStr = pidStr = 0;
    if ( deviceNode ) {
      if (( parentDev = udev_device_get_parent_with_subsystem_devtype ( dev,
          "usb", "usb_device" ))) {
        vidStr = udev_device_get_sysattr_value ( parentDev, "idVendor" );
        pidStr = udev_device_get_sysattr_value ( parentDev, "idProduct" );
      }
    }
    if ( vidStr && pidStr ) {
      vid = pid = 0;
      sscanf ( vidStr, "%x", &vid );
      sscanf ( pidStr, "%x", &pid );
      match = found = 0;
      for ( i = 0; i < numCameras && !found; i++ ) {
        if ( vid == atikCameraList[i].vendorId &&
            pid == atikCameraList[i].productId ) {
          found = 1;
          match = i;
        }
      }
      if ( found ) {
        // now we can drop the data into the list
        if (!( cam = malloc ( sizeof ( oaCameraDevice )))) {
          udev_device_unref ( dev );
          udev_enumerate_unref ( enumerate );
          udev_unref ( udev );
          return -OA_ERR_MEM_ALLOC;
        }
        if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
          free ( cam );
          _oaFreeCameraDeviceList ( deviceList );
          return -OA_ERR_MEM_ALLOC;
        }

        _oaInitCameraDeviceFunctionPointers ( cam );
        cam->initCamera = oaAtikSerialInitCamera;
        cam->interface = OA_CAM_IF_ATIK_SERIAL;
        _private->devIndex = match;
        cam->_private = _private;
        ( void ) strcpy ( cam->deviceName, atikCameraList[ match ].name );
        _private->devType = atikCameraList[ match ].devType;
        _private->vendorId = atikCameraList[ match ].vendorId;
        _private->productId = atikCameraList[ match ].productId;
        cam->hasLoadableFirmware = 0;
        ( void ) strncpy ( _private->sysPath, deviceNode, PATH_MAX );
        if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
          free ( dev );
          free ( _private );
          _oaFreeCameraDeviceList ( deviceList );
          return ret;
        }
        deviceList->cameraList[ deviceList->numCameras++ ] = cam;
        numFound++;
      }
      udev_device_unref ( dev );
    }
  }
  udev_enumerate_unref ( enumerate );
  udev_unref ( udev );

  return numFound;
}
