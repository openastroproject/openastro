/*****************************************************************************
 *
 * connect.c -- Initialise Touptek-based cameras
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

#include <openastro/camera.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "touptekprivate.h"
#include "touptekoacam.h"
#include "touptekstate.h"

// From the Touptek docs
#ifndef MAKEFOURCC
#define MAKEFOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24))
#endif


static void TT_FUNC( _, InitFunctionPointers ) ( oaCamera* );

/**
 * Initialise a given camera device
 */

oaCamera*
TT_FUNC( oa, InitCamera ) ( oaCameraDevice* device )
{
  oaCamera*			camera;
  TOUPTEK_STATE*		cameraInfo;
  COMMON_INFO*			commonInfo;
  TT_VAR_TYPE( InstV2 )			devList[ TT_DEFINE( MAX )];
  unsigned int			numCameras, min, max, def;
  unsigned short		smin, smax, sdef;
  TT_HANDLE					handle;
  DEVICE_INFO*			devInfo;
  unsigned int			i, j, numResolutions, numStillResolutions;
  unsigned int			fourcc, depth, binX, binY;
  int				x, y;
  char				toupcamId[128]; // must be longer than 64
	void*				tmpPtr;

  numCameras = ( TT_LIB_PTR( EnumV2 ))( devList );
  devInfo = device->_private;
  if ( numCameras < 1 || devInfo->devIndex > numCameras ) {
    return 0;
  }

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( TOUPTEK_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  TT_FUNC( _, InitFunctionPointers )( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;

  camera->interface = device->interface;
  cameraInfo->colour = ( devList[ devInfo->devIndex ].model->flag &
      TT_FLAG( MONO )) ? 0 : 1;

  if ( cameraInfo->colour ) {
    // Add "@" to use "RGB gain mode".  Ick :(
    ( void ) strcpy ( toupcamId, "@" );
  } else {
    *toupcamId = 0;
  }
  ( void ) strcat ( toupcamId, devInfo->deviceId );
  if (!( handle = ( TT_LIB_PTR( Open ))( toupcamId ))) {
    fprintf ( stderr, "Can't get " TT_DRIVER "handle\n" );
    FREE_DATA_STRUCTS;
    return 0;
  }

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

	camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;
	camera->features.flags |= OA_CAM_FEATURE_STREAMING;
	camera->features.flags |= OA_CAM_FEATURE_SINGLE_SHOT;

  // FIX ME -- work out how to support these
  // ..._put_AutoExpoTarget
  // ..._put_MaxAutoExpoTimeAGain
  // ..._AwbOnePush
  // ..._AwbInit
  // ..._put_Chrome
  // ..._put_Negative
  // ..._put_HZ
  // ..._put_Mode
  // ..._put_AWBAuxRect
  // ..._put_AEAuxRect
  // ..._get_MonoMode
  // ..._put_RealTime
  // ..._put_LevelRange
  // ..._put_TempTint

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_CONTRAST ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_CONTRAST ) =
			TT_DEFINE( CONTRAST_MIN );
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_CONTRAST ) =
			TT_DEFINE( CONTRAST_MAX );
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_CONTRAST ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_CONTRAST ) =
			TT_DEFINE( CONTRAST_DEF );

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAMMA ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA ) =
			TT_DEFINE( GAMMA_MIN );
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA ) =
			TT_DEFINE( GAMMA_MAX );
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAMMA ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) =
			TT_DEFINE( GAMMA_DEF );

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HFLIP ) = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HFLIP ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HFLIP ) = 1;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HFLIP ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HFLIP ) = 0;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_VFLIP ) = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_VFLIP ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_VFLIP ) = 1;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_VFLIP ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_VFLIP ) = 0;

  camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
      OA_CTRL_TYPE_BOOLEAN;
  commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 0;
  commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
  commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
  commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 0;

  if (( TT_LIB_PTR( get_ExpTimeRange ))( handle, &min, &max, &def ) < 0 ) {
    ( TT_LIB_PTR( Close ))( handle );
    FREE_DATA_STRUCTS;
    return 0;
  }

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
			OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = min;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = max;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = def;
  // make these easy to find in the controller loop
  cameraInfo->exposureMin = min;
  cameraInfo->exposureMax = max;

  if (( TT_LIB_PTR( get_ExpoAGainRange ))( handle, &smin, &smax, &sdef ) < 0 ) {
    fprintf ( stderr, TT_DRIVER "_get_ExpoAGainRange() failed\n" );
    ( TT_LIB_PTR( Close ))( handle );
    FREE_DATA_STRUCTS;
    return 0;
  }

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = smin;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = smax;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = sdef;
  // make these easy to find in the controller loop
  cameraInfo->gainMin = smin;
  cameraInfo->gainMax = smax;

  // make this easy to find in the controller loop
  cameraInfo->speedMax = devList[ devInfo->devIndex ].model->maxspeed;
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SPEED ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SPEED ) = 0;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SPEED ) = cameraInfo->speedMax;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SPEED ) = 1;
  // this is a wild guess
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SPEED ) = cameraInfo->speedMax;

  if ( devList[ devInfo->devIndex ].model->flag &
      TT_FLAG( PUTTEMPERATURE )) {
    fprintf ( stderr, TT_DRIVER " supports setting temperature, but we "
        "don't know how to get the range\n" );
    /*
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMP_SETPOINT ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TEMP_SETPOINT ) = min;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TEMP_SETPOINT ) = max;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TEMP_SETPOINT ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) = def;
     */
  }

  if ( devList[ devInfo->devIndex ].model->flag &
      TT_FLAG( GETTEMPERATURE )) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) = OA_CTRL_TYPE_READONLY;
  }

  if ( devList[ devInfo->devIndex ].model->flag & TT_FLAG( TEC_ONOFF )) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER ) = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER ) = 0;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER ) = 1;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) = 0;
  }

  if ( devList[ devInfo->devIndex ].model->flag & TT_FLAG( FAN )) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FAN ) = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_FAN ) = 0;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_FAN ) = 1;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_FAN ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN ) = 0;
  }

  if ( cameraInfo->colour ) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HUE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HUE ) = TT_DEFINE( HUE_MIN );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HUE ) = TT_DEFINE( HUE_MAX );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HUE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HUE ) = TT_DEFINE( HUE_DEF );

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SATURATION ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SATURATION ) =
				TT_DEFINE( SATURATION_MIN );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SATURATION ) =
				TT_DEFINE( SATURATION_MAX );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SATURATION ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SATURATION ) =
				TT_DEFINE( SATURATION_DEF );

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_RED_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE ) =
				TT_DEFINE( WBGAIN_MIN );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE ) =
				TT_DEFINE( WBGAIN_MAX );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_RED_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) =
				TT_DEFINE( WBGAIN_DEF );

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GREEN_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GREEN_BALANCE ) =
				TT_DEFINE( WBGAIN_MIN );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GREEN_BALANCE ) =
				TT_DEFINE( WBGAIN_MAX );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GREEN_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GREEN_BALANCE ) =
				TT_DEFINE( WBGAIN_DEF );

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE ) =
				TT_DEFINE( WBGAIN_MIN );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE ) =
				TT_DEFINE( WBGAIN_MAX );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) =
				TT_DEFINE( WBGAIN_DEF );

    // I don't see why this should be colour only, but it does appear to be
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BRIGHTNESS ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BRIGHTNESS ) =
				TT_DEFINE( BRIGHTNESS_MIN );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BRIGHTNESS ) =
				TT_DEFINE( BRIGHTNESS_MAX );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BRIGHTNESS ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) =
				TT_DEFINE( BRIGHTNESS_DEF );

    // force the camera out of raw mode
    if ((( TT_LIB_PTR( put_Option ))( handle, TT_OPTION( RAW ), 0 )) < 0 ) {
      fprintf ( stderr, TT_DRIVER "_put_Option ( raw, 0 ) returns error\n" );
      ( TT_LIB_PTR( Close ))( handle );
      FREE_DATA_STRUCTS;
      return 0;
    }

  } else {

    // It looks like mono cameras return RGB frames by default.  That
    // seems wasteful, so try to turn it off.

    if ((( TT_LIB_PTR( put_Option ))( handle, TT_OPTION( RAW ), 1 )) < 0 ) {
      fprintf ( stderr, TT_DRIVER "_put_Option ( raw, 1 ) returns error\n" );
      ( TT_LIB_PTR( Close ))( handle );
      FREE_DATA_STRUCTS;
      return 0;
    }
  }

  // There doesn't appear to be a way to tell if any of these cameras
  // have LEDs or not, so assume there's just one in all cases

