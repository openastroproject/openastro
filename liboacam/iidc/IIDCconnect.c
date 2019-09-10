/*****************************************************************************
 *
 * IIDCconnect.c -- Initialise IEEE1394/IIDC cameras
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
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

#if HAVE_LIBDC1394

#include <dc1394/dc1394.h>
#include <pthread.h>
#include <openastro/camera.h>
#include <openastro/util.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "IIDCoacam.h"
#include "IIDCstate.h"
#include "IIDCprivate.h"


static void _IIDCInitFunctionPointers ( oaCamera* );
static int  _processFormat7Modes ( oaCamera*, dc1394camera_t*,
    dc1394video_modes_t );
static int  _processNonFormat7Modes ( oaCamera*, dc1394camera_t*,
    dc1394video_modes_t );

struct iidcCtrl dc1394Controls[] = {
  { DC1394_FEATURE_BRIGHTNESS, OA_CAM_CTRL_BRIGHTNESS },
  // The IIDC spec says this one is like contrast.  Who am I to argue?
  { DC1394_FEATURE_EXPOSURE, OA_CAM_CTRL_CONTRAST },
  { DC1394_FEATURE_SHARPNESS, OA_CAM_CTRL_SHARPNESS },
  { DC1394_FEATURE_WHITE_BALANCE, OA_CAM_CTRL_WHITE_BALANCE },
  { DC1394_FEATURE_HUE, OA_CAM_CTRL_HUE },
  { DC1394_FEATURE_SATURATION, OA_CAM_CTRL_SATURATION },
  { DC1394_FEATURE_GAMMA, OA_CAM_CTRL_GAMMA },
  { DC1394_FEATURE_SHUTTER, -1 }, // have to handle this separately
  { DC1394_FEATURE_GAIN, OA_CAM_CTRL_GAIN },
  { DC1394_FEATURE_IRIS, OA_CAM_CTRL_IRIS_ABSOLUTE },
  { DC1394_FEATURE_FOCUS, OA_CAM_CTRL_FOCUS_ABSOLUTE },
  { DC1394_FEATURE_TEMPERATURE, -1 }, // have to handle this separately
  { DC1394_FEATURE_TRIGGER, -1 }, // have to handle this separately
  { DC1394_FEATURE_TRIGGER_DELAY, -1 }, // have to handle this separately
  { DC1394_FEATURE_WHITE_SHADING, 0 },
  { DC1394_FEATURE_FRAME_RATE, -1 },
  { DC1394_FEATURE_ZOOM, OA_CAM_CTRL_ZOOM_ABSOLUTE },
  { DC1394_FEATURE_PAN, OA_CAM_CTRL_PAN_ABSOLUTE },
  { DC1394_FEATURE_TILT, OA_CAM_CTRL_TILT_RESET },
  { DC1394_FEATURE_OPTICAL_FILTER, 0 },
  { DC1394_FEATURE_CAPTURE_SIZE, 0 },
  { DC1394_FEATURE_CAPTURE_QUALITY, 0 }
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
  int                   oaControl, oaAutoControl, oaOnOffControl, use1394B;
  unsigned int		i;
  DEVICE_INFO*		devInfo;
  IIDC_STATE*		cameraInfo;
  COMMON_INFO*		commonInfo;
  dc1394framerates_t	framerates;

  if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
      sizeof ( IIDC_STATE ), &commonInfo ) != OA_ERR_NONE ) {
    return 0;
  }

  _IIDCInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  devInfo = device->_private;

  iidcContext = p_dc1394_new();
  if ( !iidcContext ) {
    fprintf ( stderr, "%s: Can't get IIDC context\n", __FUNCTION__ );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if (!( iidcCam = p_dc1394_camera_new_unit ( iidcContext, devInfo->guid,
      devInfo->unit ))) {
    fprintf ( stderr, "%s: dc1394_camera_new_unit failed\n", __FUNCTION__ );
    FREE_DATA_STRUCTS;
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

  if ( p_dc1394_feature_get_all ( iidcCam, &features ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_feature_get_all failed\n", __FUNCTION__ );
    FREE_DATA_STRUCTS;
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

  if ( p_dc1394_camera_reset ( iidcCam ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_camera_reset failed\n", __FUNCTION__ );
  }
  // Allow the camera reset a little time to happen.  Doesn't seem to cause
  // a problem on Linux, but makes OSX barf if we don't.
  usleep ( 60000 );


  // FIX ME
  // There's a lot of work still to be done here.  For most stuff I'm
  // ignoring absolute_capable because it uses completely different
  // ranges and units and everything becomes a nightmare :(
  // I'm also (mostly) ignoring readout and polarity

  for ( i = 0; i < DC1394_FEATURE_NUM; i++ ) {
    if ( features.feature[ i ].available ) {

      oaControl = dc1394Controls[ i ].oaControl;
      oaAutoControl = OA_CAM_CTRL_MODE_AUTO( oaControl );
      oaOnOffControl = OA_CAM_CTRL_MODE_ON_OFF( oaControl );

      cameraInfo->absoluteSupported[i] =
          features.feature[ i ].absolute_capable;

      switch ( i + DC1394_FEATURE_MIN ) {

        case DC1394_FEATURE_BRIGHTNESS:
        case DC1394_FEATURE_SHARPNESS:
        case DC1394_FEATURE_HUE:
        case DC1394_FEATURE_SATURATION:
        case DC1394_FEATURE_GAMMA:
        case DC1394_FEATURE_GAIN:
        case DC1394_FEATURE_EXPOSURE:
        case DC1394_FEATURE_IRIS:
        case DC1394_FEATURE_FOCUS:
        case DC1394_FEATURE_ZOOM:
        case DC1394_FEATURE_PAN:
        case DC1394_FEATURE_TILT:
          // FIX ME -- will need  to make sure the units are correct for focus,
          // pan and tilt in absolute mode.  See IIDC spec.
        {
          unsigned int j;

          for ( j = 0; j < features.feature[i].modes.num; j++ ) {
            switch ( features.feature[i].modes.modes[j] ) {

              case DC1394_FEATURE_MODE_MANUAL:
                camera->OA_CAM_CTRL_TYPE( oaControl ) = OA_CTRL_TYPE_INT64;
                commonInfo->OA_CAM_CTRL_MIN( oaControl ) =
                    features.feature[i].min;
                commonInfo->OA_CAM_CTRL_MAX( oaControl ) =
                    features.feature[i].max;
                commonInfo->OA_CAM_CTRL_STEP( oaControl ) = 1; // arbitrary
                commonInfo->OA_CAM_CTRL_DEF( oaControl ) =
                    features.feature[i].value;
                break;

              case DC1394_FEATURE_MODE_AUTO:
                camera->OA_CAM_CTRL_TYPE( oaAutoControl ) =
                    OA_CTRL_TYPE_BOOLEAN;
                commonInfo->OA_CAM_CTRL_MIN( oaAutoControl ) = 0;
                commonInfo->OA_CAM_CTRL_MAX( oaAutoControl ) = 1;
                commonInfo->OA_CAM_CTRL_STEP( oaAutoControl ) = 1;
                commonInfo->OA_CAM_CTRL_DEF( oaAutoControl ) = (
                    DC1394_FEATURE_MODE_AUTO ==
                    features.feature[i].current_mode ) ? 1 : 0;
                break;

              case DC1394_FEATURE_MODE_ONE_PUSH_AUTO:
                fprintf ( stderr, "%s: unhandled feature mode %d for %d\n",
                    __FUNCTION__, features.feature[i].modes.modes[j],
                    i + DC1394_FEATURE_MIN );
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
          // shutter is actually exposure time.  The "exposure" control is
          // apparently something else

          if ( features.feature[ i ].absolute_capable ) {
            oaAutoControl =
                OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
            oaOnOffControl =
                OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
            // have to set the control to use the absolute settings...
            if (( err = p_dc1394_feature_set_absolute_control ( iidcCam,
                DC1394_FEATURE_SHUTTER, DC1394_ON ) != DC1394_SUCCESS )) {
              fprintf ( stderr, "%s: dc1394_feature_set_absolute_control "
                  "failed, err: %d\n", __FUNCTION__, err );
              FREE_DATA_STRUCTS;
              return 0;
            }
            // and disable auto mode, as we don't support that at the moment
            // FIX ME -- need to deal with this when we do -- auto mode in
            // IIDC cameras may be more than just on/off
            if ( p_dc1394_feature_set_mode ( iidcCam, DC1394_FEATURE_SHUTTER,
                DC1394_FEATURE_MODE_MANUAL ) != DC1394_SUCCESS ) {
              fprintf ( stderr, "%s: dc1394_feature_set_mode failed for "
                  "shutter speed\n", __FUNCTION__ );
              FREE_DATA_STRUCTS;
              return 0;
            }

            oaControl = OA_CAM_CTRL_EXPOSURE_ABSOLUTE;
            min = features.feature[ i ].abs_min * 1000000.0;
            max = features.feature[ i ].abs_max * 1000000.0;
            step = 1000; // arbitrary
            def = features.feature[ i ].abs_value * 1000000.0;
          } else {
            oaAutoControl =
                OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED );
            oaOnOffControl =
                OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_EXPOSURE_UNSCALED );
            oaControl = OA_CAM_CTRL_EXPOSURE_UNSCALED;
            min = features.feature[i].min;
            max = features.feature[i].max;
            step = 1; // arbitrary
            def = features.feature[ i ].value;
          }
          camera->OA_CAM_CTRL_TYPE( oaControl ) = OA_CTRL_TYPE_INT64;
          commonInfo->OA_CAM_CTRL_MIN( oaControl ) = min;
          commonInfo->OA_CAM_CTRL_MAX( oaControl ) = max;
          commonInfo->OA_CAM_CTRL_STEP( oaControl ) = step;
          commonInfo->OA_CAM_CTRL_DEF( oaControl ) = def;
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

          oaOnOffControl = OA_CAM_CTRL_MODE_ON_OFF( oaControl );
          for ( j = 0; j < features.feature[i].modes.num; j++ ) {
            switch ( features.feature[i].modes.modes[j] ) {

              case DC1394_FEATURE_MODE_MANUAL:
                camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) =
                    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_RED_BALANCE ) =
                    OA_CTRL_TYPE_INT32;
                commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE ) =
                    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE ) =
                    features.feature[i].min;
                commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE ) =
                    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE ) =
                    features.feature[i].max;
                commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLUE_BALANCE ) =
                    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_RED_BALANCE ) =
                    1;//arbitrary
                commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) =
                    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) =
                    cameraInfo->currentRedBalance =
                    cameraInfo->currentBlueBalance =
                    features.feature[i].value;
                break;

              case DC1394_FEATURE_MODE_AUTO:
                camera->OA_CAM_CTRL_TYPE( oaAutoControl ) =
                    OA_CTRL_TYPE_BOOLEAN;
                commonInfo->OA_CAM_CTRL_MIN( oaAutoControl ) = 0;
                commonInfo->OA_CAM_CTRL_MAX( oaAutoControl ) = 1;
                commonInfo->OA_CAM_CTRL_STEP( oaAutoControl ) = 1;
                commonInfo->OA_CAM_CTRL_DEF( oaAutoControl ) = (
                    DC1394_FEATURE_MODE_AUTO ==
                    features.feature[i].current_mode ) ? 1 : 0;
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
          oaOnOffControl = 0;
          break;

        case DC1394_FEATURE_TEMPERATURE:
        {
          // This feature actually allows reading of the camera temperature
          // and setting a temperature set-point for cooling.  In manual
          // mode you get permanent cooling, in auto mode it's cooling based
          // on the set-point.

          // we can always read the temperature

          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) =
              OA_CTRL_TYPE_READONLY;

          // if manual or auto modes exist we can enable OA_CAM_CTRL_COOLER as
          // a boolean.  if auto mode exists we can additionally have
          // OA_CAM_CTRL_TEMP_SETPOINT

          unsigned int j;
          cameraInfo->haveSetpointCooling = 0;
          for ( j = 0; j < features.feature[i].modes.num; j++ ) {
            switch ( features.feature[i].modes.modes[j] ) {

              case DC1394_FEATURE_MODE_MANUAL:
                oaOnOffControl = 0; // don't want this separately
                camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER ) =
                    OA_CTRL_TYPE_BOOLEAN;
                commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER ) = 0;
                commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER ) = 1;
                commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER ) = 1;
                commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) = 0;
                break;

              case DC1394_FEATURE_MODE_AUTO:
                oaOnOffControl = 0; // don't want this separately
                cameraInfo->haveSetpointCooling = 1;
                camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER ) =
                    OA_CTRL_TYPE_BOOLEAN;
                commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER ) = 0;
                commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER ) = 1;
                commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER ) = 1;
                commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) = 0;

                camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMP_SETPOINT ) =
                    OA_CTRL_TYPE_INT32; 
                commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TEMP_SETPOINT ) =
                    features.feature[i].min;
                commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TEMP_SETPOINT ) =
                    features.feature[i].max;
                commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TEMP_SETPOINT ) = 1;
                commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) =
                    features.feature[i].value;
                break;

              default:
                fprintf ( stderr, "%s: unhandled feature mode %d for %d\n",
                    __FUNCTION__, features.feature[i].modes.modes[j],
                    i + DC1394_FEATURE_MIN );
                break;
            }
          }
          break;
        }

        case DC1394_FEATURE_TRIGGER:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_MODE ) =
              OA_CTRL_TYPE_MENU;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_MODE ) = 0;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_MODE ) = 5;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_MODE ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_MODE ) =
              cameraInfo->triggerMode = features.feature[i].value;

          if ( features.feature[ i ].on_off_capable ) {
            camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_ENABLE ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_ENABLE ) = 0;
            commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_ENABLE ) = 1;
            commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_ENABLE ) = 1;
            commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_ENABLE ) =
                cameraInfo->triggerEnable =
                ( features.feature[ i ].is_on ) ? 1 : 0;
          }
          oaOnOffControl = 0;

          if ( features.feature[ i ].polarity_capable ) {
            camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_POLARITY ) =
                OA_CTRL_TYPE_MENU;
            commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_POLARITY ) = 0;
            commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_POLARITY ) = 1;
            commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_POLARITY ) = 1;
            commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_POLARITY ) =
                cameraInfo->triggerPolarity =
                ( features.feature[ i ].trigger_polarity ) ? 1 : 0;
          }
          break;

        case DC1394_FEATURE_TRIGGER_DELAY:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_DELAY ) =
              OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_DELAY ) =
              features.feature[i].min;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_DELAY ) =
              features.feature[i].max;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_DELAY ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_DELAY ) =
              cameraInfo->triggerDelay = features.feature[i].value;

          if ( features.feature[ i ].on_off_capable ) {
            camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) = 0;
            commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) = 1;
            commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) = 1;
            commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE ) =
                cameraInfo->triggerDelayEnable =
                ( features.feature[ i ].is_on ) ? 1 : 0;
          }
          oaOnOffControl = 0;
          break;

        case DC1394_FEATURE_WHITE_SHADING: // it's really not clear how these
        case DC1394_FEATURE_OPTICAL_FILTER:// two should work

        case DC1394_FEATURE_CAPTURE_SIZE: // only relevant for format6
        case DC1394_FEATURE_CAPTURE_QUALITY: // only relevant for format6
          oaOnOffControl = 0;
          fprintf ( stderr, "%s: unsupported IIDC control %d\n", __FUNCTION__,
              i + DC1394_FEATURE_MIN );
          break;

        default:
          oaOnOffControl = 0;
          fprintf ( stderr, "%s: unknown IIDC control %d\n", __FUNCTION__,
              i + DC1394_FEATURE_MIN );
          break;
      }

      if ( oaOnOffControl && features.feature[ i ].on_off_capable ) {
        camera->OA_CAM_CTRL_TYPE( oaOnOffControl ) = OA_CTRL_TYPE_BOOLEAN;
        commonInfo->OA_CAM_CTRL_MIN( oaOnOffControl ) = 0;
        commonInfo->OA_CAM_CTRL_MAX( oaOnOffControl ) = 1;
        commonInfo->OA_CAM_CTRL_STEP( oaOnOffControl ) = 1;
        commonInfo->OA_CAM_CTRL_DEF( oaOnOffControl ) =
            ( features.feature[ i ].is_on ) ? 1 : 0;
      }
    }
  }

  // Looking at the specs it appears that a number of compressed video
  // modes are supported (YUV422 etc.) as well as RGB and 8-bit or 16-bit
  // mono should
  // be supported.  For the moment we'll ignore YUY2 and assume the camera
  // is colour if RGB shows up, monochrome otherwise.

  if ( p_dc1394_video_get_supported_modes ( iidcCam, &videoModes ) !=
      DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_video_get_supported_modes failed",
        __FUNCTION__ );
    FREE_DATA_STRUCTS;
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

  cameraInfo->currentIIDCMode = 0;
  cameraInfo->currentCodec = 0;

  camera->features.flags |= OA_CAM_FEATURE_RESET;
  camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;
  camera->features.flags |= OA_CAM_FEATURE_STREAMING;

  if ( cameraInfo->haveFormat7 ) {
    if ( _processFormat7Modes ( camera, iidcCam, videoModes )) {
      // try non format7 modes if there aren't any we can use
      cameraInfo->haveFormat7 = 0;
    }
  }
  if ( !cameraInfo->haveFormat7 ) {
    if ( _processNonFormat7Modes ( camera, iidcCam, videoModes )) {
      FREE_DATA_STRUCTS;
      return 0;
    }
  }

  if ( !cameraInfo->currentIIDCMode ) {
    fprintf ( stderr, "%s: No suitable video format found", __FUNCTION__ );
		free (( void* ) cameraInfo->frameSizes[1].sizes );
    FREE_DATA_STRUCTS;
    return 0;
  }

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

  if ( !cameraInfo->haveFormat7 ) {
    if ( p_dc1394_video_get_supported_framerates ( iidcCam,
        cameraInfo->currentIIDCMode, &framerates ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_get_supported_framerates failed\n",
         __FUNCTION__ );
    }
    if ( framerates.num > 1 ) {
			camera->features.flags |= OA_CAM_FEATURE_FRAME_RATES;
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
		free (( void* ) cameraInfo->frameSizes[1].sizes );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamIIDCcallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
		free (( void* ) cameraInfo->frameSizes[1].sizes );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  // This is a nasty, nasty hack, but it's the only way I can find to
  // tell if we're actually connected to a USB camera.  We need to know
  // this because some USB cameras claim to support 1394B and then hang
  // when you attempt to set speed 800.

  use1394B = 1;
  dc1394bool_t pwr;
  if ( p_dc1394_camera_get_broadcast ( iidcCam, &pwr ) != DC1394_SUCCESS ) {
    use1394B = -1;
  }

  if ( use1394B >= 0 ) {
    if ( p_dc1394_video_set_operation_mode ( iidcCam,
        ( iidcCam->bmode_capable == DC1394_TRUE ) ?
        DC1394_OPERATION_MODE_1394B : DC1394_OPERATION_MODE_LEGACY) !=
        DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_set_operation_mode failed\n",
          __FUNCTION__ );
			free (( void* ) cameraInfo->frameSizes[1].sizes );
      FREE_DATA_STRUCTS;
      return 0;
    }
    if ( p_dc1394_video_set_iso_speed ( iidcCam,
        ( iidcCam->bmode_capable == DC1394_TRUE ) ?
        DC1394_ISO_SPEED_800 : DC1394_ISO_SPEED_400 ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_set_iso_speed failed\n",
          __FUNCTION__ );
    }
  }

  /*
  if ( p_dc1394_video_set_mode ( iidcCam, cameraInfo->currentIIDCMode ) !=
      DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_video_set_mode failed\n", __FUNCTION__ );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( p_dc1394_capture_setup ( iidcCam, OA_CAM_BUFFERS,
      DC1394_CAPTURE_FLAGS_DEFAULT ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_capture_setup failed\n", __FUNCTION__ );
    FREE_DATA_STRUCTS;
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

  camera->funcs.getMenuString = oaIIDCCameraGetMenuString;
}


static int
_processFormat7Modes ( oaCamera* camera, dc1394camera_t* iidcCam,
    dc1394video_modes_t videoModes )
{
  dc1394format7modeset_t	modeList;
  dc1394color_coding_t		coding;
  unsigned int			i, j, addResolution;
  unsigned int			numResolutions, rawModeFound, rgbModeFound;
  IIDC_STATE*			cameraInfo;
	void*						tmpPtr;

  if ( p_dc1394_format7_get_modeset ( iidcCam, &modeList ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_format7_get_modeset return error\n",
        __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }

  numResolutions = 0;
  rawModeFound = rgbModeFound = 0;
  cameraInfo = camera->_private;

  for ( i = 0; i < DC1394_VIDEO_MODE_FORMAT7_NUM; i++ ) {
    if ( modeList.mode[i].present ) {
      addResolution = 0;
      for ( j = 0; j < modeList.mode[i].color_codings.num; j++ ) {
        coding = modeList.mode[i].color_codings.codings[j];

        switch ( coding ) {
          case DC1394_COLOR_CODING_MONO8:
            if ( !rawModeFound && !rgbModeFound ) {
              cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              cameraInfo->currentFrameFormat = ( cameraInfo->isTISColour ) ?
                  OA_PIX_FMT_GBRG8 : OA_PIX_FMT_GREY8;
            }
            if ( cameraInfo->isTISColour ) {
              // TIS colour cameras apparently offer MONO8 instead of RAW8
              camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
              rawModeFound = 1;
            } else {
              camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
            }
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_YUV411:
            if ( !rgbModeFound ) {
              cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              cameraInfo->currentFrameFormat = OA_PIX_FMT_YUV411;
            }
            // 411 is not planar.  If we have planar then YUV411P is required
            camera->frameFormats[ OA_PIX_FMT_YUV411 ] = 1;
            addResolution = 1;
            break;


          case DC1394_COLOR_CODING_YUV422:
            if ( !rgbModeFound ) {
              cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              cameraInfo->currentFrameFormat = OA_PIX_FMT_YUYV;
            }
            // YUYV is not planar.  If we have planar then YUV422P is required
            camera->frameFormats[ OA_PIX_FMT_YUYV ] = 1;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_YUV444:
            if ( !rgbModeFound ) {
              cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              cameraInfo->currentFrameFormat = OA_PIX_FMT_YUV444;
            }
            // 444 is not planar.  If we have planar then YUV444P is required
            camera->frameFormats[ OA_PIX_FMT_YUV444 ] = 1;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_RGB8:
            cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
            cameraInfo->currentCodec = coding;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_RGB24;
            rgbModeFound = 1;
            camera->frameFormats[ OA_PIX_FMT_RGB24 ] = 1;
						camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_MONO16:
            if ( !rawModeFound && !rgbModeFound &&
                !camera->frameFormats[ OA_PIX_FMT_GREY8 ]) {
              cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY16LE;
            }
            // little endian is a guess here
            camera->frameFormats[ OA_PIX_FMT_GREY16LE ] = 1;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_RGB16:
            if ( !rgbModeFound ) {
              cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              cameraInfo->currentFrameFormat = OA_PIX_FMT_RGB48LE;
              rgbModeFound = 1;
            }
            // another little-endian guess
            camera->frameFormats[ OA_PIX_FMT_RGB48LE ] = 1;
						camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_RAW8:
            if ( !rgbModeFound ) {
              cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              cameraInfo->currentFrameFormat = OA_PIX_FMT_GBRG8;
            }
            // This is also a guess.  Could be GRBG, RGGB or BGGR
            camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
						camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
            rawModeFound = 1;
            addResolution = 1;
            break;

          case DC1394_COLOR_CODING_RAW16:
            if ( !rgbModeFound && !rawModeFound ) {
              cameraInfo->currentIIDCMode = DC1394_VIDEO_MODE_FORMAT7_MIN + i;
              cameraInfo->currentCodec = coding;
              cameraInfo->currentFrameFormat = OA_PIX_FMT_GBRG16LE;
            }
            // This is also a guess.  Could be GRBG, RGGB or BGGR, and could
            // be big-endian
            camera->frameFormats[ OA_PIX_FMT_GBRG16LE ] = 1;
						camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
            rawModeFound = 1;
            addResolution = 1;
            break;

          default:
            fprintf ( stderr, "%s: unhandled format7 video coding %d\n",
                __FUNCTION__, coding );
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
          if (!( tmpPtr = realloc ( cameraInfo->frameSizes[1].sizes,
								( numResolutions + 1 ) * sizeof ( FRAMESIZE )))) {
						if ( cameraInfo->frameSizes[1].sizes ) {
							free (( void* ) cameraInfo->frameSizes[1].sizes );
						}
            return -OA_ERR_MEM_ALLOC;
          }
					cameraInfo->frameSizes[1].sizes = tmpPtr;
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
	camera->features.flags |= OA_CAM_FEATURE_FIXED_FRAME_SIZES;
  return OA_ERR_NONE;
}


static int
_processNonFormat7Modes ( oaCamera* camera, dc1394camera_t* iidcCam,
    dc1394video_modes_t videoModes )
{
  unsigned int          i, j, found, numResolutions;
  unsigned int          rawModeFound, rgbModeFound;
  dc1394color_coding_t  codec;
  IIDC_STATE*		cameraInfo;
	void*					tmpPtr;

  numResolutions = 0;
  rawModeFound = rgbModeFound = 0;
  cameraInfo = camera->_private;

  for ( i = 0; i < videoModes.num; i++ ) {
    uint32_t w, h;
    if ( p_dc1394_get_image_size_from_video_mode ( iidcCam, videoModes.modes[i],
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
      if (!( tmpPtr = realloc ( cameraInfo->frameSizes[1].sizes,
					( numResolutions + 1 ) * sizeof ( FRAMESIZE )))) {
				if ( cameraInfo->frameSizes[1].sizes ) {
					free (( void* ) cameraInfo->frameSizes[1].sizes );
				}
        return -OA_ERR_MEM_ALLOC;
      }
			cameraInfo->frameSizes[1].sizes = tmpPtr;
      cameraInfo->frameSizes[1].sizes[ numResolutions ].x = w;
      cameraInfo->frameSizes[1].sizes[ numResolutions ].y = h;
      if ( w > cameraInfo->xSize ) {
        cameraInfo->xSize = w;
        cameraInfo->ySize = h;
      }
      numResolutions++;
    }

    if ( p_dc1394_get_color_coding_from_video_mode ( iidcCam,
        videoModes.modes[i], &codec ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_get_color_coding_from_video_mode failed",
          __FUNCTION__ );
      return -OA_ERR_SYSTEM_ERROR;
    }

    switch ( codec ) {
      case DC1394_COLOR_CODING_MONO8:
        if ( !rawModeFound && !rgbModeFound ) {
          cameraInfo->currentIIDCMode = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
          cameraInfo->currentFrameFormat = ( cameraInfo->isTISColour ) ?
              OA_PIX_FMT_GBRG8 : OA_PIX_FMT_GREY8;
        }
        if ( cameraInfo->isTISColour ) {
          // TIS colour cameras apparently offer MONO8 instead of RAW8
          camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
          rawModeFound = 1;
        } else {
          camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
        }
        break;

      case DC1394_COLOR_CODING_YUV411:
        if ( !rgbModeFound ) {
          cameraInfo->currentIIDCMode = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
          cameraInfo->currentFrameFormat = OA_PIX_FMT_YUV411;
        }
        // 411 is not planar.  If we have planar then YUV411P is required
        camera->frameFormats[ OA_PIX_FMT_YUV411 ] = 1;
        break;


      case DC1394_COLOR_CODING_YUV422:
        if ( !rgbModeFound ) {
          cameraInfo->currentIIDCMode = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
          cameraInfo->currentFrameFormat = OA_PIX_FMT_YUYV;
        }
        // YUYV is not planar.  If we have planar then YUV422P is required
        camera->frameFormats[ OA_PIX_FMT_YUYV ] = 1;
        break;

      case DC1394_COLOR_CODING_YUV444:
        if ( !rgbModeFound ) {
          cameraInfo->currentIIDCMode = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
          cameraInfo->currentFrameFormat = OA_PIX_FMT_YUV444;
        }
        // 444 is not planar.  If we have planar then YUV444P is required
        camera->frameFormats[ OA_PIX_FMT_YUV444 ] = 1;
        break;

      case DC1394_COLOR_CODING_RGB8:
        cameraInfo->currentIIDCMode = videoModes.modes[i];
        cameraInfo->currentCodec = codec;
        rgbModeFound = 1;
        camera->frameFormats[ OA_PIX_FMT_RGB24 ] = 1;
				camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
        break;

      case DC1394_COLOR_CODING_MONO16:
        if ( !rawModeFound && !rgbModeFound &&
            !camera->frameFormats[ OA_PIX_FMT_GREY8 ]) {
          cameraInfo->currentIIDCMode = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
          cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY16LE;
        }
        // little endian is a guess here
        camera->frameFormats[ OA_PIX_FMT_GREY16LE ] = 1;
        break;

      case DC1394_COLOR_CODING_RGB16:
        if ( !rgbModeFound ) {
          cameraInfo->currentIIDCMode = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
          cameraInfo->currentFrameFormat = OA_PIX_FMT_RGB48LE;
          rgbModeFound = 1;
        }
        // another little-endian guess
        camera->frameFormats[ OA_PIX_FMT_RGB48LE ] = 1;
				camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
        break;

      case DC1394_COLOR_CODING_RAW8:
        if ( !rgbModeFound ) {
          cameraInfo->currentIIDCMode = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
          cameraInfo->currentFrameFormat = OA_PIX_FMT_GBRG8;
        }
        // This is also a guess.  Could be GRBG, RGGB or BGGR
        camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
				camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
        rawModeFound = 1;
        break;

      case DC1394_COLOR_CODING_RAW16:
        if ( !rgbModeFound && !rawModeFound ) {
          cameraInfo->currentIIDCMode = videoModes.modes[i];
          cameraInfo->currentCodec = codec;
          cameraInfo->currentFrameFormat = OA_PIX_FMT_GBRG16LE;
        }
        // This is also a guess.  Could be GRBG, RGGB or BGGR, and could
        // be big-endian
        camera->frameFormats[ OA_PIX_FMT_GBRG16LE ] = 1;
				camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
        rawModeFound = 1;
        break;

      default:
        fprintf ( stderr, "%s: unhandled non-format7 video codec %d\n",
            __FUNCTION__, codec );
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

    p_dc1394_camera_free ( cameraInfo->iidcHandle );

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
