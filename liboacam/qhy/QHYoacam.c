/*****************************************************************************
 *
 * QHYoacam.c -- main entrypoint for QHY Cameras
 *
 * Copyright 2013,2014,2015,2016,2017 James Fidell (james@openastroproject.org)
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

#include "oacamprivate.h"
#include "unimplemented.h"
#include "QHY.h"
#include "QHYoacam.h"
#include "QHYstate.h"
#include "QHYusb.h"
#include "QHYfirmware.h"

struct qhycam cameraList[] =
{
  { 0x04b4, 0x8613, "QHY5", CAM_QHY5, 1, 0, 1, -1, 0, 0 },
  { 0x0547, 0x1002, "QHY5", CAM_QHY5, 1, 0, 1, -1, 0, 0 },
  { 0x16c0, 0x081a, "QHY5", CAM_QHY5, 1, 0, 1, -1, 0, 0 },
  { 0x16c0, 0x081e, "QHY2", CAM_QHY2, 1, 1, 0, -1, 0, 0 },
  { 0x16c0, 0x081d, "QHY6", CAM_QHY6, 1, 1, 0, -1, 0, 0 },
  { 0x16c0, 0x296d, "QHY5", CAM_QHY5, 1, 1, 1, -1, 0, 0 },
  { 0x16c0, 0x2972, "QHY8", CAM_QHY8, 1, 1, 0, -1, 0, 0 },
  { 0x16c0, 0x2981, "QHY6Pro", CAM_QHY6Pro, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x0005, "IMG132E", CAM_IMG132E, 1, 0, 1, -1, 0, 0 },
  { 0x1618, 0x0259, "QHY6+ST4", CAM_QHY6_ST4, 1, 0, 1, -1, 0, 0 },
  { 0x1618, 0x025a, "QHY6+ST4", CAM_QHY6_ST4, 1, 1, 1, -1, 0, 0 },
  { 0x1618, 0x0901, "QHY5", CAM_QHY5, 1, 0, 1, -1, 0, 0 },
  { 0x1618, 0x0910, "QHY5T", CAM_QHY5T, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x0920, "QHY5II/QHY5x-II series", CAM_QHY5II, 1, 0, 1, -1, 0, 0 },
  { 0x1618, 0x0921, "QHY5-II", CAM_QHY5II, 1, 1, 1, 0x10, 1, -1 },
  { 0x1618, 0x0921, "QHY5L-IIm", CAM_QHY5LIIM, 1, 1, 1, 0x10, 6, 0 },
  { 0x1618, 0x0921, "QHY5L-IIc", CAM_QHY5LIIC, 1, 1, 1, 0x10, 6, 1 },
  { 0x1618, 0x0921, "QHY5P-IIm", CAM_QHY5PIIM, 1, 1, 0, 0x10, 5, 0 },
  { 0x1618, 0x0921, "QHY5P-IIc", CAM_QHY5PIIC, 1, 1, 0, 0x10, 5, 1 },
  { 0x1618, 0x0921, "QHY5T-IIc", CAM_QHY5TIIC, 1, 1, 0, 0x10, 2, 1 },
  { 0x1618, 0x0921, "QHY5R-IIc", CAM_QHY5RIIC, 1, 1, 0, 0x10, 9, -1 },
  { 0x1618, 0x0921, "QHY5H-II", CAM_QHY5HII, 1, 1, 0, 0x10, 22, -1 },
  { 0x1618, 0x0931, "QHY5V", CAM_QHY5V, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0xa285, "IMG2S", CAM_IMG2S, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x1001, "QHY10", CAM_QHY10, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x1002, "QHY5", CAM_QHY5, 1, 0, 1, -1, 0, 0 },
  { 0x1618, 0x1111, "QHY11", CAM_QHY11, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x1601, "QHY16", CAM_QHY16, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x1601, "QHY16000", CAM_QHY16000, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x2851, "IMG2Pro", CAM_IMG2Pro, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x2859, "IMG2E", CAM_IMG2E, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x4023, "QHY7", CAM_QHY7, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x6005, "QHY8L", CAM_QHY8L, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x6007, "QHY8M", CAM_QHY8M, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x6741, "QHY21", CAM_QHY21, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x6669, "QHY50", CAM_QHY50, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x666A, "IMG132E", CAM_IMG132E, 1, 1, 1, -1, 0, 0 },
  { 0x1618, 0x6941, "QHY22", CAM_QHY22, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x8141, "QHY23", CAM_QHY23, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x8051, "QHY20", CAM_QHY20, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x8301, "QHY9", CAM_QHY9, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0x8311, "QHY9L", CAM_QHY9L, 1, 1, 0, -1, 0, 0 },
  { 0x1618, 0xb618, "IMG0H", CAM_IMG0H, 1, 1, 0, -1, 0, 0 },
  { 0x1856, 0x0011, "QHY5", CAM_QHY5, 1, 0, 1, -1, 0, 0 }
// Still missing?
// IMG0S, IMG0L, IMG0X, IMG1S, IMG3S, IMG5S, IMG1E, IMG3T
// QHY5T-II, QHY5R-II, QHY5R, QHY5P-II, QHY5V-II
// QHY12, QHY15
};


/**
 * Cycle through the sys filesystem looking for USBdevices with one
 * of the appropriate vendor ID and product ID
 */

