/*****************************************************************************
 *
 * SVBconnect.c -- Initialise SVBony cameras
 *
 * Copyright 2020
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

#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <openastro/video/formats.h>
#include <SVBCameraSDK.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "SVBoacam.h"
#include "SVBstate.h"
#include "SVBprivate.h"


static void _SVBInitFunctionPointers ( oaCamera* );

/**
 * Initialise a given camera device
 */

oaCamera*
oaSVBInitCamera ( oaCameraDevice* device )
{
  oaCamera*		camera;
  DEVICE_INFO*		devInfo;
  SVB_STATE*		cameraInfo;
  COMMON_INFO*		commonInfo;
  SVB_CAMERA_INFO	camInfo;
  SVB_CAMERA_PROPERTY	camProps;
  SVB_CONTROL_CAPS	controlCaps;
  int          		c, f, i, j, bin, multiplier, numControls;
  long			currentValue;
  SVB_BOOL		autoSetting;

  oacamDebugMsg ( DEBUG_CAM_INIT, "SVB: init: %s ()\n", __FUNCTION__ );

	if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
			sizeof ( SVB_STATE ), &commonInfo ) != OA_ERR_NONE ) {
		return 0;
	}

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  cameraInfo->index = -1;
  devInfo = device->_private;

  camera->interface = device->interface;
  cameraInfo->index = devInfo->devIndex;

  p_SVBGetCameraInfo ( &camInfo, cameraInfo->index );
  cameraInfo->cameraId = camInfo.CameraID;
  p_SVBGetCameraProperty ( camInfo.CameraID, &camProps );

  OA_CLEAR ( camera->controlType );
  OA_CLEAR ( camera->features );
  
  if ( p_SVBOpenCamera ( cameraInfo->cameraId )) {
    fprintf ( stderr, "open of camera %ld failed\n", cameraInfo->cameraId );
    FREE_DATA_STRUCTS;
    return 0;
  }

#if 0
  if ( p_SVBInitCamera ( cameraInfo->cameraId )) {
    fprintf ( stderr, "init of camera %ld failed\n", cameraInfo->cameraId );
    FREE_DATA_STRUCTS;
    return 0;
  }
#endif

  _SVBInitFunctionPointers ( camera );

  cameraInfo->runMode = CAM_RUN_MODE_STOPPED;

  if ( p_SVBGetNumOfControls ( cameraInfo->cameraId, &numControls )) {
    fprintf ( stderr, "%s: SVBGetNumOfControls returns error\n",
      __FUNCTION__ );
    FREE_DATA_STRUCTS;
    return 0;
  }

  for ( c = 0; c < numControls; c++ ) {
    if ( !p_SVBGetControlCaps ( cameraInfo->cameraId, c, &controlCaps )) {

      switch ( controlCaps.ControlType ) {

        case SVB_GAIN:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentGain = currentValue;
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_GAIN ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_GAIN ) = 0;
            commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_GAIN ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_GAIN ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_GAIN ) =
                cameraInfo->autoGain = autoSetting;
          }
          break;

        case SVB_EXPOSURE:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentAbsoluteExposure = currentValue;
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )
                = OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )
                = 0;
            commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )
                = 1;
            commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )
                = 1;
            commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )
                = cameraInfo->autoExposure = autoSetting;
          }
          break;

        case SVB_GAMMA:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAMMA ) = OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAMMA ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentGamma = currentValue;
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_GAMMA ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_GAMMA ) = 0;
            commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_GAMMA ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_GAMMA ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_GAMMA ) =
                cameraInfo->autoGamma = autoSetting;
          }
          break;

        case SVB_WB_R:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_RED_BALANCE ) =
              OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_RED_BALANCE ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentRedBalance = currentValue;
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_RED_BALANCE ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_RED_BALANCE ) = 0;
            commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_RED_BALANCE ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_RED_BALANCE ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_RED_BALANCE ) =
                cameraInfo->autoRedBalance = autoSetting;
          }
          break;

        case SVB_WB_B:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) =
              OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentBlueBalance = currentValue;
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_BLUE_BALANCE ) = 0;
            commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_BLUE_BALANCE ) =
                cameraInfo->autoBlueBalance = autoSetting;
          }
          break;

