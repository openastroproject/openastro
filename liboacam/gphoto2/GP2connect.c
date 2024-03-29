/*****************************************************************************
 *
 * GP2connect.c -- Initialise libgphoto2 cameras
 *
 * Copyright 2019,2020,2021
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
#include <ctype.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <gphoto2/gphoto2-camera.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "GP2oacam.h"
#include "GP2private.h"
#include "GP2state.h"


static void		_GP2InitFunctionPointers ( oaCamera* );
static int		_GP2ProcessMenuWidget ( CameraWidget*, const char*,
									CameraWidget**, CameraWidgetType*, int*, const char*,
									const char* );
static int		_GP2ProcessStringWidget ( CameraWidget*, const char*,
									CameraWidget**, const char*, const char* );
static int		_GP2ProcessToggleWidget ( CameraWidget*, const char*,
									CameraWidget**, const char*, const char* );
static int		_GP2GuessManufacturer ( const char* );


/**
 * Initialise a given camera device
 */

oaCamera*
oaGP2InitCamera ( oaCameraDevice* device )
{
  oaCamera*					camera;
	CameraList*				cameraList;
  DEVICE_INFO*			devInfo;
  GP2_STATE*				cameraInfo;
  COMMON_INFO*			commonInfo;
	const char*				camName;
	const char*				camPort;
	const char*				format;
	int								numCameras, i, j, ret, found = -1;
	CameraWidget*			tempWidget;

	if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
			sizeof ( GP2_STATE ), &commonInfo ) != OA_ERR_NONE ) {
		return 0;
	}

  _GP2InitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  devInfo = device->_private;

  // FIX ME -- This is a bit ugly.  Much of it is repeated from the
  // getCameras function.  I should join the two together somehow.

	// Not clear from the docs what this returns in case of an error, or if
	// an error is even possible
	// FIX ME -- check in source code
	if (!( cameraInfo->ctx = p_gp_context_new())) {
    FREE_DATA_STRUCTS;
		return 0;
	}

	_gp2ConfigureCallbacks ( cameraInfo->ctx );

  if ( p_gp_list_new ( &cameraList ) != GP_OK ) {
    oaLogError ( OA_LOG_CAMERA, "%s: gp_list_new failed", __func__ );
		p_gp_context_unref ( cameraInfo->ctx );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if ( p_gp_list_reset ( cameraList ) != GP_OK ) {
    oaLogError ( OA_LOG_CAMERA, "%s: gp_list_reset failed", __func__ );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    FREE_DATA_STRUCTS;
    return 0;
  }

	// gp_camera_autodetect isn't explicitly documented as returning the
	// number of cameras found, but this appears to be the case.
  if (( numCameras = p_gp_camera_autodetect ( cameraList,
			cameraInfo->ctx )) < 0 ) {
    oaLogError ( OA_LOG_CAMERA,
				"%s: gp_camera_autodetect failed: error code %d", __func__,
				numCameras );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    FREE_DATA_STRUCTS;
    return 0;
  }

	if ( numCameras < 1 ) {
    oaLogError ( OA_LOG_CAMERA, "%s: Can't see any gphoto devices now",
				__func__ );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    FREE_DATA_STRUCTS;
    return 0;
	}

  for ( i = 0; i < numCameras && found < 0; i++ ) {
		if ( p_gp_list_get_name ( cameraList, i, &camName ) != GP_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: gp_list_get_name failed", __func__ );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}
		if ( strcmp ( camName, devInfo->deviceId )) {
			continue;
		}
		if ( p_gp_list_get_value ( cameraList, i, &camPort ) != GP_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: gp_list_get_name failed", __func__ );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}
		if ( !strcmp ( camPort, devInfo->sysPath )) {
			found = i;
		}
	}

	if ( found < 0) {
    oaLogError ( OA_LOG_CAMERA, "%s: No matching libgphoto2 device found!",
				__func__ );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
	}

	cameraInfo->isoOptions = cameraInfo->whiteBalanceOptions =
			cameraInfo->shutterSpeedOptions = cameraInfo->sharpeningOptions = 0;

	if ( _gp2OpenCamera ( &cameraInfo->handle, camName, camPort,
			cameraInfo->ctx ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't open camera '%s' at port '%s'",
				__func__, camName, camPort );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
   }

	if (( ret = _gp2GetConfig ( cameraInfo->handle, &cameraInfo->rootWidget,
			cameraInfo->ctx )) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get config for camera '%s' at port '%s'", __func__,
				camName, camPort );
		oaLogError ( OA_LOG_CAMERA, "%s:   error code %d", __func__, ret );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
  }

	// Now get the widget for imgsettings so it can be used without
	// fetching it again

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "imgsettings",
			&cameraInfo->imgSettings ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get imgsettings widget for camera '%s' at port '%s'",
				__func__, camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
  }

	// Ditto for capturesettings

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "capturesettings",
			&cameraInfo->captureSettings ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get capturesettings widget for camera '%s' at port '%s'",
				__func__, camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
  }

	// And just "settings"

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "settings",
			&cameraInfo->settings ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get settings widget for camera '%s' at port '%s'",
				__func__, camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
  }

	// And "status"

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "status",
			&cameraInfo->status ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get status widget for camera '%s' at port '%s'", __func__,
				camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
  }

	// Finally, "actions"

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "actions",
			&cameraInfo->actions ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get actions widget for camera '%s' at port '%s'", __func__,
				camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
  }

	// Use the status widget to try to determine the camera manufacturer.
	// There appear to be a number of possibilities here:
	//
	//   /main/status/manufacturer
	//   /main/status/model
	//   /main/status/cameramodel
	//
	// I'll try them in that order

	if (( ret = _GP2ProcessStringWidget ( cameraInfo->status, "manufacturer",
			&tempWidget, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
	}

	if ( ret == -OA_ERR_INVALID_COMMAND ) {
		if (( ret = _GP2ProcessStringWidget ( cameraInfo->status, "model",
				&tempWidget, camName, camPort )) != OA_ERR_NONE &&
				ret != -OA_ERR_INVALID_COMMAND ) {
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}

		if ( ret == -OA_ERR_INVALID_COMMAND ) {
			if (( ret = _GP2ProcessStringWidget ( cameraInfo->status, "cameramodel",
					&tempWidget, camName, camPort )) != OA_ERR_NONE &&
					ret != -OA_ERR_INVALID_COMMAND ) {
				_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
				p_gp_list_unref ( cameraList );
				p_gp_context_unref ( cameraInfo->ctx );
				FREE_DATA_STRUCTS;
				return 0;
			}
		}
	}

	if ( ret == -OA_ERR_NONE ) {
		const char*		modelStr;

		if ( p_gp_widget_get_value ( tempWidget, &modelStr ) !=
				GP_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: can't get value of camera model string",
					__func__ );
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}

		// This may not work, but perhaps we can try some other stuff later
		cameraInfo->manufacturer = _GP2GuessManufacturer ( modelStr );
	}

	// Also from the "status" widget, the AC power status

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->status, "acpower",
			&cameraInfo->acpower, &cameraInfo->acpowerType,
			&cameraInfo->numACPowerOptions, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
	}
  if ( ret == OA_ERR_NONE ) {
		oaLogInfo ( OA_LOG_CAMERA, "%s: have acpower flag", __func__ );
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_POWER_SOURCE ) =
				OA_CTRL_TYPE_READONLY;
	}

	// And the battery level

	if (( ret = _GP2ProcessStringWidget ( cameraInfo->status, "batterylevel",
			&cameraInfo->batteryLevel, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
	}

  if ( ret == OA_ERR_NONE ) {
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BATTERY_LEVEL ) =
				OA_CTRL_TYPE_READONLY;
	}

	// Now start looking for controls that we can use

	// ISO setting

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->imgSettings, "iso",
			&cameraInfo->iso, &cameraInfo->isoType, &cameraInfo->numIsoOptions,
			camName, camPort )) != OA_ERR_NONE && ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
	}
  if ( ret == OA_ERR_NONE ) {
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_ISO ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_ISO ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_ISO ) =
				cameraInfo->numIsoOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_ISO ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_ISO ) = 0;
		// FIX ME -- is there an auto value required here?
	}

	// white balance

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->imgSettings, "whitebalance",
			&cameraInfo->whiteBalance, &cameraInfo->whiteBalanceType,
			&cameraInfo->numWBOptions, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_WHITE_BALANCE ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_WHITE_BALANCE ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_WHITE_BALANCE ) =
				cameraInfo->numWBOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_WHITE_BALANCE ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_WHITE_BALANCE ) = 0;
	}

	// shutter speed

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->captureSettings,
			"shutterspeed", &cameraInfo->shutterSpeed, &cameraInfo->shutterSpeedType,
			&cameraInfo->numShutterSpeedOptions, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHUTTER_SPEED ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHUTTER_SPEED ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHUTTER_SPEED ) =
				cameraInfo->numShutterSpeedOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHUTTER_SPEED ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHUTTER_SPEED ) = 0;
	}

	// sharpening

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->captureSettings, "sharpening",
			&cameraInfo->sharpening, &cameraInfo->sharpeningType,
			&cameraInfo->numSharpeningOptions, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHARPNESS ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHARPNESS ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHARPNESS ) =
				cameraInfo->numSharpeningOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHARPNESS ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHARPNESS ) = 0;
	}

	if ( cameraInfo->manufacturer == CAMERA_MANUF_CANON ) {

		// customfuncex (to allow mirror lock on Canon cameras)

		if (( ret = _GP2ProcessStringWidget ( cameraInfo->settings, "customfuncex",
				&cameraInfo->customfuncex, camName, camPort )) != OA_ERR_NONE &&
				ret != -OA_ERR_INVALID_COMMAND ) {
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}

		if ( ret == OA_ERR_NONE ) {
			const char*		customStr;
			char*					mlf;

			if ( p_gp_widget_get_value ( cameraInfo->customfuncex, &customStr ) !=
					GP_OK ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: can't get value of customfuncex string", __func__ );
				_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
				p_gp_list_unref ( cameraList );
				p_gp_context_unref ( cameraInfo->ctx );
				FREE_DATA_STRUCTS;
				return 0;
			} else {
				if (( mlf = strstr ( customStr, ",60f,1," )) != 0 && ( mlf[7] == '0' ||
						mlf[7] == '1' )) {
					cameraInfo->customFuncStr = strdup ( customStr );
					cameraInfo->mirrorLockupPos = ( mlf - customStr ) + 7;
					camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_MIRROR_LOCKUP ) =
							OA_CTRL_TYPE_BOOLEAN;
					commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_MIRROR_LOCKUP ) = 0;
					commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_MIRROR_LOCKUP ) = 1;
					commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_MIRROR_LOCKUP ) = 1;
					commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_MIRROR_LOCKUP ) = 0;
				}
			}
		}

		// For Canon only, the "capture" toggle

		if (( ret = _GP2ProcessToggleWidget ( cameraInfo->settings, "capture",
				&cameraInfo->capture, camName, camPort )) != OA_ERR_NONE &&
				ret != -OA_ERR_INVALID_COMMAND ) {
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}

		// And again for Canon only, /main/actions/eosremoterelease (bulb mode)

		if (( ret = _GP2ProcessMenuWidget ( cameraInfo->actions, "eosremoterelease",
				&cameraInfo->bulbMode, &cameraInfo->bulbModeType,
				&cameraInfo->numBulbModeOptions, camName, camPort )) != OA_ERR_NONE &&
				ret != -OA_ERR_INVALID_COMMAND ) {
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}

		// If the action exists, work out which are the "immediate" (or
		// "press full" if "immediate" doesn't exist) and "release full" options

		if ( ret == OA_ERR_NONE ) {
			int optsFound = 0;
			cameraInfo->bulbPressOption = cameraInfo->bulbReleaseOption = -1;
			( void ) oaGP2CameraGetMenuString ( camera, OA_CAM_CTRL_BULB_MODE, 0 );
			for ( i = 0; i < cameraInfo->numBulbModeOptions; i++ ) {
				if ( !strcasecmp ( cameraInfo->bulbModeOptions[i], "immediate" )) {
					// only increment this if we've not already found "press full"
					if ( cameraInfo->bulbPressOption == -1 ) {
						optsFound++;
					}
					cameraInfo->bulbPressOption = i;
				}
				if ( -1 == cameraInfo->bulbPressOption &&
						!strcasecmp ( cameraInfo->bulbModeOptions[i], "press full" )) {
					cameraInfo->bulbPressOption = i;
					optsFound++;
				}
				if ( !strcasecmp ( cameraInfo->bulbModeOptions[i], "release full" )) {
					cameraInfo->bulbReleaseOption = i;
					optsFound++;
				}
			}
			// This isn't going to work unless we found them both
			if ( optsFound != 2 ) {
				cameraInfo->bulbMode = 0;
			}
		}
	}

	// Check for bulb mode (/main/actions/bulb) if we didn't already find it
	// for Canon as eosremoterelease

	if ( !cameraInfo->bulbMode ) {
		oaLogInfo ( OA_LOG_CAMERA, "%s: checking for bulb", __func__ );
		if (( ret = _GP2ProcessToggleWidget ( cameraInfo->actions, "bulb",
				&cameraInfo->bulbMode, camName, camPort )) != OA_ERR_NONE &&
				ret != -OA_ERR_INVALID_COMMAND ) {
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}
		if ( ret == OA_ERR_NONE ) {
			cameraInfo->bulbModeType = GP_WIDGET_TOGGLE;
		}
	}

	// Set up an absolute exposure setting for use with bulb mode if the
	// camera has it

	if ( cameraInfo->bulbMode ) {
		oaLogInfo ( OA_LOG_CAMERA, "%s: have some bulb mode", __func__ );
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BULB_MODE ) = OA_CTRL_TYPE_READONLY;
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
							OA_CTRL_TYPE_INT64;
		// min is 1 second
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1000000;
		// max is 30 minutes
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1800000000;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = 1;
		// default is 30 seconds
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
				cameraInfo->bulbExposureTime = 30000000;
	}

	camera->features.flags |= OA_CAM_FEATURE_SINGLE_SHOT;
	camera->features.flags |= OA_CAM_FEATURE_FRAME_SIZE_UNKNOWN;
	camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;

	// I'm going to assume these will be correct
	// FIX ME -- check later based on what frame formats are supported
	// by the camera

	camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
	camera->features.flags |= OA_CAM_FEATURE_DEMOSAIC_MODE;

	// And now frame formats...
	// These look to be a right giggle as a number of different possibilities
	// exist:
	//
	//   /main/imgsettings/imageformat
	//   /main/capturesettings/imagequality
	//   /main/imgsettings/imagequality
	//
	// From those, it looks as though there are options of JPEG, RAW, NEF,
	// and those containing a "+" are probably both JPEG and RAW.  Some
	// JPEG options appear not to contain the string "JPEG" (eg. Sony).
	// There may be multiple options for the size of JPEG, further complicated
	// by the fact that the data may be in the local language.
	//
	// It looks as though Canon cameras give the best quality first in the
	// list of options, whereas Nikons give the best quality last, as do Sony

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->imgSettings, "imageformat",
			&cameraInfo->frameFormat, &cameraInfo->frameFormatType,
			&cameraInfo->numFrameFormatOptions, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    return 0;
	}

	if ( ret == -OA_ERR_INVALID_COMMAND ) {
		if (( ret = _GP2ProcessMenuWidget ( cameraInfo->captureSettings,
				"imagequality", &cameraInfo->frameFormat, &cameraInfo->frameFormatType,
				&cameraInfo->numFrameFormatOptions, camName, camPort )) !=
				OA_ERR_NONE && ret != -OA_ERR_INVALID_COMMAND ) {
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}

		if ( ret == -OA_ERR_INVALID_COMMAND ) {
			if (( ret = _GP2ProcessMenuWidget ( cameraInfo->imgSettings,
					"imagequality", &cameraInfo->frameFormat,
					&cameraInfo->frameFormatType, &cameraInfo->numFrameFormatOptions,
					camName, camPort )) != OA_ERR_NONE && ret !=
					-OA_ERR_INVALID_COMMAND ) {
				_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
				p_gp_list_unref ( cameraList );
				p_gp_context_unref ( cameraInfo->ctx );
				FREE_DATA_STRUCTS;
				return 0;
			}
		}
	}

	if ( ret == OA_ERR_NONE ) {

		( void ) oaGP2CameraGetMenuString ( camera, OA_CAM_CTRL_FRAME_FORMAT, 0 );

		// FIX ME -- might identify as-yet unknown camera manufacturers from
		// these settings?

		cameraInfo->jpegOption = -1;
		cameraInfo->rawOption = -1;
		for ( i = 0; i < cameraInfo->numFrameFormatOptions; i++ ) {
			char	lowerVal[40];
			for ( j = 0; j < 40 && cameraInfo->frameFormatOptions[i][j] != 0; j++ ) {
				lowerVal[j] = tolower ( cameraInfo->frameFormatOptions[i][j] );
			}
			lowerVal[j] = '\0';
			// FIX ME -- handle combined options at some point
			if ( !strstr ( lowerVal, "+" ) && !strstr ( lowerVal, "unknown" ) &&
					!strstr ( lowerVal, "undefined" )) {
				if ( strstr ( lowerVal, "raw" ) || strstr ( lowerVal, "nef" )) {
					if ( cameraInfo->manufacturer == CAMERA_MANUF_CANON ) {
						// because Canons appear to list best first...
						if ( cameraInfo->rawOption == -1 ) {
							cameraInfo->rawOption = i;
						}
					} else {
						cameraInfo->rawOption = i;
					}
				} else {
					// Assume anything is JPEG at this point
					if ( cameraInfo->manufacturer == CAMERA_MANUF_CANON ) {
						if ( cameraInfo->jpegOption == -1 ) {
							cameraInfo->jpegOption = i;
						}
					} else {
						cameraInfo->jpegOption = i;
					}
				}
			}
		}
	} else {
		oaLogError ( OA_LOG_CAMERA, "%s: can't determine image format", __func__ );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
		return 0;
	}

	if ( cameraInfo->jpegOption >= 0 && cameraInfo->rawOption >= 0 ) {
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) =
				OA_CTRL_TYPE_DISC_MENU;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FRAME_FORMAT ) = OA_PIX_FMT_JPEG8;
		cameraInfo->numFormatMenuValues = 2;
		cameraInfo->formatMenuValues[0] = OA_PIX_FMT_JPEG8;
		switch ( cameraInfo->manufacturer ) {
			case CAMERA_MANUF_CANON:
				cameraInfo->formatMenuValues[1] = OA_PIX_FMT_CANON_CR2;
				break;
			case CAMERA_MANUF_NIKON:
				cameraInfo->formatMenuValues[1] = OA_PIX_FMT_NIKON_NEF;
				break;
			default:
				oaLogError ( OA_LOG_CAMERA, "%s: Unknown raw camera format", __func__ );
				break;
		}
	}

	if ( cameraInfo->rawOption >= 0 && cameraInfo->jpegOption == -1 ) {
		oaLogWarning ( OA_LOG_CAMERA,
				"%s: Weird.  We have a raw option, but no JPEG", __func__ );
	}

	// Get the current format option.  If it isn't one of the two values
	// we like, set it to the one we like that's closest.

	if ( p_gp_widget_get_value ( cameraInfo->frameFormat, &format ) != GP_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't get current frame format",
				__func__ );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
		return 0;
	}

	found = 0;
	for ( i = 0; i < cameraInfo->numFrameFormatOptions; i++ ) {
		if ( !strcmp ( format, cameraInfo->frameFormatOptions[i] )) {
			cameraInfo->currentFormatOption = i;
			found = 1;
		}
	}
	if ( !found ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't find current frame format in options list", __func__ );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
		return 0;
	}

	// FIX ME -- this will need refining if combined JPEG/RAW formats are
	// allowed

	if ( cameraInfo->currentFormatOption != cameraInfo->jpegOption &&
			cameraInfo->currentFormatOption != cameraInfo->rawOption ) {
		found = 0;
		// Again, Canon appear to be "special" by listing the best format first
		if ( cameraInfo->manufacturer == CAMERA_MANUF_CANON ) {
			if ( cameraInfo->currentFormatOption < cameraInfo->rawOption ) {
				cameraInfo->currentFormatOption = cameraInfo->jpegOption;
				found = 1;
			} else {
				if ( cameraInfo->currentFormatOption > cameraInfo->rawOption ) {
					cameraInfo->currentFormatOption = cameraInfo->rawOption;
					found = 1;
				}
			}
		} else {
			if ( cameraInfo->currentFormatOption < cameraInfo->jpegOption ) {
				cameraInfo->currentFormatOption = cameraInfo->jpegOption;
				found = 1;
			} else {
				if ( cameraInfo->currentFormatOption < cameraInfo->rawOption ) {
					cameraInfo->currentFormatOption = cameraInfo->rawOption;
					found = 1;
				}
			}
		}
		if ( !found ) {
			cameraInfo->currentFormatOption = cameraInfo->jpegOption;
		}

		if ( p_gp_widget_set_value ( cameraInfo->frameFormat,
				cameraInfo->frameFormatOptions[ cameraInfo->currentFormatOption ]) !=
				GP_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: Can't set current frame format",
					__func__ );
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}

		if (( ret = p_gp_camera_set_config ( cameraInfo->handle,
			cameraInfo->rootWidget, cameraInfo->ctx )) != GP_OK ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: Failed to write config to camera, error %d", __func__, ret );
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			FREE_DATA_STRUCTS;
			return 0;
		}
	}

  cameraInfo->exposurePending = cameraInfo->exposureInProgress = 0;
	cameraInfo->captureEnabled = 0;

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();

  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;
	for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
		cameraInfo->currentBufferLength[i] = 0;
	}
	cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( frameBuffer ));

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamGP2controller, ( void* ) camera )) {
    oaLogError ( OA_LOG_CAMERA, "%s: controller thread creation failed",
				__func__ );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }

  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamGP2callbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    oaLogError ( OA_LOG_CAMERA, "%s: callback thread creation failed",
				__func__ );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		FREE_DATA_STRUCTS;
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }

  return camera;
}


