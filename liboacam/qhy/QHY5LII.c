/*****************************************************************************
 *
 * QHY5LII.c -- QHY5LII camera interface
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
#include "QHY5LII.h"
#include "QHYusb.h"


static void		_QHY5LIIInitFunctionPointers ( oaCamera* );
void*			_qhy5liiEventHandler ( void* );

static int		oaQHY5LIICameraGetFramePixelFormat ( oaCamera* );
static const FRAMESIZES* oaQHY5LIICameraGetFrameSizes ( oaCamera* );
static int		oaQHY5LIICloseCamera ( oaCamera* );


/**
 * Initialise a given camera device
 */

int
_QHY5LIIInitCamera ( oaCamera* camera )
{
  int		i, j;
  unsigned char	buf[4];
  QHY_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;
  void*		dummy;

  oacamDebugMsg ( DEBUG_CAM_INIT, "QHY5L-II: init: %s ()\n", __FUNCTION__ );

  _QHY5LIIInitFunctionPointers ( camera );

  if ( cameraInfo->isColour ) {
    cameraInfo->currentFrameFormat = OA_PIX_FMT_GRBG8;
    camera->frameFormats [ OA_PIX_FMT_GRBG8 ] = 1;
    camera->frameFormats [ OA_PIX_FMT_GRBG16BE ] = 1;
  } else {
    cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY8;
    camera->frameFormats [ OA_PIX_FMT_GREY8 ] = 1;
    camera->frameFormats [ OA_PIX_FMT_GREY16BE ] = 1;
  }
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

  cameraInfo->currentBitDepth = 8;
  cameraInfo->longExposureMode = 0;

  cameraInfo->maxResolutionX = QHY5LII_IMAGE_WIDTH;
  cameraInfo->maxResolutionY = QHY5LII_IMAGE_HEIGHT;

  if (!( cameraInfo->frameSizes[1].sizes =
      ( FRAMESIZE* ) malloc ( 5 * sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    return -OA_ERR_MEM_ALLOC;
  }

  cameraInfo->frameSizes[1].sizes[0].x = cameraInfo->maxResolutionX;
  cameraInfo->frameSizes[1].sizes[0].y = cameraInfo->maxResolutionY;
  cameraInfo->frameSizes[1].sizes[1].x = 1024;
  cameraInfo->frameSizes[1].sizes[1].y = 768;
  cameraInfo->frameSizes[1].sizes[2].x = 800;
  cameraInfo->frameSizes[1].sizes[2].y = 600;
  cameraInfo->frameSizes[1].sizes[3].x = 640;
  cameraInfo->frameSizes[1].sizes[3].y = 480;
  cameraInfo->frameSizes[1].sizes[4].x = 320;
  cameraInfo->frameSizes[1].sizes[4].y = 240;
  cameraInfo->frameSizes[1].numSizes = 5;

  cameraInfo->xSize = cameraInfo->maxResolutionX;
  cameraInfo->ySize = cameraInfo->maxResolutionY;

  memset ( buf, 0, 4 );
  _usbControlMsg ( cameraInfo, QHY_CMD_DEFAULT_OUT, 0xc1, 0, 0, buf, 4,
      USB2_TIMEOUT );

	// FIX ME -- according to libqhyccd this camera also supports
	// brightness, contrast and gamma controls

	// libqhyccd says gain is 1 to 29 step 1 default 1
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = 1;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = 1000;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = 1;
  cameraInfo->currentGain = 1;

	// libqhyccd: min 1, max 1800000000, step 1, def 20000
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = OA_CTRL_TYPE_INT64;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1800000000;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
  // convert msec to usec
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      QHY5LII_DEFAULT_EXPOSURE * 1000;
  cameraInfo->currentExposure = QHY5LII_DEFAULT_EXPOSURE;

	// libqhyccd: min 0, max 2, step 1, def 0
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HIGHSPEED ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HIGHSPEED ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HIGHSPEED ) = 1;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HIGHSPEED ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) = QHY5LII_DEFAULT_SPEED;
  cameraInfo->currentHighSpeed = QHY5LII_DEFAULT_SPEED;

	// libqhyccd: min 0, max 255, step 1, def 30
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_USBTRAFFIC ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_USBTRAFFIC ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_USBTRAFFIC ) = 255;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_USBTRAFFIC ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC ) =
			QHY5LII_DEFAULT_USBTRAFFIC;
  cameraInfo->currentUSBTraffic = QHY5LII_DEFAULT_USBTRAFFIC;

	// this doesn't appear in libqhyccd
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HDR ) = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HDR ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HDR ) = 1;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HDR ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HDR ) = 0;
  cameraInfo->currentHDR = 0;

  if ( cameraInfo->isColour ) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_RED_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE ) = 0;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE ) = 200;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_RED_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) = 100; // guess
    cameraInfo->currentRedBalance = 100;

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE ) = 0;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE ) = 200;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) = 100; // guess
    cameraInfo->currentBlueBalance = 100;

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GREEN_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GREEN_BALANCE ) = 0;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GREEN_BALANCE ) = 200;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GREEN_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GREEN_BALANCE ) = 100; // guess
    cameraInfo->currentGreenBalance = 100;
  }

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) = OA_CTRL_TYPE_READONLY;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED ) = OA_CTRL_TYPE_READONLY;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED_RESET ) = OA_CTRL_TYPE_BUTTON;

  pthread_create ( &cameraInfo->eventHandler, 0, _qhy5liiEventHandler,
      ( void* ) cameraInfo );

  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;

  cameraInfo->frameSize = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY;
  cameraInfo->captureLength = cameraInfo->frameSize + QHY5LII_EOF_LEN;
  cameraInfo->imageBufferLength = 2 * ( cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY ) + QHY5LII_EOF_LEN;

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
      oacamQHY5LIIcontroller, ( void* ) camera )) {
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

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    pthread_join ( cameraInfo->eventHandler, &dummy );
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
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
  camera->features.pixelSizeX = 3750;
  camera->features.pixelSizeY = 3750;

  oaQHY5LIISetAllControls ( camera );

  return OA_ERR_NONE;
}


