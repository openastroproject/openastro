/*****************************************************************************
 *
 * IIDCgetState.c -- state querying for IEEE1394/IIDC cameras
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

#if HAVE_LIBDC1394

#include <dc1394/dc1394.h>
#include <openastro/camera.h>

#include "oacamprivate.h"
#include "IIDCoacam.h"
#include "IIDCstate.h"


int
oaIIDCCameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
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

  if ( dc1394_video_get_supported_framerates ( cameraInfo->iidcHandle,
      cameraInfo->videoCurrent, &framerates ) != DC1394_SUCCESS ) {
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
oaIIDCCameraGetFramePixelFormat ( oaCamera* camera, int depth )
{
  IIDC_STATE*		cameraInfo = camera->_private;
  dc1394color_coding_t  codec;

  if ( dc1394_get_color_coding_from_video_mode ( cameraInfo->iidcHandle,
      cameraInfo->videoCurrent, &codec ) == DC1394_SUCCESS ) {

    switch ( codec ) {
      case DC1394_COLOR_CODING_MONO8:
        // Need to fake up the correct response for a TIS colour camera
        // here
        if ( cameraInfo->isTISColour ) {
          return OA_PIX_FMT_GBRG8;
        } else {
          return OA_PIX_FMT_GREY8;
        }
        break;

      case DC1394_COLOR_CODING_MONO16:
        return OA_PIX_FMT_GREY16LE; // FIX ME -- guessing.  Could be BE
        break;

      case DC1394_COLOR_CODING_RGB8:
        return OA_PIX_FMT_RGB24;
        break;

      case DC1394_COLOR_CODING_RAW8:
        // FIX ME -- may not be at all, of course
        // There is a way to tell this, but at the moment it only appears
        // to be on a frame-by-frame basis rather than when the camera is
        // configured
        return OA_PIX_FMT_GBRG8;
        break;

      case DC1394_COLOR_CODING_YUV422:
        // normally this would be OA_PIX_FMT_YUYV, but by definition IIDC
        // cameras use UYVY for YUV422
        return OA_PIX_FMT_UYVY;
        break;

      default:
        break;
    }
  }

  fprintf ( stderr, "%s: dc1394_get_color_coding_from_video_mode failed\n",
      __FUNCTION__ );
    
  return OA_PIX_FMT_RGB24;
}

#endif /* HAVE_LIBDC1394 */