#if 0
        case SVB_BRIGHTNESS:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BRIGHTNESS ) =
              OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BRIGHTNESS ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BRIGHTNESS ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BRIGHTNESS ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentBrightness = currentValue;
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_BRIGHTNESS ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_BRIGHTNESS ) = 0;
            commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_BRIGHTNESS ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_BRIGHTNESS ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_BRIGHTNESS ) =
                cameraInfo->autoBrightness = autoSetting;
          }
          break;
#endif

#if 0
        case SVB_BANDWIDTHOVERLOAD:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_USBTRAFFIC ) =
              OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_USBTRAFFIC ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_USBTRAFFIC ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_USBTRAFFIC ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentUSBTraffic = currentValue;
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_USBTRAFFIC ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_USBTRAFFIC ) = 0;
            commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_USBTRAFFIC ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_USBTRAFFIC ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_USBTRAFFIC ) =
                cameraInfo->autoUSBTraffic = autoSetting;
          }
          break;
#endif

#if 0
        case SVB_OVERCLOCK:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_OVERCLOCK ) =
              OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_OVERCLOCK ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_OVERCLOCK ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_OVERCLOCK ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_OVERCLOCK ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentOverclock = currentValue;
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_OVERCLOCK ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_OVERCLOCK ) = 0;
            commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_OVERCLOCK ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_OVERCLOCK ) = 1;
            commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_OVERCLOCK ) =
                cameraInfo->autoOverclock = autoSetting;
          }
          break;
#endif

#if 0
        case SVB_HIGH_SPEED_MODE:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HIGHSPEED ) =
              OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HIGHSPEED ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HIGHSPEED ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HIGHSPEED ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentHighSpeed = currentValue;
          /*
          if ( controlCaps.IsAutoSupported ) {
            camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_AUTO_HIGHSPEED ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_AUTO_HIGHSPEED ) = 0;
            commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_AUTO_HIGHSPEED ) = 1;
            commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_AUTO_HIGHSPEED ) = 1;
            commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_AUTO_HIGHSPEED ) =
                cameraInfo->autoHighSpeed = autoSetting;
          }
          */
          cameraInfo->autoHighSpeed = 0;
          break;
#endif

        case SVB_FLIP:
          if ( controlCaps.MaxValue >= SVB_FLIP_HORIZ ) {
            camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HFLIP ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HFLIP ) = 0;
            commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HFLIP ) = 1;
            commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HFLIP ) = 1;
            commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HFLIP ) =
                cameraInfo->currentHFlip = 0;
          }
          if ( controlCaps.MaxValue >= SVB_FLIP_VERT ) {
            camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_VFLIP ) =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_VFLIP ) = 0;
            commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_VFLIP ) = 1;
            commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_VFLIP ) = 1;
            commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_VFLIP ) =
                cameraInfo->currentVFlip = 0;
          }
          break;

#if 0
        case SVB_COOLER_ON:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER ) =
                OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER ) =
                controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER ) =
                controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) =
                controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->coolerEnabled = currentValue;
          break;
#endif

#if 0
        case SVB_MONO_BIN:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
              OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
                controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
                controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_MONO_BIN_COLOUR ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->monoBinning = currentValue;
          break;
#endif

#if 0
        case SVB_FAN_ON:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FAN ) =
                OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_FAN ) = controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_FAN ) = controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_FAN ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN ) =
                controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->fanEnabled = currentValue;
          break;
#endif

#if 0
        case SVB_PATTERN_ADJUST:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_PATTERN_ADJUST ) =
                OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_PATTERN_ADJUST ) =
                controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_PATTERN_ADJUST ) =
                controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_PATTERN_ADJUST ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PATTERN_ADJUST ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->patternAdjust = currentValue;
          break;
#endif

#if 0
        case SVB_ANTI_DEW_HEATER:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DEW_HEATER ) =
                OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_DEW_HEATER ) =
                controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_DEW_HEATER ) =
                controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_DEW_HEATER ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_DEW_HEATER ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->dewHeater = currentValue;
          break;
#endif

#if 0
#if HAVE_DECL_SVB_AUTO_MAX_EXP_MS
        case SVB_AUTO_MAX_EXP_MS:
#else
        case SVB_AUTO_MAX_EXP:
#endif
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_MAX_AUTO_EXPOSURE ) =
              OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_MAX_AUTO_EXPOSURE ) =
              controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_MAX_AUTO_EXPOSURE ) =
              controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_MAX_AUTO_EXPOSURE ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MAX_AUTO_EXPOSURE ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentSetPoint = currentValue;
          break;
