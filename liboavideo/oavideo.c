/*****************************************************************************
 *
 * oavideo.c -- main oavideo library entrypoint
 *
 * Copyright 2014,2017,2018 James Fidell (james@openastroproject.org)
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
#include "to8Bit.h"


int
oaconvert ( void* source, void* target, int xSize, int ySize, int sourceFormat,
    int targetFormat )
{
  int		result = -1;
  unsigned int	length;

  // I wonder if this might be better off table-driven in the end

  switch ( sourceFormat ) {
    case OA_PIX_FMT_GREY10_16BE:
    case OA_PIX_FMT_GREY12_16BE:
    case OA_PIX_FMT_GREY14_16BE:
    case OA_PIX_FMT_GREY16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_GREY8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_CMYG16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_CMYG8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_MCGY16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_MCGY8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_YGCM16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_YGCM8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_GYMC16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_GYMC8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_GREY10_16LE:
    case OA_PIX_FMT_GREY12_16LE:
    case OA_PIX_FMT_GREY14_16LE:
    case OA_PIX_FMT_GREY16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_GREY8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_CMYG16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_CMYG8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_MCGY16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_MCGY8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_YGCM16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_YGCM8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_GYMC16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_GYMC8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_GREY12:
      length = xSize * ySize * 3 / 2;
      if ( OA_PIX_FMT_GREY8 == targetFormat ) {
        oaPackedGrey12ToGrey8 ( source, target, length );
        result = 0;
      }
      break;

    case OA_PIX_FMT_BGGR10_16BE:
    case OA_PIX_FMT_BGGR12_16BE:
    case OA_PIX_FMT_BGGR14_16BE:
    case OA_PIX_FMT_BGGR16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_BGGR8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_RGGB10_16BE:
    case OA_PIX_FMT_RGGB12_16BE:
    case OA_PIX_FMT_RGGB14_16BE:
    case OA_PIX_FMT_RGGB16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_RGGB8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_GRBG10_16BE:
    case OA_PIX_FMT_GRBG12_16BE:
    case OA_PIX_FMT_GRBG14_16BE:
    case OA_PIX_FMT_GRBG16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_GRBG8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_GBRG10_16BE:
    case OA_PIX_FMT_GBRG12_16BE:
    case OA_PIX_FMT_GBRG14_16BE:
    case OA_PIX_FMT_GBRG16BE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_GBRG8 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_BGGR10_16LE:
    case OA_PIX_FMT_BGGR12_16LE:
    case OA_PIX_FMT_BGGR14_16LE:
    case OA_PIX_FMT_BGGR16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_BGGR8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_RGGB10_16LE:
    case OA_PIX_FMT_RGGB12_16LE:
    case OA_PIX_FMT_RGGB14_16LE:
    case OA_PIX_FMT_RGGB16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_RGGB8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_GRBG10_16LE:
    case OA_PIX_FMT_GRBG12_16LE:
    case OA_PIX_FMT_GRBG14_16LE:
    case OA_PIX_FMT_GRBG16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_GRBG8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_GBRG10_16LE:
    case OA_PIX_FMT_GBRG12_16LE:
    case OA_PIX_FMT_GBRG14_16LE:
    case OA_PIX_FMT_GBRG16LE:
      length = 2 * xSize * ySize;
      if ( OA_PIX_FMT_GBRG8 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_RGB30BE:
    case OA_PIX_FMT_RGB36BE:
    case OA_PIX_FMT_RGB42BE:
    case OA_PIX_FMT_RGB48BE:
      length = 6 * xSize * ySize;
      if ( OA_PIX_FMT_RGB24 == targetFormat ) {
        oaBigEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_RGB30LE:
    case OA_PIX_FMT_RGB36LE:
    case OA_PIX_FMT_RGB42LE:
    case OA_PIX_FMT_RGB48LE:
      length = 6 * xSize * ySize;
      if ( OA_PIX_FMT_RGB24 == targetFormat ) {
        oaLittleEndian16BitTo8Bit ( source, target, length );
        result = 0;
      }
      break;
    case OA_PIX_FMT_YUV444P:
      if ( OA_PIX_FMT_RGB24 == targetFormat ) {
        oaYUV444PtoRGB888 ( source, target, xSize, ySize );
        result = 0;
      }
      break;
    case OA_PIX_FMT_YUV422P:
      if ( OA_PIX_FMT_RGB24 == targetFormat ) {
        oaYUV422PtoRGB888 ( source, target, xSize, ySize );
        result = 0;
      }
      break;
    case OA_PIX_FMT_YUV420P:
      if ( OA_PIX_FMT_RGB24 == targetFormat ) {
        oaYUV420PtoRGB888 ( source, target, xSize, ySize );
        result = 0;
      }
      break;
    case OA_PIX_FMT_YUYV:
      if ( OA_PIX_FMT_RGB24 == targetFormat ) {
        oaYUYVtoRGB888 ( source, target, xSize, ySize );
        result = 0;
      }
      break;
    case OA_PIX_FMT_UYVY:
      if ( OA_PIX_FMT_RGB24 == targetFormat ) {
        oaUYVYtoRGB888 ( source, target, xSize, ySize );
        result = 0;
      }
      break;
    case OA_PIX_FMT_YUV411:
      if ( OA_PIX_FMT_RGB24 == targetFormat ) {
        oaYUV411toRGB888 ( source, target, xSize, ySize );
        result = 0;
      } else {
        fprintf ( stderr, "unknown target format for YUV411: %d\n",
            targetFormat );
      }
      break;
    case OA_PIX_FMT_BGGR10:
    case OA_PIX_FMT_BGGR12:
    case OA_PIX_FMT_BGGR14:
    case OA_PIX_FMT_RGGB10:
    case OA_PIX_FMT_RGGB12:
    case OA_PIX_FMT_RGGB14:
    case OA_PIX_FMT_GRBG10:
    case OA_PIX_FMT_GRBG12:
    case OA_PIX_FMT_GRBG14:
    case OA_PIX_FMT_GBRG10:
    case OA_PIX_FMT_GBRG12:
    case OA_PIX_FMT_GBRG14:
    default:
      fprintf ( stderr, "unhandled format %d in %s\n", sourceFormat,
          __FUNCTION__ );
  }

  return result;
}
