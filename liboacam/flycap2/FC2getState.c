/*****************************************************************************
 *
 * FC2getState.c -- state querying for Point Grey Gig-E cameras
 *
 * Copyright 2015,2016,2018,2019 James Fidell (james@openastroproject.org)
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

#include <flycapture/C/FlyCapture2_C.h>
#include <openastro/camera.h>

#include "oacamprivate.h"
#include "FC2oacam.h"
#include "FC2state.h"
#include "FC2private.h"


int
oaFC2CameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
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


int
oaFC2CameraGetControlDiscreteSet ( oaCamera* camera, int control,
    int32_t* count, int64_t** values )
{
  FC2_STATE*	cameraInfo = camera->_private;

  if ( control != OA_CAM_CTRL_TRIGGER_MODE ) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *count = cameraInfo->triggerModeCount;
  *values = cameraInfo->triggerModes;
  return OA_ERR_NONE;
}


const FRAMESIZES*
oaFC2CameraGetFrameSizes ( oaCamera* camera )
{
  FC2_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[ cameraInfo->binMode ];
}


const FRAMERATES*
oaFC2CameraGetFrameRates ( oaCamera* camera, int resX, int resY )
{
fprintf ( stderr, "implement %s\n", __FUNCTION__ );
  FC2_STATE*		cameraInfo = camera->_private;
/*
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
*/
  return &cameraInfo->frameRates;
}


int
oaFC2CameraGetFramePixelFormat ( oaCamera* camera )
{
  FC2_STATE*		cameraInfo = camera->_private;
  fc2GigEImageSettings	settings;

  if (( *p_fc2GetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get image info\n" );
    return 0;
  }

  switch ( settings.pixelFormat ) {
    case FC2_PIXEL_FORMAT_MONO8:
      return OA_PIX_FMT_GREY8;
      break;
    case FC2_PIXEL_FORMAT_RGB8:
      return OA_PIX_FMT_RGB24;
      break;
    case FC2_PIXEL_FORMAT_RAW8:
      return cameraInfo->cfaPattern;
      break;
    case FC2_PIXEL_FORMAT_BGR:
      return OA_PIX_FMT_BGR24;
      break;
    case FC2_PIXEL_FORMAT_MONO12:
    case FC2_PIXEL_FORMAT_MONO16:
    case FC2_PIXEL_FORMAT_S_MONO16:
      return cameraInfo->bigEndian ? OA_PIX_FMT_GREY16BE : OA_PIX_FMT_GREY16LE;
      break;
    case FC2_PIXEL_FORMAT_RGB16:
    case FC2_PIXEL_FORMAT_S_RGB16:
    case FC2_PIXEL_FORMAT_RAW12:
    case FC2_PIXEL_FORMAT_RAW16:
    case FC2_PIXEL_FORMAT_BGR16:
      fprintf ( stderr, "No idea of byte order for >8bit colour format %d\n",
          settings.pixelFormat );
      break;

    case FC2_PIXEL_FORMAT_411YUV8:
      return OA_PIX_FMT_YUV411;

    case FC2_PIXEL_FORMAT_422YUV8:
      return OA_PIX_FMT_YUV422;

    case FC2_PIXEL_FORMAT_444YUV8:
      return OA_PIX_FMT_YUV444;

    case FC2_PIXEL_FORMAT_BGRU:
    case FC2_PIXEL_FORMAT_422YUV8_JPEG:
      fprintf ( stderr, "Unhandled colour format %d\n",
          settings.pixelFormat );
      break;

    default:
      fprintf ( stderr, "Unknown colour format %d\n", settings.pixelFormat );
      break;
  }

  return OA_PIX_FMT_RGB24;
}