/*
 * Commented out because I can't find a camera this does actually work on
 *
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_LED_STATE ) = OA_CTRL_TYPE_DISC_MENU;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_LED_STATE ) = 1;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_LED_STATE ) = 3;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_LED_STATE ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_LED_STATE ) =
      cameraInfo->ledState = 2;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_LED_PERIOD ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_LED_PERIOD ) = 500;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_LED_PERIOD ) = 0xffff;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_LED_PERIOD ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_LED_PERIOD ) =
      cameraInfo->ledState = 500;
*/

  if ( devList[ devInfo->devIndex ].model->flag & TT_FLAG( ROI_HARDWARE )) {
		camera->features.flags |= OA_CAM_FEATURE_ROI;
  }

  cameraInfo->maxBitDepth = TT_LIB_PTR( get_MaxBitDepth )( handle );
  if ( cameraInfo->colour ) {
    camera->frameFormats[ OA_PIX_FMT_RGB24 ] = 1;
  } else {
    camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
  }

  // According to the documentation I have there are only two options for
  // setting bit depth: 8-bit or maximum depth (which we already know), so
  // I'm not sure what use these flags are.
  // For the time being I'll try to do some sort of sanity check here

  // FIX ME -- This looks to be broken for colour cameras.  In testing I
  // still only see a 24-bit colour frame.  For now I'm disabling it for
  // colour cameras.
  if ( !cameraInfo->colour ) {
  if ( cameraInfo->maxBitDepth > 8 ) {
    if ( devList[ devInfo->devIndex ].model->flag & TT_FLAG( BITDEPTH10 )) {
      if ( 10 == cameraInfo->maxBitDepth ) {
        camera->frameFormats[ cameraInfo->colour ? OA_PIX_FMT_RGB30LE :
            OA_PIX_FMT_GREY10_16LE ] = 1;
      } else {
        fprintf ( stderr, "Camera claims 10-bit is available, but only %d"
            "-bit is available\n", cameraInfo->maxBitDepth );
      }
    }
    if ( devList[ devInfo->devIndex ].model->flag & TT_FLAG( BITDEPTH12 )) {
      if ( 12 == cameraInfo->maxBitDepth ) {
        camera->frameFormats[ cameraInfo->colour ? OA_PIX_FMT_RGB36LE :
            OA_PIX_FMT_GREY12_16LE ] = 1;
      } else {
        fprintf ( stderr, "Camera claims 12-bit is available, but only %d"
            "-bit is available\n", cameraInfo->maxBitDepth );
      }
    }
    if ( devList[ devInfo->devIndex ].model->flag & TT_FLAG( BITDEPTH14 )) {
      if ( 14 == cameraInfo->maxBitDepth ) {
        camera->frameFormats[ cameraInfo->colour ? OA_PIX_FMT_RGB42LE :
            OA_PIX_FMT_GREY14_16LE ] = 1;
      } else {
        fprintf ( stderr, "Camera claims 14-bit is available, but only %d"
            "-bit is available\n", cameraInfo->maxBitDepth );
      }
    }
    if ( devList[ devInfo->devIndex ].model->flag & TT_FLAG( BITDEPTH16 )) {
      if ( 16 == cameraInfo->maxBitDepth ) {
        camera->frameFormats[ cameraInfo->colour ? OA_PIX_FMT_RGB48LE :
            OA_PIX_FMT_GREY16LE ] = 1;
      } else {
        fprintf ( stderr, "Camera claims 16-bit is available, but only %d"
            "-bit is available\n", cameraInfo->maxBitDepth );
      }
    }
  }
  }

  // force camera into 8-bit mode

  if ( cameraInfo->maxBitDepth > 8 ) {
    if ((( TT_LIB_PTR( put_Option ))( handle,
        TT_OPTION( BITDEPTH ), 0 )) < 0 ) {
      fprintf ( stderr, TT_DRIVER
          "_put_Option ( bitdepth, 0 ) returns error\n" );
      ( TT_LIB_PTR( Close ))( handle );
      FREE_DATA_STRUCTS;
      return 0;
    }
  }

  // FIX ME -- this may not be right
  cameraInfo->currentBitsPerPixel = 8;

  if ( devList[ devInfo->devIndex ].model->flag &
      TT_FLAG( BINSKIP_SUPPORTED )) {
    fprintf ( stderr, "bin/skip mode supported but not handled\n" );
  }

  cameraInfo->currentBytesPerPixel = cameraInfo->maxBytesPerPixel =
      cameraInfo->colour ? 3 : 1;

  if ( cameraInfo->maxBitDepth > 8 ) {
    if ( cameraInfo->colour ) {
      cameraInfo->maxBytesPerPixel = 6; // RGB48
    } else {
      cameraInfo->maxBytesPerPixel = 2;
    }
  }

  if ( cameraInfo->colour ) {
    int found = 0;

    if ((( TT_LIB_PTR( get_RawFormat ))( handle, &fourcc, &depth )) < 0 ) {
      fprintf ( stderr, "get_RawFormat returns error\n" );
      ( TT_LIB_PTR( Close ))( handle );
      FREE_DATA_STRUCTS;
      return 0;
    }

    // The docs aren't clear, so I'm assuming that raw mode is available for
    // all colour cameras
		camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
		camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
    cameraInfo->currentVideoFormat = OA_PIX_FMT_RGB24;

    // Some weird stuff appears to be going on here.  When I enable raw
    // mode, the image flips vertically from its non-raw version.  That
    // has the effect of changing the claimed raw image format, so we need
    // to account for that here.

    if (( MAKEFOURCC('G', 'B', 'R', 'G')) == fourcc ) {
      camera->frameFormats[ OA_PIX_FMT_GRBG8 ] = 1;
      if ( cameraInfo->maxBitDepth == 10 ) {
        camera->frameFormats[ OA_PIX_FMT_GRBG10_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 12 ) {
        camera->frameFormats[ OA_PIX_FMT_GRBG12_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 14 ) {
        camera->frameFormats[ OA_PIX_FMT_GRBG14_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 16 ) {
        camera->frameFormats[ OA_PIX_FMT_GRBG16LE ] = 1;
      }
      found = 1;
    }
    if (( MAKEFOURCC('G', 'R', 'B', 'G')) == fourcc ) {
      camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
      if ( cameraInfo->maxBitDepth == 10 ) {
        camera->frameFormats[ OA_PIX_FMT_GBRG10_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 12 ) {
        camera->frameFormats[ OA_PIX_FMT_GBRG12_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 14 ) {
        camera->frameFormats[ OA_PIX_FMT_GBRG14_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 16 ) {
        camera->frameFormats[ OA_PIX_FMT_GBRG16LE ] = 1;
      }
      found = 1;
    }
    if (( MAKEFOURCC('R', 'G', 'G', 'B')) == fourcc ) {
      camera->frameFormats[ OA_PIX_FMT_BGGR8 ] = 1;
      if ( cameraInfo->maxBitDepth == 10 ) {
        camera->frameFormats[ OA_PIX_FMT_BGGR10_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 12 ) {
        camera->frameFormats[ OA_PIX_FMT_BGGR12_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 14 ) {
        camera->frameFormats[ OA_PIX_FMT_BGGR14_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 16 ) {
        camera->frameFormats[ OA_PIX_FMT_BGGR16LE ] = 1;
      }
      found = 1;
    }
    if (( MAKEFOURCC('B', 'G', 'G', 'R')) == fourcc ) {
      camera->frameFormats[ OA_PIX_FMT_RGGB8 ] = 1;
      if ( cameraInfo->maxBitDepth == 10 ) {
        camera->frameFormats[ OA_PIX_FMT_RGGB10_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 12 ) {
        camera->frameFormats[ OA_PIX_FMT_RGGB12_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 14 ) {
        camera->frameFormats[ OA_PIX_FMT_RGGB14_16LE ] = 1;
      }
      if ( cameraInfo->maxBitDepth == 16 ) {
        camera->frameFormats[ OA_PIX_FMT_RGGB16LE ] = 1;
      }
      found = 1;
    }
    if ( !found ) {
      fprintf ( stderr, "raw format '%08x' not supported\n", fourcc );
			camera->features.flags &= ~OA_CAM_FEATURE_RAW_MODE;
    }
  } else {
    cameraInfo->currentVideoFormat = OA_PIX_FMT_GREY8;
  }

  /*
   * FIX ME -- Commenting this out because it causes the driver to crash
   *
  // Have to do this last otherise it messes up the raw stuff above
  if ( cameraInfo->maxBitDepth > 8 ) {
    if ( cameraInfo->colour ) {
      camera->frameFormats[ OA_PIX_FMT_RGB48LE ] = 1;
    }
  }
   */

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

  if (( numStillResolutions = devList[ devInfo->devIndex ].model->still )) {
    for ( i = 0; i < numStillResolutions; i++ ) {
      if ((( TT_LIB_PTR( get_StillResolution ))( handle, i, &x, &y )) < 0 ) {
        fprintf ( stderr, "failed to get still resolution %d\n", i );
        ( TT_LIB_PTR( Close ))( handle );
        FREE_DATA_STRUCTS;
        return 0;
      }
      fprintf ( stderr, "still resolution %d (%dx%d) unhandled\n", i, x, y );
    }
  }

  // Touptek cameras appear to mean "bin mode" when they talk about
  // different resolutions -- that is, the framing of the image remains
  // the same.  It is not ROI.

  numResolutions = devList[ devInfo->devIndex ].model->preview;
  cameraInfo->currentXSize = cameraInfo->currentYSize = 0;
  cameraInfo->currentXResolution = cameraInfo->currentYResolution = 0;

  if ( numResolutions > OA_MAX_BINNING ) {
    fprintf ( stderr, "Can't cope with %d resolutions\n", numResolutions );
    numResolutions = OA_MAX_BINNING;
  }

  for ( i = 0; i < numResolutions; i++ ) {
    if ((( TT_LIB_PTR( get_Resolution ))( handle, i, &x, &y )) < 0 ) {
      fprintf ( stderr, "failed to get resolution %d\n", i );
      ( TT_LIB_PTR( Close ))( handle );
			for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
				if ( cameraInfo->frameSizes[ j ].numSizes ) {
					free (( void* ) cameraInfo->frameSizes[ j ].sizes );
				}
			}
      FREE_DATA_STRUCTS;
      return 0;
    }

    // First resolution appears to be the full size of the sensor
    if ( !i ) {
      cameraInfo->currentXSize = cameraInfo->currentXResolution = x;
      cameraInfo->currentYSize = cameraInfo->currentYResolution = y;
    }

    binX = cameraInfo->currentXSize / x;
    binY = cameraInfo->currentYSize / y;

    if ( binX == binY && binX == ( i + 1 )) { 
      cameraInfo->frameSizes[ binX ].numSizes = 1;

      if (!( tmpPtr = realloc ( cameraInfo->frameSizes[ binX ].sizes,
						sizeof ( FRAMESIZE ) * 2 ))) {
        fprintf ( stderr, "realloc for frame sizes failed\n" );
        ( TT_LIB_PTR( Close ))( handle );
				for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
					if ( cameraInfo->frameSizes[ j ].numSizes ) {
						free (( void* ) cameraInfo->frameSizes[ j ].sizes );
					}
				}
        FREE_DATA_STRUCTS;
        return 0;
      }
			cameraInfo->frameSizes[ binX ].sizes = tmpPtr;
      cameraInfo->frameSizes[ binX ].sizes[0].x = x;
      cameraInfo->frameSizes[ binX ].sizes[0].y = y;

    } else {
      fprintf ( stderr, "Can't handle resolution %dx%d for camera\n", x, y );
    }
  }
  camera->features.flags |= OA_CAM_FEATURE_FIXED_FRAME_SIZES;

  cameraInfo->maxResolutionX = cameraInfo->currentXSize;
  cameraInfo->maxResolutionY = cameraInfo->currentYSize;
  cameraInfo->binMode = 1;

  if ( numResolutions > 1 ) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) = OA_CTRL_TYPE_DISCRETE;
  }

  // The largest buffer size we should need

  cameraInfo->buffers = 0;
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * cameraInfo->maxBytesPerPixel;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof (
      struct Touptekbuffer ));
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
      ( TT_LIB_PTR( Close ))( handle );
			free (( void* ) cameraInfo->buffers );
			for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
				if ( cameraInfo->frameSizes[ j ].numSizes ) {
					free (( void* ) cameraInfo->frameSizes[ j ].sizes );
				}
			}
      FREE_DATA_STRUCTS;
      return 0;
    }
  }

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      TT_FUNC( oacam, controller ), ( void* ) camera )) {
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->buffers );
    for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
      if ( cameraInfo->frameSizes[ j ].numSizes ) {
        free (( void* ) cameraInfo->frameSizes[ j ].sizes );
      }
    }
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      TT_FUNC( oacam, callbackHandler ), ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->buffers );
    for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
      if ( cameraInfo->frameSizes[ j ].numSizes ) {
        free (( void* ) cameraInfo->frameSizes[ j ].sizes );
      }
    }
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  cameraInfo->handle = handle;
  cameraInfo->initialised = 1;
  return camera;
}


