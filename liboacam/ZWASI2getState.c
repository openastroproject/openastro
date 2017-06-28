/*****************************************************************************
 *
 * ZWASI2getState.c -- state querying for ZW ASI cameras API v2
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

#include <ASICamera2.h>

#include "oacamprivate.h"
#include "ZWASIoacam.h"
#include "ZWASI2oacam.h"
#include "ZWASIstate.h"


int
oaZWASI2CameraGetFramePixelFormat ( oaCamera *camera, int depth )
{
  ZWASI_STATE*		cameraInfo = camera->_private;

  if ( depth <= 0 ) {
    switch ( cameraInfo->videoCurrent ) {
      case ASI_IMG_RGB24:
        return OA_PIX_FMT_BGR24;
        break;
      case ASI_IMG_RAW16:
        if ( cameraInfo->colour ) {
          return OA_PIX_FMT_GRBG16LE;
        }
        return OA_PIX_FMT_GREY16LE;
        break;
      case ASI_IMG_RAW8:
        if ( cameraInfo->colour ) {
          return OA_PIX_FMT_GRBG8;
        } else {
          return OA_PIX_FMT_GREY8;
        }
        break;
      case ASI_IMG_Y8:
        // I believe the colour camera returns this as a monochrome RGB
        // image
        if ( cameraInfo->colour ) {
          return OA_PIX_FMT_BGR24;
        } else {
          return OA_PIX_FMT_GREY8;
        }
        break;
    }
  } else {
    if ( 12 == depth || 16 == depth ) {
      if ( cameraInfo->colour ) {
        return OA_PIX_FMT_GRBG16LE;
      }
      return OA_PIX_FMT_GREY16LE;
    }
    if ( 8 == depth ) {
      if ( cameraInfo->colour ) {
        if ( cameraInfo->videoCurrent == ASI_IMG_RAW8 ||
            cameraInfo->videoCurrent == ASI_IMG_RAW16 ) {
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
