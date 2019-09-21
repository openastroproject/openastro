/*****************************************************************************
 *
 * roi.c -- region of interest management for Touptek cameras
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
#include "touptekstate.h"
#include "touptekoacam.h"


int
TT_FUNC( oa, CameraTestROISize )( oaCamera* camera, unsigned int tryX,
    unsigned int tryY, unsigned int* suggX, unsigned int* suggY )
{
  if (( tryX % 2 == 0 ) && ( tryY % 2 == 0 ) && tryX >= 16 && tryY >= 16 ) {
    return OA_ERR_NONE;
  }

  if ( tryX < 16 ) { tryX = 16; }
  if ( tryY < 16 ) { tryY = 16; }
  if ( tryX % 2 ) { tryX--; }
  if ( tryY % 2 ) { tryY--; }

  *suggX = tryX;
  *suggY = tryY;

  return -OA_ERR_INVALID_SIZE;
}
