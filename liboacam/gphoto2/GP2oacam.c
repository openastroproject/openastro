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

#if HAVE_LIBGPHOTO2

#include <openastro/camera.h>
#include <gphoto2/gphoto2-camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "GP2oacam.h"
#include "GP2private.h"


static void		_gp2ErrorCallback ( GPContext*, const char*, void* );
static void		_gp2StatusCallback ( GPContext*, const char*, void* );
static void		_gp2CancelCallback ( GPContext*, const char*, void* );
static void		_gp2MessageCallback ( GPContext*, const char*, void* );


int
oaGP2GetCameras ( CAMERA_LIST* deviceList, int flags )
{
  unsigned int			numFound = 0, i;
  int								ret, numCameras;
  oaCameraDevice*		dev;
  DEVICE_INFO*			_private;
	GPContext					*ctx;
	CameraList				*cameraList;
	const char				*camName;
	const char				*camPort;

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

	p_gp_context_set_error_func ( ctx, _gp2ErrorCallback, 0 );
	p_gp_context_set_status_func ( ctx, _gp2StatusCallback, 0 );
	p_gp_context_set_cancel_func ( ctx, _gp2CancelCallback, 0 );
	p_gp_context_set_message_func ( ctx, _gp2MessageCallback, 0 );

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
		p_gp_list_free ( cameraList );
		p_gp_context_unref ( ctx );
		return 0;
	}

  for ( i = 0; i < numCameras; i++ ) {
		if ( p_gp_list_get_name ( cameraList, i, &camName ) != GP_OK ) {
			fprintf ( stderr, "gp_list_get_name failed\n" );
			p_gp_list_free ( cameraList );
			p_gp_context_unref ( ctx );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( p_gp_list_get_value ( cameraList, i, &camPort ) != GP_OK ) {
			fprintf ( stderr, "gp_list_get_name failed\n" );
			p_gp_list_free ( cameraList );
			p_gp_context_unref ( ctx );
			return -OA_ERR_SYSTEM_ERROR;
		}

    fprintf ( stderr, "found camera '%s' at port '%s'\n", camName, camPort );

    // now we can drop the data into the list
    if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
			p_gp_list_free ( cameraList );
			p_gp_context_unref ( ctx );
      return -OA_ERR_MEM_ALLOC;
    }

    if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
      ( void ) free (( void* ) dev );
			p_gp_list_free ( cameraList );
			p_gp_context_unref ( ctx );
      _oaFreeCameraDeviceList ( deviceList );
      return -OA_ERR_MEM_ALLOC;
    }

    _oaInitCameraDeviceFunctionPointers ( dev );
    dev->interface = OA_CAM_IF_GPHOTO2;
    _private->devIndex = 0;
    dev->_private = _private;

    ( void ) strcpy ( dev->deviceName, camName );

    dev->initCamera = oaGP2InitCamera;
    if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
      ( void ) free (( void* ) dev );
      ( void ) free (( void* ) _private );
			p_gp_list_free ( cameraList );
			p_gp_context_unref ( ctx );
      return ret;
    }
    deviceList->cameraList[ deviceList->numCameras++ ] = dev;
    numFound++;
  }

	p_gp_list_free ( cameraList );
	p_gp_context_unref ( ctx );
  return numFound;
}


static void
_gp2ErrorCallback ( GPContext* ctx, const char* str, void* data )
{
	fprintf ( stderr, "gphoto2::ERROR: %s\n", str ? str : "no text" );
	fflush ( stderr );
}


static void
_gp2StatusCallback ( GPContext* ctx, const char* str, void* data )
{
	fprintf ( stderr, "gphoto2::STATUS: %s\n", str ? str : "no text" );
	fflush ( stderr );
}


static void
_gp2CancelCallback ( GPContext* ctx, const char* str, void* data )
{
	fprintf ( stderr, "gphoto2::CANCEL: %s\n", str ? str : "no text" );
	fflush ( stderr );
}


static void
_gp2MessageCallback ( GPContext* ctx, const char* str, void* data )
{
	fprintf ( stderr, "gphoto2::MESSAGE: %s\n", str ? str : "no text" );
	fflush ( stderr );
}


#endif	/* HAVE_LIBGPHOTO2 */
