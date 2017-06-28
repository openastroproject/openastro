/*****************************************************************************
 *
 * QHYgetState.c -- state querying for QHY cameras
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
#include <openastro/util.h>

#include "oacamprivate.h"
#include "QHYoacam.h"
#include "QHYstate.h"


int
oaQHYCameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
    int64_t* max, int64_t* step, int64_t* def )
{
  COMMON_INFO*	commonInfo = camera->_common;

  if ( !camera->controls[ control ] ) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *min = commonInfo->min [ control ];
  *max = commonInfo->max [ control ];
  *step = commonInfo->step [ control ];
  *def = commonInfo->def [ control ];
  return OA_ERR_NONE;
}
