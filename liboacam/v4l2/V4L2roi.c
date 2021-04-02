/*****************************************************************************
 *
 * V4L2roi.c -- region of interest management for V4L2 cameras
 *
 * Copyright 2021 James Fidell (james@openastroproject.org)
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
#include "V4L2oacam.h"
#include "V4L2state.h"


int
oaV4L2CameraTestROISize ( oaCamera* camera, unsigned int tryX,
    unsigned int tryY, unsigned int* suggX, unsigned int* suggY )
{
  V4L2_STATE*		cameraInfo = camera->_private;
	int						err = OA_ERR_NONE;
	unsigned int	n, t;

	*suggX = tryX;
	*suggY = tryY;

	if ( tryX > cameraInfo->maxWidth ) {
		*suggX = cameraInfo->maxWidth;
		err = -OA_ERR_INVALID_SIZE;
	}
	if ( tryX < cameraInfo->minWidth ) {
		*suggX = cameraInfo->minWidth;
		err = -OA_ERR_INVALID_SIZE;
	}
	if ( tryY > cameraInfo->maxHeight ) {
		*suggY = cameraInfo->maxHeight;
		err = -OA_ERR_INVALID_SIZE;
	}
	if ( tryY < cameraInfo->minHeight ) {
		*suggY = cameraInfo->minHeight;
		err = -OA_ERR_INVALID_SIZE;
	}

	if ( err == OA_ERR_NONE ) {
		if (( cameraInfo->maxWidth - tryX ) % cameraInfo->stepWidth != 0 ) {
			n = ( cameraInfo->maxWidth - tryX ) / cameraInfo->stepWidth + 1;
			t = cameraInfo->maxWidth - n * cameraInfo->stepWidth;
			if ( t < cameraInfo->minWidth ) {
				t = cameraInfo->minWidth;
			}
			*suggX = t;
			err = -OA_ERR_INVALID_SIZE;
		}
		if (( cameraInfo->maxHeight - tryY ) % cameraInfo->stepHeight != 0 ) {
			n = ( cameraInfo->maxHeight - tryY ) / cameraInfo->stepHeight + 1;
			t = cameraInfo->maxHeight - n * cameraInfo->stepHeight;
			if ( t < cameraInfo->minHeight ) {
				t = cameraInfo->minHeight;
			}
			*suggY = t;
			err = -OA_ERR_INVALID_SIZE;
		}
	}

	return err;
}