static void
TT_FUNC( _, InitFunctionPointers )( oaCamera* camera )
{
  camera->funcs.initCamera = TT_FUNC( oa, InitCamera );
  camera->funcs.closeCamera = TT_FUNC( oa, CloseCamera );

  camera->funcs.setControl = TT_FUNC( oa, CameraSetControl );
  camera->funcs.readControl = TT_FUNC( oa, CameraReadControl );
  camera->funcs.testControl = TT_FUNC( oa, CameraTestControl );
  camera->funcs.getControlRange = TT_FUNC( oa, CameraGetControlRange );
  camera->funcs.getControlDiscreteSet =
		TT_FUNC( oa, CameraGetControlDiscreteSet );

  camera->funcs.startStreaming = TT_FUNC( oa, CameraStartStreaming );
  camera->funcs.stopStreaming = TT_FUNC( oa, CameraStopStreaming );
  camera->funcs.isStreaming = TT_FUNC( oa, CameraIsStreaming );

  camera->funcs.setResolution = TT_FUNC( oa, CameraSetResolution );
  camera->funcs.setROI = TT_FUNC( oa, CameraSetROI );
  camera->funcs.testROISize = TT_FUNC( oa, CameraTestROISize );

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = TT_FUNC( oa, CameraGetFrameSizes );
  camera->funcs.getFramePixelFormat = TT_FUNC( oa, CameraGetFramePixelFormat );

  camera->funcs.getMenuString = TT_FUNC( oa, CameraGetMenuString );

	camera->funcs.startExposure = TT_FUNC( oa, CameraStartExposure );
	camera->funcs.abortExposure = TT_FUNC( oa, CameraAbortExposure );
}


int
TT_FUNC( oa, CloseCamera )( oaCamera* camera )
{
  void*			dummy;
  TOUPTEK_STATE*	cameraInfo;
	int				j;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
  
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    ( TT_LIB_PTR( Close )) ( cameraInfo->handle );

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->buffers );
    for ( j = 1; j <= OA_MAX_BINNING; j++ ) {
      if ( cameraInfo->frameSizes[ j ].numSizes ) {
        free (( void* ) cameraInfo->frameSizes[ j ].sizes );
      }
    }
    free (( void* ) camera->_common );
    free (( void* ) cameraInfo );
    free (( void* ) camera );

  } else {
    return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
