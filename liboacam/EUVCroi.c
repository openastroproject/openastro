/*****************************************************************************
 *
 * EUVCroi.c -- region of interest management for EUVC cameras
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
#include <openastro/errno.h>
#include <openastro/util.h>

#include <ASICamera.h>

#include "oacamprivate.h"
#include "EUVC.h"
#include "EUVCstate.h"
#include "EUVCoacam.h"


int
oaEUVCCameraTestROISize ( oaCamera* camera, unsigned int tryX,
    unsigned int tryY, unsigned int* suggX, unsigned int* suggY )
{
  EUVC_STATE*	cameraInfo = camera->_private;
  FRAMESIZE*	currentSize;

  currentSize = &( cameraInfo->frameSizes[ cameraInfo->binMode ].sizes[
      cameraInfo->sizeIndex ]);

  if ( tryX % 8 == 0 && tryY % 4 == 0 ) {
    if ( tryX <= currentSize->x && tryY <= currentSize->y ) {
      return OA_ERR_NONE;
    }
  }

  *suggX = ( tryX & ~0x7 ) + 8;
  if ( *suggX > currentSize->x ) {
    *suggX = currentSize->x;
  }
  *suggY = ( tryY & ~0x3 ) + 4;
  if ( *suggX > currentSize->y ) {
    *suggX = currentSize->y;
  }

  return -OA_ERR_INVALID_SIZE;
}
