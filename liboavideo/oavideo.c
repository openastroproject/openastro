/*****************************************************************************
 *
 * oavideo.c -- main oavideo library entrypoint
 *
 * Copyright 2014 James Fidell (james@openastroproject.org)
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
#include <openastro/video.h>
#include <openastro/video/formats.h>

#include "yuv.h"


int
oaconvert ( void* source, void* target, int xSize, int ySize, int sourceFormat,
    int targetFormat )
{
  if ( targetFormat != OA_PIX_FMT_RGB24 ) {
    // FIX ME -- set errno
    return -1;
  }

  switch ( sourceFormat ) {
    case OA_PIX_FMT_YUV444P:
      oaYUV444PtoRGB888 ( source, target, xSize, ySize );
      break;
    case OA_PIX_FMT_YUV422P:
      oaYUV422PtoRGB888 ( source, target, xSize, ySize );
      break;
    case OA_PIX_FMT_YUV420P:
      oaYUV422PtoRGB888 ( source, target, xSize, ySize );
      break;
    case OA_PIX_FMT_YUYV:
      oaYUYVtoRGB888 ( source, target, xSize, ySize );
      break;
    case OA_PIX_FMT_UYVY:
      oaUYVYtoRGB888 ( source, target, xSize, ySize );
      break;
    case OA_PIX_FMT_YUV411:
      oaYUV411toRGB888 ( source, target, xSize, ySize );
      break;
    default:
      // FIX ME -- set errno
      return -1;
  }

  return 0;
}
