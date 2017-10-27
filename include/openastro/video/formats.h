/*****************************************************************************
 *
 * oacam.h -- camera API (sub)header for frame formats
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

#ifndef OPENASTRO_CAMERA_FORMATS_H
#define OPENASTRO_CAMERA_FORMATS_H

#define DEFINE_OA_PIX_FMT(OA_PIX_FMT) \
  OA_PIX_FMT(RGB24) \
  OA_PIX_FMT(BGR24) \
  OA_PIX_FMT(GREY8) \
  OA_PIX_FMT(GREY16BE) \
  OA_PIX_FMT(GREY16LE) \
  OA_PIX_FMT(BGGR8) \
  OA_PIX_FMT(RGGB8) \
  OA_PIX_FMT(GBRG8) \
  OA_PIX_FMT(GRBG8) \
  OA_PIX_FMT(BGGR16LE) \
  OA_PIX_FMT(BGGR16BE) \
  OA_PIX_FMT(RGGB16LE) \
  OA_PIX_FMT(RGGB16BE) \
  OA_PIX_FMT(GBRG16LE) \
  OA_PIX_FMT(GBRG16BE) \
  OA_PIX_FMT(GRBG16LE) \
  OA_PIX_FMT(GRBG16BE) \
  OA_PIX_FMT(RGB48BE) \
  OA_PIX_FMT(RGB48LE) \
  OA_PIX_FMT(BGR48BE) \
  OA_PIX_FMT(BGR48LE) \
  \
  OA_PIX_FMT(YUV444P) \
  OA_PIX_FMT(YUV422P) \
  OA_PIX_FMT(YUV420P) \
  OA_PIX_FMT(YUV410P) \
  OA_PIX_FMT(YUYV) \
  OA_PIX_FMT(UYVY) \
  OA_PIX_FMT(YUV420) \
  OA_PIX_FMT(YUV411) \
  OA_PIX_FMT(YUV410) \


#define OA_PIX_FMT_GRAY8    OA_PIX_FMT_GREY8 
#define OA_PIX_FMT_GRAY16BE OA_PIX_FMT_GREY16BE
#define OA_PIX_FMT_GRAY16LE OA_PIX_FMT_GREY16LE


#define ENUM(FMT) OA_PIX_FMT_##FMT,
enum { OA_PIX_FMT_NONE = 0,
       DEFINE_OA_PIX_FMT(ENUM)
       OA_PIX_FMT_MAX };

#define STRING(FMT) #FMT,
static const char *oa_pix_fmt_strings[] = {
    "error", DEFINE_OA_PIX_FMT(STRING)
};


#define OA_PIX_FMT_STRING(x) \
    oa_pix_fmt_strings[x>0 && x<OA_PIX_FMT_MAX ? x : 0]

#define OA_ISGREYSCALE(x) \
    (( x == OA_PIX_FMT_GREY8 ) || ( x == OA_PIX_FMT_GREY16BE ) || \
    ( x == OA_PIX_FMT_GREY16LE ))

#define OA_ISLITTE_ENDIAN(x) \
    (( x == OA_PIX_FMT_BGGR16LE ) || ( x == OA_PIX_FMT_RGGB16LE ) || \
    ( x == OA_PIX_FMT_GBRG16LE ) || ( x == OA_PIX_FMT_GRBG16LE ) || \
    ( x == OA_PIX_FMT_GREY16LE ))

#define OA_ISBAYER(x) \
    (( x == OA_PIX_FMT_BGGR8 ) || ( x == OA_PIX_FMT_RGGB8 ) || \
    ( x == OA_PIX_FMT_GBRG8 ) || ( x == OA_PIX_FMT_GRBG8 ) || \
    ( x == OA_PIX_FMT_BGGR16LE ) || ( x == OA_PIX_FMT_BGGR16BE ) || \
    ( x == OA_PIX_FMT_RGGB16LE ) || ( x == OA_PIX_FMT_RGGB16BE ) || \
    ( x == OA_PIX_FMT_GBRG16LE ) || ( x == OA_PIX_FMT_GBRG16BE ) || \
    ( x == OA_PIX_FMT_GRBG16LE ) || ( x == OA_PIX_FMT_GRBG16BE ))

#define OA_ISBAYER8(x) \
    (( x == OA_PIX_FMT_BGGR8 ) || ( x == OA_PIX_FMT_RGGB8 ) || \
    ( x == OA_PIX_FMT_GBRG8 ) || ( x == OA_PIX_FMT_GRBG8 ))

#define OA_ISBAYER16(x) \
    (( x == OA_PIX_FMT_BGGR16LE ) || ( x == OA_PIX_FMT_BGGR16BE ) || \
    ( x == OA_PIX_FMT_RGGB16LE ) || ( x == OA_PIX_FMT_RGGB16BE ) || \
    ( x == OA_PIX_FMT_GBRG16LE ) || ( x == OA_PIX_FMT_GBRG16BE ) || \
    ( x == OA_PIX_FMT_GRBG16LE ) || ( x == OA_PIX_FMT_GRBG16BE ))

#define OA_BYTES_PER_PIXEL(x) \
    ((( x == OA_PIX_FMT_BGGR16LE ) || ( x == OA_PIX_FMT_BGGR16BE ) || \
    ( x == OA_PIX_FMT_RGGB16LE ) || ( x == OA_PIX_FMT_RGGB16BE ) || \
    ( x == OA_PIX_FMT_GBRG16LE ) || ( x == OA_PIX_FMT_GBRG16BE ) || \
    ( x == OA_PIX_FMT_GRBG16LE ) || ( x == OA_PIX_FMT_GRBG16BE ) || \
    ( x == OA_PIX_FMT_GREY16BE ) || ( x == OA_PIX_FMT_GREY16LE ) || \
    ( x == OA_PIX_FMT_YUYV ) || ( x == OA_PIX_FMT_YUV422P ) || \
    ( x == OA_PIX_FMT_UYVY )) ? 2 : \
    (( x == OA_PIX_FMT_RGB24 ) || ( x == OA_PIX_FMT_BGR24 ) || \
    ( x == OA_PIX_FMT_YUV444P )) ? 3 : \
    ( x == OA_PIX_FMT_YUV411 ) ? ( 4.0/6.0) : \
    (( x == OA_PIX_FMT_RGB48BE ) || ( x == OA_PIX_FMT_RGB48LE ) || \
    ( x == OA_PIX_FMT_BGR48BE ) || ( x == OA_PIX_FMT_BGR48LE )) ? 6 : 1 )

#define OA_IS_LUM_CHROM(x) \
  ( x >= OA_PIX_FMT_YUV444P && x <= OA_PIX_FMT_YUV410 )

#define OA_DEMOSAIC_FMT(x) \
  ((( x == OA_PIX_FMT_BGGR8 ) || ( x == OA_PIX_FMT_RGGB8 ) || \
  ( x == OA_PIX_FMT_GBRG8 ) || ( x == OA_PIX_FMT_GRBG8 )) \
  ? OA_PIX_FMT_RGB24 : \
  (( x == OA_PIX_FMT_BGGR16LE ) || ( x == OA_PIX_FMT_RGGB16LE ) || \
  ( x == OA_PIX_FMT_GBRG16LE ) || ( x == OA_PIX_FMT_GRBG16LE )) \
  ?  OA_PIX_FMT_RGB48LE : \
  (( x == OA_PIX_FMT_BGGR16BE ) || ( x == OA_PIX_FMT_RGGB16BE ) || \
  ( x == OA_PIX_FMT_GBRG16BE ) || ( x == OA_PIX_FMT_GRBG16BE )) \
  ?  OA_PIX_FMT_RGB48BE : 0 )

#endif	/* OPENASTRO_CAMERA_FORMATS_H */
