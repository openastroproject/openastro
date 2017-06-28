/*****************************************************************************
 *
 * Mallincamconnect.c -- Initialise Mallincam cameras
 *
 * Copyright 2016 James Fidell (james@openastroproject.org)
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
#include "Mallincamoacam.h"
#include "Mallincamstate.h"

// From the Touptek docs
#ifndef MAKEFOURCC
#define MAKEFOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24))
#endif


static void _MallincamInitFunctionPointers ( oaCamera* );

/**
 * Initialise a given camera device
 */

oaCamera*
oaMallincamInitCamera ( oaCameraDevice* device )
{
  oaCamera*			camera;
  MALLINCAM_STATE*		cameraInfo;
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

  numCameras = ( *p_Mallincam_Enum )( devList );
  devInfo = device->_private;
  if ( numCameras < 1 || devInfo->devIndex > numCameras ) {
    return 0;
  }

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }

  if (!( cameraInfo = ( MALLINCAM_STATE* ) malloc (
      sizeof ( MALLINCAM_STATE )))) {
    free (( void* ) camera );
    perror ( "malloc MALLINCAM_STATE failed" );
    return 0;
  }
  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    perror ( "malloc COMMON_INFO failed" );
    return 0;
  }
  OA_CLEAR ( *cameraInfo );
  OA_CLEAR ( *commonInfo );
  OA_CLEAR ( camera->controls );
  OA_CLEAR ( camera->features );
  camera->_private = cameraInfo;
  camera->_common = commonInfo;

  _oaInitCameraFunctionPointers ( camera );
  _MallincamInitFunctionPointers ( camera );

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
  if (!( handle = ( *p_Mallincam_Open )( toupcamId ))) {
    fprintf ( stderr, "Can't get Mallincam handle\n" );
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

  // FIX ME -- work out how to support these
  // Mallincam_put_AutoExpoTarget
  // Mallincam_put_MaxAutoExpoTimeAGain
  // Mallincam_AwbOnePush
  // Mallincam_AwbInit
  // Mallincam_put_Chrome
  // Mallincam_put_Negative
  // Mallincam_put_HZ
  // Mallincam_put_Mode
  // Mallincam_put_AWBAuxRect
  // Mallincam_put_AEAuxRect
  // Mallincam_get_MonoMode
  // Mallincam_put_RealTime
  // Mallincam_put_LevelRange
  // Mallincam_put_TempTint

  camera->controls[ OA_CAM_CTRL_CONTRAST ] = OA_CTRL_TYPE_INT32;
  commonInfo->min[ OA_CAM_CTRL_CONTRAST ] = TOUPCAM_CONTRAST_MIN;
  commonInfo->max[ OA_CAM_CTRL_CONTRAST ] = TOUPCAM_CONTRAST_MAX;
  commonInfo->step[ OA_CAM_CTRL_CONTRAST ] = 1;
  commonInfo->def[ OA_CAM_CTRL_CONTRAST ] = TOUPCAM_CONTRAST_DEF;

  camera->controls[ OA_CAM_CTRL_GAMMA ] = OA_CTRL_TYPE_INT32;
  commonInfo->min[ OA_CAM_CTRL_GAMMA ] = TOUPCAM_GAMMA_MIN;
  commonInfo->max[ OA_CAM_CTRL_GAMMA ] = TOUPCAM_GAMMA_MAX;
  commonInfo->step[ OA_CAM_CTRL_GAMMA ] = 1;
  commonInfo->def[ OA_CAM_CTRL_GAMMA ] = TOUPCAM_GAMMA_DEF;

  camera->controls[ OA_CAM_CTRL_HFLIP ] = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->min[ OA_CAM_CTRL_HFLIP ] = 0;
  commonInfo->max[ OA_CAM_CTRL_HFLIP ] = 1;
  commonInfo->step[ OA_CAM_CTRL_HFLIP ] = 1;
  commonInfo->def[ OA_CAM_CTRL_HFLIP ] = 0;

  camera->controls[ OA_CAM_CTRL_VFLIP ] = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->min[ OA_CAM_CTRL_VFLIP ] = 0;
  commonInfo->max[ OA_CAM_CTRL_VFLIP ] = 1;
  commonInfo->step[ OA_CAM_CTRL_VFLIP ] = 1;
  commonInfo->def[ OA_CAM_CTRL_VFLIP ] = 0;

  camera->controls[ OA_CAM_CTRL_AUTO_EXPOSURE ] = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->min[ OA_CAM_CTRL_AUTO_EXPOSURE ] = 0;
  commonInfo->max[ OA_CAM_CTRL_AUTO_EXPOSURE ] = 1;
  commonInfo->step[ OA_CAM_CTRL_AUTO_EXPOSURE ] = 1;
  commonInfo->def[ OA_CAM_CTRL_AUTO_EXPOSURE ] = 0;

  if (( *p_Mallincam_get_ExpTimeRange )( handle, &min, &max, &def ) < 0 ) {
    ( *p_Mallincam_Close )( handle );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  camera->controls[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = OA_CTRL_TYPE_INT32;
  commonInfo->min[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = min;
  commonInfo->max[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = max;
  commonInfo->step[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = 1;
  commonInfo->def[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = def;
  // make these easy to find in the controller loop
  cameraInfo->exposureMin = min;
  cameraInfo->exposureMax = max;

  if (( *p_Mallincam_get_ExpoAGainRange )( handle, &smin, &smax, &sdef ) < 0 ) {
    fprintf ( stderr, "Mallincam_get_ExpoAGainRange() failed\n" );
    ( *p_Mallincam_Close )( handle );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  camera->controls[ OA_CAM_CTRL_GAIN ] = OA_CTRL_TYPE_INT32;
  commonInfo->min[ OA_CAM_CTRL_GAIN ] = smin;
  commonInfo->max[ OA_CAM_CTRL_GAIN ] = smax;
  commonInfo->step[ OA_CAM_CTRL_GAIN ] = 1;
  commonInfo->def[ OA_CAM_CTRL_GAIN ] = sdef;
  // make these easy to find in the controller loop
  cameraInfo->gainMin = smin;
  cameraInfo->gainMax = smax;

  // make this easy to find in the controller loop
  cameraInfo->speedMax = devList[ devInfo->devIndex ].model->maxspeed;
  camera->controls[ OA_CAM_CTRL_SPEED ] = OA_CTRL_TYPE_INT32;
  commonInfo->min[ OA_CAM_CTRL_SPEED ] = 0;
  commonInfo->max[ OA_CAM_CTRL_SPEED ] = cameraInfo->speedMax;
  commonInfo->step[ OA_CAM_CTRL_SPEED ] = 1;
  // this is a wild guess
  commonInfo->def[ OA_CAM_CTRL_SPEED ] = cameraInfo->speedMax;

  if ( devList[ devInfo->devIndex ].model->flag &
      TOUPCAM_FLAG_PUTTEMPERATURE ) {
    fprintf ( stderr, "Mallincam supports setting temperature, but we "
        "don't know how to get the range\n" );
    /*
    camera->controls[ OA_CAM_CTRL_TEMP_SETPOINT ] = OA_CTRL_TYPE_INT32;
    commonInfo->min[ OA_CAM_CTRL_TEMP_SETPOINT ] = min;
    commonInfo->max[ OA_CAM_CTRL_TEMP_SETPOINT ] = max;
    commonInfo->step[ OA_CAM_CTRL_TEMP_SETPOINT ] = 1;
    commonInfo->def[ OA_CAM_CTRL_TEMP_SETPOINT ] = def;
     */
  }

  if ( devList[ devInfo->devIndex ].model->flag &
      TOUPCAM_FLAG_GETTEMPERATURE ) {
    camera->controls[ OA_CAM_CTRL_TEMPERATURE ] = OA_CTRL_TYPE_READONLY;
  }

  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_COOLERONOFF ) {
    camera->controls[ OA_CAM_CTRL_COOLER ] = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->min[ OA_CAM_CTRL_COOLER ] = 0;
    commonInfo->max[ OA_CAM_CTRL_COOLER ] = 1;
    commonInfo->step[ OA_CAM_CTRL_COOLER ] = 1;
    commonInfo->def[ OA_CAM_CTRL_COOLER ] = 0;
  }

  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_FAN ) {
    camera->controls[ OA_CAM_CTRL_FAN ] = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->min[ OA_CAM_CTRL_FAN ] = 0;
    commonInfo->max[ OA_CAM_CTRL_FAN ] = 1;
    commonInfo->step[ OA_CAM_CTRL_FAN ] = 1;
    commonInfo->def[ OA_CAM_CTRL_FAN ] = 0;
  }

  if ( cameraInfo->colour ) {
    camera->controls[ OA_CAM_CTRL_HUE ] = OA_CTRL_TYPE_INT32;
    commonInfo->min[ OA_CAM_CTRL_HUE ] = TOUPCAM_HUE_MIN;
    commonInfo->max[ OA_CAM_CTRL_HUE ] = TOUPCAM_HUE_MAX;
    commonInfo->step[ OA_CAM_CTRL_HUE ] = 1;
    commonInfo->def[ OA_CAM_CTRL_HUE ] = TOUPCAM_HUE_DEF;

    camera->controls[ OA_CAM_CTRL_SATURATION ] = OA_CTRL_TYPE_INT32;
    commonInfo->min[ OA_CAM_CTRL_SATURATION ] = TOUPCAM_SATURATION_MIN;
    commonInfo->max[ OA_CAM_CTRL_SATURATION ] = TOUPCAM_SATURATION_MAX;
    commonInfo->step[ OA_CAM_CTRL_SATURATION ] = 1;
    commonInfo->def[ OA_CAM_CTRL_SATURATION ] = TOUPCAM_SATURATION_DEF;

    camera->controls[ OA_CAM_CTRL_RED_BALANCE ] = OA_CTRL_TYPE_INT32;
    commonInfo->min[ OA_CAM_CTRL_RED_BALANCE ] = TOUPCAM_WBGAIN_MIN;
    commonInfo->max[ OA_CAM_CTRL_RED_BALANCE ] = TOUPCAM_WBGAIN_MAX;
    commonInfo->step[ OA_CAM_CTRL_RED_BALANCE ] = 1;
    commonInfo->def[ OA_CAM_CTRL_RED_BALANCE ] = TOUPCAM_WBGAIN_DEF;

    camera->controls[ OA_CAM_CTRL_GREEN_BALANCE ] = OA_CTRL_TYPE_INT32;
    commonInfo->min[ OA_CAM_CTRL_GREEN_BALANCE ] = TOUPCAM_WBGAIN_MIN;
    commonInfo->max[ OA_CAM_CTRL_GREEN_BALANCE ] = TOUPCAM_WBGAIN_MAX;
    commonInfo->step[ OA_CAM_CTRL_GREEN_BALANCE ] = 1;
    commonInfo->def[ OA_CAM_CTRL_GREEN_BALANCE ] = TOUPCAM_WBGAIN_DEF;

    camera->controls[ OA_CAM_CTRL_BLUE_BALANCE ] = OA_CTRL_TYPE_INT32;
    commonInfo->min[ OA_CAM_CTRL_BLUE_BALANCE ] = TOUPCAM_WBGAIN_MIN;
    commonInfo->max[ OA_CAM_CTRL_BLUE_BALANCE ] = TOUPCAM_WBGAIN_MAX;
    commonInfo->step[ OA_CAM_CTRL_BLUE_BALANCE ] = 1;
    commonInfo->def[ OA_CAM_CTRL_BLUE_BALANCE ] = TOUPCAM_WBGAIN_DEF;

    // I don't see why this should be colour only, but it does appear to be
    camera->controls[ OA_CAM_CTRL_BRIGHTNESS ] = OA_CTRL_TYPE_INT32;
    commonInfo->min[ OA_CAM_CTRL_BRIGHTNESS ] = TOUPCAM_BRIGHTNESS_MIN;
    commonInfo->max[ OA_CAM_CTRL_BRIGHTNESS ] = TOUPCAM_BRIGHTNESS_MAX;
    commonInfo->step[ OA_CAM_CTRL_BRIGHTNESS ] = 1;
    commonInfo->def[ OA_CAM_CTRL_BRIGHTNESS ] = TOUPCAM_BRIGHTNESS_DEF;

    camera->controls[ OA_CAM_CTRL_COLOUR_MODE ] = OA_CTRL_TYPE_DISCRETE;

    // force the camera out of raw mode
    if ((( *p_Mallincam_put_Option )( handle, TOUPCAM_OPTION_RAW, 0 ))) {
      fprintf ( stderr, "Mallincam_put_Option ( raw, 0 ) returns error\n" );
      ( *p_Mallincam_Close )( handle );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }

  } else {

    // It looks like mono cameras return RGB frames by default.  That
    // seems wasteful, so try to turn it off.

    if ((( *p_Mallincam_put_Option )( handle, TOUPCAM_OPTION_RAW, 1 ))) {
      fprintf ( stderr, "Mallincam_put_Option ( raw, 1 ) returns error\n" );
      ( *p_Mallincam_Close )( handle );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
  }

  /*
  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_ROI_HARDWARE ) {
    camera->features.ROI = 1;
  }
   */

  // force camera into 8-bit mode

  if ((( *p_Mallincam_put_Option )( handle, TOUPCAM_OPTION_BITDEPTH, 0 ))) {
    fprintf ( stderr, "Mallincam_put_Option ( bitdepth, 0 ) returns error\n" );
    ( *p_Mallincam_Close )( handle );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }
  // FIX ME -- this may not be right
  cameraInfo->currentBitsPerPixel = 8;

  if ( devList[ devInfo->devIndex ].model->flag &
      TOUPCAM_FLAG_BINSKIP_SUPPORTED ) {
    fprintf ( stderr, "bin/skip mode supported but not handled\n" );
  }

  // The docs aren't clear, so I'm assuming that raw mode is available for
  // all colour cameras

  cameraInfo->videoRaw = cameraInfo->videoRGB24 = cameraInfo->colour;
  cameraInfo->videoGrey = !cameraInfo->colour;
  cameraInfo->currentBytesPerPixel = cameraInfo->bytesPerPixel =
      cameraInfo->colour ? 3 : 1;

  cameraInfo->maxBitDepth = 8;
  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_BITDEPTH10 ) {
    cameraInfo->maxBitDepth = 10;
  }
  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_BITDEPTH12 ) {
    cameraInfo->maxBitDepth = 12;
  }
  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_BITDEPTH14 ) {
    cameraInfo->maxBitDepth = 14;
  }
  if ( devList[ devInfo->devIndex ].model->flag & TOUPCAM_FLAG_BITDEPTH16 ) {
    cameraInfo->maxBitDepth = 16;
  }

  if ( cameraInfo->maxBitDepth > 8 ) {
    if ( cameraInfo->colour ) {
      fprintf ( stderr, "Check up on RGB48 camera input\n" );
      cameraInfo->bytesPerPixel = 6; // RGB48
    } else {
      cameraInfo->bytesPerPixel = 2;
      cameraInfo->videoGrey16 = 1;
    }
    camera->controls[ OA_CAM_CTRL_BIT_DEPTH ] = OA_CTRL_TYPE_DISCRETE;
  }

  if ( cameraInfo->colour ) {
    camera->features.rawMode = camera->features.demosaicMode = 1;
    cameraInfo->currentVideoFormat = OA_PIX_FMT_RGB24;
    if ((( *p_Mallincam_get_RawFormat )( handle, &fourcc, &depth )) < 0 ) {
      fprintf ( stderr, "get_RawFormat returns error\n" );
      ( *p_Mallincam_Close )( handle );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }

    // Some weird stuff appears to be going on here.  When I enable raw
    // mode, the image flips vertically from its non-raw version.  That
    // has the effect of changing the claimed raw image format, so we need
    // to account for that here.

    if (( MAKEFOURCC('G', 'B', 'R', 'G')) == fourcc ) {
      cameraInfo->cfaPattern = OA_PIX_FMT_GRBG8;
    }
    if (( MAKEFOURCC('G', 'R', 'B', 'G')) == fourcc ) {
      cameraInfo->cfaPattern = OA_PIX_FMT_GBRG8;
    }
    if (( MAKEFOURCC('R', 'G', 'G', 'B')) == fourcc ) {
      cameraInfo->cfaPattern = OA_PIX_FMT_BGGR8;
    }
    if (( MAKEFOURCC('B', 'G', 'G', 'R')) == fourcc ) {
      cameraInfo->cfaPattern = OA_PIX_FMT_RGGB8;
    }
    if ( !cameraInfo->cfaPattern ) {
      fprintf ( stderr, "raw format '%08x' not supported\n", fourcc );
      camera->features.rawMode = 0;
    }
  } else {
    cameraInfo->currentVideoFormat = OA_PIX_FMT_GREY8;
  }

  if (( numStillResolutions = devList[ devInfo->devIndex ].model->still )) {
    for ( i = 0; i < numStillResolutions; i++ ) {
      if ((( *p_Mallincam_get_StillResolution )( handle, i, &x, &y )) < 0 ) {
        fprintf ( stderr, "failed to get still resolution %d\n", i );
        ( *p_Mallincam_Close )( handle );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
      fprintf ( stderr, "still resolution %d (%dx%d) unhandled\n", i, x, y );
    }
  }

  // Mallintek cameras appear to mean "bin mode" when they talk about
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
    if ((( *p_Mallincam_get_Resolution )( handle, i, &x, &y )) < 0 ) {
      fprintf ( stderr, "failed to get resolution %d\n", i );
      ( *p_Mallincam_Close )( handle );
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
        ( *p_Mallincam_Close )( handle );
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

  if ( numResolutions > 1 ) {
    camera->controls[ OA_CAM_CTRL_BINNING ] = OA_CTRL_TYPE_DISCRETE;
  }

  // The largest buffer size we should need

  cameraInfo->buffers = 0;
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * cameraInfo->bytesPerPixel;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof (
      struct Mallincambuffer ));
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
      ( *p_Mallincam_Close )( handle );
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
      oacamMallincamcontroller, ( void* ) camera )) {
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamMallincamcallbackHandler, ( void* ) camera )) {

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
_MallincamInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaMallincamInitCamera;
  camera->funcs.closeCamera = oaMallincamCloseCamera;

  camera->funcs.setControl = oaMallincamCameraSetControl;
  camera->funcs.readControl = oaMallincamCameraReadControl;
  camera->funcs.testControl = oaMallincamCameraTestControl;
  camera->funcs.getControlRange = oaMallincamCameraGetControlRange;

  camera->funcs.startStreaming = oaMallincamCameraStartStreaming;
  camera->funcs.stopStreaming = oaMallincamCameraStopStreaming;
  camera->funcs.isStreaming = oaMallincamCameraIsStreaming;

  camera->funcs.setResolution = oaMallincamCameraSetResolution;
  // camera->funcs.setROI = oaMallincamCameraSetROI;
  // camera->funcs.testROISize = oaMallincamCameraTestROISize;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaMallincamCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaMallincamCameraGetFramePixelFormat;
}


int
oaMallincamCloseCamera ( oaCamera* camera )
{
  void*			dummy;
  MALLINCAM_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
  
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    ( *p_Mallincam_Close ) ( cameraInfo->handle );

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
