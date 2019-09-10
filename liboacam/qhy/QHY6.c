/*****************************************************************************
 *
 * QHY6.c -- QHY6 camera interface
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
#include <openastro/errno.h>

#include <libusb-1.0/libusb.h>

#include "oacamprivate.h"
#include "QHY.h"
#include "QHYoacam.h"
#include "QHYstate.h"
#include "QHY6.h"
#include "QHYusb.h"


static void	_QHY6InitFunctionPointers ( oaCamera* );
static int      oaQHY6CameraGetFramePixelFormat ( oaCamera* );
static const FRAMESIZES* oaQHY6CameraGetFrameSizes ( oaCamera* );
static int      closeCamera ( oaCamera* );

/**
 * Initialise a given camera device
 */

int
_QHY6InitCamera ( oaCamera* camera )
{
  int		i, j;
  QHY_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;

  OA_CLEAR ( camera->controlType );
  OA_CLEAR ( camera->features );
  _QHY6InitFunctionPointers ( camera );

	// libqhyccd: min 0, max 63, step 1, def 0
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = 63;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = QHY6_DEFAULT_GAIN;

	// libqhyccd: min 1000, max 1800000000, step 1000, def 1 (clearly wrong)
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
			OA_CTRL_TYPE_INT64;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1800000000;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      QHY6_DEFAULT_EXPOSURE * 1000;

	// libqhyccd: min 0, max 1, step 1, def 1
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HIGHSPEED ) = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HIGHSPEED ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HIGHSPEED ) = 1;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HIGHSPEED ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) = QHY6_DEFAULT_SPEED;
  cameraInfo->currentHighSpeed = QHY6_DEFAULT_SPEED;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) = OA_CTRL_TYPE_DISCRETE;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED ) = OA_CTRL_TYPE_READONLY;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED_RESET ) = OA_CTRL_TYPE_BUTTON;

  cameraInfo->maxResolutionX = QHY6_SENSOR_WIDTH;
  cameraInfo->maxResolutionY = QHY6_SENSOR_HEIGHT;

  cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY16BE;
  camera->frameFormats[ OA_PIX_FMT_GREY16BE ] = 1;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

  if (!( cameraInfo->frameSizes[1].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    return -OA_ERR_MEM_ALLOC;
  }
  if (!( cameraInfo->frameSizes[2].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    return -OA_ERR_MEM_ALLOC;
  }
  cameraInfo->frameSizes[1].sizes[0].x = cameraInfo->maxResolutionX;
  cameraInfo->frameSizes[1].sizes[0].y = cameraInfo->maxResolutionY;
  cameraInfo->frameSizes[1].numSizes = 1;

  cameraInfo->binMode = cameraInfo->horizontalBinMode =
      cameraInfo->verticalBinMode = OA_BIN_MODE_NONE;

  cameraInfo->frameSizes[2].sizes[0].x = QHY6_SENSOR_WIDTH / 2;
  cameraInfo->frameSizes[2].sizes[0].y = QHY6_SENSOR_HEIGHT / 2;
  cameraInfo->frameSizes[2].numSizes = 1;

  cameraInfo->xSize = cameraInfo->maxResolutionX;
  cameraInfo->ySize = cameraInfo->maxResolutionY;
  cameraInfo->xOffset = QHY6_OFFSET_X;

  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;

  camera->features.flags |= OA_CAM_FEATURE_RESET;
  camera->features.pixelSizeX = 6500;
  camera->features.pixelSizeY = 6250;

  cameraInfo->topOffset = cameraInfo->bottomOffset = 0;

  cameraInfo->currentGain = commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN );
  cameraInfo->currentExposure = QHY6_DEFAULT_EXPOSURE;
  cameraInfo->correctedExposureTime = QHY6_DEFAULT_EXPOSURE -
      QHY6_DEFAULT_EXPOSURE / 10;

  // FIX ME -- need to add amp on/off/auto when I understand what it does
  cameraInfo->requestedAmpMode = QHY6_AMP_MODE_AUTO;
  if ( cameraInfo->correctedExposureTime > 550 &&
      QHY6_AMP_MODE_AUTO == cameraInfo->requestedAmpMode ) {
    cameraInfo->currentAmpMode = 1;
  } else {
    cameraInfo->currentAmpMode = cameraInfo->requestedAmpMode ? 0 : 1;
  }

  oaQHY6RecalculateSizes ( cameraInfo );
  if (!( cameraInfo->xferBuffer = malloc ( cameraInfo->captureLength ))) {
    fprintf ( stderr, "malloc of transfer buffer failed in %s\n",
        __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    return -OA_ERR_MEM_ALLOC;
  }

  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * 2;
  if (!( cameraInfo->buffers = calloc ( OA_CAM_BUFFERS,
      sizeof ( struct QHYbuffer )))) {
    fprintf ( stderr, "malloc of buffer array failed in %s\n",
        __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    free (( void* ) cameraInfo->xferBuffer );
    return -OA_ERR_MEM_ALLOC;
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
      free (( void* ) cameraInfo->xferBuffer );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      free (( void* ) cameraInfo->frameSizes[2].sizes );
      free (( void* ) cameraInfo->buffers );
      return -OA_ERR_MEM_ALLOC;
    }
  }

  cameraInfo->buffersFree = cameraInfo->configuredBuffers;
  cameraInfo->nextBuffer = 0;
  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamQHY6controller, ( void* ) camera )) {
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->xferBuffer );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamQHYcallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->xferBuffer );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return -OA_ERR_SYSTEM_ERROR;
  }

  return OA_ERR_NONE;
}


