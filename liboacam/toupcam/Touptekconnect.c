/*****************************************************************************
 *
 * Touptekconnect.c -- Initialise Touptek cameras
 *
 * Copyright 2016,2017,2018 James Fidell (james@openastroproject.org)
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
#include <toupcam.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "Touptekoacam.h"
#include "Touptekstate.h"

// From the Touptek docs
#ifndef MAKEFOURCC
#define MAKEFOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24))
#endif


static void _TouptekInitFunctionPointers ( oaCamera* );

/**
 * Initialise a given camera device
 */

oaCamera*
oaTouptekInitCamera ( oaCameraDevice* device )
{
  oaCamera*			camera;
  TOUPTEK_STATE*		cameraInfo;
  COMMON_INFO*			commonInfo;
  ToupcamInst			devList[ TOUPCAM_MAX ];
  unsigned int			numCameras, min, max, def;
  unsigned short		smin, smax, sdef;
  HToupCam			handle;
  DEVICE_INFO*			devInfo;
  unsigned int			i, j, numResolutions, numStillResolutions;
  unsigned int			fourcc, depth, binX, binY;
  int				x, y;
  char				toupcamId[128]; // must be longer than 64

  numCameras = ( p_Toupcam_Enum )( devList );
  devInfo = device->_private;
  if ( numCameras < 1 || devInfo->devIndex > numCameras ) {
    return 0;
  }

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }

  if (!( cameraInfo = ( TOUPTEK_STATE* ) malloc (
      sizeof ( TOUPTEK_STATE )))) {
    free (( void* ) camera );
    perror ( "malloc TOUPTEK_STATE failed" );
    return 0;
  }
  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    perror ( "malloc COMMON_INFO failed" );
    return 0;
  }
  OA_CLEAR ( *camera );
  OA_CLEAR ( *cameraInfo );
  OA_CLEAR ( *commonInfo );
  camera->_private = cameraInfo;
  camera->_common = commonInfo;

  _oaInitCameraFunctionPointers ( camera );
  _TouptekInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;

  camera->interface = device->interface;
  cameraInfo->colour = ( devList[ devInfo->devIndex ].model->flag &
      TOUPCAM_FLAG_MONO ) ? 0 : 1;

  if ( cameraInfo->colour ) {
    // Add "@" to use "RGB gain mode".  Ick :(
    ( void ) strcpy ( toupcamId, "@" );
  } else {
    *toupcamId = 0;
  }
  ( void ) strcat ( toupcamId, devInfo->toupcamId );
  if (!( handle = ( p_Toupcam_Open )( toupcamId ))) {
    fprintf ( stderr, "Can't get Toupcam handle\n" );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

	camera->features.readableControls = 1;

  // FIX ME -- work out how to support these
  // Toupcam_put_AutoExpoTarget
  // Toupcam_put_MaxAutoExpoTimeAGain
  // Toupcam_AwbOnePush
  // Toupcam_AwbInit
  // Toupcam_put_Chrome
  // Toupcam_put_Negative
  // Toupcam_put_HZ
  // Toupcam_put_Mode
  // Toupcam_put_AWBAuxRect
  // Toupcam_put_AEAuxRect
  // Toupcam_get_MonoMode
  // Toupcam_put_RealTime
  // Toupcam_put_LevelRange
  // Toupcam_put_TempTint

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_CONTRAST ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_CONTRAST ) = TOUPCAM_CONTRAST_MIN;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_CONTRAST ) = TOUPCAM_CONTRAST_MAX;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_CONTRAST ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_CONTRAST ) = TOUPCAM_CONTRAST_DEF;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAMMA ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA ) = TOUPCAM_GAMMA_MIN;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA ) = TOUPCAM_GAMMA_MAX;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAMMA ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) = TOUPCAM_GAMMA_DEF;

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

  if (( p_Toupcam_get_ExpTimeRange )( handle, &min, &max, &def ) < 0 ) {
    ( p_Toupcam_Close )( handle );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = OA_CTRL_TYPE_INT32;
  commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = min;
  commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = max;
  commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
  commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = def;
  // make these easy to find in the controller loop
  cameraInfo->exposureMin = min;
  cameraInfo->exposureMax = max;

  if (( p_Toupcam_get_ExpoAGainRange )( handle, &smin, &smax, &sdef ) < 0 ) {
    fprintf ( stderr, "Toupcam_get_ExpoAGainRange() failed\n" );
    ( p_Toupcam_Close )( handle );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
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
      TOUPCAM_FLAG_PUTTEMPERATURE ) {
    fprintf ( stderr, "Toupcam supports setting temperature, but we "
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
      TOUPCAM_FLAG_GETTEMPERATURE ) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) = OA_CTRL_TYPE_READONLY;
  }

  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_COOLERONOFF ) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER ) = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER ) = 0;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER ) = 1;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) = 0;
  }

  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_FAN ) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FAN ) = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_FAN ) = 0;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_FAN ) = 1;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_FAN ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN ) = 0;
  }

  if ( cameraInfo->colour ) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HUE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HUE ) = TOUPCAM_HUE_MIN;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HUE ) = TOUPCAM_HUE_MAX;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HUE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HUE ) = TOUPCAM_HUE_DEF;

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SATURATION ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SATURATION ) = TOUPCAM_SATURATION_MIN;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SATURATION ) = TOUPCAM_SATURATION_MAX;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SATURATION ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SATURATION ) = TOUPCAM_SATURATION_DEF;

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_RED_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE ) = TOUPCAM_WBGAIN_MIN;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE ) = TOUPCAM_WBGAIN_MAX;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_RED_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) = TOUPCAM_WBGAIN_DEF;

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GREEN_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GREEN_BALANCE ) = TOUPCAM_WBGAIN_MIN;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GREEN_BALANCE ) = TOUPCAM_WBGAIN_MAX;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GREEN_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GREEN_BALANCE ) = TOUPCAM_WBGAIN_DEF;

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE ) = TOUPCAM_WBGAIN_MIN;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE ) = TOUPCAM_WBGAIN_MAX;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) = TOUPCAM_WBGAIN_DEF;

    // I don't see why this should be colour only, but it does appear to be
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BRIGHTNESS ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BRIGHTNESS ) = TOUPCAM_BRIGHTNESS_MIN;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BRIGHTNESS ) = TOUPCAM_BRIGHTNESS_MAX;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BRIGHTNESS ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) = TOUPCAM_BRIGHTNESS_DEF;

    // force the camera out of raw mode
    if ((( p_Toupcam_put_Option )( handle, TOUPCAM_OPTION_RAW, 0 )) < 0 ) {
      fprintf ( stderr, "Toupcam_put_Option ( raw, 0 ) returns error\n" );
      ( p_Toupcam_Close )( handle );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }

  } else {

    // It looks like mono cameras return RGB frames by default.  That
    // seems wasteful, so try to turn it off.

    if ((( p_Toupcam_put_Option )( handle, TOUPCAM_OPTION_RAW, 1 )) < 0 ) {
      fprintf ( stderr, "Toupcam_put_Option ( raw, 1 ) returns error\n" );
      ( p_Toupcam_Close )( handle );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
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

  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_ROI_HARDWARE ) {
    camera->features.ROI = 1;
  }

  cameraInfo->maxBitDepth = p_Toupcam_get_MaxBitDepth ( handle );
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
    if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_BITDEPTH10 ) {
      if ( 10 == cameraInfo->maxBitDepth ) {
        camera->frameFormats[ cameraInfo->colour ? OA_PIX_FMT_RGB30LE :
            OA_PIX_FMT_GREY10_16LE ] = 1;
      } else {
        fprintf ( stderr, "Camera claims 10-bit is available, but only %d"
            "-bit is available\n", cameraInfo->maxBitDepth );
      }
    }
    if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_BITDEPTH12 ) {
      if ( 12 == cameraInfo->maxBitDepth ) {
        camera->frameFormats[ cameraInfo->colour ? OA_PIX_FMT_RGB36LE :
            OA_PIX_FMT_GREY12_16LE ] = 1;
      } else {
        fprintf ( stderr, "Camera claims 12-bit is available, but only %d"
            "-bit is available\n", cameraInfo->maxBitDepth );
      }
    }
    if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_BITDEPTH14 ) {
      if ( 14 == cameraInfo->maxBitDepth ) {
        camera->frameFormats[ cameraInfo->colour ? OA_PIX_FMT_RGB42LE :
            OA_PIX_FMT_GREY14_16LE ] = 1;
      } else {
        fprintf ( stderr, "Camera claims 14-bit is available, but only %d"
            "-bit is available\n", cameraInfo->maxBitDepth );
      }
    }
    if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_BITDEPTH16 ) {
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
    if ((( p_Toupcam_put_Option )( handle,
        TOUPCAM_OPTION_BITDEPTH, 0 )) < 0 ) {
      fprintf ( stderr,
          "Toupcam_put_Option ( bitdepth, 0 ) returns error\n" );
      ( p_Toupcam_Close )( handle );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
  }

  // FIX ME -- this may not be right
  cameraInfo->currentBitsPerPixel = 8;

  if ( devList[ devInfo->devIndex ].model->flag &
      TOUPCAM_FLAG_BINSKIP_SUPPORTED ) {
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

    if ((( p_Toupcam_get_RawFormat )( handle, &fourcc, &depth )) < 0 ) {
      fprintf ( stderr, "get_RawFormat returns error\n" );
      ( p_Toupcam_Close )( handle );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }

    // The docs aren't clear, so I'm assuming that raw mode is available for
    // all colour cameras
    camera->features.rawMode = camera->features.demosaicMode = 1;
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
      camera->features.rawMode = 0;
    }
  } else {
    cameraInfo->currentVideoFormat = OA_PIX_FMT_GREY8;
  }

  /*
   * FIX ME -- Commenting this out because the Touptek driver just seems
   * to return an empty frame in this mode
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
      if ((( p_Toupcam_get_StillResolution )( handle, i, &x, &y )) < 0 ) {
        fprintf ( stderr, "failed to get still resolution %d\n", i );
        ( p_Toupcam_Close )( handle );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
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
    if ((( p_Toupcam_get_Resolution )( handle, i, &x, &y )) < 0 ) {
      fprintf ( stderr, "failed to get resolution %d\n", i );
      ( p_Toupcam_Close )( handle );
      // FIX ME -- free the other sizes here too
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
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

      if (!(  cameraInfo->frameSizes[ binX ].sizes = realloc (
          cameraInfo->frameSizes[ binX ].sizes, sizeof ( FRAMESIZE ) * 2 ))) {
        fprintf ( stderr, "malloc for frame sizes failed\n" );
        ( p_Toupcam_Close )( handle );
        // FIX ME -- free the other sizes here too
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
      cameraInfo->frameSizes[ binX ].sizes[0].x = x;
      cameraInfo->frameSizes[ binX ].sizes[0].y = y;

    } else {
      fprintf ( stderr, "Can't handle resolution %dx%d for camera\n", x, y );
    }
  }

  cameraInfo->maxResolutionX = cameraInfo->currentXSize;
  cameraInfo->maxResolutionY = cameraInfo->currentYSize;
  cameraInfo->binMode = 1;
  camera->features.fixedFrameSizes = 1;

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
          cameraInfo->buffers[j].start = 0;
        }
      }
      // FIX ME -- free frame data
      ( p_Toupcam_Close )( handle );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
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
      oacamTouptekcontroller, ( void* ) camera )) {
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamTouptekcallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }

  cameraInfo->handle = handle;
  cameraInfo->initialised = 1;
  return camera;
}


static void
_TouptekInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaTouptekInitCamera;
  camera->funcs.closeCamera = oaTouptekCloseCamera;

  camera->funcs.setControl = oaTouptekCameraSetControl;
  camera->funcs.readControl = oaTouptekCameraReadControl;
  camera->funcs.testControl = oaTouptekCameraTestControl;
  camera->funcs.getControlRange = oaTouptekCameraGetControlRange;
  camera->funcs.getControlDiscreteSet = oaTouptekCameraGetControlDiscreteSet;

  camera->funcs.startStreaming = oaTouptekCameraStartStreaming;
  camera->funcs.stopStreaming = oaTouptekCameraStopStreaming;
  camera->funcs.isStreaming = oaTouptekCameraIsStreaming;

  camera->funcs.setResolution = oaTouptekCameraSetResolution;
  camera->funcs.setROI = oaTouptekCameraSetROI;
  camera->funcs.testROISize = oaTouptekCameraTestROISize;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaTouptekCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaTouptekCameraGetFramePixelFormat;

  camera->funcs.getMenuString = oaTouptekCameraGetMenuString;
}


int
oaTouptekCloseCamera ( oaCamera* camera )
{
  void*			dummy;
  TOUPTEK_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
  
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    ( p_Toupcam_Close ) ( cameraInfo->handle );

    free (( void* ) cameraInfo->frameSizes[1].sizes );

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

    free (( void* ) camera->_common );
    free (( void* ) cameraInfo );
    free (( void* ) camera );

  } else {
    return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
