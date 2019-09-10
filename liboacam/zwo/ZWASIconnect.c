/*****************************************************************************
 *
 * ZWASIconnect.c -- Initialise ZW ASI cameras
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

#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <ASICamera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "ZWASI.h"
#include "ZWASIoacam.h"
#include "ZWASIstate.h"


static void _ZWASIInitFunctionPointers ( oaCamera* );

/**
 * Initialise a given camera device
 */

oaCamera*
oaZWASIInitCamera ( oaCameraDevice* device )
{
  oaCamera*		camera;
  DEVICE_INFO*		devInfo;
  ZWASI_STATE*		cameraInfo;
  COMMON_INFO*		commonInfo;
  int          		i, j, multiplier;

  oacamDebugMsg ( DEBUG_CAM_INIT, "ZWASI: init: %s ()\n", __FUNCTION__ );

	if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
			sizeof ( ZWASI_STATE ), &commonInfo ) != OA_ERR_NONE ) {
		return 0;
	}

  camera->_private = cameraInfo;
  camera->_common = commonInfo;

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  cameraInfo->index = -1;
  devInfo = device->_private;

  if ( !openCamera ( devInfo->devIndex )) {
    fprintf ( stderr, "open of camera %ld failed\n", devInfo->devIndex );
    FREE_DATA_STRUCTS;
    return 0;
  }

  camera->interface = device->interface;
  cameraInfo->index = devInfo->devIndex;
  cameraInfo->cameraType = devInfo->devType;

  OA_CLEAR ( camera->controlType );
  OA_CLEAR ( camera->features );
  _ZWASIInitFunctionPointers ( camera );

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  // FIX ME -- should check from scratch for the genuine default values
  // as the current code isn't reliable for warm-started cameras.  Perhaps
  // a table of per-camera default values?

  // bool here comes from the ASI header file
  if ( isAvailable ( CONTROL_GAIN )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) = getMin ( CONTROL_GAIN );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) = getMax ( CONTROL_GAIN );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAIN ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) = getValue ( CONTROL_GAIN,
        &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) =
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAIN );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAIN );
    }
    cameraInfo->currentGain = commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAIN );
    if ( isAutoSupported ( CONTROL_GAIN )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_GAIN ) = OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_GAIN ) = 0;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_GAIN ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_GAIN ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_GAIN ) =
          cameraInfo->autoGain = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_EXPOSURE )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
        OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
        getMin ( CONTROL_EXPOSURE );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
        getMax ( CONTROL_EXPOSURE );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
        getValue ( CONTROL_EXPOSURE, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
    }
    cameraInfo->currentAbsoluteExposure =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
    if ( isAutoSupported ( CONTROL_EXPOSURE )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
          OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 0;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
          cameraInfo->autoExposure = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_GAMMA )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_GAMMA ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA ) = getMin ( CONTROL_GAMMA );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA ) = getMax ( CONTROL_GAMMA );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_GAMMA ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) = getValue ( CONTROL_GAMMA,
        &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) =
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_GAMMA );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_GAMMA );
    }
    cameraInfo->currentGamma = commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_GAMMA );
    if ( isAutoSupported ( CONTROL_GAMMA )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_GAMMA ) =
          OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_GAMMA ) = 0;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_GAMMA ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_GAMMA ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_GAMMA ) =
          cameraInfo->autoGamma = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_WB_R )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_RED_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE ) =
        getMin ( CONTROL_WB_R );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE ) =
        getMax ( CONTROL_WB_R );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_RED_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) =
        getValue ( CONTROL_WB_R, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) =
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE );
    }
    cameraInfo->currentRedBalance =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE );
    if ( isAutoSupported ( CONTROL_WB_R )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_RED_BALANCE ) =
          OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_RED_BALANCE ) = 0;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_RED_BALANCE ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_RED_BALANCE ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_RED_BALANCE ) =
          cameraInfo->autoRedBalance = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_WB_B )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE ) =
        getMin ( CONTROL_WB_B );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE ) =
        getMax ( CONTROL_WB_B );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) =
        getValue ( CONTROL_WB_B, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) =
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE );
    }
    cameraInfo->currentBlueBalance =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE );
    if ( isAutoSupported ( CONTROL_WB_B )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) =
        OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_BLUE_BALANCE ) = 0;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_BLUE_BALANCE ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_BLUE_BALANCE ) =
          cameraInfo->autoBlueBalance = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_BRIGHTNESS )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BRIGHTNESS ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BRIGHTNESS ) =
        getMin ( CONTROL_BRIGHTNESS );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BRIGHTNESS ) =
        getMax ( CONTROL_BRIGHTNESS );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BRIGHTNESS ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) =
        getValue ( CONTROL_BRIGHTNESS, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BRIGHTNESS ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BRIGHTNESS );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BRIGHTNESS ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BRIGHTNESS );
    }
    cameraInfo->currentBrightness =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BRIGHTNESS );
    if ( isAutoSupported ( CONTROL_BRIGHTNESS )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_BRIGHTNESS ) =
          OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_BRIGHTNESS ) = 0;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_BRIGHTNESS ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_BRIGHTNESS ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_BRIGHTNESS ) =
          cameraInfo->autoBrightness = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_BANDWIDTHOVERLOAD )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_USBTRAFFIC ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_USBTRAFFIC ) =
        getMin ( CONTROL_BANDWIDTHOVERLOAD );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_USBTRAFFIC ) =
        getMax ( CONTROL_BANDWIDTHOVERLOAD );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_USBTRAFFIC ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC ) =
        getValue ( CONTROL_BANDWIDTHOVERLOAD, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_USBTRAFFIC ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_USBTRAFFIC );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_USBTRAFFIC ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_USBTRAFFIC );
    }
    cameraInfo->currentUSBTraffic =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_USBTRAFFIC );
    if ( isAutoSupported ( CONTROL_BANDWIDTHOVERLOAD )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_USBTRAFFIC ) =
          OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_USBTRAFFIC ) = 0;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_USBTRAFFIC ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_USBTRAFFIC ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_USBTRAFFIC ) =
          cameraInfo->autoUSBTraffic = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_OVERCLOCK )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_OVERCLOCK ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_OVERCLOCK ) =
        getMin ( CONTROL_OVERCLOCK );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_OVERCLOCK ) =
        getMax ( CONTROL_OVERCLOCK );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_OVERCLOCK ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_OVERCLOCK ) =
        getValue ( CONTROL_OVERCLOCK, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_OVERCLOCK ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_OVERCLOCK ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_OVERCLOCK ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_OVERCLOCK );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_OVERCLOCK ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_OVERCLOCK ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_OVERCLOCK ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_OVERCLOCK );
    }
    cameraInfo->currentOverclock =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_OVERCLOCK );
    if ( isAutoSupported ( CONTROL_OVERCLOCK )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_OVERCLOCK ) =
          OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_OVERCLOCK ) = 0;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_OVERCLOCK ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_OVERCLOCK ) = 1;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_OVERCLOCK ) =
          cameraInfo->autoOverclock = autoEnabled;
    }
  }

  if ( isAvailable ( CONTROL_HIGHSPEED )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_HIGHSPEED ) = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HIGHSPEED ) =
        getMin ( CONTROL_HIGHSPEED );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HIGHSPEED ) =
        getMax ( CONTROL_HIGHSPEED );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_HIGHSPEED ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) =
        getValue ( CONTROL_HIGHSPEED, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HIGHSPEED ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_HIGHSPEED );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HIGHSPEED ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_HIGHSPEED );
    }
    cameraInfo->currentHighSpeed =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_HIGHSPEED );
    /*
    if ( isAutoSupported ( CONTROL_HIGHSPEED )) {
      camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_AUTO_HIGHSPEED ) =
          OA_CTRL_TYPE_BOOLEAN;
      commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_AUTO_HIGHSPEED ) = 0;
      commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_AUTO_HIGHSPEED ) = 1;
      commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_AUTO_HIGHSPEED ) = 1;
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_AUTO_HIGHSPEED ) =
          cameraInfo->autoHighSpeed = autoEnabled;
    }
    */
    cameraInfo->autoHighSpeed = 0;
  }

  if ( isAvailable ( CONTROL_COOLER_ON )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER ) = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER ) =
        getMin ( CONTROL_COOLER_ON );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER ) =
        getMax ( CONTROL_COOLER_ON );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) =
        getValue ( CONTROL_COOLER_ON, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER );
    }
    cameraInfo->coolerEnabled =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER );
  }

  if ( isAvailable ( CONTROL_MONO_BIN )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
        OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
        getMin ( CONTROL_MONO_BIN );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
        getMax ( CONTROL_MONO_BIN );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_MONO_BIN_COLOUR ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
        getValue ( CONTROL_MONO_BIN, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MONO_BIN_COLOUR ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_MONO_BIN_COLOUR ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_MONO_BIN_COLOUR );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MONO_BIN_COLOUR ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_MONO_BIN_COLOUR ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MONO_BIN_COLOUR ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_MONO_BIN_COLOUR );
    }
    cameraInfo->monoBinning =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MONO_BIN_COLOUR );
  }

  if ( isAvailable ( CONTROL_FAN_ON )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FAN ) = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_FAN ) =
        getMin ( CONTROL_FAN_ON );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_FAN ) =
        getMax ( CONTROL_FAN_ON );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_FAN ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN ) =
        getValue ( CONTROL_FAN_ON, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_FAN ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_FAN );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_FAN ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_FAN );
    }
    cameraInfo->fanEnabled = commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FAN );
  }

  if ( isAvailable ( CONTROL_PATTERN_ADJUST )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_PATTERN_ADJUST ) =
        OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_PATTERN_ADJUST ) =
        getMin ( CONTROL_PATTERN_ADJUST );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_PATTERN_ADJUST ) =
        getMax ( CONTROL_PATTERN_ADJUST );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_PATTERN_ADJUST ) = 1; 
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PATTERN_ADJUST ) =
        getValue ( CONTROL_PATTERN_ADJUST, &autoEnabled ); 
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PATTERN_ADJUST ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_PATTERN_ADJUST ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PATTERN_ADJUST ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_PATTERN_ADJUST ); 
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PATTERN_ADJUST ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_PATTERN_ADJUST ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PATTERN_ADJUST ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_PATTERN_ADJUST ); 
    }
    cameraInfo->patternAdjust =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PATTERN_ADJUST );
  }

  if ( isAvailable ( CONTROL_ANTI_DEW_HEATER )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DEW_HEATER ) = OA_CTRL_TYPE_BOOLEAN;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_DEW_HEATER ) =
        getMin ( CONTROL_ANTI_DEW_HEATER );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_DEW_HEATER ) =
        getMax ( CONTROL_ANTI_DEW_HEATER );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_DEW_HEATER ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_DEW_HEATER ) =
        getValue ( CONTROL_ANTI_DEW_HEATER, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_DEW_HEATER ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_DEW_HEATER ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_DEW_HEATER ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_DEW_HEATER );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_DEW_HEATER ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_DEW_HEATER ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_DEW_HEATER ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_DEW_HEATER );
    }
    cameraInfo->dewHeater =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_DEW_HEATER );
  }

  if ( isAvailable ( CONTROL_TARGETTEMP )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMP_SETPOINT ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TEMP_SETPOINT ) =
        getMin ( CONTROL_TARGETTEMP );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TEMP_SETPOINT ) =
        getMax ( CONTROL_TARGETTEMP );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TEMP_SETPOINT ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) =
        getValue ( CONTROL_TARGETTEMP, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TEMP_SETPOINT ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TEMP_SETPOINT );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TEMP_SETPOINT ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TEMP_SETPOINT );
    }
    cameraInfo->currentSetPoint =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TEMP_SETPOINT );
  }

  // Ignore this one.  It's read-only anyway
