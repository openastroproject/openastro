/*****************************************************************************
 *
 * utils.c -- random support functions for cameras
 *
 * Copyright 2014,2015,2018,2019
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

#include "oacamprivate.h"
#include "unimplemented.h"


void
_oaFreeCameraDeviceList ( CAMERA_LIST* deviceList )
{
  unsigned int		i;

  // FIX ME -- free private data
  // FIX ME -- don't free for spinnaker devices until getCameras code is
  // fixed up
  for ( i = 0; i < deviceList->numCameras; i++ ) {
    free (( void* ) deviceList->cameraList[i] );
  }
  free (( void* ) deviceList->cameraList );
  deviceList->cameraList = 0;
  deviceList->maxCameras = deviceList->numCameras = 0;
}


int
_oaCheckCameraArraySize ( CAMERA_LIST* deviceList )
{
  oaCameraDevice**	newList;
  int			newNum;

  if ( deviceList->maxCameras > deviceList->numCameras ) {
    return OA_ERR_NONE;
  }

  newNum = ( deviceList->maxCameras + 1 ) * 8;
  if (!( newList = realloc ( deviceList->cameraList, newNum * sizeof (
      oaCameraDevice* )))) {
    return -OA_ERR_MEM_ALLOC;
  }

  deviceList->cameraList = newList;
  deviceList->maxCameras = newNum;
  return OA_ERR_NONE;
}


int
_oaInitCameraStructs ( oaCamera** camera, void** state, size_t stateSize,
		COMMON_INFO** common )
{
	oaCamera*			p_camera;
	void*					p_state;
	COMMON_INFO*	p_common;

	if (!( p_camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
		perror ( "malloc of oaCamera struct failed" );
		return -OA_ERR_MEM_ALLOC;
	}
	*camera = p_camera;

	if (!( p_state = malloc ( stateSize ))) {
		perror ( "malloc of camera state struct failed" );
		free (( void* ) p_camera );
		*camera = 0;
		return -OA_ERR_MEM_ALLOC;
	}
	*state = p_state;

	if (!( p_common = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
		perror ( "malloc of COMMON_INFO struct failed" );
		free (( void* ) p_camera );
		free ( p_state );
		*camera = 0;
		*state = 0;
		return -OA_ERR_MEM_ALLOC;
	}
	*common = p_common;

	memset ( p_camera, 0, sizeof ( oaCamera ));
	memset ( p_state, 0, stateSize );
	memset ( p_common, 0, sizeof ( COMMON_INFO ));
	p_camera->_private = p_state;
	p_camera->_common = p_common;

	_oaInitCameraFunctionPointers ( p_camera );

	return OA_ERR_NONE;
}
