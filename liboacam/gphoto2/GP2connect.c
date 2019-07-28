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
#include "GP2private.h"
#include "GP2state.h"


static void		_GP2InitFunctionPointers ( oaCamera* );
static int		_GP2ProcessMenuWidget ( CameraWidget*, const char*,
									CameraWidget**, CameraWidgetType*, int*, const char*,
									const char* camPort );


/**
 * Initialise a given camera device
 */

oaCamera*
oaGP2InitCamera ( oaCameraDevice* device )
{
  oaCamera*					camera;
	CameraList*				cameraList;
	Camera*						gp2camera;
  DEVICE_INFO*			devInfo;
  GP2_STATE*				cameraInfo;
  COMMON_INFO*			commonInfo;
	const char*				camName;
	const char*				camPort;
	int								numCameras, i, found = -1;

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

	if ( _gp2OpenCamera ( &gp2camera, camName, camPort, cameraInfo->ctx ) !=
			OA_ERR_NONE ) {
		fprintf ( stderr, "Can't open camera '%s' at port '%s'\n", camName,
				camPort );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
     return 0;
   }

	if ( _gp2GetConfig ( gp2camera, &cameraInfo->rootWidget, cameraInfo->ctx ) !=
			OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get config for camera '%s' at port '%s'\n",
				camName, camPort );
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
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
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
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
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
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
	//
	// FIX ME -- not having this shouldn't be an error.  Check the source to
	// find how to tell if the widget doesn't exist rather than there being
	// some other error

	if ( _GP2ProcessMenuWidget ( cameraInfo->imgSettings, "iso",
				&cameraInfo->iso, &cameraInfo->isoType, &cameraInfo->numIsoOptions,
				camName, camPort )) {
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
	}

	camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_ISO ) = OA_CTRL_TYPE_MENU;
	commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_ISO ) = 0;
	commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_ISO ) =
			cameraInfo->numIsoOptions - 1;
	commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_ISO ) = 1;
	commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_ISO ) = 0;
	// FIX ME -- is there an auto value required here?

	// white balance
	//
	// FIX ME -- not having this shouldn't be an error.  Check the source to
	// find how to tell if the widget doesn't exist rather than there being
	// some other error

	if ( _GP2ProcessMenuWidget ( cameraInfo->imgSettings, "whitebalance",
				&cameraInfo->whiteBalance, &cameraInfo->whiteBalanceType,
				&cameraInfo->numWBOptions, camName, camPort )) {
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}

	camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_WHITE_BALANCE ) = OA_CTRL_TYPE_MENU;
	commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_WHITE_BALANCE ) = 0;
	commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_WHITE_BALANCE ) =
			cameraInfo->numWBOptions - 1;
	commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_WHITE_BALANCE ) = 1;
	commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_WHITE_BALANCE ) = 0;

	// shutter speed
	//
	// FIX ME -- not having this shouldn't be an error.  Check the source to
	// find how to tell if the widget doesn't exist rather than there being
	// some other error

	if ( _GP2ProcessMenuWidget ( cameraInfo->captureSettings, "shutterspeed",
				&cameraInfo->shutterSpeed, &cameraInfo->shutterSpeedType,
				&cameraInfo->numShutterSpeedOptions, camName, camPort )) {
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}

	camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHUTTER_SPEED ) = OA_CTRL_TYPE_MENU;
	commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHUTTER_SPEED ) = 0;
	commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHUTTER_SPEED ) =
			cameraInfo->numShutterSpeedOptions - 1;
	commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHUTTER_SPEED ) = 1;
	commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHUTTER_SPEED ) = 0;

	// sharpening
	//
	// FIX ME -- not having this shouldn't be an error.  Check the source to
	// find how to tell if the widget doesn't exist rather than there being
	// some other error

	if ( _GP2ProcessMenuWidget ( cameraInfo->captureSettings, "sharpening",
				&cameraInfo->sharpening, &cameraInfo->sharpeningType,
				&cameraInfo->numSharpeningOptions, camName, camPort )) {
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}

	camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHARPNESS ) = OA_CTRL_TYPE_MENU;
	commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHARPNESS ) = 0;
	commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHARPNESS ) =
			cameraInfo->numSharpeningOptions - 1;
	commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHARPNESS ) = 1;
	commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHARPNESS ) = 0;

  return camera;
}


static void
_GP2InitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaGP2InitCamera;
/*
  camera->funcs.closeCamera = oaGP2CloseCamera;

  camera->funcs.setControl = oaGP2CameraSetControl;
  camera->funcs.readControl = oaGP2CameraReadControl;
  camera->funcs.testControl = oaGP2CameraTestControl;
  camera->funcs.getControlRange = oaGP2CameraGetControlRange;
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

  camera->funcs.getMenuString = oaGP2CameraGetMenuString;
*/
}


static int
_GP2ProcessMenuWidget ( CameraWidget* parent, const char* name,
		CameraWidget** ptarget, CameraWidgetType* ptargetType, int* numVals,
		const char* camName, const char* camPort )
{
	if ( _gp2FindWidget ( parent, name, ptarget ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get %s widget for camera '%s' "
				"at port '%s'\n", name, camName, camPort );
    return -OA_ERR_SYSTEM_ERROR;
	}

  if ( _gp2GetWidgetType ( *ptarget, ptargetType ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get type for %s widget for camera "
				"'%s' at port '%s'\n", name, camName, camPort );
    return -OA_ERR_SYSTEM_ERROR;
	}

	// We'll accept RADIO and MENU types for menus
	if ( *ptargetType != GP_WIDGET_RADIO && *ptargetType != GP_WIDGET_MENU ) {
		fprintf ( stderr, "Unexpected type %d for %s widget for "
				"camera '%s' at port '%s'\n", *ptargetType, name , camName, camPort );
    return -OA_ERR_SYSTEM_ERROR;
	}

	// By the looks of it, radio and menu widgets have sets of options that
	// always have a starting value of zero and step up in units, so just
	// counting the number of children for this widget should be sufficient
	// to set the min and max values for this command.  I'll assume zero is
	// the default.

	if (( *numVals = p_gp_widget_count_choices ( *ptarget )) < GP_OK ) {
		fprintf ( stderr, "Can't get number of choices for %s "
				"widget for camera '%s' at port '%s'\n", name, camName, camPort );
    return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}
