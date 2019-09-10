/*****************************************************************************
 *
 * QHY5.c -- QHY5 camera interface
 *
 * Copyright 2013,2014,2015,2017,2018,2019
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
#include <openastro/errno.h>

#include <libusb-1.0/libusb.h>

#include "oacamprivate.h"
#include "QHY.h"
#include "QHY5.h"
#include "QHYoacam.h"
#include "QHYstate.h"
#include "QHYusb.h"


static void			_QHY5InitFunctionPointers ( oaCamera* );
static int			oaQHY5CameraGetFramePixelFormat ( oaCamera* );
static const FRAMESIZES*	oaQHY5CameraGetFrameSizes ( oaCamera* );
static int      		oaQHY5CloseCamera ( oaCamera* );

/**
 * Initialise a given camera device
 */

int
_QHY5InitCamera ( oaCamera* camera )
{
  int		i, j;
  QHY_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;

  _QHY5InitFunctionPointers ( camera );

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = 1;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = 74;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = 10; // completely arbitrary

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  // I made it up
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 100000000;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      QHY5_DEFAULT_EXPOSURE * 1000;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED ) = OA_CTRL_TYPE_READONLY;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED_RESET ) = OA_CTRL_TYPE_BUTTON;

  cameraInfo->maxResolutionX = QHY5_IMAGE_WIDTH;
  cameraInfo->maxResolutionY = QHY5_IMAGE_HEIGHT;

  cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY8;
  camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

  if (!( cameraInfo->frameSizes[1].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    return -OA_ERR_MEM_ALLOC;
  }
  cameraInfo->frameSizes[1].sizes[0].x = cameraInfo->maxResolutionX;
  cameraInfo->frameSizes[1].sizes[0].y = cameraInfo->maxResolutionY;
  cameraInfo->frameSizes[1].numSizes = 1;

  cameraInfo->xSize = cameraInfo->maxResolutionX;
  cameraInfo->ySize = cameraInfo->maxResolutionY;
  cameraInfo->xOffset = QHY5_DARK_WIDTH_X;

  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;

  camera->features.flags |= OA_CAM_FEATURE_RESET;
  camera->features.flags |= OA_CAM_FEATURE_STREAMING;
  camera->features.pixelSizeX = 5200;
  camera->features.pixelSizeY = 5200;

  cameraInfo->captureLength = QHY5_SENSOR_WIDTH *
      ( QHY5_IMAGE_HEIGHT + QHY5_VBLANK );

  if (!( cameraInfo->xferBuffer = malloc ( cameraInfo->captureLength ))) {
    fprintf ( stderr, "malloc of transfer buffer failed in %s\n",
        __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    return -OA_ERR_MEM_ALLOC;
  }

  cameraInfo->currentGain = commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN );
  cameraInfo->currentExposure = QHY5_DEFAULT_EXPOSURE;

  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY;
  if (!( cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof (
      struct QHYbuffer )))) {
    fprintf ( stderr, "malloc of buffers failed in %s\n", __FUNCTION__ );
    free (( void* ) cameraInfo->xferBuffer );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    return -OA_ERR_MEM_ALLOC;
  }

  for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
    void* m = malloc ( cameraInfo->imageBufferLength );
    if ( m ) {
      cameraInfo->buffers[i].start = m;
      cameraInfo->configuredBuffers++;
    } else {
      fprintf ( stderr, "%s malloc for buffer failed\n", __FUNCTION__ );
      if ( i ) {
        for ( j = 0; j < i; j++ ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
      free ( cameraInfo->xferBuffer );
      free (( void* ) cameraInfo->buffers );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      return -OA_ERR_MEM_ALLOC;
    }
  }

  cameraInfo->buffersFree = cameraInfo->configuredBuffers;
  cameraInfo->nextBuffer = 0;
  cameraInfo->firstTimeSetup = 1;
  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamQHY5controller, ( void* ) camera )) {
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free ( cameraInfo->xferBuffer );
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
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free ( cameraInfo->xferBuffer );
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
oaQHY5CameraTestControl ( oaCamera* camera, int control, oaControlValue* val )
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
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      if ( val->int32 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val->int32 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val->int32 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_BINNING:
      return -OA_ERR_INVALID_CONTROL;
      break;

    default:
      fprintf ( stderr, "QHY5: %s not yet implemented for control %d\n",
          __FUNCTION__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  // And if we reach here it's because the value wasn't valid
  return -OA_ERR_OUT_OF_RANGE;
}


static int
oaQHY5CameraGetFramePixelFormat ( oaCamera* camera )
{
  QHY_STATE*	cameraInfo = camera->_private;

  return cameraInfo->currentFrameFormat;
}


const static FRAMESIZES*
oaQHY5CameraGetFrameSizes ( oaCamera* camera )
{
  QHY_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[1];
}


static int
oaQHY5CloseCamera ( oaCamera* camera )
{
  int		j;
  void*		dummy;
  QHY_STATE*	cameraInfo;

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

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

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


static void
_QHY5InitFunctionPointers ( oaCamera* camera )
{
  // Set by QHYinit()
  // camera->funcs.initCamera = oaQHYInitCamera;
  camera->funcs.closeCamera = oaQHY5CloseCamera;

  camera->funcs.setControl = oaQHYCameraSetControl;
  camera->funcs.readControl = oaQHYCameraReadControl;
  camera->funcs.testControl = oaQHY5CameraTestControl;
  camera->funcs.getControlRange = oaQHYCameraGetControlRange;

  camera->funcs.startStreaming = oaQHYCameraStartStreaming;
  camera->funcs.stopStreaming = oaQHYCameraStopStreaming;
  camera->funcs.isStreaming = oaQHYCameraIsStreaming;

  camera->funcs.setResolution = oaQHYCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;

  camera->funcs.enumerateFrameSizes = oaQHY5CameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaQHY5CameraGetFramePixelFormat;
}

