/*****************************************************************************
 *
 * getState.c -- state querying for Touptek cameras
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#include "touptek-conf.h"
#include "oacamprivate.h"
#include "touptekoacam.h"
#include "touptekstate.h"


int
TT_FUNC( oa, CameraGetControlRange )( oaCamera* camera, int control, int64_t* min,
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
TT_FUNC( oa, CameraGetFrameSizes )( oaCamera* camera )
{
  TOUPTEK_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[ cameraInfo->binMode ];
}


int
TT_FUNC( oa, CameraGetFramePixelFormat )( oaCamera* camera )
{
  TOUPTEK_STATE*	cameraInfo = camera->_private;

  return cameraInfo->currentVideoFormat;
}


int
TT_FUNC( oa, CameraGetControlDiscreteSet )( oaCamera* camera, int control,
    int32_t* count, int64_t** values )
{
  static int64_t  ledModeValues[3] = { 1, 2, 3 };

  if ( control != OA_CAM_CTRL_LED_STATE ) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *count = 3;
  *values = ledModeValues;
  return OA_ERR_NONE;
}
