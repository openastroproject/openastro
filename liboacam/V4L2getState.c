/*****************************************************************************
 *
 * V4L2getState.c -- state querying for V4L2 cameras
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

#if HAVE_LIBV4L2

#include <openastro/camera.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "V4L2.h"
#include "V4L2oacam.h"
#include "V4L2ioctl.h"
#include "V4L2state.h"


int
oaV4L2CameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
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


const FRAMESIZES*
oaV4L2CameraGetFrameSizes ( oaCamera* camera )
{
  V4L2_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[1];
}


const FRAMERATES*
oaV4L2CameraGetFrameRates ( oaCamera* camera, int resX, int resY )
{
  V4L2_STATE*	cameraInfo = camera->_private;
  struct	v4l2_frmivalenum fint;
  int		k, numRates = 0;

  k = 0;
  do {
    OA_CLEAR( fint );
    fint.index = k;
    fint.pixel_format = cameraInfo->videoCurrent;
    fint.width = resX;
    fint.height = resY;
    if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_ENUM_FRAMEINTERVALS,
        &fint )) { 
      if ( EINVAL != errno) { 
        perror("VIDIOC_ENUM_FRAMEINTERVALS");
      }
      break;
    } 
    if ( V4L2_FRMIVAL_TYPE_DISCRETE == fint.type ) { 
      if (!( cameraInfo->frameRates.rates = realloc (
          cameraInfo->frameRates.rates, ( numRates + 1 ) *
          sizeof ( FRAMERATE )))) {
        fprintf ( stderr, "%s: realloc failed\n", __FUNCTION__ );
        return 0;
      }
      cameraInfo->frameRates.rates[numRates].numerator =
          fint.discrete.numerator;
      cameraInfo->frameRates.rates[numRates].denominator =
          fint.discrete.denominator;
      numRates++;
    } 
    k++;
  } while ( 1 );

  cameraInfo->frameRates.numRates = numRates;
  return &cameraInfo->frameRates;
}


int
oaV4L2CameraGetFramePixelFormat ( oaCamera* camera, int depth )
{
  V4L2_STATE*		cameraInfo = camera->_private;

  // the driver lies for the colour TIS DxK cameras, so we fix it here
  if ( cameraInfo->colourDxK ) {
    return OA_PIX_FMT_GBRG8;
  }

  switch ( cameraInfo->videoCurrent ) {
    case V4L2_PIX_FMT_RGB24:
      return OA_PIX_FMT_RGB24;
      break;
    case V4L2_PIX_FMT_GREY:
      return OA_PIX_FMT_GREY8;
      break;
    case V4L2_PIX_FMT_SBGGR8:
      return OA_PIX_FMT_BGGR8;
    case V4L2_PIX_FMT_SRGGB8:
      return OA_PIX_FMT_RGGB8;
    case V4L2_PIX_FMT_SGRBG8:
      return OA_PIX_FMT_GRBG8;
    case V4L2_PIX_FMT_SGBRG8:
      return OA_PIX_FMT_GBRG8;
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_Y16:
    default:
      fprintf ( stderr, "%s: can't handle pixel format %d\n", __FUNCTION__,
          cameraInfo->videoCurrent );
      return OA_PIX_FMT_RGB24;
      break;
  }
}

#endif /* HAVE_LIBV4L2 */
