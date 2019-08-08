/*****************************************************************************
 *
 * GP2connect.c -- Initialise libgphoto2 cameras
 *
 * Copyright 2019
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
	int								numCameras, i, ret, found = -1;

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }
  if (!( cameraInfo = ( GP2_STATE* ) malloc ( sizeof ( GP2_STATE )))) {
    free ( camera );
    perror ( "malloc GP2_STATE failed" );
    return 0;
  }
  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    free ( cameraInfo );
    free ( camera );
    perror ( "malloc COMMON_INFO failed" );
    return 0;
  }
  OA_CLEAR ( *camera );
  OA_CLEAR ( *cameraInfo );
  OA_CLEAR ( *commonInfo );
  camera->_private = cameraInfo;
  camera->_common = commonInfo;

  _oaInitCameraFunctionPointers ( camera );
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
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
		return 0;
	}

	_gp2ConfigureCallbacks ( cameraInfo->ctx );

  if ( p_gp_list_new ( &cameraList ) != GP_OK ) {
    fprintf ( stderr, "gp_list_new failed\n" );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }
  if ( p_gp_list_reset ( cameraList ) != GP_OK ) {
    fprintf ( stderr, "gp_list_reset failed\n" );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

	// gp_camera_autodetect isn't explicitly documented as returning the
	// number of cameras found, but this appears to be the case.
  if (( numCameras = p_gp_camera_autodetect ( cameraList,
			cameraInfo->ctx )) < 0 ) {
    fprintf ( stderr, "gp_camera_autodetect failed: error code %d\n",
				numCameras );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

	if ( numCameras < 1 ) {
    fprintf ( stderr, "Can't see any UVC devices now\n" );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
	}

  for ( i = 0; i < numCameras && found < 0; i++ ) {
		if ( p_gp_list_get_name ( cameraList, i, &camName ) != GP_OK ) {
			fprintf ( stderr, "gp_list_get_name failed\n" );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			free (( void* ) commonInfo );
			free (( void* ) cameraInfo );
			free (( void* ) camera );
			return 0;
		}
		if ( strcmp ( camName, devInfo->deviceId )) {
			continue;
		}
		if ( p_gp_list_get_value ( cameraList, i, &camPort ) != GP_OK ) {
			fprintf ( stderr, "gp_list_get_name failed\n" );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			free (( void* ) commonInfo );
			free (( void* ) cameraInfo );
			free (( void* ) camera );
			return 0;
		}
		if ( !strcmp ( camPort, devInfo->sysPath )) {
			found = i;
		}
	}

	if ( found < 0) {
    fprintf ( stderr, "No matching libgphoto2 device found!\n" );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
	}

	cameraInfo->isoOptions = cameraInfo->whiteBalanceOptions =
			cameraInfo->shutterSpeedOptions = cameraInfo->sharpeningOptions = 0;

	if ( _gp2OpenCamera ( &cameraInfo->handle, camName, camPort,
			cameraInfo->ctx ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't open camera '%s' at port '%s'\n", camName,
				camPort );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
     return 0;
   }

	if ( _gp2GetConfig ( cameraInfo->handle, &cameraInfo->rootWidget,
			cameraInfo->ctx ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get config for camera '%s' at port '%s'\n",
				camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
  }

	// Now get the widget for imgsettings so it can be used without
	// fetching it again

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "imgsettings",
			&cameraInfo->imgSettings ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get imgsettings widget for camera '%s' "
				"at port '%s'\n", camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
  }

	// Ditto for capturesettings

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "capturesettings",
			&cameraInfo->captureSettings ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get capturesettings widget for camera '%s' "
				"at port '%s'\n", camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
  }

	// And just "settings"

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "settings",
			&cameraInfo->settings ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get settings widget for camera '%s' "
				"at port '%s'\n", camName, camPort );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
  }

	// Now start looking for controls that we can use

	// ISO setting

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->imgSettings, "iso",
			&cameraInfo->iso, &cameraInfo->isoType, &cameraInfo->numIsoOptions,
			camName, camPort )) != OA_ERR_NONE && ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
	}
  if ( ret == OA_ERR_NONE ) {
fprintf ( stderr, "have iso, min = 0, max = %d\n", cameraInfo->numIsoOptions - 1 );
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
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
fprintf ( stderr, "have wb, min = 0, max = %d\n", cameraInfo->numWBOptions - 1 );
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
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
fprintf ( stderr, "have shutter speed, min = 0, max = %d\n", cameraInfo->numShutterSpeedOptions - 1 );
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
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
fprintf ( stderr, "have sharpening, min = 0, max = %d\n", cameraInfo->numSharpeningOptions - 1 );
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHARPNESS ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHARPNESS ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHARPNESS ) =
				cameraInfo->numSharpeningOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHARPNESS ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHARPNESS ) = 0;
	}

	// customfuncex (to allow mirror lock on Canon cameras)

	if (( ret = _GP2ProcessStringWidget ( cameraInfo->settings, "customfuncex",
			&cameraInfo->customfuncex, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}

	if ( ret == OA_ERR_NONE ) {
		const char*		customStr;
		char*					mlf;

		if ( p_gp_widget_get_value ( cameraInfo->customfuncex, &customStr ) !=
				GP_OK ) {
			fprintf ( stderr, "can't get value of customfuncex string\n" );
			_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
			// FIX ME -- free rootWidget?
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			free (( void* ) commonInfo );
			free (( void* ) cameraInfo );
			free (( void* ) camera );
			return 0;
		} else {
fprintf ( stderr, "customfuncex value = '%s'\n", customStr );
			if (( mlf = strstr ( customStr, "60f,0," )) != 0 && ( mlf[6] == '0' ||
					mlf[6] == '1' )) {
fprintf ( stderr, "mirror lockup supported\n" );
			}
		}
	}

	camera->features.frameSizeUnknown = 1;
  camera->features.hasReadableControls = 1;

	// I'm going to assume these will be correct
	// FIX ME -- check later based on what frame formats are supported
	// by the camera

  camera->features.hasRawMode = camera->features.hasDemosaicMode = 1;

/*
  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;
*/

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
/*
  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;
*/
  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamGP2controller, ( void* ) camera )) {
    fprintf ( stderr, "controller thread creation failed\n" );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
/*
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamGP2callbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    fprintf ( stderr, "callback thread creation failed\n" );
		_gp2CloseCamera ( cameraInfo->handle, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
*/
  return camera;
}


