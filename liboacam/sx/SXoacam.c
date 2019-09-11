/*****************************************************************************
 *
 * SXoacam.c -- main entrypoint for Starlight Xpress Cameras
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
#include <openastro/util.h>

#include <libusb-1.0/libusb.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "SX.h"
#include "SXstate.h"
#include "SXoacam.h"

struct sxcam SXCameraList[] =
{
  { 0x1278, 0x0105, "SXVF-M5", CAM_SXVF_M5 },
  { 0x1278, 0x0305, "SXVF-M5C", CAM_SXVF_M5C },
  { 0x1278, 0x0107, "SXVF-M7", CAM_SXVF_M7 },
  { 0x1278, 0x0307, "SXVF-M7C", CAM_SXVF_M7C },
  { 0x1278, 0x0308, "SXVF-M8C", CAM_SXVF_M8C },
  { 0x1278, 0x0109, "SXVF-M9", CAM_SXVF_M9 },
  { 0x1278, 0x0325, "SXVR-M25C", CAM_SXVR_M25C },
  { 0x1278, 0x0326, "SXVR-M26C", CAM_SXVR_M26C },
  { 0x1278, 0x0128, "SXVR-H18", CAM_SXVR_H18 },
  { 0x1278, 0x0126, "SXVR-H16", CAM_SXVR_H16 },
  { 0x1278, 0x0135, "SXVR-H35", CAM_SXVR_H35 },
  { 0x1278, 0x0136, "SXVR-H36", CAM_SXVR_H36 },
  { 0x1278, 0x0119, "SXVR-H9", CAM_SXVR_H9 },
  { 0x1278, 0x0319, "SXVR-H9C", CAM_SXVR_H9C },
  { 0x1278, 0x0100, "SXVR-H9", CAM_SXVR_H9 },
  { 0x1278, 0x0100, "SXVR-H9C", CAM_SXVR_H9C },
  { 0x1278, 0x0507, "Lodestar", CAM_LODESTAR },
  { 0x1278, 0x0507, "Lodestar-C", CAM_LODESTAR_C },
  { 0x1278, 0x0517, "Costar", CAM_COSTAR },
  { 0x1278, 0x0509, "Superstar", CAM_SUPERSTAR },
  { 0x1278, 0x0200, "MX Camera", CAM_MX }
};


/**
 * Cycle through the sys filesystem looking for USBdevices with one
 * of the appropriate vendor ID and product ID
 */

int
oaSXGetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  unsigned int				numFound = 0;
  unsigned int         		 	numUSBDevices, i, j;
  unsigned int  		        index, cameraModel;
  int					transferred, ret;
  libusb_context*      		 	ctx = 0;
  libusb_device**      		 	devlist;
  libusb_device*			device;
  libusb_device_handle*			handle;
  struct libusb_device_descriptor	desc;
  oaCameraDevice*			dev;
  unsigned int				matched, colour;
  unsigned int				numCameras;
  unsigned short			busNum, addr;
  DEVICE_INFO*				_private;

  numCameras = sizeof ( SXCameraList ) / sizeof ( struct sxcam );

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
      if ( desc.idVendor == SXCameraList[j].vendorId &&
          desc.idProduct == SXCameraList[j].productId ) {
        // We check for the device being a colour camera here because
        // then it's possible to describe a Lodestar correctly as either
        // mono or colour.  Both cameras share the same VID/PID unlike
        // most of the other mono/colour versions of the same camera

        unsigned char buff[ SXUSB_REQUEST_BUFSIZE ];

        if ( LIBUSB_SUCCESS != libusb_open ( device, &handle )) {
          fprintf ( stderr, "libusb_open for SX camera failed\n" );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if ( libusb_kernel_driver_active ( handle, 1 )) {
          libusb_detach_kernel_driver( handle, 1 );
        }

        if (( ret = libusb_claim_interface ( handle, 1 ))) {
          fprintf ( stderr, "Unable to claim interface for USB device: %d\n",
              ret );
          libusb_exit ( ctx );
          return 0;
        }

        OA_CLEAR( buff );
        buff[ SXUSB_REQ_CMD_TYPE ] = SXUSB_CMD_REQUEST;
        buff[ SXUSB_REQ_CMD ] = SXUSB_CAMERA_MODEL;
        buff[ SXUSB_REQ_LENGTH_L ] = SXUSB_CAMERA_MODEL_BUFSIZE;
        if (( ret = libusb_bulk_transfer ( handle, SXUSB_BULK_ENDP_OUT,
            buff, SXUSB_REQUEST_BUFSIZE, &transferred, SXUSB_TIMEOUT ))) {
          fprintf ( stderr, "request MODEL for SX failed: %d\n", ret );
          libusb_release_interface ( handle, 1 );
          libusb_close ( handle );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_SYSTEM_ERROR;
        }
        if (( ret = libusb_bulk_transfer ( handle, SXUSB_BULK_ENDP_IN, buff,
            SXUSB_CAMERA_MODEL_BUFSIZE, &transferred, SXUSB_TIMEOUT ))) {
          fprintf ( stderr, "response MODEL for SX failed: %d\n", ret );
          libusb_release_interface ( handle, 1 );
          libusb_close ( handle );
          libusb_free_device_list ( devlist, 1 );
          libusb_exit ( ctx );
          return -OA_ERR_SYSTEM_ERROR;
        }
        libusb_release_interface ( handle, 1 );
        libusb_close ( handle );

        cameraModel = buff[0] | ( buff[1] << 8 );
        colour = cameraModel & SX_MODEL_MASK_COLOUR;

        // skip to the next entry for filling out the device details if
        // this is a colour Lodestar
        if ( colour && ( CAM_LODESTAR == SXCameraList[j].devType )) {
          j++;
        }

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
        dev->interface = OA_CAM_IF_SX;
        ( void ) strcpy ( dev->deviceName, SXCameraList[j].name );
        _private->devIndex = index;
        _private->devType = SXCameraList[j].devType;
        _private->vendorId = SXCameraList[j].vendorId;
        _private->productId = SXCameraList[j].productId;
        dev->_private = _private;
        dev->initCamera = oaSXInitCamera;
        dev->hasLoadableFirmware = 0;
        // store the device data here so we can use it later
        _private->misc = j;
        _private->colour = colour;
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
