/*****************************************************************************
 *
 * UVCgetState.c -- state querying for UVC cameras
 *
 * Copyright 2014,2015 James Fidell (james@openastroproject.org)
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

  frame = cameraInfo->videoCurrent->frame_descs;
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
oaUVCCameraGetFramePixelFormat ( oaCamera* camera, int depth )
{
  UVC_STATE*	cameraInfo = camera->_private;

  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "BY8 ", 4 )) {
    // may not be at all, of course
    return OA_PIX_FMT_GBRG8;
  }

  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "GRBG", 4 )) {
    return OA_PIX_FMT_GRBG8;
  }
  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "GBRG", 4 )) {
    return OA_PIX_FMT_GBRG8;
  }
  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "RGGB", 4 )) {
    return OA_PIX_FMT_RGGB8;
  }
  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "BGGR", 4 )) {
    return OA_PIX_FMT_BGGR8;
  }

  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "BA81", 4 )) {
    return OA_PIX_FMT_BGGR8;
  }

  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "Y800", 4 )) {
    return OA_PIX_FMT_GREY8;
  }

  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "Y16 ", 4 )) {
    // this is a guess until someone can tell me definitively what it is
    return OA_PIX_FMT_GREY16LE;
  }

  if ( !memcmp ( cameraInfo->videoCurrent->fourccFormat, "YUY2", 4 )) {
    return OA_PIX_FMT_YUYV;
  }

  fprintf ( stderr, "%s can't handle pixel format %16s\n", __FUNCTION__,
      cameraInfo->videoCurrent->guidFormat );
  return OA_PIX_FMT_RGB24;
}


#endif /* HAVE_LIBUVC */