/*
  if ( isAvailable ( CONTROL_COOLERPOWERPERC )) {
    bool autoEnabled;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_COOLER_POWER ) = OA_CTRL_TYPE_INT32;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER_POWER ) =
        getMin ( CONTROL_COOLERPOWERPERC );
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER_POWER ) =
        getMax ( CONTROL_COOLERPOWERPERC );
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_COOLER_POWER ) = 1;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER_POWER ) =
        getValue ( CONTROL_COOLERPOWERPERC, &autoEnabled );
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER_POWER ) <
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER_POWER ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER_POWER ) =
        commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_COOLER_POWER );
    }
    if ( commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER_POWER ) >
        commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER_POWER ) ) {
      commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER_POWER ) =
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_COOLER_POWER );
    }
    cameraInfo->currentCoolerPower =
        commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_COOLER_POWER );
  }
*/

  if ( isAvailable ( CONTROL_HARDWAREBIN )) {
    fprintf ( stderr, "Unimplemented CONTROL_HARDWAREBIN available\n" );
    int min = getMin ( CONTROL_HARDWAREBIN );
    int max = getMax ( CONTROL_HARDWAREBIN );
    fprintf ( stderr, "  min/max = %d, %d\n", min, max );
  }

  cameraInfo->maxResolutionX = getMaxWidth();
  cameraInfo->maxResolutionY = getMaxHeight();

  if ( isBinSupported ( 2 ) || isBinSupported ( 3 ) || isBinSupported ( 4 )) {
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BINNING ) = OA_CTRL_TYPE_DISCRETE;
  }
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_DROPPED ) = OA_CTRL_TYPE_READONLY;

  // These appear to be supported by all cameras (I think )

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

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TEMPERATURE ) = OA_CTRL_TYPE_READONLY;

  // All cameras support ROI according to Sam@ZWO
  camera->features.flags |= OA_CAM_FEATURE_ROI;
  camera->features.flags |= OA_CAM_FEATURE_RESET;
  camera->features.flags |= OA_CAM_FEATURE_STREAMING;

  // Ok, now we need to find out what frame formats are supported and
  // which one we want to use

  cameraInfo->videoRGB24 = cameraInfo->videoGrey16 = 0;
  cameraInfo->videoGrey = 0;
  cameraInfo->videoCurrent = -1;
  // The mono ASI120MM will do RGB24 as a greyscale RGB image if we ask it
  // to, but that's rather wasteful, so we only support this for colour
  // cameras
  if ( isColorCam()) {
    cameraInfo->colour = 1;
    if ( isImgTypeSupported ( IMG_RGB24 )) {
      cameraInfo->videoCurrent = IMG_RGB24;
      cameraInfo->videoRGB24 = 1;
			camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;
    }
    if ( isImgTypeSupported ( IMG_RAW8 )) {
			camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
    }
  } else {
    cameraInfo->colour = 0;
    if ( isImgTypeSupported ( IMG_RAW8 )) { // prefer 8-bit to 16-bit initially
      cameraInfo->videoCurrent = IMG_RAW8;
      cameraInfo->videoGrey = 1;
    }
    if ( isImgTypeSupported ( IMG_RAW16 )) {
      if ( cameraInfo->videoCurrent < 0 ) {
        cameraInfo->videoCurrent = IMG_RAW16;
      }
      cameraInfo->videoGrey16 = 1;
    }
    if ( isImgTypeSupported ( IMG_Y8 )) {
      if ( cameraInfo->videoCurrent < 0 ) {
        cameraInfo->videoCurrent = IMG_Y8;
      }
      cameraInfo->videoGrey = 1;
    }
  }

  if ( -1 == cameraInfo->videoCurrent ) {
    fprintf ( stderr, "No suitable video format found on camera %d\n",
        cameraInfo->index );
    FREE_DATA_STRUCTS;
    return 0;
  }

  // This sets up a finite state machine to handle switching between 8-bit
  // RGB, 8-bit raw and 16-bit raw nicely.  See the controller code for
  // more details
  if ( cameraInfo->colour ) {
    switch ( cameraInfo->videoCurrent ) {
      case IMG_RAW16:
        cameraInfo->FSMState = 1;
        cameraInfo->currentBitDepth = 16;
        break;
      case IMG_RAW8:
        cameraInfo->FSMState = 3;
        cameraInfo->currentBitDepth = 8;
        break;
      case IMG_RGB24:
      default:
        cameraInfo->currentBitDepth = 8;
        cameraInfo->FSMState = 0;
    }
  }
  cameraInfo->binMode = OA_BIN_MODE_NONE;

  for ( i = 1; i <= OA_MAX_BINNING; i++ ) {
    cameraInfo->frameSizes[i].numSizes = 0;
    cameraInfo->frameSizes[i].sizes = 0;
  }

  switch ( cameraInfo->cameraType ) {

    case ZWOCAM_ASI130MM:
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          10, sizeof ( FRAMESIZE )))) {
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
      if (!( cameraInfo->frameSizes[4].sizes =
          ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
        fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
        free (( void* ) cameraInfo->frameSizes[1].sizes );
        free (( void* ) cameraInfo->frameSizes[2].sizes );
        FREE_DATA_STRUCTS;
        return 0;
      }
      cameraInfo->frameSizes[1].sizes[0].x = 1280;
      cameraInfo->frameSizes[1].sizes[0].y = 1024;
      cameraInfo->frameSizes[1].sizes[1].x = 1280;
      cameraInfo->frameSizes[1].sizes[1].y = 600;
      cameraInfo->frameSizes[1].sizes[2].x = 1280;
      cameraInfo->frameSizes[1].sizes[2].y = 400;
      cameraInfo->frameSizes[1].sizes[3].x = 800;
      cameraInfo->frameSizes[1].sizes[3].y = 600;
      cameraInfo->frameSizes[1].sizes[4].x = 800;
      cameraInfo->frameSizes[1].sizes[4].y = 400;
      cameraInfo->frameSizes[1].sizes[5].x = 640;
      cameraInfo->frameSizes[1].sizes[5].y = 480;
      cameraInfo->frameSizes[1].sizes[6].x = 600;
      cameraInfo->frameSizes[1].sizes[6].y = 400;
      cameraInfo->frameSizes[1].sizes[7].x = 400;
      cameraInfo->frameSizes[1].sizes[7].y = 400;
      cameraInfo->frameSizes[1].sizes[8].x = 480;
      cameraInfo->frameSizes[1].sizes[8].y = 320;
      cameraInfo->frameSizes[1].sizes[9].x = 320;
      cameraInfo->frameSizes[1].sizes[9].y = 240;
      cameraInfo->frameSizes[1].numSizes = 10;

      cameraInfo->frameSizes[2].sizes[0].x = 640;
      cameraInfo->frameSizes[2].sizes[0].y = 512;
      cameraInfo->frameSizes[2].numSizes = 1;

      cameraInfo->frameSizes[4].sizes[0].x = 320;
      cameraInfo->frameSizes[4].sizes[0].y = 256;
      cameraInfo->frameSizes[4].numSizes = 1;

      camera->features.pixelSizeX = 5200;
      camera->features.pixelSizeY = 5200;

      break;

    case ZWOCAM_ASI120MM_S:
    case ZWOCAM_ASI120MC_S:
      cameraInfo->usb3Cam = 1;
      /* fallthrough */

    case ZWOCAM_ASI120MM:
    case ZWOCAM_ASI120MC:
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

    case ZWOCAM_ASI035MM:
      if (!( cameraInfo->frameSizes[2].sizes =
          ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
        fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
        FREE_DATA_STRUCTS;
        return 0;
      }
      cameraInfo->frameSizes[2].sizes[0].x = 376;
      cameraInfo->frameSizes[2].sizes[0].y = 240;
      cameraInfo->frameSizes[2].numSizes = 1;

      // fallthrough

    case ZWOCAM_ASI035MC:
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          5, sizeof ( FRAMESIZE )))) {
        fprintf ( stderr, "%s: calloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
        free (( void* ) cameraInfo->frameSizes[2].sizes );
        FREE_DATA_STRUCTS;
        return 0;
      }

      cameraInfo->frameSizes[1].sizes[0].x = 752;
      cameraInfo->frameSizes[1].sizes[0].y = 480;
      cameraInfo->frameSizes[1].sizes[1].x = 640;
      cameraInfo->frameSizes[1].sizes[1].y = 480;
      cameraInfo->frameSizes[1].sizes[2].x = 600;
      cameraInfo->frameSizes[1].sizes[2].y = 400;
      cameraInfo->frameSizes[1].sizes[3].x = 400;
      cameraInfo->frameSizes[1].sizes[3].y = 400;
      cameraInfo->frameSizes[1].sizes[4].x = 320;
      cameraInfo->frameSizes[1].sizes[4].y = 240;
      cameraInfo->frameSizes[1].numSizes = 5;

      camera->features.pixelSizeX = 6000;
      camera->features.pixelSizeY = 6000;

      break;

    case ZWOCAM_ASI030MC:
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          3, sizeof ( FRAMESIZE )))) {
        fprintf ( stderr, "%s: calloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
        FREE_DATA_STRUCTS;
        return 0;
      }
      cameraInfo->frameSizes[1].sizes[0].x = 640;
      cameraInfo->frameSizes[1].sizes[0].y = 480;
      cameraInfo->frameSizes[1].sizes[1].x = 480;
      cameraInfo->frameSizes[1].sizes[1].y = 320;
      cameraInfo->frameSizes[1].sizes[2].x = 320;
      cameraInfo->frameSizes[1].sizes[2].y = 240;
      cameraInfo->frameSizes[1].numSizes = 3;

      camera->features.pixelSizeX = 6000;
      camera->features.pixelSizeY = 6000;

      break;

    case ZWOCAM_ASI034MC:
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          4, sizeof ( FRAMESIZE )))) {
        fprintf ( stderr, "%s: calloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
        FREE_DATA_STRUCTS;
        return 0;
      }
      cameraInfo->frameSizes[1].sizes[0].x = 728;
      cameraInfo->frameSizes[1].sizes[0].y = 512;
      cameraInfo->frameSizes[1].sizes[1].x = 640;
      cameraInfo->frameSizes[1].sizes[1].y = 480;
      cameraInfo->frameSizes[1].sizes[2].x = 480;
      cameraInfo->frameSizes[1].sizes[2].y = 320;
      cameraInfo->frameSizes[1].sizes[3].x = 320;
      cameraInfo->frameSizes[1].sizes[3].y = 240;
      cameraInfo->frameSizes[1].numSizes = 4;

      camera->features.pixelSizeX = 5600;
      camera->features.pixelSizeY = 5600;

      break;

    case ZWOCAM_ASI174MM:
    case ZWOCAM_ASI174MC:
      cameraInfo->usb3Cam = 1;
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          3, sizeof ( FRAMESIZE )))) {
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
      cameraInfo->frameSizes[1].sizes[0].x = 1936;
      cameraInfo->frameSizes[1].sizes[0].y = 1216;
      cameraInfo->frameSizes[1].sizes[1].x = 640;
      cameraInfo->frameSizes[1].sizes[1].y = 480;
      cameraInfo->frameSizes[1].sizes[2].x = 320;
      cameraInfo->frameSizes[1].sizes[2].y = 240;
      cameraInfo->frameSizes[1].numSizes = 3;

      cameraInfo->frameSizes[2].sizes[0].x = 968;
      cameraInfo->frameSizes[2].sizes[0].y = 608;
      cameraInfo->frameSizes[2].numSizes = 1;

      camera->features.pixelSizeX = 5860;
      camera->features.pixelSizeY = 5860;

      break;

    case ZWOCAM_ASI178MM:
    case ZWOCAM_ASI178MC:
      cameraInfo->usb3Cam = 1;
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          7, sizeof ( FRAMESIZE )))) {
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
      cameraInfo->frameSizes[1].sizes[0].x = 3096;
      cameraInfo->frameSizes[1].sizes[0].y = 2080;
      cameraInfo->frameSizes[1].sizes[1].x = 2560;
      cameraInfo->frameSizes[1].sizes[1].y = 2048;
      cameraInfo->frameSizes[1].sizes[2].x = 2048;
      cameraInfo->frameSizes[1].sizes[2].y = 1080;
      cameraInfo->frameSizes[1].sizes[3].x = 1280;
      cameraInfo->frameSizes[1].sizes[3].y = 960;
      cameraInfo->frameSizes[1].sizes[4].x = 800;
      cameraInfo->frameSizes[1].sizes[4].y = 600;
      cameraInfo->frameSizes[1].sizes[5].x = 640;
      cameraInfo->frameSizes[1].sizes[5].y = 480;
      cameraInfo->frameSizes[1].sizes[6].x = 320;
      cameraInfo->frameSizes[1].sizes[6].y = 240;
      cameraInfo->frameSizes[1].numSizes = 7;

      cameraInfo->frameSizes[2].sizes[0].x = 1548;
      cameraInfo->frameSizes[2].sizes[0].y = 1040;
      cameraInfo->frameSizes[2].numSizes = 1;

      camera->features.pixelSizeX = 2400;
      camera->features.pixelSizeY = 2400;

      break;

    case ZWOCAM_ASI185MC:
      cameraInfo->usb3Cam = 1;
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          6, sizeof ( FRAMESIZE )))) {
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
      cameraInfo->frameSizes[1].sizes[0].x = 1944;
      cameraInfo->frameSizes[1].sizes[0].y = 1224;
      cameraInfo->frameSizes[1].sizes[1].x = 1920;
      cameraInfo->frameSizes[1].sizes[1].y = 1080;
      cameraInfo->frameSizes[1].sizes[2].x = 1024;
      cameraInfo->frameSizes[1].sizes[2].y = 768;
      cameraInfo->frameSizes[1].sizes[3].x = 800;
      cameraInfo->frameSizes[1].sizes[3].y = 800;
      cameraInfo->frameSizes[1].sizes[4].x = 640;
      cameraInfo->frameSizes[1].sizes[4].y = 480;
      cameraInfo->frameSizes[1].sizes[5].x = 320;
      cameraInfo->frameSizes[1].sizes[5].y = 240;
      cameraInfo->frameSizes[1].numSizes = 6;

      cameraInfo->frameSizes[2].sizes[0].x = 972;
      cameraInfo->frameSizes[2].sizes[0].y = 612;
      cameraInfo->frameSizes[2].numSizes = 1;

      camera->features.pixelSizeX = 3750;
      camera->features.pixelSizeY = 3750;

      break;

    case ZWOCAM_ASI224MC:
      cameraInfo->usb3Cam = 1;
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          5, sizeof ( FRAMESIZE )))) {
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
      cameraInfo->frameSizes[1].sizes[0].x = 1304;
      cameraInfo->frameSizes[1].sizes[0].y = 976;
      cameraInfo->frameSizes[1].sizes[1].x = 1280;
      cameraInfo->frameSizes[1].sizes[1].y = 960;
      cameraInfo->frameSizes[1].sizes[2].x = 800;
      cameraInfo->frameSizes[1].sizes[2].y = 600;
      cameraInfo->frameSizes[1].sizes[3].x = 640;
      cameraInfo->frameSizes[1].sizes[3].y = 480;
      cameraInfo->frameSizes[1].sizes[4].x = 320;
      cameraInfo->frameSizes[1].sizes[4].y = 250;
      cameraInfo->frameSizes[1].numSizes = 5;

      cameraInfo->frameSizes[2].sizes[0].x = 652;
      cameraInfo->frameSizes[2].sizes[0].y = 488;
      cameraInfo->frameSizes[2].numSizes = 1;

      camera->features.pixelSizeX = 3750;
      camera->features.pixelSizeY = 3750;

      break;

    case ZWOCAM_ASI290MM:
    case ZWOCAM_ASI290MC:
      cameraInfo->usb3Cam = 1;
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          5, sizeof ( FRAMESIZE )))) {
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
      cameraInfo->frameSizes[1].sizes[0].x = 1936;
      cameraInfo->frameSizes[1].sizes[0].y = 1096;
      cameraInfo->frameSizes[1].sizes[1].x = 1280;
      cameraInfo->frameSizes[1].sizes[1].y = 960;
      cameraInfo->frameSizes[1].sizes[2].x = 640;
      cameraInfo->frameSizes[1].sizes[2].y = 480;
      cameraInfo->frameSizes[1].sizes[3].x = 320;
      cameraInfo->frameSizes[1].sizes[3].y = 240;
      cameraInfo->frameSizes[1].numSizes = 4;

      cameraInfo->frameSizes[2].sizes[0].x = 968;
      cameraInfo->frameSizes[2].sizes[0].y = 548;
      cameraInfo->frameSizes[2].numSizes = 1;

      camera->features.pixelSizeX = 2900;
      camera->features.pixelSizeY = 2900;

      break;

    case ZWOCAM_ASI1600MM:
    case ZWOCAM_ASI1600MC:
      cameraInfo->usb3Cam = 1;
      if (!( cameraInfo->frameSizes[1].sizes = ( FRAMESIZE* ) calloc (
          5, sizeof ( FRAMESIZE )))) {
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

      camera->features.pixelSizeX = 3800;
      camera->features.pixelSizeY = 3800;

      break;

    default:
      fprintf ( stderr, "unknown camera type %d. Using limited resolutions\n",
          cameraInfo->cameraType );
      cameraInfo->usb3Cam = isUSB3Camera() ? 1 : 0;
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

      break;
  }

  cameraInfo->xSize = cameraInfo->maxResolutionX;
  cameraInfo->ySize = cameraInfo->maxResolutionY;
  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;

  if (!( initCamera())) {
    fprintf ( stderr, "%s: initCamera() failed\n", __FUNCTION__ );
    FREE_DATA_STRUCTS;
  }

  setImageFormat ( cameraInfo->xSize, cameraInfo->ySize,
      cameraInfo->binMode, cameraInfo->videoCurrent );

  // The largest buffer size we should need
  // RGB colour is 3 bytes per pixel, mono one for 8-bit, two for 16-bit,
  // RAW is one for 8-bit, 2 for 16-bit.
  
  multiplier = ( IMG_RGB24 == cameraInfo->videoCurrent ) ? 3 : 1:
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * multiplier;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( struct ZWASIbuffer ));
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
      oacamZWASIcontroller, ( void* ) camera )) {
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
      oacamZWASIcallbackHandler, ( void* ) camera )) {

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
_ZWASIInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaZWASIInitCamera;
  camera->funcs.closeCamera = oaZWASICloseCamera;

  camera->funcs.testControl = oaZWASICameraTestControl;
  camera->funcs.readControl = oaZWASICameraReadControl;
  camera->funcs.setControl = oaZWASICameraSetControl;
  camera->funcs.getControlRange = oaZWASICameraGetControlRange;

  camera->funcs.startStreaming = oaZWASICameraStartStreaming;
  camera->funcs.stopStreaming = oaZWASICameraStopStreaming;
  camera->funcs.isStreaming = oaZWASICameraIsStreaming;

  camera->funcs.setResolution = oaZWASICameraSetResolution;
  camera->funcs.setROI = oaZWASICameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  //  camera->funcs.isAuto = oaIsAuto;

  camera->funcs.enumerateFrameSizes = oaZWASICameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaZWASICameraGetFramePixelFormat;
  camera->funcs.testROISize = oaZWASICameraTestROISize;
}


int
oaZWASICloseCamera ( oaCamera* camera )
{
  int		j;
  void*		dummy;
  ZWASI_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    closeCamera();

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
