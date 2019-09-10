/*****************************************************************************
 *
 * QHY5II.c -- QHY5II camera interface
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
#include "QHYoacam.h"
#include "QHYstate.h"
#include "QHY5II.h"
#include "QHYusb.h"


static void		_QHY5IIInitFunctionPointers ( oaCamera* );
void*			_qhy5iiEventHandler ( void* );

static int		oaQHY5IICameraGetFramePixelFormat ( oaCamera* );
static const FRAMESIZES* oaQHY5IICameraGetFrameSizes ( oaCamera* );
static int		oaQHY5IICloseCamera ( oaCamera* );


/**
 * Initialise a given camera device
 */

int
_QHY5IIInitCamera ( oaCamera* camera )
{
  int		i, j;
  QHY_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;
  void		*dummy;

  oacamDebugMsg ( DEBUG_CAM_INIT, "QHY5-II: init: %s ()\n", __FUNCTION__ );

  _QHY5IIInitFunctionPointers ( camera );

  cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY8;
  camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

  cameraInfo->currentBitDepth = 8;
  cameraInfo->longExposureMode = 0;

  cameraInfo->maxResolutionX = QHY5II_IMAGE_WIDTH;
  cameraInfo->maxResolutionY = QHY5II_IMAGE_HEIGHT;

  if (!( cameraInfo->frameSizes[1].sizes =
      // ( FRAMESIZE* ) malloc ( 10 * sizeof ( FRAMESIZE )))) {
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    return -OA_ERR_MEM_ALLOC;
  }

  cameraInfo->frameSizes[1].sizes[0].x = cameraInfo->maxResolutionX;
  cameraInfo->frameSizes[1].sizes[0].y = cameraInfo->maxResolutionY;
  /*
   * If I could but get these to work...
  cameraInfo->frameSizes[1].sizes[1].x = 1280;
  cameraInfo->frameSizes[1].sizes[1].y = 720;
  cameraInfo->frameSizes[1].sizes[2].x = 1024;
  cameraInfo->frameSizes[1].sizes[2].y = 768;
  cameraInfo->frameSizes[1].sizes[3].x = 1024;
  cameraInfo->frameSizes[1].sizes[3].y = 1024;
  cameraInfo->frameSizes[1].sizes[4].x = 960;
  cameraInfo->frameSizes[1].sizes[4].y = 720;
  cameraInfo->frameSizes[1].sizes[5].x = 800;
  cameraInfo->frameSizes[1].sizes[5].y = 800;
  cameraInfo->frameSizes[1].sizes[6].x = 800;
  cameraInfo->frameSizes[1].sizes[6].y = 600;
  cameraInfo->frameSizes[1].sizes[7].x = 640;
  cameraInfo->frameSizes[1].sizes[7].y = 480;
  cameraInfo->frameSizes[1].sizes[8].x = 400;
  cameraInfo->frameSizes[1].sizes[8].y = 400;
  cameraInfo->frameSizes[1].sizes[9].x = 320;
  cameraInfo->frameSizes[1].sizes[9].y = 240;
  cameraInfo->frameSizes[1].numSizes = 10;
   */
  cameraInfo->frameSizes[1].numSizes = 1;

  cameraInfo->xSize = cameraInfo->maxResolutionX;
  cameraInfo->ySize = cameraInfo->maxResolutionY;

	// FIX ME -- According to libqhyccd, the camera supports brightness,
	// contrast, gamma, row noise reduction

	// libqhyccd: min 0, max 100, step 1, def 50
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = QHY5II_MONO_GAIN_MIN;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = QHY5II_MONO_GAIN_MAX;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = 50;
  cameraInfo->currentGain = 50;

	// libqhyccd: min 1, max 1800000000, step 1000, def 20000
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      OA_CTRL_TYPE_INT64;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1800000000;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  // convert msec to usec
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      QHY5II_DEFAULT_EXPOSURE * 1000;
  cameraInfo->currentExposure = QHY5II_DEFAULT_EXPOSURE;

	// libqhyccd: min 0, max 2, step 1, def 0
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HIGHSPEED ) = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HIGHSPEED ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HIGHSPEED ) = 2;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HIGHSPEED ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) = QHY5II_DEFAULT_SPEED;
  cameraInfo->currentHighSpeed = QHY5II_DEFAULT_SPEED;

	// libqhyccd: min 0, max 255, step 1, def 30
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_USBTRAFFIC ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_USBTRAFFIC ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_USBTRAFFIC ) = 255;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_USBTRAFFIC ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC ) =
      QHY5II_DEFAULT_USBTRAFFIC;
  cameraInfo->currentUSBTraffic = QHY5II_DEFAULT_USBTRAFFIC;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED ) = OA_CTRL_TYPE_READONLY;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED_RESET ) = OA_CTRL_TYPE_BUTTON;

  pthread_create ( &cameraInfo->eventHandler, 0, _qhy5iiEventHandler,
      ( void* ) cameraInfo );

  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;

  cameraInfo->frameSize = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY;
  cameraInfo->captureLength = cameraInfo->frameSize + QHY5II_EOF_LEN;
  cameraInfo->imageBufferLength = 2 * ( cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY ) + QHY5II_EOF_LEN;

  if (!( cameraInfo->buffers = calloc ( OA_CAM_BUFFERS,
      sizeof ( struct QHYbuffer )))) {
    fprintf ( stderr, "malloc of buffer array failed in %s\n",
        __FUNCTION__ );
    cameraInfo->stopCallbackThread = 1;
    pthread_join ( cameraInfo->eventHandler, &dummy );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
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
      cameraInfo->stopCallbackThread = 1;
      pthread_join ( cameraInfo->eventHandler, &dummy );
      free (( void* ) cameraInfo->buffers );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      return -OA_ERR_MEM_ALLOC;
    }
  }

  cameraInfo->buffersFree = cameraInfo->configuredBuffers;
  cameraInfo->nextBuffer = 0;
  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamQHY5IIcontroller, ( void* ) camera )) {
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    cameraInfo->stopCallbackThread = 1;
    pthread_join ( cameraInfo->eventHandler, &dummy );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
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
    cameraInfo->stopCallbackThread = 1;
    pthread_join ( cameraInfo->eventHandler, &dummy );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return -OA_ERR_SYSTEM_ERROR;
  }

  camera->features.flags |= OA_CAM_FEATURE_RESET;
  camera->features.flags |= OA_CAM_FEATURE_STREAMING;
  camera->features.pixelSizeX = 5200;
  camera->features.pixelSizeY = 5200;

  oaQHY5IISetAllControls ( cameraInfo );

  return OA_ERR_NONE;
}


