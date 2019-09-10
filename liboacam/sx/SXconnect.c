/*****************************************************************************
 *
 * SXconnect.c -- Initialise Starlight Xpress cameras
 *
 * Copyright 2014,2015,2017,2018,2019
 *   James Fidell (james@openastroproject.org)
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


static void _SXInitFunctionPointers ( oaCamera* );


/**
 * Initialise a given camera device
 */

oaCamera*
oaSXInitCamera ( oaCameraDevice* device )
{
  oaCamera*				camera;
  int                   		i, j, matched, ret, transferred;
  int					deviceAddr, deviceBus, numUSBDevices;
  libusb_device**			devlist;
  libusb_device*			usbDevice;
  libusb_device_handle*			usbHandle = 0;
  struct libusb_device_descriptor	desc;
  unsigned char				buff[ SXUSB_GET_CCD_BUFSIZE ];
  uint32_t				extraCaps, cameraModel;
  DEVICE_INFO*				devInfo;
  SX_STATE*				cameraInfo;
  COMMON_INFO*				commonInfo;

  devInfo = device->_private;
  switch ( devInfo->devType ) {
    case CAM_LODESTAR:
    case CAM_COSTAR:
      break;
    default:
      fprintf ( stderr, "Unsupported camera %ld: %s\n", devInfo->devType,
          device->deviceName );
      return 0;
      break;
  }

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( SX_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  _SXInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  cameraInfo->index = -1;

  // FIX ME -- This is a bit ugly.  Much of it is repeated from the
  // getCameras function.  I should join the two together somehow.

  libusb_init ( &cameraInfo->usbContext );
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
    if ( desc.idVendor == SXCameraList[ devInfo->misc ].vendorId &&
        desc.idProduct == SXCameraList[ devInfo->misc ].productId &&
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

  if ( libusb_kernel_driver_active ( usbHandle, 1 )) {
    libusb_detach_kernel_driver( usbHandle, 1 );
  }

  if ( libusb_claim_interface ( usbHandle, 1 )) {
    fprintf ( stderr, "Unable to claim interface for USB device!\n" );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  OA_CLEAR( buff );
  buff[ SXUSB_REQ_CMD_TYPE ] = SXUSB_CMD_REQUEST;
  buff[ SXUSB_REQ_CMD ] = SXUSB_GET_CCD;
  buff[ SXUSB_REQ_LENGTH_L ] = SXUSB_GET_CCD_BUFSIZE;
  if (( ret = libusb_bulk_transfer ( usbHandle, SXUSB_BULK_ENDP_OUT, buff,
      SXUSB_REQUEST_BUFSIZE, &transferred, SXUSB_TIMEOUT ))) {
    fprintf ( stderr, "request GET_CCD for SX failed: %d\n", ret );
    libusb_release_interface ( usbHandle, 1 );
    libusb_close ( usbHandle );
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if (( ret = libusb_bulk_transfer ( usbHandle, SXUSB_BULK_ENDP_IN, buff,
      SXUSB_GET_CCD_BUFSIZE, &transferred, SXUSB_TIMEOUT ))) {
    fprintf ( stderr, "request GET_CCD for SX failed: %d\n", ret );
    libusb_release_interface ( usbHandle, 1 );
    libusb_close ( usbHandle );
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  cameraInfo->horizontalFrontPorch = buff[0];
  cameraInfo->horizontalBackPorch = buff[1];
  cameraInfo->maxResolutionX = buff[2] | ( buff[3] << 8 );
  cameraInfo->verticalFrontPorch = buff[4];
  cameraInfo->verticalBackPorch = buff[5];
  cameraInfo->maxResolutionY = buff[6] | ( buff[7] << 8 );
  cameraInfo->colourMatrix = buff[12] | ( buff[13] << 8 );
  cameraInfo->bitDepth = buff[14];
  extraCaps = buff[16];

  OA_CLEAR( buff );
  buff[ SXUSB_REQ_CMD_TYPE ] = SXUSB_CMD_REQUEST;
  buff[ SXUSB_REQ_CMD ] = SXUSB_CAMERA_MODEL;
  buff[ SXUSB_REQ_LENGTH_L ] = SXUSB_CAMERA_MODEL_BUFSIZE;
  if (( ret = libusb_bulk_transfer ( usbHandle, SXUSB_BULK_ENDP_OUT, buff,
      SXUSB_REQUEST_BUFSIZE, &transferred, SXUSB_TIMEOUT ))) {
    fprintf ( stderr, "request GET_CCD for SX failed: %d\n", ret );
    libusb_release_interface ( usbHandle, 1 );
    libusb_close ( usbHandle );
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if (( ret = libusb_bulk_transfer ( usbHandle, SXUSB_BULK_ENDP_IN, buff,
      SXUSB_CAMERA_MODEL_BUFSIZE, &transferred, SXUSB_TIMEOUT ))) {
    fprintf ( stderr, "request GET_CCD for SX failed: %d\n", ret );
    libusb_release_interface ( usbHandle, 1 );
    libusb_close ( usbHandle );
    libusb_free_device_list ( devlist, 1 );
    libusb_exit ( cameraInfo->usbContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  cameraModel = buff[0] | ( buff[1] << 8 );
  cameraInfo->isInterlaced = ( cameraModel & SX_MODEL_MASK_INTERLACE ) ? 1 : 0;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
			OA_CTRL_TYPE_INT64;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
			0xffffffff * 1000;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
			SX_DEFAULT_EXPOSURE * 1000;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED ) = OA_CTRL_TYPE_READONLY;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED_RESET ) = OA_CTRL_TYPE_BUTTON;

  if ( extraCaps & SXUSB_CAPS_COOLER ) {
    // These are just made up as I have no documentation
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMP_SETPOINT ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TEMP_SETPOINT ) = -320;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TEMP_SETPOINT ) = 10000;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TEMP_SETPOINT ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) = 0;

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) = OA_CTRL_TYPE_READONLY;
  }

  camera->interface = device->interface;
  cameraInfo->usbHandle = usbHandle;
  cameraInfo->index = devInfo->devIndex;
  cameraInfo->cameraType = devInfo->devType;
  cameraInfo->droppedFrames = 0;
  cameraInfo->isColour = devInfo->colour;

  if ( cameraInfo->bitDepth <= 8 ) {
    cameraInfo->bytesPerPixel = 1;
    if ( cameraInfo->isColour ) {
      cameraInfo->currentFrameFormat = OA_PIX_FMT_BGGR8;
      camera->frameFormats[ OA_PIX_FMT_BGGR8 ] = 1;
    } else {
      cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY8;
      camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
    }
  } else {
    cameraInfo->bytesPerPixel = 2;
    if ( cameraInfo->isColour ) {
      cameraInfo->currentFrameFormat = OA_PIX_FMT_BGGR16LE;
      camera->frameFormats[ OA_PIX_FMT_BGGR16LE ] = 1;
    } else {
      cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY16LE;
      camera->frameFormats[ OA_PIX_FMT_GREY16LE ] = 1;
    }
  }

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) = OA_CTRL_TYPE_DISCRETE;
  cameraInfo->binMode = OA_BIN_MODE_NONE;

  camera->features.flags |= OA_CAM_FEATURE_ROI;
  camera->features.flags |= OA_CAM_FEATURE_RESET;
  switch ( devInfo->devType ) {
    case CAM_LODESTAR:
      camera->features.pixelSizeX = 8200;
      camera->features.pixelSizeY = 8400;
			camera->features.flags |= OA_CAM_FEATURE_STREAMING;
      break;
    case CAM_LODESTAR_C:
      camera->features.pixelSizeX = 8600;
      camera->features.pixelSizeY = 8300;
			camera->features.flags |= OA_CAM_FEATURE_STREAMING;
			break;
    case CAM_COSTAR:
      camera->features.pixelSizeX = 5200;
      camera->features.pixelSizeY = 5200;
			camera->features.flags |= OA_CAM_FEATURE_STREAMING;
      break;
  }

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  // If the camera is interlaced it reports half the number of lines for
  // the full image, so multiply by two
  if ( cameraInfo->isInterlaced ) {
    cameraInfo->maxResolutionY *= 2;
  }

  if (!( cameraInfo->frameSizes[1].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if (!( cameraInfo->frameSizes[2].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    FREE_DATA_STRUCTS;
    return 0;
  }

  cameraInfo->frameSizes[1].sizes[0].x = cameraInfo->maxResolutionX;
  cameraInfo->frameSizes[1].sizes[0].y = cameraInfo->maxResolutionY;
  cameraInfo->frameSizes[1].numSizes = 1;

  cameraInfo->frameSizes[2].sizes[0].x = cameraInfo->maxResolutionX / 2;
  cameraInfo->frameSizes[2].sizes[0].y = cameraInfo->maxResolutionY / 2;
  cameraInfo->frameSizes[2].numSizes = 1;

  cameraInfo->xSubframeSize = cameraInfo->xImageSize =
			cameraInfo->maxResolutionX;
  cameraInfo->ySubframeSize = cameraInfo->yImageSize =
			cameraInfo->maxResolutionY;


  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;
  cameraInfo->actualImageLength = cameraInfo->imageBufferLength =
			cameraInfo->maxResolutionX * cameraInfo->maxResolutionY *
			cameraInfo->bytesPerPixel;
  if (!( cameraInfo->xferBuffer = malloc ( cameraInfo->imageBufferLength ))) {
    fprintf ( stderr, "malloc of transfer buffer failed in %s\n",
        __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if (!( cameraInfo->buffers = calloc ( OA_CAM_BUFFERS,
      sizeof ( struct SXbuffer )))) {
    fprintf ( stderr, "malloc of buffer array failed in %s\n",
        __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    free (( void* ) cameraInfo->xferBuffer );
    FREE_DATA_STRUCTS;
    return 0;
  }

  for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
    void* m = malloc ( cameraInfo->imageBufferLength );
    if ( m ) {
      cameraInfo->buffers[i].start = m;
      cameraInfo->configuredBuffers++;
    } else {
      fprintf ( stderr, "%s malloc failed\n", __FUNCTION__ );
      if ( i ) {
        for ( j = 0; j < i; j++ ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      free (( void* ) cameraInfo->frameSizes[2].sizes );
      free (( void* ) cameraInfo->xferBuffer );
      free (( void* ) cameraInfo->buffers );
      FREE_DATA_STRUCTS;
      return 0;
    }
  }

  cameraInfo->buffersFree = cameraInfo->configuredBuffers;
  cameraInfo->currentExposure = SX_DEFAULT_EXPOSURE * 1000;

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamSXcontroller, ( void* ) camera )) {
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->xferBuffer );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamSXcallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->xferBuffer );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  _SXsetTimer ( cameraInfo, SX_DEFAULT_EXPOSURE );

  return camera;
}


static void
_SXInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaSXInitCamera;
  camera->funcs.closeCamera = oaSXCloseCamera;

  camera->funcs.setControl = oaSXCameraSetControl;
  camera->funcs.readControl = oaSXCameraReadControl;
  camera->funcs.testControl = oaSXCameraTestControl;
  camera->funcs.getControlRange = oaSXCameraGetControlRange;

  camera->funcs.startStreaming = oaSXCameraStartStreaming;
  camera->funcs.stopStreaming = oaSXCameraStopStreaming;
  camera->funcs.isStreaming = oaSXCameraIsStreaming;

  camera->funcs.setResolution = oaSXCameraSetResolution;
  camera->funcs.setROI = oaSXCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaSXCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaSXCameraGetFramePixelFormat;
	camera->funcs.testROISize = oaSXCameraTestROISize;
}


int
oaSXCloseCamera ( oaCamera* camera )
{
  int		j;
  void*		dummy;
  SX_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    libusb_release_interface ( cameraInfo->usbHandle, 0 );
    libusb_close ( cameraInfo->usbHandle );
    libusb_exit ( cameraInfo->usbContext );

    if ( cameraInfo->buffers ) {
      for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
        if ( cameraInfo->buffers[j].start ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    free (( void* ) cameraInfo->xferBuffer );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo );
    free (( void* ) camera->_common );
    free (( void* ) camera );
  } else {
    return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
