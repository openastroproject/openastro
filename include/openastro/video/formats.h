/*****************************************************************************
 *
 * oacam.h -- camera API (sub)header for frame formats
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

#ifndef OPENASTRO_CAMERA_FORMATS_H
#define OPENASTRO_CAMERA_FORMATS_H

#define	OA_PIX_FMT_RGB24		1
#define	OA_PIX_FMT_BGR24		2
#define	OA_PIX_FMT_GREY8		3
#define	OA_PIX_FMT_GRAY8		OA_PIX_FMT_GREY8
#define	OA_PIX_FMT_GREY16BE		4
#define	OA_PIX_FMT_GRAY16BE		OA_PIX_FMT_GREY16BE
#define	OA_PIX_FMT_GREY16LE		5
#define	OA_PIX_FMT_GRAY16LE		OA_PIX_FMT_GREY16LE
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

#define OA_PIX_FMT_RGB30BE		22
#define OA_PIX_FMT_RGB30LE		23
#define OA_PIX_FMT_RGB36BE		24
#define OA_PIX_FMT_RGB36LE		25
#define OA_PIX_FMT_RGB42BE		26
#define OA_PIX_FMT_RGB42LE		27
#define OA_PIX_FMT_GREY10		28
#define OA_PIX_FMT_GRAY10		OA_PIX_FMT_GREY10
#define OA_PIX_FMT_GREY12		29
#define OA_PIX_FMT_GRAY12		OA_PIX_FMT_GRAY12
#define OA_PIX_FMT_GREY14		30
#define OA_PIX_FMT_GRAY14		OA_PIX_FMT_GRAY14

#define OA_PIX_FMT_BGGR10LE             31
#define OA_PIX_FMT_BGGR10BE             32
#define OA_PIX_FMT_RGGB10LE             33
#define OA_PIX_FMT_RGGB10BE             34
#define OA_PIX_FMT_GBRG10LE             35
#define OA_PIX_FMT_GBRG10BE             36
#define OA_PIX_FMT_GRBG10LE             37
#define OA_PIX_FMT_GRBG10BE             38

#define OA_PIX_FMT_BGGR12LE             39
#define OA_PIX_FMT_BGGR12BE             40
#define OA_PIX_FMT_RGGB12LE             41
#define OA_PIX_FMT_RGGB12BE             42
#define OA_PIX_FMT_GBRG12LE             43
#define OA_PIX_FMT_GBRG12BE             44
#define OA_PIX_FMT_GRBG12LE             45
#define OA_PIX_FMT_GRBG12BE             46

#define OA_PIX_FMT_BGGR14LE             47
#define OA_PIX_FMT_BGGR14BE             48
#define OA_PIX_FMT_RGGB14LE             49
#define OA_PIX_FMT_RGGB14BE             50
#define OA_PIX_FMT_GBRG14LE             51
#define OA_PIX_FMT_GBRG14BE             52
#define OA_PIX_FMT_GRBG14LE             53
#define OA_PIX_FMT_GRBG14BE             54

#define OA_PIX_FMT_YUV444P 		55
#define OA_PIX_FMT_YUV422P 		56
#define OA_PIX_FMT_YUV420P		57
#define OA_PIX_FMT_YUV411P		58
#define OA_PIX_FMT_YUV410P		59
#define OA_PIX_FMT_YUV444  		60
#define OA_PIX_FMT_YUV422  		61
#define OA_PIX_FMT_YUYV  		OA_PIX_FMT_YUV422
#define OA_PIX_FMT_UYVY  		62
#define OA_PIX_FMT_YUV420 		63
#define OA_PIX_FMT_YUV411 		64
#define OA_PIX_FMT_YUV410 		65

#define OA_PIX_FMT_GREY10_16BE          66
#define OA_PIX_FMT_GRAY10_16BE          OA_PIX_FMT_GREY10_16BE
#define OA_PIX_FMT_GREY10_16LE          67
#define OA_PIX_FMT_GRAY10_16LE          OA_PIX_FMT_GREY10_16LE
#define OA_PIX_FMT_GREY12_16BE          68
#define OA_PIX_FMT_GRAY12_16BE          OA_PIX_FMT_GREY12_16BE
#define OA_PIX_FMT_GREY12_16LE          69
#define OA_PIX_FMT_GRAY12_16LE          OA_PIX_FMT_GREY12_16LE
#define OA_PIX_FMT_GREY14_16BE          70
#define OA_PIX_FMT_GRAY14_16BE          OA_PIX_FMT_GREY14_16BE
#define OA_PIX_FMT_GREY14_16LE          71
#define OA_PIX_FMT_GRAY14_16LE          OA_PIX_FMT_GREY14_16LE

// Adding more frame formats here requires the oaFrameFormats table
// updating in liboavideo/formats.c

#define OA_PIX_FMT_LAST_P1		OA_PIX_FMT_GREY14_16LE+1

#define OA_ISBAYER8(x) \
    (( x == OA_PIX_FMT_BGGR8 ) || ( x == OA_PIX_FMT_RGGB8 ) || \
    ( x == OA_PIX_FMT_GBRG8 ) || ( x == OA_PIX_FMT_GRBG8 ))

#define OA_ISBAYER16(x) \
    (( x == OA_PIX_FMT_BGGR16LE ) || ( x == OA_PIX_FMT_BGGR16BE ) || \
    ( x == OA_PIX_FMT_RGGB16LE ) || ( x == OA_PIX_FMT_RGGB16BE ) || \
    ( x == OA_PIX_FMT_GBRG16LE ) || ( x == OA_PIX_FMT_GBRG16BE ) || \
    ( x == OA_PIX_FMT_GRBG16LE ) || ( x == OA_PIX_FMT_GRBG16BE ))

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

typedef struct {
  const char*	name;
  const char*	simpleName;
  float		bytesPerPixel;
  unsigned int	bitsPerPixel;
  unsigned int	cfaPattern;
  unsigned int	littleEndian : 1;
  unsigned int	monochrome : 1;
  unsigned int	rawColour : 1;
  unsigned int	fullColour : 1;
  unsigned int	lumChrom : 1;
  unsigned int	lossless : 1;
  unsigned int	packed : 1;
} frameFormatInfo;

extern frameFormatInfo oaFrameFormats[ OA_PIX_FMT_LAST_P1 ];

#endif	/* OPENASTRO_CAMERA_FORMATS_H */
