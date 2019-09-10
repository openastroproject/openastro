/*****************************************************************************
 *
 * dummyconnect.c -- Initialise dummy cameras
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/video/formats.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "dummyoacam.h"
#include "dummystate.h"


static void _dummyInitFunctionPointers ( oaCamera* );

/**
 * Initialise a given camera device
 */

oaCamera*
oaDummyInitCamera ( oaCameraDevice* device )
{
  oaCamera*		camera;
  DEVICE_INFO*		devInfo;
  DUMMY_STATE*		cameraInfo;
  COMMON_INFO*		commonInfo;
  int          		i, j, multiplier;

  oacamDebugMsg ( DEBUG_CAM_INIT, "dummy: init: %s ()\n", __FUNCTION__ );

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( DUMMY_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  cameraInfo->index = -1;
  devInfo = device->_private;

  camera->interface = device->interface;
  cameraInfo->index = devInfo->devIndex;
  cameraInfo->cameraType = devInfo->devType;

  OA_CLEAR ( camera->controlType );
  OA_CLEAR ( camera->features );
  
  _dummyInitFunctionPointers ( camera );

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = 100;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) =
			cameraInfo->currentGain = 50;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      OA_CTRL_TYPE_INT64;
	// this is in microseconds
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
			cameraInfo->cameraType ? 300000000 : 30000000;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
			cameraInfo->currentAbsoluteExposure =
			cameraInfo->cameraType ? 1000000 : 10000;

	// Skip gamma for the moment
	/*
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAMMA ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA ) = 0
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA ) = 100;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAMMA ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) =
			cameraInfo->currentGamma = 50;
	 */

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BRIGHTNESS ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BRIGHTNESS ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BRIGHTNESS ) = 32;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BRIGHTNESS ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) =
			cameraInfo->currentBrightness = 0;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HFLIP ) = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HFLIP ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HFLIP ) = 1;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HFLIP ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HFLIP ) =
      cameraInfo->currentHFlip = 0;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_VFLIP ) = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_VFLIP ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_VFLIP ) = 1;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_VFLIP ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_VFLIP ) =
      cameraInfo->currentVFlip = 0;

  // might be handy to have this one later?
	/*
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER ) = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER ) = 1;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) = 
      cameraInfo->coolerEnabled = currentValue = 0;
	 */

  // and this one
	/*
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMP_SETPOINT ) = OA_CTRL_TYPE_INT32;
	// 1/10ths of a degree
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TEMP_SETPOINT ) = -3000;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TEMP_SETPOINT ) = 100;
        controlCaps.MaxValue;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TEMP_SETPOINT ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) =
			cameraInfo->currentSetPoint = 0;
	 */

	camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) = OA_CTRL_TYPE_DISCRETE;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) = OA_CTRL_TYPE_READONLY;

  camera->features.flags |= OA_CAM_FEATURE_ROI;
  camera->features.flags |= OA_CAM_FEATURE_RESET;
  camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;
  camera->features.flags |= OA_CAM_FEATURE_FIXED_FRAME_SIZES;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;
  cameraInfo->binMode = OA_BIN_MODE_NONE;

  for ( i = 1; i <= OA_MAX_BINNING; i++ ) {
    cameraInfo->frameSizes[i].numSizes = 0;
    cameraInfo->frameSizes[i].sizes = 0;
  }

	switch ( cameraInfo->cameraType ) {
		case 0:  // planetary
      camera->frameFormats[ OA_PIX_FMT_GRBG8 ] = 1;
			camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
			cameraInfo->maxResolutionX = 1280;
			cameraInfo->maxResolutionY = 960;
			camera->features.flags |= OA_CAM_FEATURE_STREAMING;
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          18, sizeof ( FRAMESIZE )))) {
        fprintf ( stderr, "%s: calloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
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
      cameraInfo->frameSizes[1].sizes[0].x = 1280;
      cameraInfo->frameSizes[1].sizes[0].y = 960;
      cameraInfo->frameSizes[1].sizes[1].x = 1280;
      cameraInfo->frameSizes[1].sizes[1].y = 720;
      cameraInfo->frameSizes[1].sizes[2].x = 1280;
      cameraInfo->frameSizes[1].sizes[2].y = 600;
      cameraInfo->frameSizes[1].sizes[3].x = 1280;
      cameraInfo->frameSizes[1].sizes[3].y = 400;
      cameraInfo->frameSizes[1].sizes[4].x = 960;
      cameraInfo->frameSizes[1].sizes[4].y = 960;
      cameraInfo->frameSizes[1].sizes[5].x = 1024;
      cameraInfo->frameSizes[1].sizes[5].y = 768;
      cameraInfo->frameSizes[1].sizes[6].x = 1024;
      cameraInfo->frameSizes[1].sizes[6].y = 600;
      cameraInfo->frameSizes[1].sizes[7].x = 1024;
      cameraInfo->frameSizes[1].sizes[7].y = 400;
      cameraInfo->frameSizes[1].sizes[8].x = 800;
      cameraInfo->frameSizes[1].sizes[8].y = 800;
      cameraInfo->frameSizes[1].sizes[9].x = 800;
      cameraInfo->frameSizes[1].sizes[9].y = 640;
      cameraInfo->frameSizes[1].sizes[10].x = 800;
      cameraInfo->frameSizes[1].sizes[10].y = 512;
      cameraInfo->frameSizes[1].sizes[11].x = 800;
      cameraInfo->frameSizes[1].sizes[11].y = 320;
      cameraInfo->frameSizes[1].sizes[12].x = 640;
      cameraInfo->frameSizes[1].sizes[12].y = 560;
      cameraInfo->frameSizes[1].sizes[13].x = 640;
      cameraInfo->frameSizes[1].sizes[13].y = 480;
      cameraInfo->frameSizes[1].sizes[14].x = 512;
      cameraInfo->frameSizes[1].sizes[14].y = 440;
      cameraInfo->frameSizes[1].sizes[15].x = 512;
      cameraInfo->frameSizes[1].sizes[15].y = 400;
      cameraInfo->frameSizes[1].sizes[16].x = 480;
      cameraInfo->frameSizes[1].sizes[16].y = 320;
      cameraInfo->frameSizes[1].sizes[17].x = 320;
      cameraInfo->frameSizes[1].sizes[17].y = 240;
      cameraInfo->frameSizes[1].numSizes = 18;

      cameraInfo->frameSizes[2].sizes[0].x = 640;
      cameraInfo->frameSizes[2].sizes[0].y = 480;
      cameraInfo->frameSizes[2].numSizes = 1;

      camera->features.pixelSizeX = 3750;
      camera->features.pixelSizeY = 3750;
			break;

		case 1:  // DSO
      camera->frameFormats[ OA_PIX_FMT_GREY16BE ] = 16;
			camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
			cameraInfo->maxResolutionX = 4656;
			cameraInfo->maxResolutionY = 3250;
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          6, sizeof ( FRAMESIZE )))) {
        fprintf ( stderr, "%s: calloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
        FREE_DATA_STRUCTS;
        return 0;
      }

      cameraInfo->frameSizes[1].sizes[0].x = 4656;
      cameraInfo->frameSizes[1].sizes[0].y = 3520;
      cameraInfo->frameSizes[1].sizes[1].x = 3840;
      cameraInfo->frameSizes[1].sizes[1].y = 2160;
      cameraInfo->frameSizes[1].sizes[2].x = 1920;
      cameraInfo->frameSizes[1].sizes[2].y = 1680;
      cameraInfo->frameSizes[1].sizes[3].x = 1280;
      cameraInfo->frameSizes[1].sizes[3].y = 960;
      cameraInfo->frameSizes[1].sizes[4].x = 640;
      cameraInfo->frameSizes[1].sizes[4].y = 480;
      cameraInfo->frameSizes[1].sizes[5].x = 320;
      cameraInfo->frameSizes[1].sizes[5].y = 240;
      cameraInfo->frameSizes[1].numSizes = 6;

      cameraInfo->frameSizes[2].sizes[0].x = 2328;
      cameraInfo->frameSizes[2].sizes[0].y = 1760;
      cameraInfo->frameSizes[2].numSizes = 1;

      camera->features.pixelSizeX = 3800;
      camera->features.pixelSizeY = 3800;
			break;
  }

  cameraInfo->xSize = cameraInfo->maxResolutionX;
  cameraInfo->ySize = cameraInfo->maxResolutionY;
  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;

  multiplier = cameraInfo->cameraType ? 2 : 1;
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * multiplier;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( struct dummyBuffer ));
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
      for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
        if ( cameraInfo->frameSizes[j].sizes ) {
          free (( void* ) cameraInfo->frameSizes[j].sizes );
        }
      }
      FREE_DATA_STRUCTS;
      return 0;
    }
  }
  cameraInfo->nextBuffer = 0;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamDummyController, ( void* ) camera )) {
    for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
      free (( void* ) cameraInfo->buffers[i].start );
    }
    for ( i = 1; i <= OA_MAX_BINNING; i++ ) {
      if ( cameraInfo->frameSizes[i].sizes )
        free (( void* ) cameraInfo->frameSizes[i].sizes );
    }
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamDummyCallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
      free (( void* ) cameraInfo->buffers[i].start );
    }
    for ( i = 1; i <= OA_MAX_BINNING; i++ ) {
      if ( cameraInfo->frameSizes[i].sizes )
        free (( void* ) cameraInfo->frameSizes[i].sizes );
    }
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  return camera;
}