#endif

#if 0
        case SVB_TARGET_TEMP:
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMP_SETPOINT ) =
                OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TEMP_SETPOINT ) =
                controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TEMP_SETPOINT ) =
                controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TEMP_SETPOINT ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentSetPoint = currentValue;
          break;
#endif

#if 0
        case SVB_COOLER_POWER_PERC:
          // Ignore this one -- it's read-only anyhow
/*
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER_POWER ) =
                OA_CTRL_TYPE_INT32;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER_POWER ) =
                controlCaps.MinValue;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER_POWER ) =
                controlCaps.MaxValue;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER_POWER ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER_POWER ) =
              controlCaps.DefaultValue;
          p_SVBGetControlValue ( cameraInfo->cameraId, c, &currentValue,
              &autoSetting );
          cameraInfo->currentCoolerPower = currentValue;
*/
          break;
#endif

#if 0
        case SVB_AUTO_MAX_GAIN:
#endif
#if HAVE_DECL_SVB_AUTO_MAX_EXP_MS
        case SVB_AUTO_MAX_EXP:
#endif
        case SVB_AUTO_MAX_BRIGHTNESS:
#if 0
        case SVB_HARDWARE_BIN:
#endif
          fprintf ( stderr, "%s: control %s is not supported\n", __FUNCTION__,
              controlCaps.Name );
          break;

#if 0
        case SVB_TEMPERATURE:
          break; // handled elsewhere
