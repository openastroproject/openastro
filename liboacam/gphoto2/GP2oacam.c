/*****************************************************************************
 *
 * GP2oacam.c -- main entrypoint for libgphoto2 Cameras
 *
 * Copyright 2019
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

#include <openastro/camera.h>
#include <gphoto2/gphoto2-camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "GP2oacam.h"
#include "GP2private.h"


int
oaGP2GetCameras ( CAMERA_LIST* deviceList, int flags )
{
  unsigned int			numFound = 0, i;
  int								ret, numCameras;
  oaCameraDevice*		dev;
  DEVICE_INFO*			_private;
	GPContext*				ctx;
	CameraList*				cameraList;
	CameraWidget*			rootWidget;
	CameraWidget*			tempWidget;
	Camera*						camera;
	CameraWidgetType	widgetType;
	const char*				camName;
	const char*				camPort;
	const char*				widgetValue;

	if (( ret = _gp2InitLibraryFunctionPointers()) != OA_ERR_NONE ) {
		return ret;
	}

	// Not clear from the docs what this returns in case of an error, or if
	// an error is even possible
	// FIX ME -- check in source code
	if (!( ctx = p_gp_context_new())) {
		return -OA_ERR_SYSTEM_ERROR;
	}

	// These aren't strictly required, but keep them for debugging
	// for the time being
	_gp2ConfigureCallbacks ( ctx );

  if ( p_gp_list_new ( &cameraList ) != GP_OK ) {
    fprintf ( stderr, "gp_list_new failed\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }
  if ( p_gp_list_reset ( cameraList ) != GP_OK ) {
    fprintf ( stderr, "gp_list_reset failed\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

	// gp_camera_autodetect isn't explicitly documented as returning the
	// number of cameras found, but this appears to be the case.
  if (( numCameras = p_gp_camera_autodetect ( cameraList, ctx )) < 0 ) {
    fprintf ( stderr, "gp_camera_autodetect failed: error code %d\n", ret );
    return -OA_ERR_SYSTEM_ERROR;
  }
	if ( numCameras < 1 ) {
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( ctx );
		return 0;
	}

  for ( i = 0; i < numCameras; i++ ) {
		if ( p_gp_list_get_name ( cameraList, i, &camName ) != GP_OK ) {
			fprintf ( stderr, "gp_list_get_name failed\n" );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( ctx );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( p_gp_list_get_value ( cameraList, i, &camPort ) != GP_OK ) {
			fprintf ( stderr, "gp_list_get_name failed\n" );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( ctx );
			return -OA_ERR_SYSTEM_ERROR;
		}

		if ( _gp2OpenCamera ( &camera, camName, camPort, ctx ) != OA_ERR_NONE ) {
			fprintf ( stderr, "Can't open camera '%s' at port '%s'\n", camName,
					camPort );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( ctx );
      return -OA_ERR_MEM_ALLOC;
    }

		if ( _gp2GetConfig ( camera, &rootWidget, ctx ) != OA_ERR_NONE ) {
			fprintf ( stderr, "Can't get config for camera '%s' at port '%s'\n",
					camName, camPort );
			_gp2CloseCamera ( camera, ctx );
			// FIX ME -- free rootWidget?
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( ctx );
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
			// FIX ME -- free rootWidget?
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( ctx );
      return -OA_ERR_MEM_ALLOC;
    }

		if ( _gp2FindWidget ( rootWidget, "cameramodel", &tempWidget ) ==
				OA_ERR_NONE || _gp2FindWidget ( rootWidget, "model", &tempWidget ) ==
				OA_ERR_NONE ) {
			if ( _gp2GetWidgetType ( tempWidget, &widgetType ) != OA_ERR_NONE ) {
				// FIX ME -- free rootWidget and tempWidget?
				p_gp_list_unref ( cameraList );
				p_gp_context_unref ( ctx ); 
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( widgetType != GP_WIDGET_TEXT ) {
				fprintf ( stderr, "unexpected type %d for camera model widget type\n",
						widgetType );
				// FIX ME -- free rootWidget and tempWidget?
				p_gp_list_unref ( cameraList );
				p_gp_context_unref ( ctx ); 
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( p_gp_widget_get_value ( tempWidget, &widgetValue ) != GP_OK ) {
				fprintf ( stderr, "failed to get camera model value\n" );
				// FIX ME -- free rootWidget and tempWidget?
				p_gp_list_unref ( cameraList );
				p_gp_context_unref ( ctx ); 
				return -OA_ERR_SYSTEM_ERROR;
			}
			( void ) strcpy ( dev->deviceName, widgetValue );
		} else {
			( void ) strcpy ( dev->deviceName, camName );
		}

		// FIX ME -- free rootWidget and tempWidget?
		_gp2CloseCamera ( camera, ctx );

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( ctx );
      _oaFreeCameraDeviceList ( deviceList );
      return -OA_ERR_MEM_ALLOC;
    }

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_GPHOTO2;
    _private->devIndex = 0;
    dev->_private = _private;
		( void ) strncpy ( _private->deviceId, camName, 256 );
		( void ) strncpy ( _private->sysPath, camPort, PATH_MAX );

    dev->initCamera = oaGP2InitCamera;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( ctx );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
    numFound++;
  }

	p_gp_list_unref ( cameraList );
	p_gp_context_unref ( ctx );
  return numFound;
}
