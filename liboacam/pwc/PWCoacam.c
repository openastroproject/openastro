/*****************************************************************************
 *
 * PWCoacam.c -- main entrypoint for non V4L2 PWC camera
 *
 * Copyright 2013,2014,2015,2016,2018
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

#include <libusb-1.0/libusb.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "PWCoacam.h"
#include "PWC.h"


/**
 * Cycle through the sys filesystem looking for USBdevices with the
 * appropriate vendor ID and product ID
 */

int
oaPWCGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  unsigned int				numFound = 0;
  unsigned int         		 	numUSBDevices, i;
  unsigned int  		        index;
  int					ret;
  libusb_context*      		 	ctx = 0;
  libusb_device**      		 	devlist;
  libusb_device*			device;
  struct libusb_device_descriptor	desc;
  char					manufacturer[ OA_MAX_NAME_LEN+1 ];
  char					product[ OA_MAX_NAME_LEN+1 ];
  char					fullname[ OA_MAX_NAME_LEN+1 ];
  oaCameraDevice*			dev;
  DEVICE_INFO*				_private;

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
    if ( VENDOR_ID == desc.idVendor && PRODUCT_ID == desc.idProduct ) {
      /*
       * This doesn't work without root access
       *
      if ( LIBUSB_SUCCESS != libusb_open ( device, &handle )) {
        libusb_free_device_list ( devlist, 1 );
        libusb_exit ( ctx );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if ( desc.iManufacturer > 0 ) {
        if ( LIBUSB_SUCCESS != libusb_get_string_descriptor_ascii (
            handle, desc.iManufacturer, manufacturer, OA_MAX_NAME_LEN )) {
          libusb_close ( handle );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_SYSTEM_ERROR;
        }
      } else {
        *manufacturer = 0;
      }
      if ( desc.iProduct > 0 ) {
        if ( LIBUSB_SUCCESS != libusb_get_string_descriptor_ascii (
            handle, desc.iProduct, product, OA_MAX_NAME_LEN )) {
          libusb_close ( handle );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_SYSTEM_ERROR;
        }
      } else {
        *product = 0;
      }
      libusb_close ( handle );
       */
      ( void ) strcpy ( manufacturer, "Philips" );
      ( void ) strcpy ( product, "SPC900NC" );
      index = libusb_get_bus_number ( device ) << 8;
      index |= libusb_get_device_address ( device );

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
      dev->interface = OA_CAM_IF_PWC;
      ( void ) snprintf ( fullname, OA_MAX_NAME_LEN, "%s %s", manufacturer,
          product );
      ( void ) strcpy ( dev->deviceName, fullname );
      _private->devIndex = index;
      dev->initCamera = oaPWCInitCamera;
      dev->_private = _private;
      if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
        ( void ) free (( void* ) dev );
        ( void ) free (( void* ) _private );
        libusb_free_device_list ( devlist, 1 );
        libusb_exit ( ctx );
        return ret;
      }
      numFound++;
    }
  }

  libusb_free_device_list ( devlist, 1 );
  libusb_exit ( ctx );

  return numFound;
}


/*
int
oaPWCCloseCamera ( oaCamera* camera )
{
  return -OA_ERR_UNIMPLEMENTED;
}
*/