int
oaQHYGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  unsigned int				numFound = 0;
  unsigned int         		 	numUSBDevices, i, j;
  unsigned int  		        index;
  libusb_context*      		 	ctx = 0;
  libusb_device**      		 	devlist;
  libusb_device*			device;
  libusb_device_handle*			handle;
  struct libusb_device_descriptor	desc;
  oaCameraDevice*			dev;
  DEVICE_INFO*				_private;
  unsigned int				matched, colour;
  unsigned int				numCameras;
  unsigned short			busNum, addr;
  int					lastWindex = -1;
  unsigned char				buf[16];

  numCameras = sizeof ( cameraList ) / sizeof ( struct qhycam );

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
    matched = colour = 0;
    for ( j = 0; j < numCameras && !matched; j++ ) {
      if ( desc.idVendor == cameraList[j].vendorId &&
          desc.idProduct == cameraList[j].productId ) {
        int ret;
        if ( cameraList[j].wIndex >= 0 ) {
          // Need to check in more detail to find out the camera subtype
          // eg. for QHY5-II models
          if ( cameraList[j].wIndex != lastWindex ) {
            lastWindex = cameraList[j].wIndex;
            if ( LIBUSB_SUCCESS != libusb_open ( device, &handle )) {
              fprintf ( stderr, "libusb_open for QHY camera failed\n" );
              libusb_free_device_list ( devlist, 1 );
              libusb_exit ( ctx );
              return -OA_ERR_SYSTEM_ERROR;
            }
            // here be magic numbers
            if (( ret = libusb_control_transfer ( handle,
                QHY_CMD_DEFAULT_IN, 0xca, 0, cameraList[j].wIndex, buf, 16,
                2000 )) < 16 ) {
              fprintf ( stderr, "read QHY EEPROM failed: %d\n", ret );
              libusb_close ( handle );
              libusb_free_device_list ( devlist, 1 );
              libusb_exit ( ctx );
              return -OA_ERR_SYSTEM_ERROR;
            }
            libusb_close ( handle );
          }
          if ( cameraList[j].subtype != buf[0] || ( cameraList[j].colour >= 0 &&
              buf[1] != cameraList[j].colour )) {
            continue;
          }
          // FIX ME -- not sure this is true for QHY5H-II and QHY5R-IIc
          if ( 1 == buf[1] ) {
            colour = 1;
          }
        }

        // now we can drop the data into the list
        if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_MEM_ALLOC;
        }
        if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
          ( void ) free (( void* ) dev );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_MEM_ALLOC;
        }

        _oaInitCameraDeviceFunctionPointers ( dev );
        dev->interface = OA_CAM_IF_QHY;
        _private->devIndex = index;
        ( void ) strcpy ( dev->deviceName, cameraList[j].name );
        _private->devType = cameraList[j].devType;
        _private->vendorId = cameraList[j].vendorId;
        _private->productId = cameraList[j].productId;
        dev->_private = _private;
        dev->initCamera = oaQHYInitCamera;
        dev->hasLoadableFirmware = 1;
        dev->firmwareLoaded = cameraList[j].firmwareLoaded;
        dev->loadFirmware = oaQHYCameraDeviceLoadFirmware;
        // store the device data here so we can use it later
        _private->misc = j;
        _private->colour = colour;

        if ( dev->hasLoadableFirmware && dev->firmwareLoaded ) {
          if ( !cameraList[j].supported ) {
            ( void ) strcat ( dev->deviceName, " (unsupported)" );
          }
        } else {
          if ( dev->hasLoadableFirmware && !dev->firmwareLoaded &&
              cameraList[j].supported ) {
            ( void ) strcat ( dev->deviceName, " (no firmware)" );
          } else {
            ( void ) strcat ( dev->deviceName, " (unsupported)" );
          }
        }
        if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
          ( void ) free (( void* ) dev );
          ( void ) free (( void* ) _private );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return ret;
        }
        deviceList->cameraList[ deviceList->numCameras++ ] = dev;
        numFound++;
        matched = 1;
        // skip the QHY5LII if we just found a QHY5II
        // if ( CAM_QHY5II == dev->_devType ) { j++; }
      }
    }
  }

  libusb_free_device_list ( devlist, 1 );
  libusb_exit ( ctx );

  return numFound;
}
