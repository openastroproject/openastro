/*****************************************************************************
 *
 * TouptekgetState.c -- state querying for Touptek cameras
 *
 * Copyright 2016 James Fidell (james@openastroproject.org)
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
#include "Touptekoacam.h"
#include "Touptekstate.h"


int
oaTouptekCameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
    int64_t* max, int64_t* step, int64_t* def )
{
  COMMON_INFO*	commonInfo = camera->_common;

  if ( !camera->controls[ control ] ) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *min = commonInfo->min [ control ];
  *max = commonInfo->max [ control ];
  *step = commonInfo->step [ control ];
  *def = commonInfo->def [ control ];
  return OA_ERR_NONE;
}


const FRAMESIZES*
oaTouptekCameraGetFrameSizes ( oaCamera* camera )
{
  TOUPTEK_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[ cameraInfo->binMode ];
}


int
oaTouptekCameraGetFramePixelFormat ( oaCamera* camera, int depth )
{
  TOUPTEK_STATE*	cameraInfo = camera->_private;

  if ( !depth || depth == 8 ) {
    return cameraInfo->currentVideoFormat;
  }

  switch ( cameraInfo->currentVideoFormat ) {
    case OA_PIX_FMT_GREY8:
fprintf ( stderr, "%s: check >8-bit mono byte order\n", __FUNCTION__ );
      return OA_PIX_FMT_GREY16LE;

    case OA_PIX_FMT_RGB24:
fprintf ( stderr, "%s: check >8-bit RGB byte order\n", __FUNCTION__ );
      return OA_PIX_FMT_RGB48LE;
  }

fprintf ( stderr, "%s: check >8-bit byte order for video format %d\n", __FUNCTION__, cameraInfo->currentVideoFormat );
  return OA_PIX_FMT_RGB24;
}
