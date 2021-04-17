/*****************************************************************************
 *
 * Spincontrol.c -- control functions for Spinnaker cameras
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

#include <pthread.h>
#include <openastro/camera.h>

#include "oacamprivate.h"
#include "Spinstate.h"


int
oaSpinCameraTestControl ( oaCamera* camera, int control,
    oaControlValue* valp )
{
	SPINNAKER_STATE*	cameraInfo = camera->_private;
	int32_t						val_s32;

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->OA_CAM_CTRL_TYPE( control ) != valp->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

	if ( OA_CAM_CTRL_BINNING == control ) {
		val_s32 = valp->discrete;
		if ( val_s32 < 0 || val_s32 > OA_MAX_BINNING ||
				cameraInfo->frameSizes[ val_s32 ].numSizes < 1 ) {
			return -OA_ERR_OUT_OF_RANGE;
		}
		return OA_ERR_NONE;
	}

  return OA_ERR_NONE;
}
