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

#define	OA_PIX_FMT_RGB24		1
#define	OA_PIX_FMT_BGR24		2
#define	OA_PIX_FMT_GREY8		3
#define	OA_PIX_FMT_GRAY8		3
#define	OA_PIX_FMT_GREY16BE		4
#define	OA_PIX_FMT_GRAY16BE		4
#define	OA_PIX_FMT_GREY16LE		5
#define	OA_PIX_FMT_GRAY16LE		5
#define	OA_PIX_FMT_BGGR8		6
#define	OA_PIX_FMT_RGGB8		7
#define	OA_PIX_FMT_GBRG8		8
#define	OA_PIX_FMT_GRBG8		9
#define	OA_PIX_FMT_BGGR16LE		10
#define	OA_PIX_FMT_BGGR16BE		11
#define	OA_PIX_FMT_RGGB16LE		12
#define	OA_PIX_FMT_RGGB16BE		13
#define	OA_PIX_FMT_GBRG16LE		14
#define	OA_PIX_FMT_GBRG16BE		15
#define	OA_PIX_FMT_GRBG16LE		16
#define	OA_PIX_FMT_GRBG16BE		17
#define OA_PIX_FMT_RGB48BE		18
#define OA_PIX_FMT_RGB48LE		19
#define OA_PIX_FMT_BGR48BE		20
#define OA_PIX_FMT_BGR48LE		21

#define OA_PIX_FMT_YUV444P 		22
#define OA_PIX_FMT_YUV422P 		23
#define OA_PIX_FMT_YUV420P		24
#define OA_PIX_FMT_YUV410P		25
#define OA_PIX_FMT_YUYV  		27
#define OA_PIX_FMT_UYVY  		28
#define OA_PIX_FMT_YUV420 		29
#define OA_PIX_FMT_YUV411 		30
#define OA_PIX_FMT_YUV410 		31

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
