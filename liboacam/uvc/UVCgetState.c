/*****************************************************************************
 *
 * UVCgetState.c -- state querying for UVC cameras
 *
 * Copyright 2014,2015,2017,2018 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBUVC

#include <openastro/camera.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "UVC.h"
#include "UVCoacam.h"
#include "UVCstate.h"


int
oaUVCCameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
    int64_t* max, int64_t* step, int64_t* def )
{
  COMMON_INFO*	commonInfo = camera->_common;

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL; 
  }

  *min = commonInfo->OA_CAM_CTRL_MIN( control );
  *max = commonInfo->OA_CAM_CTRL_MAX( control );
  *step = commonInfo->OA_CAM_CTRL_STEP( control );
  *def = commonInfo->OA_CAM_CTRL_DEF( control );
  return OA_ERR_NONE;
}


const FRAMESIZES*
oaUVCCameraGetFrameSizes ( oaCamera* camera )
{
  UVC_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[1];
}


const FRAMERATES*
oaUVCCameraGetFrameRates ( oaCamera* camera, int resX, int resY )
{
  UVC_STATE*		cameraInfo = camera->_private;
  uvc_frame_desc_t*	frame;
  uint32_t*		interval;
  int			i;

  frame = cameraInfo->currentUVCFormat->frame_descs;
  do {
    if ( frame->wWidth == resX && frame->wHeight == resY ) {
      break;
    }
    frame = frame->next;
  } while ( frame );

  if ( !frame ) {
    fprintf ( stderr, "%s: no frame rates size matches found\n", __FUNCTION__ );
    return 0;
  }

  if ( cameraInfo->frameRates.numRates ) {
   free (( void* ) cameraInfo->frameRates.rates );
  }
  cameraInfo->frameRates.rates = 0;

  i = 0;
  interval = frame->intervals;
  while ( interval && *interval ) {
    if (!( cameraInfo->frameRates.rates = realloc (
      cameraInfo->frameRates.rates, ( i + 1 ) * sizeof ( FRAMERATE )))) {
      fprintf ( stderr, "%s: realloc failed\n", __FUNCTION__ );
      return 0;
    }
    // interval units are 100ns
    cameraInfo->frameRates.rates[ i ].numerator = 1;
    cameraInfo->frameRates.rates[ i ].denominator =
        ( int ) 10000000 / *interval;
    i++;
    interval++;
  }

  if ( !i ) {
    fprintf ( stderr, "%s: no frame rates found\n", __FUNCTION__ );
    return 0;
  }

  cameraInfo->frameRates.numRates = i;
  return &cameraInfo->frameRates;
}


int
oaUVCCameraGetFramePixelFormat ( oaCamera* camera )
{
  UVC_STATE*	cameraInfo = camera->_private;

  return cameraInfo->currentFrameFormat;
}


int
oaUVCCameraGetControlDiscreteSet ( oaCamera* camera, int control,
    int32_t* count, int64_t** values )
{
  UVC_STATE*    cameraInfo = camera->_private;

  if ( control != OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) &&
      control != OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *count = cameraInfo->numAutoExposureItems;
  *values = cameraInfo->autoExposureMenuItems;
  return OA_ERR_NONE;
}

#endif /* HAVE_LIBUVC */
