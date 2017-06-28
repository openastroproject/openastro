/*****************************************************************************
 *
 * EUVCgetState.c -- state querying for EUVC cameras
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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
#include <openastro/util.h>

#include "oacamprivate.h"
#include "EUVC.h"
#include "EUVCstate.h"
#include "EUVCoacam.h"


int
oaEUVCCameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
    int64_t* max, int64_t* step, int64_t* def )
{
  COMMON_INFO*	commonInfo = camera->_common;

  if ( !camera->controls[ control ] ) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *min = commonInfo->min[ control ];
  *max = commonInfo->max[ control ];
  *step = commonInfo->step[ control ];
  *def = commonInfo->def[ control ];
  return OA_ERR_NONE;
}


const FRAMESIZES*
oaEUVCCameraGetFrameSizes ( oaCamera* camera )
{
  EUVC_STATE*	cameraInfo = camera->_private;

  switch ( cameraInfo->binMode ) {
    case OA_BIN_MODE_NONE:
      return &cameraInfo->frameSizes[1];
      break;
    case OA_BIN_MODE_2x2:
      return &cameraInfo->frameSizes[2];
      break;
  }
  fprintf ( stderr, "oaEUVCCameraGetFrameSizes: unknown bin mode %d\n",
      cameraInfo->binMode );
  return 0;
}


const FRAMERATES*
oaEUVCCameraGetFrameRates ( oaCamera* camera, int resX, int resY )
{
  EUVC_STATE*           cameraInfo = camera->_private;

  return cameraInfo->frameRates;
}


int
oaEUVCCameraGetFramePixelFormat ( oaCamera *camera, int depth )
{
  EUVC_STATE*           cameraInfo = camera->_private;

  // I'm really not sure what these values should be so I'm just
  // guessing really

  switch ( cameraInfo->colourFormats ) {
    case 0x03:
      return OA_PIX_FMT_GRBG8;
      break;
    case 0x04:
      return OA_PIX_FMT_RGGB8;
      break;
    case 0x05:
      return OA_PIX_FMT_BGGR8;
      break;
    case 0x06:
      return OA_PIX_FMT_GBRG8;
      break;
  }

  return OA_PIX_FMT_GREY8;
}
