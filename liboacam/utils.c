/*****************************************************************************
 *
 * utils.c -- random support functions for cameras
 *
 * Copyright 2014,2015 James Fidell (james@openastroproject.org)
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


void
_oaFreeCameraDeviceList ( CAMERA_LIST* deviceList )
{
  unsigned int		i;

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
