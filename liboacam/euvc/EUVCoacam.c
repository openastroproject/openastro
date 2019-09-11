/*****************************************************************************
 *
 * EUVCoacam.c -- main entrypoint for TIS EUVC cameras
 *
 * Copyright 2015,2016,2018 James Fidell (james@openastroproject.org)
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
#include "EUVC.h"
#include "EUVCstate.h"
#include "EUVCoacam.h"

struct euvccam EUVCCameraList[] =
{
  { 0x8201, "unknown", EUVC_CAM_UNKNOWN }, // unknown by me, that is
  { 0x8202, "DFK22",   EUVC_CAM_DFK22 },
  { 0x8203, "DFK61",   EUVC_CAM_DFK61 },
  { 0x8204, "DFK41",   EUVC_CAM_DFK41 },
  { 0x8205, "DFx51",   EUVC_CAM_DFx51 },
  { 0x8206, "DFx41",   EUVC_CAM_DFx41 },
  { 0x8207, "DFK72",   EUVC_CAM_DFK72 },
  { 0x8208, "DFK42",   EUVC_CAM_DFK42 }
};


/**
 * Cycle through the sys filesystem looking for USBdevices with one
 * of the appropriate vendor ID and product ID
 */

int
oaEUVCGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  unsigned int				numFound = 0;
  unsigned int         		 	numUSBDevices, i, j;
  unsigned int  		        index;
  int					ret;
  libusb_context*      		 	ctx = 0;
  libusb_device**      		 	devlist;
  libusb_device*			device;
  libusb_device_handle*			handle;
  unsigned char				manufacturer[ OA_MAX_NAME_LEN+1 ];
  unsigned char				product[ OA_MAX_NAME_LEN+1 ];
  char					fullname[ OA_MAX_NAME_LEN+1 ];
  struct libusb_device_descriptor	desc;
  oaCameraDevice*			dev;
  unsigned int				matched, colour;
  unsigned int				numCameras;
  unsigned short			busNum, addr;
  DEVICE_INFO*				_private;

  numCameras = sizeof ( EUVCCameraList ) / sizeof ( struct euvccam );

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
      if ( TIS_VENDOR_ID == desc.idVendor &&
          desc.idProduct == EUVCCameraList[j].productId ) {
        if ( LIBUSB_SUCCESS != libusb_open ( device, &handle )) {
          fprintf ( stderr, "libusb_open for EUVC camera failed\n" );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if ( desc.iManufacturer > 0 ) {
          if ( libusb_get_string_descriptor_ascii ( handle,
              desc.iManufacturer, manufacturer, OA_MAX_NAME_LEN ) < 1 ) {
            libusb_close ( handle );
            libusb_free_device_list ( devlist, 1 );
            libusb_exit ( ctx );
            return -OA_ERR_SYSTEM_ERROR;
          }
        } else {
          *manufacturer = 0;
        }
        if ( desc.iProduct > 0 ) {
          if ( libusb_get_string_descriptor_ascii ( handle, desc.iProduct,
              product, OA_MAX_NAME_LEN ) < 1 ) {
            libusb_close ( handle );
            libusb_free_device_list ( devlist, 1 );
            libusb_exit ( ctx );
            return -OA_ERR_SYSTEM_ERROR;
          }
        } else {
          *product = 0;
        }
        libusb_close ( handle );
        ( void ) snprintf ( fullname, OA_MAX_NAME_LEN, "%s %s", manufacturer,
            product );

        // now we can drop the data into the list
        if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_MEM_ALLOC;
        }
        if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
          ( void ) free (( void* ) dev );
          return -OA_ERR_MEM_ALLOC;
        }

        _oaInitCameraDeviceFunctionPointers ( dev );
        dev->interface = OA_CAM_IF_EUVC;
        ( void ) strcpy ( dev->deviceName, fullname );
        _private->devIndex = index;
        _private->devType = EUVCCameraList[j].devType;
        _private->vendorId = TIS_VENDOR_ID;
        _private->productId = EUVCCameraList[j].productId;
        dev->_private = _private;
        dev->initCamera = oaEUVCInitCamera;
        dev->hasLoadableFirmware = 0;
        // store the device data here so we can use it later
        _private->misc = j;
        if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
          ( void ) free (( void* ) dev );
          ( void ) free (( void* ) _private );
          return ret;
        }
        deviceList->cameraList[ deviceList->numCameras++ ] = dev;
        numFound++;
        matched = 1;
      }
    }
  }

  libusb_free_device_list ( devlist, 1 );
  libusb_exit ( ctx );

  return numFound;
}
