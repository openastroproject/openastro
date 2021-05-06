/*****************************************************************************
 *
 * getState.c -- state querying for Basler Pylon cameras
 *
 * Copyright 2020,2021 James Fidell (james@openastroproject.org)
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
#include "oacam.h"
#include "state.h"
#include "private.h"


int
oaPylonCameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
    int64_t* max, int64_t* step, int64_t* def )
{
  COMMON_INFO*	commonInfo = camera->_common;

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *min = commonInfo->OA_CAM_CTRL_MIN( control );
  *max = commonInfo->OA_CAM_CTRL_MAX( control );
  *step = commonInfo->OA_CAM_CTRL_STEP( control );
  *def = commonInfo->OA_CAM_CTRL_DEF( control );
  return OA_ERR_NONE;
}


const FRAMESIZES*
oaPylonCameraGetFrameSizes ( oaCamera* camera )
{
  PYLON_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[ cameraInfo->binMode ];
}


int
oaPylonCameraGetFramePixelFormat ( oaCamera* camera )
{
  PYLON_STATE*		cameraInfo = camera->_private;
	char						buffer[256];
	size_t					len;
	int							numFormats, i;

	len = sizeof ( buffer );
	( void ) p_PylonDeviceFeatureToString ( cameraInfo->deviceHandle,
			"PixelFormat", buffer, &len );

	numFormats = sizeof ( _frameFormats ) / sizeof ( pylonFrameInfo );
	for ( i = 0; i < numFormats; i++ ) {
		if ( !strcmp ( buffer, _frameFormats[i].pylonName )) {
			return _frameFormats[i].pixFormat;
		}
	}

	oaLogError ( OA_LOG_CAMERA, "%s: frame format '%s' not recognised", __func__,
			buffer );

	// return something as a default
  return OA_PIX_FMT_RGB24;
}