static void
_GP2InitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaGP2InitCamera;
  camera->funcs.closeCamera = oaGP2CloseCamera;

/*
  camera->funcs.testControl = oaGP2CameraTestControl;
*/
  camera->funcs.getControlRange = oaGP2CameraGetControlRange;
  camera->funcs.getControlDiscreteSet = oaGP2CameraGetControlDiscreteSet;

/*
  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaGP2CameraGetFrameSizes;
*/
  camera->funcs.getFramePixelFormat = oaGP2CameraGetFramePixelFormat;
  camera->funcs.getMenuString = oaGP2CameraGetMenuString;
}


static int
_GP2ProcessMenuWidget ( CameraWidget* parent, const char* name,
		CameraWidget** ptarget, CameraWidgetType* ptargetType, int* numVals,
		const char* camName, const char* camPort )
{
	int				ret;

	if (( ret = _gp2FindWidget ( parent, name, ptarget )) != OA_ERR_NONE ) {
		if ( ret != -OA_ERR_INVALID_COMMAND ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: Can't get %s widget for camera '%s' at port '%s'", __func__,
					name, camName, camPort );
		}
    return ret;
	}

  if ( _gp2GetWidgetType ( *ptarget, ptargetType ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get type for %s widget for camera '%s' at port '%s'",
				__func__, name, camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	// We'll accept RADIO and MENU types for menus
	if ( *ptargetType != GP_WIDGET_RADIO && *ptargetType != GP_WIDGET_MENU ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Unexpected type %d for %s widget for camera '%s' at port '%s'",
				__func__, *ptargetType, name , camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	// By the looks of it, radio and menu widgets have sets of options that
	// always have a starting value of zero and step up in units, so just
	// counting the number of children for this widget should be sufficient
	// to set the min and max values for this command.  I'll assume zero is
	// the default.

	if (( *numVals = p_gp_widget_count_choices ( *ptarget )) < GP_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't get number of choices for %s "
				"widget for camera '%s' at port '%s'", __func__, name, camName,
				camPort );
    return -OA_ERR_CAMERA_IO;
	}

	return OA_ERR_NONE;
}


static int
_GP2ProcessStringWidget ( CameraWidget* parent, const char* name,
		CameraWidget** ptarget, const char* camName, const char* camPort )
{
	int									ret;
	CameraWidgetType		type;

	if (( ret = _gp2FindWidget ( parent, name, ptarget )) != OA_ERR_NONE ) {
		if ( ret != -OA_ERR_INVALID_COMMAND ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: Can't get %s widget for camera '%s' at port '%s'", __func__,
					name, camName, camPort );
		}
    return ret;
	}

  if ( _gp2GetWidgetType ( *ptarget, &type ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get type for %s widget for camera '%s' at port '%s'",
				__func__, name, camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	if ( type != GP_WIDGET_TEXT ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unexpected type %d for %s widget for "
				"camera '%s' at port '%s'", __func__, type, name , camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	return OA_ERR_NONE;
}


static int
_GP2ProcessToggleWidget ( CameraWidget* parent, const char* name,
		CameraWidget** ptarget, const char* camName, const char* camPort )
{
	int									ret;
	CameraWidgetType		type;

	if (( ret = _gp2FindWidget ( parent, name, ptarget )) != OA_ERR_NONE ) {
		if ( ret != -OA_ERR_INVALID_COMMAND ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: Can't get %s widget for camera '%s' at port '%s'", __func__,
					name, camName, camPort );
		}
    return ret;
	}

  if ( _gp2GetWidgetType ( *ptarget, &type ) != OA_ERR_NONE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Can't get type for %s widget for camera '%s' at port '%s'",
				__func__, name, camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	if ( type != GP_WIDGET_TOGGLE ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: Unexpected type %d for %s widget for camera '%s' at port '%s'",
				__func__, type, name , camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	return OA_ERR_NONE;
}


static int
_GP2GuessManufacturer ( const char* mstring )
{
	// FIX ME -- should probably copy the string and convert it to non-mixed
	// case before doing a single test, but I'm feeling lazy right now

	if ( strstr ( mstring, "canon" ) || strstr ( mstring, "Canon" )) {
		return CAMERA_MANUF_CANON;
	}

	if ( strstr ( mstring, "nikon" ) || strstr ( mstring, "Nikon" )) {
		return CAMERA_MANUF_NIKON;
	}

	if ( strstr ( mstring, "sony" ) || strstr ( mstring, "Sony" )) {
		return CAMERA_MANUF_SONY;
	}

	return CAMERA_MANUF_UNKNOWN;
}


int
oaGP2CloseCamera ( oaCamera* camera )
{
  int		j;
  void*		dummy;
  GP2_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

/*
    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );
*/

		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		p_gp_context_unref ( cameraInfo->ctx );

    if ( cameraInfo->buffers ) {
      for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
        if ( cameraInfo->currentBufferLength[j] ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
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
