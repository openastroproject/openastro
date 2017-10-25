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

#include <string.h>

#include "yuv.h"
#include "grey.h"

int
oaconvert ( const void* source, void* target, int xSize, int ySize, int sourceFormat,
    int targetFormat )
{
  if ( OA_ISGREYSCALE( targetFormat ) ) {
    oaconvert_greyscale( source, target, xSize, ySize,
        sourceFormat, targetFormat );
  }

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



int
oaconvert_greyscale ( const void* source, void* target, int xSize, int ySize, int sourceFormat,
    int targetFormat )
{
  int length;
  bool swapRB;

  if ( !OA_ISGREYSCALE( targetFormat ) ) {
    // FIX ME -- set errno
    return -1;
  }

  if ( !source || !target )
  {
    // FIX ME -- set errno
    return -1;
  }

  switch ( sourceFormat ) {
    case OA_PIX_FMT_GREY8:
    case OA_PIX_FMT_BGGR8:
    case OA_PIX_FMT_RGGB8:
    case OA_PIX_FMT_GBRG8:
    case OA_PIX_FMT_GRBG8:
      length = xSize * ySize;
      switch ( targetFormat ) {
        case OA_PIX_FMT_GREY8:
          if ( target != source ) memcpy( target, source, length );
          break;
        case OA_PIX_FMT_GREY16LE:
          oaGreyscale_8to16LE( source, target, length );
          break;
        case OA_PIX_FMT_GREY16BE:
          oaGreyscale_8to16BE( source, target, length );
          break;
        default:
          return -1; // FIX ME -- set errno
      }
      break;

/* FIXME awaiting OA_PIX_FMTXXXX10X
    //case OA_PIX_FMT_BGGR10P:
    //case OA_PIX_FMT_RGGB10P:
    //case OA_PIX_FMT_GBRG10P:
    case OA_PIX_FMT_GRBG10P:
      length = xSize * ySize * 1.25;
      switch ( targetFormat ) {
        case OA_PIX_FMT_GREY8:
          oaGreyscale_10to8( source, target, length, true );
          break;
        case OA_PIX_FMT_GREY16LE:
          oaGreyscale_10to16LE( source, target, length, true );
          break;
        case OA_PIX_FMT_GREY16BE:
          oaGreyscale_10to16BE( source, target, length, true );
          break;
        default:
          return -1; // FIX ME -- set errno
      }
      break;

    //case OA_PIX_FMT_BGGR10:
    //case OA_PIX_FMT_RGGB10:
    //case OA_PIX_FMT_GBRG10:
    case OA_PIX_FMT_GRBG10:
      length = xSize * ySize * 2;
      switch ( targetFormat ) {
        case OA_PIX_FMT_GREY8:
          oaGreyscale_10to8( source, target, length, false );
          break;
        case OA_PIX_FMT_GREY16LE:
          oaGreyscale_10to16LE( source, target, length, false );
          break;
        case OA_PIX_FMT_GREY16BE:
          oaGreyscale_10to16BE( source, target, length, false );
          break;
        default:
          return -1; // FIX ME -- set errno
      }
      break;
*/
    case OA_PIX_FMT_GREY16LE:
    case OA_PIX_FMT_BGGR16LE:
    case OA_PIX_FMT_RGGB16LE:
    case OA_PIX_FMT_GBRG16LE:
    case OA_PIX_FMT_GRBG16LE:
      length = xSize * ySize * 2;
      switch ( targetFormat ) {
        case OA_PIX_FMT_GREY8:
          oaGreyscale_16LEto8( source, target, length );
          break;
        case OA_PIX_FMT_GREY16LE:
          if ( target != source) memcpy( target, source, length );
          break;
        case OA_PIX_FMT_GREY16BE:
          oaGreyscale_16swap( source, target, length );
          break;
        default:
          return -1; // FIX ME -- set errno
      }
      break;
      
    case OA_PIX_FMT_GREY16BE:
    case OA_PIX_FMT_BGGR16BE:
    case OA_PIX_FMT_RGGB16BE:
    case OA_PIX_FMT_GBRG16BE:
    case OA_PIX_FMT_GRBG16BE:
      length = xSize * ySize * 2;
      switch ( targetFormat ) {
        case OA_PIX_FMT_GREY8:
          oaGreyscale_16BEto8( source, target, length );
          break;
        case OA_PIX_FMT_GREY16LE:
          oaGreyscale_16swap( source, target, length );
          break;
        case OA_PIX_FMT_GREY16BE:
          if ( target != source ) memcpy( target, source, length );
          break;
        default:
          return -1; // FIX ME -- set errno
      }
      break;

    case OA_PIX_FMT_RGB24:
    case OA_PIX_FMT_BGR24:
      length = xSize * ySize * 3;
      swapRB = sourceFormat == OA_PIX_FMT_BGR24;
      switch ( targetFormat ) {
        case OA_PIX_FMT_GREY8:
          oaGreyscale_RGB24to8( source, target, length, swapRB );
          break;
        case OA_PIX_FMT_GREY16LE:
          oaGreyscale_RGB24to16LE( source, target, length, swapRB );
          break;
        case OA_PIX_FMT_GREY16BE:
          oaGreyscale_RGB24to16BE( source, target, length, swapRB );
          break;
        default:
          return -1; // FIX ME -- set errno
      }
      break;
      
    case OA_PIX_FMT_RGB48LE:
    case OA_PIX_FMT_BGR48LE:
      length = xSize * ySize * 6;
      swapRB = sourceFormat == OA_PIX_FMT_BGR48LE;
      switch ( targetFormat ) {
        case OA_PIX_FMT_GREY8:
          oaGreyscale_RGB48LEto8( source, target, length, swapRB );
          break;
        case OA_PIX_FMT_GREY16LE:
          oaGreyscale_RGB48LEto16LE( source, target, length, swapRB );
          break;
        case OA_PIX_FMT_GREY16BE:
          oaGreyscale_RGB48LEto16BE( source, target, length, swapRB );
          break;
        default:
          return -1; // FIX ME -- set errno
      }
      break;
      
    case OA_PIX_FMT_RGB48BE:
    case OA_PIX_FMT_BGR48BE:
      length = xSize * ySize * 6;
      swapRB = sourceFormat == OA_PIX_FMT_BGR48BE;
      switch ( targetFormat ) {
        case OA_PIX_FMT_GREY8:
          oaGreyscale_RGB48BEto8( source, target, length, swapRB );
          break;
        case OA_PIX_FMT_GREY16LE:
          oaGreyscale_RGB48BEto16LE( source, target, length, swapRB );
          break;
        case OA_PIX_FMT_GREY16BE:
          oaGreyscale_RGB48BEto16BE( source, target, length, swapRB );
          break;
        default:
          return -1; // FIX ME -- set errno
      }
      break;

    //FIXME TODO
    case OA_PIX_FMT_YUV444P:
    case OA_PIX_FMT_YUV422P:
    case OA_PIX_FMT_YUV420P:
    case OA_PIX_FMT_YUV410P:
    case OA_PIX_FMT_YUYV:
    case OA_PIX_FMT_UYVY:
    case OA_PIX_FMT_YUV420:
    case OA_PIX_FMT_YUV411:
    case OA_PIX_FMT_YUV410:
    default:
      return -1; // FIX ME -- set errno
  }
  return 0;
}  

