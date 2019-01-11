/*****************************************************************************
 *
 * SXroi.c -- region of interest management for SX cameras
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
#include <openastro/errno.h>

#include "oacamprivate.h"
#include "SXstate.h"
#include "SXoacam.h"


int
oaSXCameraTestROISize ( oaCamera* camera, unsigned int tryX,
    unsigned int tryY, unsigned int* suggX, unsigned int* suggY )
{
  SX_STATE*	cameraInfo = camera->_private;

  if ( tryX % 2 == 0 && tryY % 2 == 0 ) {
    if (( tryX * cameraInfo->binMode ) <= cameraInfo->maxResolutionX &&
				( tryY * cameraInfo->binMode ) <= cameraInfo->maxResolutionY ) {
      return OA_ERR_NONE;
    }
  }
  *suggX = ( tryX & ~0x1 ) + 1;
  if (( *suggX * cameraInfo->binMode ) > cameraInfo->maxResolutionX ) {
      *suggX = cameraInfo->maxResolutionX;
  }
  if (( *suggY * cameraInfo->binMode ) > cameraInfo->maxResolutionY ) {
      *suggY = cameraInfo->maxResolutionY;
  }

	// FIX ME -- need to handle binning here

  return -OA_ERR_INVALID_SIZE;
}
