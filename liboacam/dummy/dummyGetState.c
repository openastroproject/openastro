/*****************************************************************************
 *
 * dummyGetState.c -- state querying for dummy cameras
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

#include "oacamprivate.h"
#include "dummyoacam.h"
#include "dummystate.h"


int
oaDummyCameraGetFramePixelFormat ( oaCamera *camera )
{
  DUMMY_STATE*		cameraInfo = camera->_private;

  return cameraInfo->cameraType ? OA_PIX_FMT_GREY16BE :
			OA_PIX_FMT_GRBG8;
}


int
oaDummyCameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
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
oaDummyCameraGetFrameSizes ( oaCamera* camera )
{
  DUMMY_STATE*		cameraInfo = camera->_private;

  switch ( cameraInfo->binMode ) {
    case OA_BIN_MODE_NONE:
      return &cameraInfo->frameSizes[1];
      break;
    case OA_BIN_MODE_2x2:
      return &cameraInfo->frameSizes[2];
      break;
    case OA_BIN_MODE_3x3:
      return &cameraInfo->frameSizes[3];
      break;
    case OA_BIN_MODE_4x4:
      return &cameraInfo->frameSizes[4];
      break;
    default:
      return &cameraInfo->frameSizes[1];
      break;
  }
}
