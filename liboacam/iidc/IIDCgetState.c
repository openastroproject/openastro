/*****************************************************************************
 *
 * IIDCgetState.c -- state querying for IEEE1394/IIDC cameras
 *
 * Copyright 2013,2014,2015,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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

#if HAVE_LIBDC1394

#include <dc1394/dc1394.h>
#include <openastro/camera.h>

#include "oacamprivate.h"
#include "IIDCoacam.h"
#include "IIDCstate.h"
#include "IIDCprivate.h"


int
oaIIDCCameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
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
oaIIDCCameraGetFrameSizes ( oaCamera* camera )
{
  IIDC_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[1];
}


const FRAMERATES*
oaIIDCCameraGetFrameRates ( oaCamera* camera, int resX, int resY )
{
  IIDC_STATE*		cameraInfo = camera->_private;
  dc1394framerates_t    framerates;
  int			numRates, numerator, denominator;
  unsigned int		i, matched;

  if ( p_dc1394_video_get_supported_framerates ( cameraInfo->iidcHandle,
      cameraInfo->currentIIDCMode, &framerates ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_video_get_supported_framerates failed\n",
         __FUNCTION__ );
    return 0;
  }
  if ( !framerates.num ) {
    fprintf ( stderr, "%s: dc1394_video_get_supported_framerates returns "
        "no frame rates\n", __FUNCTION__ );
    return 0;
  }

  if ( cameraInfo->frameRates.numRates ) {
   free (( void* ) cameraInfo->frameRates.rates );
  }
  cameraInfo->frameRates.rates = 0;

  numRates = 0;
  for ( i = 0; i < framerates.num; i++ ) {
    matched = 1;
    switch ( framerates.framerates[i] ) {
      case DC1394_FRAMERATE_1_875:
        numerator = 8;
        denominator = 15;
        break;
      case DC1394_FRAMERATE_3_75:
        numerator = 4;
        denominator = 15;
        break;
      case DC1394_FRAMERATE_7_5:
        numerator = 2;
        denominator = 15;
        break;
      case DC1394_FRAMERATE_15:
        numerator = 1;
        denominator = 15;
        break;
      case DC1394_FRAMERATE_30:
        numerator = 1;
        denominator = 30;
        break;
      case DC1394_FRAMERATE_60:
        numerator = 1;
        denominator = 60;
        break;
      case DC1394_FRAMERATE_120:
        numerator = 1;
        denominator = 120;
        break;
      case DC1394_FRAMERATE_240:
        numerator = 1;
        denominator = 240;
        break;
      default:
        fprintf ( stderr, "%s: unknown frame rate %d\n", __FUNCTION__,
            framerates.framerates[i] );
        matched = 0;
        break;
    }

    if ( matched ) {
      if (!( cameraInfo->frameRates.rates = realloc (
          cameraInfo->frameRates.rates, ( numRates + 1 ) *
          sizeof ( FRAMERATE )))) {
        fprintf ( stderr, "%s: realloc failed\n", __FUNCTION__ );
        return 0;
      }
      cameraInfo->frameRates.rates[ numRates ].numerator = numerator;
      cameraInfo->frameRates.rates[ numRates ].denominator = denominator;
      numRates++;
    }
  }

  if ( !numRates ) {
    fprintf ( stderr, "%s: no frame rates found\n", __FUNCTION__ );
    return 0;
  }

  cameraInfo->frameRates.numRates = numRates;
  return &cameraInfo->frameRates;
}


int
oaIIDCCameraGetFramePixelFormat ( oaCamera* camera )
{
  IIDC_STATE*		cameraInfo = camera->_private;

  return cameraInfo->currentFrameFormat;
}

#endif /* HAVE_LIBDC1394 */
