/*****************************************************************************
 *
 * roi.c -- region of interest management for Basler Pylon cameras
 *
 * Copyright 2020 James Fidell (james@openastroproject.org)
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

#include "oacamprivate.h"
#include "state.h"
#include "oacam.h"
#include "private.h"


int
oaPylonCameraTestROISize ( oaCamera* camera, unsigned int tryX,
    unsigned int tryY, unsigned int* suggX, unsigned int* suggY )
{
  PYLON_STATE*			cameraInfo = camera->_private;
	int								dX, dY;

	dX = tryX - cameraInfo->minResolutionX;
	dY = tryY - cameraInfo->minResolutionY;

  if ( tryX <= cameraInfo->minResolutionX &&
			tryY <= cameraInfo->minResolutionY &&
      (( dX % cameraInfo->xSizeStep ) == 0 ) &&
			(( dY % cameraInfo->ySizeStep ) == 0 )) {
    return OA_ERR_NONE;
  }

  tryX = cameraInfo->minResolutionX;
	tryX += ( dX / cameraInfo->xSizeStep ) * cameraInfo->xSizeStep;
	tryX += cameraInfo->xSizeStep;
	if ( tryX > cameraInfo->maxResolutionX ) {
		tryX = cameraInfo->maxResolutionX;
	}
  tryY = cameraInfo->minResolutionY;
	tryY += ( dY / cameraInfo->ySizeStep ) * cameraInfo->ySizeStep;
	tryY += cameraInfo->ySizeStep;
	if ( tryY > cameraInfo->maxResolutionY ) {
		tryY = cameraInfo->maxResolutionY;
	}

  *suggX = tryX;
  *suggY = tryY;
  return -OA_ERR_INVALID_SIZE;
}
