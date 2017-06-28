/*****************************************************************************
 *
 * atikSerialgetState.c -- state querying for Atik serial cameras
 *
 * Copyright 2014,2015,2016 James Fidell (james@openastroproject.org)
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
#include <libusb-1.0/libusb.h>
#include <openastro/camera.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "atikSerial.h"
#include "atikSerialoacam.h"
#include "atikSerialstate.h"


int
oaAtikSerialCameraGetControlRange ( oaCamera* camera, int control,
    int64_t* min, int64_t* max, int64_t* step, int64_t* def )
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
oaAtikSerialCameraGetFrameSizes ( oaCamera* camera )
{
  AtikSerial_STATE*	cameraInfo = camera->_private;

  switch ( cameraInfo->binMode ) {
    case OA_BIN_MODE_NONE:
      return &cameraInfo->frameSizes[1];
      break;
/*
    case OA_BIN_MODE_2x2:
      return cameraInfo->binMode2Resolutions;
      break;
    case OA_BIN_MODE_3x3:
      return cameraInfo->binMode3Resolutions;
      break;
    case OA_BIN_MODE_4x4:
      return cameraInfo->binMode4Resolutions;
      break;
*/
  }
  fprintf ( stderr, "oaAtikSerialCameraGetFrameSizes: unknown bin mode %d\n",
      cameraInfo->binMode );
  return 0;
}


int
oaAtikSerialCameraGetFramePixelFormat ( oaCamera *camera, int depth )
{
  AtikSerial_STATE*	cameraInfo = camera->_private;

  // bit depth is irrelevant -- there can be only one
  if ( cameraInfo->colour ) {
    return OA_PIX_FMT_RGGB16LE;
  }
  return OA_PIX_FMT_GREY16LE;
}
