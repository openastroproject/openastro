/*****************************************************************************
 *
 * FC2roi.c -- region of interest management for Point Grey GigE cameras
 *
 * Copyright 2015,2018 James Fidell (james@openastroproject.org)
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
#include "FC2.h"
#include "FC2state.h"
#include "FC2oacam.h"
#include "FC2private.h"


int
oaFC2CameraTestROISize ( oaCamera* camera, unsigned int tryX,
    unsigned int tryY, unsigned int* suggX, unsigned int* suggY )
{
  FC2_STATE*			cameraInfo = camera->_private;
  fc2GigEImageSettingsInfo	imageInfo;

  if (( *p_fc2GetGigEImageSettingsInfo )( cameraInfo->pgeContext,
      &imageInfo ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get image settings info\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if ( tryX <= imageInfo.maxWidth && tryY <= imageInfo.maxHeight &&
      (( tryX % imageInfo.imageHStepSize ) == 0 ) &&
      (( tryY % imageInfo.imageVStepSize ) == 0 )) {
    return OA_ERR_NONE;
  }

  tryX /= imageInfo.imageHStepSize;
  tryX *= imageInfo.imageHStepSize;
  tryX += imageInfo.imageHStepSize;
  if ( tryX > imageInfo.maxWidth ) {
    tryX = imageInfo.maxWidth;
  }
  tryY /= imageInfo.imageVStepSize;
  tryY *= imageInfo.imageVStepSize;
  tryY += imageInfo.imageVStepSize;
  if ( tryY > imageInfo.maxHeight ) {
    tryY = imageInfo.maxHeight;
  }
  *suggX = tryX;
  *suggY = tryY;
  return -OA_ERR_INVALID_SIZE;
}