static int
oaQHY6CameraTestControl ( oaCamera* camera, int control, oaControlValue* val )
{
  COMMON_INFO*	commonInfo = camera->_common;

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->OA_CAM_CTRL_TYPE( control ) != val->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      if ( val->int32 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val->int32 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val->int32 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      if ( val->int64 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val->int64 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val->int64 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_BINNING:
      if ( OA_CTRL_TYPE_DISCRETE == val->valueType &&
          ( OA_BIN_MODE_NONE == val->discrete || OA_BIN_MODE_2x2 ==
          val->discrete )) {
        return OA_ERR_NONE;
      }
      break;

    default:
      fprintf ( stderr, "QHY6: %s not yet implemented for control %d\n",
          __FUNCTION__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  // And if we reach here it's because the value wasn't valid
  return -OA_ERR_OUT_OF_RANGE;
}


static int
oaQHY6CameraGetFramePixelFormat ( oaCamera* camera )
{
  QHY_STATE*	cameraInfo = camera->_private;

  return cameraInfo->currentFrameFormat;
}


const static FRAMESIZES*
oaQHY6CameraGetFrameSizes ( oaCamera* camera )
{
  QHY_STATE*    cameraInfo = camera->_private;

  if ( cameraInfo->binMode == OA_BIN_MODE_2x2 ) {
    return &cameraInfo->frameSizes[2];
  }
  return &cameraInfo->frameSizes[1];
}


static int
closeCamera ( oaCamera* camera )
{
  int		j;
  QHY_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;
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
  return 0;
}


static void
_QHY6InitFunctionPointers ( oaCamera* camera )
{
  // Set by QHYinit()
  // camera->funcs.initCamera = oaQHYInitCamera;
  camera->funcs.closeCamera = closeCamera;

  camera->funcs.setControl = oaQHYCameraSetControl;
  camera->funcs.readControl = oaQHYCameraReadControl;
  camera->funcs.testControl = oaQHY6CameraTestControl;
  camera->funcs.getControlRange = oaQHYCameraGetControlRange;

  camera->funcs.startStreaming = oaQHYCameraStartStreaming;
  camera->funcs.stopStreaming = oaQHYCameraStopStreaming;
  camera->funcs.isStreaming = oaQHYCameraIsStreaming;

  camera->funcs.setResolution = oaQHYCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;

  camera->funcs.enumerateFrameSizes = oaQHY6CameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaQHY6CameraGetFramePixelFormat;
}
