/*****************************************************************************
 *
 * ZWASIgetState.c -- state querying for ZW ASI cameras
 *
 * Copyright 2013,2014,2015 James Fidell (james@openastroproject.org)
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

#include <ASICamera.h>

#include "oacamprivate.h"
#include "ZWASIoacam.h"
#include "ZWASIstate.h"


int
oaZWASICameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
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
oaZWASICameraGetFrameSizes ( oaCamera* camera )
{
  ZWASI_STATE*		cameraInfo = camera->_private;

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


int
oaZWASICameraGetFramePixelFormat ( oaCamera *camera, int depth )
{
  ZWASI_STATE*		cameraInfo = camera->_private;

  if ( depth <= 0 ) {
    switch ( cameraInfo->videoCurrent ) {
      case IMG_RGB24:
        return OA_PIX_FMT_BGR24;
        break;
      case IMG_RAW16:
        if ( isColorCam()) {
          return OA_PIX_FMT_GRBG16LE;
        }
        return OA_PIX_FMT_GREY16LE;
        break;
      case IMG_RAW8:
        if ( isColorCam()) {
          return OA_PIX_FMT_GRBG8;
        } else {
          return OA_PIX_FMT_GREY8;
        }
        break;
      case IMG_Y8:
        // I believe the colour camera returns this as a monochrome RGB
        // image
        if ( isColorCam()) {
          return OA_PIX_FMT_BGR24;
        } else {
          return OA_PIX_FMT_GREY8;
        }
        break;
    }
  } else {
    if ( 12 == depth || 16 == depth ) {
      if ( isColorCam()) {
        return OA_PIX_FMT_GRBG16LE;
      }
      return OA_PIX_FMT_GREY16LE;
    }
    if ( 8 == depth ) {
      if ( isColorCam()) {
        if ( cameraInfo->videoCurrent == IMG_RAW8 ||
            cameraInfo->videoCurrent == IMG_RAW16 ) {
          return OA_PIX_FMT_GRBG8;
        } else {
          return OA_PIX_FMT_BGR24;
        }
      }
      return OA_PIX_FMT_GREY8;
    }
  }

  // default
  return OA_PIX_FMT_BGR24;
}
