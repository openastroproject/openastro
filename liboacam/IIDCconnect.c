/*****************************************************************************
 *
 * IIDCconnect.c -- Initialise IEEE1394/IIDC cameras
 *
 * Copyright 2013,2014,2015,2016 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBDC1394

#include <dc1394/dc1394.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "IIDCoacam.h"
#include "IIDCstate.h"


static void _IIDCInitFunctionPointers ( oaCamera* );
static int  _processFormat7Modes ( oaCamera*, dc1394camera_t*,
    dc1394video_modes_t );
static int  _processNonFormat7Modes ( oaCamera*, dc1394camera_t*,
    dc1394video_modes_t );

struct iidcCtrl dc1394Controls[] = {
  { DC1394_FEATURE_BRIGHTNESS, OA_CAM_CTRL_BRIGHTNESS,
      OA_CAM_CTRL_AUTO_BRIGHTNESS },
  // The IIDC spec says this one is like contrast.  Who am I to argue?
  { DC1394_FEATURE_EXPOSURE, OA_CAM_CTRL_CONTRAST, OA_CAM_CTRL_AUTO_CONTRAST },
  { DC1394_FEATURE_SHARPNESS, OA_CAM_CTRL_SHARPNESS, 0 },
  { DC1394_FEATURE_WHITE_BALANCE, OA_CAM_CTRL_WHITE_BALANCE,
      OA_CAM_CTRL_AUTO_WHITE_BALANCE },
  { DC1394_FEATURE_HUE, OA_CAM_CTRL_HUE, OA_CAM_CTRL_HUE_AUTO },
  { DC1394_FEATURE_SATURATION, OA_CAM_CTRL_SATURATION, 0 },
  { DC1394_FEATURE_GAMMA, OA_CAM_CTRL_GAMMA, OA_CAM_CTRL_AUTO_GAMMA },
  { DC1394_FEATURE_SHUTTER, -1, -1 },
  { DC1394_FEATURE_GAIN, OA_CAM_CTRL_GAIN, OA_CAM_CTRL_AUTO_GAIN },
  { DC1394_FEATURE_IRIS, 0, 0 },
  { DC1394_FEATURE_FOCUS, 0, 0 },
  { DC1394_FEATURE_TEMPERATURE, 0, 0 }, // FIX ME -- actually we have this one
  { DC1394_FEATURE_TRIGGER, OA_CAM_CTRL_TRIGGER_MODE, 0 },
  { DC1394_FEATURE_TRIGGER_DELAY, OA_CAM_CTRL_TRIGGER_DELAY, 0 },
  { DC1394_FEATURE_WHITE_SHADING, 0, 0 },
  { DC1394_FEATURE_FRAME_RATE, -1, -1 },
  { DC1394_FEATURE_ZOOM, 0, 0 },
  { DC1394_FEATURE_PAN, 0, 0 },
  { DC1394_FEATURE_TILT, 0, 0 },
  { DC1394_FEATURE_OPTICAL_FILTER, 0, 0 },
  { DC1394_FEATURE_CAPTURE_SIZE, 0, 0 },
  { DC1394_FEATURE_CAPTURE_QUALITY, 0, 0 }
};

unsigned int numIIDCControls = sizeof ( dc1394Controls ) /
    sizeof ( struct iidcCtrl );

struct iidcFrameRate dc1394FrameRates[] = {
  { DC1394_FRAMERATE_1_875, 8, 15 },
  { DC1394_FRAMERATE_3_75, 4, 15 },
  { DC1394_FRAMERATE_7_5, 2, 15 },
  { DC1394_FRAMERATE_15, 1, 15 },
  { DC1394_FRAMERATE_30, 1, 30 },
  { DC1394_FRAMERATE_60, 1, 60 },
  { DC1394_FRAMERATE_120, 1, 120 },
  { DC1394_FRAMERATE_240, 1, 240 }
};

unsigned int numIIDCFrameRates = sizeof ( dc1394FrameRates ) /
    sizeof ( struct iidcFrameRate );

/**
 * Initialise a given camera device
 */