#endif

        default:
          fprintf ( stderr, "%s: Unrecognised control '%s'\n", __FUNCTION__,
              controlCaps.Name );
          break;
      }
    }
  }

  cameraInfo->maxResolutionX = camProps.MaxWidth;
  cameraInfo->maxResolutionY = camProps.MaxHeight;

  memcpy ( cameraInfo->binModes, camProps.SupportedBins,
      sizeof ( camProps.SupportedBins ));
  i = 0;
  while (( bin = camProps.SupportedBins[i] )) {
    if ( 2 == bin || 3 == bin || 4 == bin ) {
      camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) = OA_CTRL_TYPE_DISCRETE;
    }
    i++;
  }
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED ) = OA_CTRL_TYPE_READONLY;

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) = OA_CTRL_TYPE_READONLY;

  camera->features.flags |= OA_CAM_FEATURE_ROI;
  camera->features.flags |= OA_CAM_FEATURE_RESET;
  camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;
  camera->features.flags |= OA_CAM_FEATURE_STREAMING;

  // Ok, now we need to find out what frame formats are supported and
  // which one we want to use

  cameraInfo->currentMode = -1;
  cameraInfo->colour = camProps.IsColorCam ? 1 : 0;
  cameraInfo->maxBitDepth = 8;

  i = 0;
  while (( f = camProps.SupportedVideoFormat[ i ]) != SVB_IMG_END ) {
    switch ( f ) {
      case SVB_IMG_RGB24:
        if ( cameraInfo->colour ) {
          camera->frameFormats[ OA_PIX_FMT_BGR24 ] = 1;
					camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
          cameraInfo->currentMode = f;
          cameraInfo->currentFormat = OA_PIX_FMT_BGR24;
          cameraInfo->maxBitDepth =
              oaFrameFormats[ OA_PIX_FMT_BGR24 ].bitsPerPixel;
        }
        break;
      case SVB_IMG_Y8:
      case SVB_IMG_RAW8:
        if ( cameraInfo->colour ) {
          switch ( camProps.BayerPattern ) {
            case SVB_BAYER_RG:
              camera->frameFormats[ OA_PIX_FMT_RGGB8 ] = 1;
              break;
            case SVB_BAYER_BG:
              camera->frameFormats[ OA_PIX_FMT_BGGR8 ] = 1;
              break;
            case SVB_BAYER_GR:
              camera->frameFormats[ OA_PIX_FMT_GRBG8 ] = 1;
              break;
            case SVB_BAYER_GB:
              camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
              break;
          }
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
        } else {
          camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
          cameraInfo->greyscaleMode = f;
          cameraInfo->currentMode = f;
          cameraInfo->currentFormat = OA_PIX_FMT_GREY8;
        }
        break;
      case SVB_IMG_RAW16:
        if ( cameraInfo->colour ) {
          switch ( camProps.BayerPattern ) {
            case SVB_BAYER_RG:
              camera->frameFormats[ OA_PIX_FMT_RGGB16LE ] = 1;
              break;
            case SVB_BAYER_BG:
              camera->frameFormats[ OA_PIX_FMT_BGGR16LE ] = 1;
              break;
            case SVB_BAYER_GR:
              camera->frameFormats[ OA_PIX_FMT_GRBG16LE ] = 1;
              break;
            case SVB_BAYER_GB:
              camera->frameFormats[ OA_PIX_FMT_GBRG16LE ] = 1;
              break;
          }
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
        } else {
          camera->frameFormats[ OA_PIX_FMT_GREY16LE ] = 1;
        }
        if ( cameraInfo->maxBitDepth < 16 ) {
          cameraInfo->maxBitDepth = 16;
        }
        break;
    }
    i++;
  }

  if ( -1 == cameraInfo->currentMode ) {
    fprintf ( stderr, "No suitable video format found on camera %ld\n",
        cameraInfo->index );
    FREE_DATA_STRUCTS;
    return 0;
  }

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;
  cameraInfo->binMode = OA_BIN_MODE_NONE;

  for ( i = 1; i <= OA_MAX_BINNING; i++ ) {
    cameraInfo->frameSizes[i].numSizes = 0;
    cameraInfo->frameSizes[i].sizes = 0;
  }

  if (!( cameraInfo->frameSizes[1].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    FREE_DATA_STRUCTS;
    return 0;
  }
  cameraInfo->frameSizes[1].sizes[0].x = cameraInfo->maxResolutionX;
  cameraInfo->frameSizes[1].sizes[0].y = cameraInfo->maxResolutionY;
  cameraInfo->frameSizes[1].numSizes = 1;
  // Fake up some resolutions for 2x binning
  if ( camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING )) {
    if (!( cameraInfo->frameSizes[2].sizes =
        ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
      fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      FREE_DATA_STRUCTS;
      return 0;
    }
    cameraInfo->frameSizes[2].sizes[0].x = cameraInfo->maxResolutionX / 2;
    cameraInfo->frameSizes[2].sizes[0].y = cameraInfo->maxResolutionY / 2;
    cameraInfo->frameSizes[2].numSizes = 1;
  }

  cameraInfo->xSize = cameraInfo->maxResolutionX;
  cameraInfo->ySize = cameraInfo->maxResolutionY;
  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;

  p_SVBSetROIFormat ( cameraInfo->cameraId, cameraInfo->xSize,
      cameraInfo->ySize, cameraInfo->binMode, cameraInfo->currentMode );

  // The largest buffer size we should need
  // RGB colour is 3 bytes per pixel, mono one for 8-bit, two for 16-bit,
  // RAW is one for 8-bit, 2 for 16-bit.
  multiplier = cameraInfo->maxBitDepth / 8;
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * multiplier;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( frameBuffer ));
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
      free (( void* ) cameraInfo->buffers );
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
      oacamSVBcontroller, ( void* ) camera )) {
    for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
      free (( void* ) cameraInfo->buffers[i].start );
    }
    for ( i = 1; i <= OA_MAX_BINNING; i++ ) {
      if ( cameraInfo->frameSizes[i].sizes )
        free (( void* ) cameraInfo->frameSizes[i].sizes );
    }
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    free (( void* ) cameraInfo->buffers );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamSVBcallbackHandler, ( void* ) camera )) {

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
    free (( void* ) cameraInfo->buffers );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    return 0;
  }

  return camera;
}


static void
_SVBInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaSVBInitCamera;
  camera->funcs.closeCamera = oaSVBCloseCamera;

  camera->funcs.testControl = oaSVBCameraTestControl;
  camera->funcs.getControlRange = oaSVBCameraGetControlRange;

  camera->funcs.setResolution = oaSVBCameraSetResolution;
  camera->funcs.setROI = oaSVBCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  //  camera->funcs.isAuto = oaIsAuto;

  camera->funcs.enumerateFrameSizes = oaSVBCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaSVBCameraGetFramePixelFormat;
  camera->funcs.testROISize = oaSVBCameraTestROISize;
}


int
oaSVBCloseCamera ( oaCamera* camera )
{
  int		j;
  void*		dummy;
  SVB_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    p_SVBCloseCamera ( cameraInfo->cameraId );

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
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );

    free (( void* ) cameraInfo->buffers );
		free (( void* ) cameraInfo );
		free (( void* ) camera->_common );
		free (( void* ) camera );

  } else {
   return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}