const static FRAMESIZES*
oaQHY5IICameraGetFrameSizes ( oaCamera* camera )
{
  QHY_STATE*    cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[1];
}


static int
oaQHY5IICloseCamera ( oaCamera* camera )
{
  int		j, res;
  QHY_STATE*	cameraInfo;
  void*		dummy;

  oacamDebugMsg ( DEBUG_CAM_CMD, "QHY5-II: command: %s()\n",
      __FUNCTION__ );

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;

    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    if ( cameraInfo->statusTransfer ) {
      res = libusb_cancel_transfer ( cameraInfo->statusTransfer );
      if ( res < 0 && res != LIBUSB_ERROR_NOT_FOUND ) {
        free ( cameraInfo->statusBuffer );
        libusb_free_transfer ( cameraInfo->statusTransfer );
        cameraInfo->statusTransfer = 0;
      }
    }
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    pthread_join ( cameraInfo->eventHandler, &dummy );

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

    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo );
    free (( void* ) camera->_common );
    free (( void* ) camera );
  } else {
   return -OA_ERR_INVALID_CAMERA;
  }

  return 0;
}


void*
_qhy5iiEventHandler ( void* param )
{
  struct timeval        tv;
  QHY_STATE*            cameraInfo = param;
  int                   exitThread;

  tv.tv_sec = 1;
  tv.tv_usec = 0;
  do { 
    libusb_handle_events_timeout_completed ( cameraInfo->usbContext, &tv, 0 );
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    exitThread = cameraInfo->stopCallbackThread;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
  } while ( !exitThread );
  return 0;
}


static int
oaQHY5IICameraGetFramePixelFormat ( oaCamera* camera )
{
  QHY_STATE*	cameraInfo = camera->_private;

  return cameraInfo->currentFrameFormat;
}


static int
oaQHY5IICameraTestControl ( oaCamera* camera, int control,
    oaControlValue* valp )
{
  int32_t	val_s32;
  int64_t	val_s64;
  COMMON_INFO*	commonInfo = camera->_common;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5-II: control: %s ( %d, ? )\n",
      __FUNCTION__, control );

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->OA_CAM_CTRL_TYPE( control ) != valp->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
    case OA_CAM_CTRL_USBTRAFFIC:
      val_s32 = valp->int32;
      if ( val_s32 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val_s32 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val_s32 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val_s64 = valp->int64;
      if ( val_s64 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val_s64 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val_s64 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_HIGHSPEED:
      return OA_ERR_NONE;
      break;

    default:
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return -OA_ERR_OUT_OF_RANGE;
}


static void
_QHY5IIInitFunctionPointers ( oaCamera* camera )
{
  // Set by QHYinit()
  // camera->funcs.initCamera = oaQHYInitCamera;
  camera->funcs.closeCamera = oaQHY5IICloseCamera;

  camera->funcs.setControl = oaQHYCameraSetControl;
  camera->funcs.readControl = oaQHYCameraReadControl;
  camera->funcs.testControl = oaQHY5IICameraTestControl;
  camera->funcs.getControlRange = oaQHYCameraGetControlRange;

  camera->funcs.startStreaming = oaQHYCameraStartStreaming;
  camera->funcs.stopStreaming = oaQHYCameraStopStreaming;
  camera->funcs.isStreaming = oaQHYCameraIsStreaming;

  camera->funcs.setResolution = oaQHYCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;

  camera->funcs.enumerateFrameSizes = oaQHY5IICameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaQHY5IICameraGetFramePixelFormat;
}