oaCamera*
oaIIDCInitCamera ( oaCameraDevice* device )
{
  dc1394_t*	        iidcContext;
  dc1394camera_t*       iidcCam;
  dc1394video_modes_t   videoModes;
  dc1394featureset_t    features;
  oaCamera*             camera;
  int                   oaControl, oaAutoControl, use1394B;
  unsigned int		i;
  DEVICE_INFO*		devInfo;
  IIDC_STATE*		cameraInfo;
  COMMON_INFO*		commonInfo;
  dc1394framerates_t	framerates;

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }
  if (!( cameraInfo = ( IIDC_STATE* ) malloc ( sizeof ( IIDC_STATE )))) {
    free (( void* ) camera );
    perror ( "malloc IIDC_STATE failed" );
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
  _IIDCInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  devInfo = device->_private;

  iidcContext = dc1394_new();
  if ( !iidcContext ) {
    fprintf ( stderr, "%s: Can't get IIDC context\n", __FUNCTION__ );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  if (!( iidcCam = dc1394_camera_new_unit ( iidcContext, devInfo->guid,
      devInfo->unit ))) {
    fprintf ( stderr, "%s: dc1394_camera_new_unit failed\n", __FUNCTION__ );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  // Check if this appears to be a TIS colour camera
  if ( !strcasecmp ( "imaging source", iidcCam->vendor )) {
    if ( !strncasecmp ( "DB", iidcCam->model, 2 ) ||
        !strncasecmp ( "DF", iidcCam->model, 2 )) {
      cameraInfo->isTISColour = 1;
      fprintf ( stderr, "vendor '%s', model '%s', vendor_id %04x, "
          "model_id %04x\n", iidcCam->vendor, iidcCam->model,
          iidcCam->vendor_id, iidcCam->model_id );
    }
    if ( iidcCam->vendor_id != 0x0748 ) {
      fprintf ( stderr, "unexpected TIS camera configuration:\n  "
          "vendor '%s', model '%s', vendor_id %04x, model_id %04x\n",
          iidcCam->vendor, iidcCam->model, iidcCam->vendor_id,
          iidcCam->model_id );
    }
  } else {
    if ( 0x0748 == iidcCam->vendor_id ) {
      fprintf ( stderr, "unexpected non-TIS camera configuration:\n  "
          "vendor '%s', model '%s', vendor_id %04x, model_id %04x\n",
          iidcCam->vendor, iidcCam->model, iidcCam->vendor_id,
          iidcCam->model_id );
    }
  }

  if ( dc1394_feature_get_all ( iidcCam, &features ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_feature_get_all failed\n", __FUNCTION__ );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  camera->interface = device->interface;

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  // Reset the camera here.  That way hopefully we can be sure that the
  // values we read from the current settings are actually the default
  // values for those controls

  if ( dc1394_camera_reset ( iidcCam ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_camera_reset failed\n", __FUNCTION__ );
  }
  // Allow the camera reset a little time to happen.  Doesn't seem to cause
  // a problem on Linux, but makes OSX barf if we don't.
  usleep ( 60000 );


  // There's a lot of work still to be done here.  For most stuff I'm
  // ignoring absolute_capable because it uses completely different
  // ranges and units and everything becomes a nightmare :(
  // I'm also (mostly) ignoring on/off, readout and polarity

  for ( i = 0; i < DC1394_FEATURE_NUM; i++ ) {
    if ( features.feature[ i ].available &&
        (( features.feature[ i ].on_off_capable &&
        features.feature[ i ].is_on ) ||
        !features.feature[ i ].on_off_capable )) {

      oaControl = dc1394Controls[ i ].oaControl;
      oaAutoControl = dc1394Controls[ i ].oaAutoControl;

      switch ( i + DC1394_FEATURE_MIN ) {

        case DC1394_FEATURE_BRIGHTNESS:
        case DC1394_FEATURE_SHARPNESS:
        case DC1394_FEATURE_HUE:
        case DC1394_FEATURE_SATURATION:
        case DC1394_FEATURE_GAMMA:
        case DC1394_FEATURE_GAIN:
        case DC1394_FEATURE_EXPOSURE:
        {
          unsigned int j;

          for ( j = 0; j < features.feature[i].modes.num; j++ ) {
            switch ( features.feature[i].modes.modes[j] ) {

              case DC1394_FEATURE_MODE_MANUAL:
                camera->controls[ oaControl ] = OA_CTRL_TYPE_INT64;
                commonInfo->min[ oaControl ] = features.feature[i].min;
                commonInfo->max[ oaControl ] = features.feature[i].max;
                commonInfo->step[ oaControl ] = 1; // arbitrary
                commonInfo->def[ oaControl ] = features.feature[i].value;
                break;

              case DC1394_FEATURE_MODE_AUTO:
                if ( oaAutoControl ) {
                  camera->controls[ oaAutoControl ] = OA_CTRL_TYPE_BOOLEAN;
                  commonInfo->min[ oaAutoControl ] = 0;
                  commonInfo->max[ oaAutoControl ] = 1;
                  commonInfo->step[ oaAutoControl ] = 1;
                  commonInfo->def[ oaAutoControl ] = (
                      DC1394_FEATURE_MODE_AUTO ==
                      features.feature[i].current_mode ) ? 1 : 0;
                } else {
                  fprintf ( stderr, "%s: have auto for control %d, but "
                      "liboacam does not", __FUNCTION__, oaControl );
                }
                break;

              default:
                fprintf ( stderr, "%s: unexpected feature mode %d for %d\n",
                    __FUNCTION__, features.feature[i].modes.modes[j],
                    i + DC1394_FEATURE_MIN );
                break;
            }
          }
          break;
        }
        case DC1394_FEATURE_SHUTTER:
        {
          unsigned int min, max, step, def;
          int err;
          // shutter is actually exposure time.  exposure is something
          // else
          oaAutoControl = OA_CAM_CTRL_AUTO_EXPOSURE;
          if ( features.feature[ i ].absolute_capable ) {
            // have to set the control to use the absolute settings...
            if (( err = dc1394_feature_set_absolute_control ( iidcCam,
                DC1394_FEATURE_SHUTTER, DC1394_ON ) != DC1394_SUCCESS )) {
              fprintf ( stderr, "%s: dc1394_feature_set_absolute_control "
                  "failed, err: %d\n", __FUNCTION__, err );
              free ( commonInfo );
              free ( cameraInfo );
              free ( camera );
              return 0;
            }
            // and disable auto mode, as we don't support that at the moment
            // FIX ME -- need to deal with this when we do -- auto mode in
            // IIDC cameras may be more than just on/off
            if ( dc1394_feature_set_mode ( iidcCam, DC1394_FEATURE_SHUTTER,
                DC1394_FEATURE_MODE_MANUAL ) != DC1394_SUCCESS ) {
              fprintf ( stderr, "%s: dc1394_feature_set_mode failed for "
                  "shutter speed\n", __FUNCTION__ );
              free ( commonInfo );
              free ( cameraInfo );
              free ( camera );
              return 0;
            }

            oaControl = OA_CAM_CTRL_EXPOSURE_ABSOLUTE;
            min = features.feature[ i ].abs_min * 1000000.0;
            max = features.feature[ i ].abs_max * 1000000.0;
            /*
             * Fudge no longer required
            unsigned long m = features.feature[ i ].abs_max * 1000000.0;
            if ( m > 0x7fffffff ) {
              m = 0x7fffffff;
              fprintf ( stderr, "fudging IIDC maximum exposure time\n" );
            }
            max = m;
             */
            step = 1000; // arbitrary
            def = features.feature[ i ].abs_value * 1000000.0;
          } else {
            oaControl = OA_CAM_CTRL_EXPOSURE;
            min = features.feature[i].min;
            max = features.feature[i].max;
            step = 1; // arbitrary
            def = features.feature[ i ].value;
          }
          camera->controls[ oaControl ] = OA_CTRL_TYPE_INT64;
          commonInfo->min[ oaControl ] = min;
          commonInfo->max[ oaControl ] = max;
          commonInfo->step[ oaControl ] = step;
          commonInfo->def[ oaControl ] = def;
          break;
        }
        case DC1394_FEATURE_WHITE_BALANCE:
        {
          // This is something of a nightmare.
          // The manual white balance control is actually achieved by
          // setting the red and blue balances independently (though they
          // both have to be set at the same time).  Auto mode however
          // controls both at once.
          // So, we need manual red and blue balance controls and an auto
          // white balance control
          unsigned int j;

          for ( j = 0; j < features.feature[i].modes.num; j++ ) {
            switch ( features.feature[i].modes.modes[j] ) {

              case DC1394_FEATURE_MODE_MANUAL:
                camera->controls[ OA_CAM_CTRL_BLUE_BALANCE ] =
                    camera->controls[ OA_CAM_CTRL_RED_BALANCE ] =
                    OA_CTRL_TYPE_INT32;
                commonInfo->min[ OA_CAM_CTRL_BLUE_BALANCE ] =
                    commonInfo->min[ OA_CAM_CTRL_RED_BALANCE ] =
                    features.feature[i].min;
                commonInfo->max[ OA_CAM_CTRL_BLUE_BALANCE ] =
                    commonInfo->max[ OA_CAM_CTRL_RED_BALANCE ] =
                    features.feature[i].max;
                commonInfo->step[ OA_CAM_CTRL_BLUE_BALANCE ] =
                    commonInfo->step[ OA_CAM_CTRL_RED_BALANCE ] = 1;//arbitrary
                commonInfo->def[ OA_CAM_CTRL_BLUE_BALANCE ] =
                    commonInfo->def[ OA_CAM_CTRL_RED_BALANCE ] =
                    cameraInfo->currentRedBalance =
                    cameraInfo->currentBlueBalance =
                    features.feature[i].value;
                break;

              case DC1394_FEATURE_MODE_AUTO:
                if ( oaAutoControl ) {
                  camera->controls[ oaAutoControl ] = OA_CTRL_TYPE_BOOLEAN;
                  commonInfo->min[ oaAutoControl ] = 0;
                  commonInfo->max[ oaAutoControl ] = 1;
                  commonInfo->step[ oaAutoControl ] = 1;
                  commonInfo->def[ oaAutoControl ] = (
                      DC1394_FEATURE_MODE_AUTO ==
                      features.feature[i].current_mode ) ? 1 : 0;
                } else {
                  fprintf ( stderr, "%s: have auto for control %d, but "
                      "liboacam does not", __FUNCTION__, oaControl );
                }
                break;

              default:
                fprintf ( stderr, "%s: unexpected feature mode %d for %d\n",
                    __FUNCTION__, features.feature[i].modes.modes[j],
                    i + DC1394_FEATURE_MIN );
                break;
            }
          }
          break;
        }
        case DC1394_FEATURE_FRAME_RATE:
          // this is used, but handled elsewhere
          break;

        case DC1394_FEATURE_IRIS:
        case DC1394_FEATURE_FOCUS:
        case DC1394_FEATURE_TEMPERATURE:
        case DC1394_FEATURE_TRIGGER:
        case DC1394_FEATURE_TRIGGER_DELAY:
        case DC1394_FEATURE_WHITE_SHADING:
        case DC1394_FEATURE_ZOOM:
        case DC1394_FEATURE_PAN:
        case DC1394_FEATURE_TILT:
        case DC1394_FEATURE_OPTICAL_FILTER:
        case DC1394_FEATURE_CAPTURE_SIZE:
        case DC1394_FEATURE_CAPTURE_QUALITY:
          fprintf ( stderr, "%s: unsupported IIDC control %d\n", __FUNCTION__,
              i + DC1394_FEATURE_MIN );
          break;

        default:
          fprintf ( stderr, "%s: unknown IIDC control %d\n", __FUNCTION__,
              i + DC1394_FEATURE_MIN );
          break;
      }
    }
  }

  // Looking at the specs it appears that a number of compressed video
  // modes are supported (YUV422 etc.) as well as RGB and 8-bit or 16-bit
  // mono should
  // be supported.  For the moment we'll ignore YUY2 and assume the camera
  // is colour if RGB shows up, monochrome otherwise.

  if ( dc1394_video_get_supported_modes ( iidcCam, &videoModes ) !=
      DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_video_get_supported_modes failed",
        __FUNCTION__ );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  // Check to see if we have any Format7 modes listed.  We'll prefer
  // those if they do exist

  cameraInfo->haveFormat7 = 0;
  for ( i = 0; i < videoModes.num; i++ ) {
    if ( videoModes.modes[i] >= DC1394_VIDEO_MODE_FORMAT7_MIN &&
        videoModes.modes[i] <= DC1394_VIDEO_MODE_FORMAT7_MAX ) {
      cameraInfo->haveFormat7 = 1;
    }
  }

  // There are problems here if not all colour modes are supported in
  // all resolutions

  cameraInfo->videoRGB24 = 0;
  cameraInfo->videoGrey16 = 0;
  cameraInfo->videoGrey = 0;
  cameraInfo->mosaicFormat = 0;
  cameraInfo->videoCurrent = 0;
  cameraInfo->currentCodec = 0;

  camera->features.rawMode = camera->features.demosaicMode = 0;
  camera->features.hasReset = 1;

  if ( cameraInfo->haveFormat7 ) {
    if ( _processFormat7Modes ( camera, iidcCam, videoModes )) {
      // try non format7 modes if there aren't any we can use
      cameraInfo->haveFormat7 = 0;
    }
  }
  if ( !cameraInfo->haveFormat7 ) {
    if ( _processNonFormat7Modes ( camera, iidcCam, videoModes )) {
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
  }

  if ( !cameraInfo->videoCurrent ) {
    fprintf ( stderr, "%s: No suitable video format found", __FUNCTION__ );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  if ( !cameraInfo->haveFormat7 ) {
    if ( dc1394_video_get_supported_framerates ( iidcCam,
        cameraInfo->videoCurrent, &framerates ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_get_supported_framerates failed\n",
         __FUNCTION__ );
    }
    if ( framerates.num > 1 ) {
      camera->features.frameRates = 1;
    }
  }

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamIIDCcontroller, ( void* ) camera )) {
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamIIDCcallbackHandler, ( void* ) camera )) {

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

  // This is a nasty, nasty hack, but it's the only way I can find to
  // tell if we're actually connected to a USB camera.  We need to know
  // this because some USB cameras claim to support 1394B and then hang
  // when you attempt to set speed 800.

  use1394B = 1;
  dc1394bool_t pwr;
  if ( dc1394_camera_get_broadcast ( iidcCam, &pwr ) != DC1394_SUCCESS ) {
    use1394B = -1;
  }

  if ( use1394B >= 0 ) {
    if ( dc1394_video_set_operation_mode ( iidcCam,
        ( iidcCam->bmode_capable == DC1394_TRUE ) ?
        DC1394_OPERATION_MODE_1394B : DC1394_OPERATION_MODE_LEGACY) !=
        DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_set_operation_mode failed\n",
          __FUNCTION__ );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
    if ( dc1394_video_set_iso_speed ( iidcCam,
        ( iidcCam->bmode_capable == DC1394_TRUE ) ?
        DC1394_ISO_SPEED_800 : DC1394_ISO_SPEED_400 ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_set_iso_speed failed\n",
          __FUNCTION__ );
    }
  }

  /*
  if ( dc1394_video_set_mode ( iidcCam, cameraInfo->videoCurrent ) !=
      DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_video_set_mode failed\n", __FUNCTION__ );
    free ( commonInfo );
    free ( cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  if ( dc1394_capture_setup ( iidcCam, OA_CAM_BUFFERS,
      DC1394_CAPTURE_FLAGS_DEFAULT ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_capture_setup failed\n", __FUNCTION__ );
    free ( commonInfo );
    free ( cameraInfo );
    free (( void* ) camera );
    return 0;
  }
  */

  cameraInfo->frameRates.numRates = 0;

  cameraInfo->iidcHandle = iidcCam;
  cameraInfo->initialised = 1;

  return camera;
}


static void
_IIDCInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaIIDCInitCamera;
  camera->funcs.closeCamera = oaIIDCCloseCamera;

  camera->funcs.setControl = oaIIDCCameraSetControl;
  camera->funcs.readControl = oaIIDCCameraReadControl;
  camera->funcs.testControl = oaIIDCCameraTestControl;
  camera->funcs.getControlRange = oaIIDCCameraGetControlRange;

  camera->funcs.startStreaming = oaIIDCCameraStartStreaming;
  camera->funcs.stopStreaming = oaIIDCCameraStopStreaming;
  camera->funcs.isStreaming = oaIIDCCameraIsStreaming;

  camera->funcs.setResolution = oaIIDCCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaIIDCCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaIIDCCameraGetFramePixelFormat;

  camera->funcs.enumerateFrameRates = oaIIDCCameraGetFrameRates;
  camera->funcs.setFrameInterval = oaIIDCCameraSetFrameInterval;
}


static int
_processFormat7Modes ( oaCamera* camera, dc1394camera_t* iidcCam,
    dc1394video_modes_t videoModes )
{
  dc1394format7modeset_t	modeList;
  dc1394color_coding_t		coding;
  unsigned int			i, j, addResolution;
  unsigned int			numResolutions, preferredFound;
  IIDC_STATE*			cameraInfo;

  if ( dc1394_format7_get_modeset ( iidcCam, &modeList ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_format7_get_modeset return error\n",
        __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }

  numResolutions = preferredFound = 0;
  cameraInfo = camera->_private;

  for ( i = 0; i < DC1394_VIDEO_MODE_FORMAT7_NUM; i++ ) {
    if ( modeList.mode[i].present ) {
      addResolution = 0;
      for ( j = 0; j < modeList.mode[i].color_codings.num; j++ ) {
        coding = modeList.mode[i].color_codings.codings[j];

        switch ( coding ) {
          case DC1394_COLOR_CODING_MONO8:
            if ( !cameraInfo->videoRGB24 ) {
              preferredFound = 1;
              cameraInfo->videoCurrent = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
            }
            // TIS colour cameras apparently offer MONO8 instead of
            // RAW8
            if ( cameraInfo->isTISColour ) {
              camera->features.rawMode = 1;
              cameraInfo->videoRaw = 1;
            } else {
              cameraInfo->videoGrey = 1;
            }
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_RGB8:
            preferredFound = 1;
            cameraInfo->videoCurrent = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
            cameraInfo->currentCodec = coding;
            cameraInfo->videoRGB24 = 1;
            camera->features.demosaicMode = 1;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_MONO16:
            if ( !preferredFound ) {
              preferredFound = 1;
              cameraInfo->videoCurrent = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
            }
            cameraInfo->videoGrey16 = 1;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_RAW8:
            if ( !preferredFound ) {
              preferredFound = 1;
              cameraInfo->videoCurrent = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
            }
            camera->features.rawMode = 1;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_YUV422:
            if ( !preferredFound ) {
              cameraInfo->videoCurrent = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              addResolution = 1;
            }
            break;

          default:
            fprintf ( stderr, "%s: unhandled video coding %d\n", __FUNCTION__,
                coding );
            break;
        }
      }

      if ( addResolution ) {
        uint32_t w, h, found;
        found = 0;
        w = modeList.mode[i].size_x;
        h = modeList.mode[i].size_y;
        if ( numResolutions ) {
          for ( j = 0; j < numResolutions && !found; j++ ) {
            if ( cameraInfo->frameSizes[1].sizes[j].x == w &&
                cameraInfo->frameSizes[1].sizes[j].y == h ) {
              found = 1;
            }
          }
        }
        if ( !found ) {
          if (!(  cameraInfo->frameSizes[1].sizes = realloc (
              cameraInfo->frameSizes[1].sizes, ( numResolutions + 1 ) *
              sizeof ( FRAMESIZE )))) {
            return -OA_ERR_MEM_ALLOC;
          }
          cameraInfo->frameSizes[1].sizes[ numResolutions ].x = w;
          cameraInfo->frameSizes[1].sizes[ numResolutions ].y = h;
          if ( w > cameraInfo->xSize ) {
            cameraInfo->xSize = w;
            cameraInfo->ySize = h;
          }
          numResolutions++;
        }
      }
    }
  }

  if ( !numResolutions ) {
    fprintf ( stderr, "%s: no suitable resolutions found\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->frameSizes[1].numSizes = numResolutions;
  return OA_ERR_NONE;
}


static int
_processNonFormat7Modes ( oaCamera* camera, dc1394camera_t* iidcCam,
    dc1394video_modes_t videoModes )
{
  unsigned int          i, j, found, numResolutions, preferredFound;
  dc1394color_coding_t  codec;
  IIDC_STATE*		cameraInfo;

  numResolutions = preferredFound = 0;
  cameraInfo = camera->_private;

  for ( i = 0; i < videoModes.num; i++ ) {
    uint32_t w, h;
    if ( dc1394_get_image_size_from_video_mode ( iidcCam, videoModes.modes[i],
        &w, &h ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_get_image_size_from_video_mode failed",
          __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }
    found = 0;
    if ( numResolutions ) {
      for ( j = 0; j < numResolutions && !found; j++ ) {
        if ( cameraInfo->frameSizes[1].sizes[j].x == w &&
            cameraInfo->frameSizes[1].sizes[j].y == h ) {
          found = 1;
        }
      }
    }
    if ( !found ) {
      if (!(  cameraInfo->frameSizes[1].sizes = realloc (
          cameraInfo->frameSizes[1].sizes, ( numResolutions + 1 ) *
          sizeof ( FRAMESIZE )))) {
        return 0;
      }
      cameraInfo->frameSizes[1].sizes[ numResolutions ].x = w;
      cameraInfo->frameSizes[1].sizes[ numResolutions ].y = h;
      if ( w > cameraInfo->xSize ) {
        cameraInfo->xSize = w;
        cameraInfo->ySize = h;
      }
      numResolutions++;
    }

    if ( dc1394_get_color_coding_from_video_mode ( iidcCam,
        videoModes.modes[i], &codec ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_get_color_coding_from_video_mode failed",
          __FUNCTION__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    switch ( codec ) {
      case DC1394_COLOR_CODING_MONO8:
        if ( !cameraInfo->videoRGB24 ) {
          preferredFound = 1;
          cameraInfo->videoCurrent = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
        }
        // TIS colour cameras apparently offer MONO8 instead of
        // RAW8
        if ( cameraInfo->isTISColour ) {
          camera->features.rawMode = 1;
          cameraInfo->videoRaw = 1;
        } else {
          cameraInfo->videoGrey = 1;
        }
        break;

      case DC1394_COLOR_CODING_RGB8:
        preferredFound = 1;
        cameraInfo->videoCurrent = videoModes.modes[i];
        cameraInfo->currentCodec = codec;
        cameraInfo->videoRGB24 = 1;
        camera->features.demosaicMode = 1;
        break;

      case DC1394_COLOR_CODING_MONO16:
        if ( !preferredFound ) {
          preferredFound = 1;
          cameraInfo->videoCurrent = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
        }
        cameraInfo->videoGrey16 = 1;
        break;

      case DC1394_COLOR_CODING_RAW8:
        if ( !preferredFound ) {
          preferredFound = 1;
          cameraInfo->videoCurrent = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
        }
        camera->features.rawMode = 1;
        break;

      case DC1394_COLOR_CODING_YUV422:
        if ( !preferredFound ) {
          cameraInfo->videoCurrent = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
        }
        break;

      default:
        fprintf ( stderr, "%s: unhandled video codec %d\n", __FUNCTION__,
            codec );
        break;
    }
  }

  if ( !numResolutions ) {
    fprintf ( stderr, "%s: no suitable resolutions found\n", __FUNCTION__ );
    return -OA_ERR_OUT_OF_RANGE;
  }
  cameraInfo->frameSizes[1].numSizes = numResolutions;

  return OA_ERR_NONE;
}


int
oaIIDCCloseCamera ( oaCamera* camera )
{
  void*		dummy;
  IIDC_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
  
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    dc1394_camera_free ( cameraInfo->iidcHandle );

    if ( cameraInfo->frameRates.numRates ) {
     free (( void* ) cameraInfo->frameRates.rates );
    }
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

#endif	/* HAVE_LIBDC1394 */
