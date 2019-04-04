/*****************************************************************************
 *
 * dummyroi.c -- region of interest management for dummy cameras
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
#include "dummyoacam.h"
#include "dummystate.h"


int
oaDummyCameraTestROISize ( oaCamera* camera, unsigned int tryX,
    unsigned int tryY, unsigned int* suggX, unsigned int* suggY )
{
  int		done;
  unsigned int	n;
  DUMMY_STATE*	cameraInfo = camera->_private;

  if ( tryX % 2 == 0 && tryY % 2 == 0 ) {
    if (( tryX * cameraInfo->binMode ) <= cameraInfo->frameSizes[1].sizes[0].x
        && ( tryY * cameraInfo->binMode ) <=
        cameraInfo->frameSizes[1].sizes[0].y ) {
      return OA_ERR_NONE;
    }
  }

  for ( n = 0, done = -1; n <
      cameraInfo->frameSizes[ cameraInfo->binMode].numSizes && done < 0; n++ ) {
    if ( cameraInfo->frameSizes[ cameraInfo->binMode ].sizes[n].x <= tryX ) {
      done = n;
    }
  }
  if ( done < 0 ) {
    done = cameraInfo->frameSizes[ cameraInfo->binMode ].numSizes;
  }

  *suggX = cameraInfo->frameSizes[ cameraInfo->binMode ].sizes[done].x;
  *suggY = cameraInfo->frameSizes[ cameraInfo->binMode ].sizes[done].y;

  return -OA_ERR_INVALID_SIZE;
}