static void
_GP2InitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaGP2InitCamera;
/*
  camera->funcs.closeCamera = oaGP2CloseCamera;
*/

  camera->funcs.setControl = oaGP2CameraSetControl;
  camera->funcs.readControl = oaGP2CameraReadControl;
/*
  camera->funcs.testControl = oaGP2CameraTestControl;
*/
  camera->funcs.getControlRange = oaGP2CameraGetControlRange;
/*
  camera->funcs.getControlDiscreteSet = oaGP2CameraGetControlDiscreteSet;

  camera->funcs.startStreaming = oaGP2CameraStartStreaming;
  camera->funcs.stopStreaming = oaGP2CameraStopStreaming;
  camera->funcs.isStreaming = oaGP2CameraIsStreaming;

  camera->funcs.setResolution = oaGP2CameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaGP2CameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaGP2CameraGetFramePixelFormat;

  camera->funcs.enumerateFrameRates = oaGP2CameraGetFrameRates;
  camera->funcs.setFrameInterval = oaGP2CameraSetFrameInterval;
*/
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
			fprintf ( stderr, "Can't get %s widget for camera '%s' "
					"at port '%s'\n", name, camName, camPort );
		}
    return ret;
	}

  if ( _gp2GetWidgetType ( *ptarget, ptargetType ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get type for %s widget for camera "
				"'%s' at port '%s'\n", name, camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	// We'll accept RADIO and MENU types for menus
	if ( *ptargetType != GP_WIDGET_RADIO && *ptargetType != GP_WIDGET_MENU ) {
		fprintf ( stderr, "Unexpected type %d for %s widget for "
				"camera '%s' at port '%s'\n", *ptargetType, name , camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	// By the looks of it, radio and menu widgets have sets of options that
	// always have a starting value of zero and step up in units, so just
	// counting the number of children for this widget should be sufficient
	// to set the min and max values for this command.  I'll assume zero is
	// the default.

	if (( *numVals = p_gp_widget_count_choices ( *ptarget )) < GP_OK ) {
		fprintf ( stderr, "Can't get number of choices for %s "
				"widget for camera '%s' at port '%s'\n", name, camName, camPort );
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
			fprintf ( stderr, "Can't get %s widget for camera '%s' "
					"at port '%s'\n", name, camName, camPort );
		}
    return ret;
	}

  if ( _gp2GetWidgetType ( *ptarget, &type ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get type for %s widget for camera "
				"'%s' at port '%s'\n", name, camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	if ( type != GP_WIDGET_TEXT ) {
		fprintf ( stderr, "Unexpected type %d for %s widget for "
				"camera '%s' at port '%s'\n", type, name , camName, camPort );
    return -OA_ERR_CAMERA_IO;
	}

	return OA_ERR_NONE;
}
