/*****************************************************************************
 *
 * qhyccdroi.c -- region of interest management for QHYCCD cameras
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
#include <openastro/util.h>

#include "oacamprivate.h"
#include "qhyccdstate.h"
#include "qhyccdoacam.h"


int
oaQHYCCDCameraTestROISize ( oaCamera* camera, unsigned int tryX,
    unsigned int tryY, unsigned int* suggX, unsigned int* suggY )
{
	QHYCCD_STATE*		cameraInfo = camera->_private;

	if ( tryX % cameraInfo->binMode == 0 && tryY % cameraInfo->binMode == 0 ) {
		return OA_ERR_NONE;
	}

	*suggX = (( int )( tryX / cameraInfo->binMode )) * cameraInfo->binMode;
	*suggY = (( int )( tryY / cameraInfo->binMode )) * cameraInfo->binMode;

  return -OA_ERR_INVALID_SIZE;
}