const static FRAMESIZES*
oaQHY5LIICameraGetFrameSizes ( oaCamera* camera )
{
  QHY_STATE*    cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[1];
}


static int
oaQHY5LIICloseCamera ( oaCamera* camera )
{
  int		j, res;
  QHY_STATE*	cameraInfo;
  void*		dummy;

  oacamDebugMsg ( DEBUG_CAM_CMD, "QHY5L-II: command: %s()\n",
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
_qhy5liiEventHandler ( void* param )
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
oaQHY5LIICameraGetFramePixelFormat ( oaCamera* camera )
{
  QHY_STATE*	cameraInfo = camera->_private;

  return cameraInfo->currentFrameFormat;
}


static int
oaQHY5LIICameraTestControl ( oaCamera* camera, int control,
    oaControlValue* valp )
{
  int32_t	val_s32;
  int64_t	val_s64;
  COMMON_INFO*	commonInfo = camera->_common;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d, ? )\n",
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
    case OA_CAM_CTRL_RED_BALANCE:
    case OA_CAM_CTRL_BLUE_BALANCE:
    case OA_CAM_CTRL_GREEN_BALANCE:
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

    case OA_CAM_CTRL_HDR:
      fprintf ( stderr, "QHY5L-II: %s not yet implemented for control %d\n",
          __FUNCTION__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;

    case OA_CAM_CTRL_TEMPERATURE:
    default:
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return -OA_ERR_OUT_OF_RANGE;
}


static void
_QHY5LIIInitFunctionPointers ( oaCamera* camera )
{
  // Set by QHYinit()
  // camera->funcs.initCamera = oaQHYInitCamera;
  camera->funcs.closeCamera = oaQHY5LIICloseCamera;

  camera->funcs.setControl = oaQHYCameraSetControl;
  camera->funcs.readControl = oaQHYCameraReadControl;
  camera->funcs.testControl = oaQHY5LIICameraTestControl;
  camera->funcs.getControlRange = oaQHYCameraGetControlRange;

  camera->funcs.startStreaming = oaQHYCameraStartStreaming;
  camera->funcs.stopStreaming = oaQHYCameraStopStreaming;
  camera->funcs.isStreaming = oaQHYCameraIsStreaming;

  camera->funcs.setResolution = oaQHYCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;

  camera->funcs.enumerateFrameSizes = oaQHY5LIICameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaQHY5LIICameraGetFramePixelFormat;
}