static void
_dummyInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaDummyInitCamera;
  camera->funcs.closeCamera = oaDummyCloseCamera;

  camera->funcs.testControl = oaDummyCameraTestControl;
  camera->funcs.readControl = oaDummyCameraReadControl;
  camera->funcs.setControl = oaDummyCameraSetControl;
  camera->funcs.getControlRange = oaDummyCameraGetControlRange;

  camera->funcs.startStreaming = oaDummyCameraStartStreaming;
  camera->funcs.stopStreaming = oaDummyCameraStopStreaming;
  camera->funcs.isStreaming = oaDummyCameraIsStreaming;

  // camera->funcs.setResolution = oaDummyCameraSetResolution;
  // camera->funcs.setROI = oaDummyCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  //  camera->funcs.isAuto = oaIsAuto;

  camera->funcs.enumerateFrameSizes = oaDummyCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaDummyCameraGetFramePixelFormat;
  camera->funcs.testROISize = oaDummyCameraTestROISize;
}


int
oaDummyCloseCamera ( oaCamera* camera )
{
  int		j;
  void*		dummy;
  DUMMY_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    if ( cameraInfo->buffers ) {
      for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
        if ( cameraInfo->buffers[j].start ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
    }
    for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
      if ( cameraInfo->frameSizes[j].sizes )
        free (( void* ) cameraInfo->frameSizes[j].sizes );
    }

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo );
    free (( void* ) camera->_common );
    free (( void* ) camera );

  } else {
   return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
