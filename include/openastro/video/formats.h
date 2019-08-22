/*****************************************************************************
 *
 * formats.h -- camera API (sub)header for frame formats
 *
 * Copyright 2014,2017,2018,2019 James Fidell (james@openastroproject.org)
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

#define OA_PIX_FMT_BGGR10               31
#define OA_PIX_FMT_RGGB10               32
#define OA_PIX_FMT_GBRG10               33
#define OA_PIX_FMT_GRBG10               34
#define OA_PIX_FMT_BGGR12               35
#define OA_PIX_FMT_RGGB12               36
#define OA_PIX_FMT_GBRG12               37
#define OA_PIX_FMT_GRBG12               38
#define OA_PIX_FMT_BGGR14               39
#define OA_PIX_FMT_RGGB14               40
#define OA_PIX_FMT_GBRG14               41
#define OA_PIX_FMT_GRBG14               42

#define OA_PIX_FMT_YUV444P 		43	/* here, P is for planar */
#define OA_PIX_FMT_YUV422P 		44
#define OA_PIX_FMT_YUV420P		45
#define OA_PIX_FMT_YUV411P		46
#define OA_PIX_FMT_YUV410P		47
#define OA_PIX_FMT_YUV444  		48
#define OA_PIX_FMT_YUV422  		49
#define OA_PIX_FMT_YUYV  		OA_PIX_FMT_YUV422
#define OA_PIX_FMT_UYVY  		50
#define OA_PIX_FMT_YUV420 		51
#define OA_PIX_FMT_YUV411 		52
#define OA_PIX_FMT_YUV410 		53

#define OA_PIX_FMT_GREY10_16BE          54
#define OA_PIX_FMT_GRAY10_16BE          OA_PIX_FMT_GREY10_16BE
#define OA_PIX_FMT_GREY10_16LE          55
#define OA_PIX_FMT_GRAY10_16LE          OA_PIX_FMT_GREY10_16LE
#define OA_PIX_FMT_GREY12_16BE          56
#define OA_PIX_FMT_GRAY12_16BE          OA_PIX_FMT_GREY12_16BE
#define OA_PIX_FMT_GREY12_16LE          57
#define OA_PIX_FMT_GRAY12_16LE          OA_PIX_FMT_GREY12_16LE
#define OA_PIX_FMT_GREY14_16BE          58
#define OA_PIX_FMT_GRAY14_16BE          OA_PIX_FMT_GREY14_16BE
#define OA_PIX_FMT_GREY14_16LE          59
#define OA_PIX_FMT_GRAY14_16LE          OA_PIX_FMT_GREY14_16LE

#define OA_PIX_FMT_BGGR10_16BE          60
#define OA_PIX_FMT_BGGR10_16LE          61
#define OA_PIX_FMT_RGGB10_16BE          62
#define OA_PIX_FMT_RGGB10_16LE          63
#define OA_PIX_FMT_GBRG10_16BE          64
#define OA_PIX_FMT_GBRG10_16LE          65
#define OA_PIX_FMT_GRBG10_16BE          66
#define OA_PIX_FMT_GRBG10_16LE          67
#define OA_PIX_FMT_BGGR12_16BE          68
#define OA_PIX_FMT_BGGR12_16LE          69
#define OA_PIX_FMT_RGGB12_16BE          70
#define OA_PIX_FMT_RGGB12_16LE          71
#define OA_PIX_FMT_GBRG12_16BE          72
#define OA_PIX_FMT_GBRG12_16LE          73
#define OA_PIX_FMT_GRBG12_16BE          74
#define OA_PIX_FMT_GRBG12_16LE          75
#define OA_PIX_FMT_BGGR14_16BE          76
#define OA_PIX_FMT_BGGR14_16LE          77
#define OA_PIX_FMT_RGGB14_16BE          78
#define OA_PIX_FMT_RGGB14_16LE          79
#define OA_PIX_FMT_GBRG14_16BE          80
#define OA_PIX_FMT_GBRG14_16LE          81
#define OA_PIX_FMT_GRBG14_16BE          82
#define OA_PIX_FMT_GRBG14_16LE          83

#define OA_PIX_FMT_CMYG8                84
#define OA_PIX_FMT_MCGY8                85
#define OA_PIX_FMT_YGCM8                86
#define OA_PIX_FMT_GYMC8                87
#define OA_PIX_FMT_CMYG16BE             88
#define OA_PIX_FMT_CMYG16LE             89
#define OA_PIX_FMT_MCGY16BE             90
#define OA_PIX_FMT_MCGY16LE             91
#define OA_PIX_FMT_YGCM16BE             92
#define OA_PIX_FMT_YGCM16LE             93
#define OA_PIX_FMT_GYMC16BE             94
#define OA_PIX_FMT_GYMC16LE             95

#define	OA_PIX_FMT_JPEG8								96

#define	OA_PIX_FMT_CANON_CRW						97
#define	OA_PIX_FMT_CANON_CR2						98
#define	OA_PIX_FMT_CANON_CR3						99
#define	OA_PIX_FMT_NIKON_NEF						100

// Adding more frame formats here requires the oaFrameFormats table
// updating in liboavideo/formats.c

#define OA_PIX_FMT_LAST_P1		OA_PIX_FMT_NIKON_NEF+1

#define OA_DEMOSAIC_FMT(x) \
  ((( x == OA_PIX_FMT_BGGR8 ) || ( x == OA_PIX_FMT_RGGB8 ) || \
  ( x == OA_PIX_FMT_GBRG8 ) || ( x == OA_PIX_FMT_GRBG8 )) \
  ? OA_PIX_FMT_RGB24 : \
  (( x == OA_PIX_FMT_BGGR16LE ) || ( x == OA_PIX_FMT_RGGB16LE ) || \
  ( x == OA_PIX_FMT_GBRG16LE ) || ( x == OA_PIX_FMT_GRBG16LE )) \
  ?  OA_PIX_FMT_RGB48LE : \
  (( x == OA_PIX_FMT_BGGR16BE ) || ( x == OA_PIX_FMT_RGGB16BE ) || \
  ( x == OA_PIX_FMT_GBRG16BE ) || ( x == OA_PIX_FMT_GRBG16BE )) \
  ?  OA_PIX_FMT_RGB48BE : \
  (( x == OA_PIX_FMT_CMYG8 ) || ( x == OA_PIX_FMT_MCGY8 ) || \
   ( x == OA_PIX_FMT_YGCM8 ) || ( x == OA_PIX_FMT_GYMC8 )) \
   ? OA_PIX_FMT_RGB24 : \
  (( x == OA_PIX_FMT_CMYG16LE ) || ( x == OA_PIX_FMT_MCGY16LE ) || \
   ( x == OA_PIX_FMT_YGCM16LE ) || ( x == OA_PIX_FMT_GYMC16LE )) \
   ? OA_PIX_FMT_RGB48LE : \
  (( x == OA_PIX_FMT_CMYG16BE ) || ( x == OA_PIX_FMT_MCGY16BE ) || \
   ( x == OA_PIX_FMT_YGCM16BE ) || ( x == OA_PIX_FMT_GYMC16BE )) \
   ? OA_PIX_FMT_RGB48BE : 0 )

#define QUICKTIME_OK(f)	(( f == OA_PIX_FMT_RGB24 ) || \
    ( f == OA_PIX_FMT_BGR24 ) || ( f == OA_PIX_FMT_GREY8 ))

#define UTVIDEO_OK(f) (!(( f == OA_PIX_FMT_GREY8 ) || \
    ( f == OA_PIX_FMT_GREY16LE ) || ( f == OA_PIX_FMT_GREY16BE ) || \
    ( f == OA_PIX_FMT_BGGR8 ) || ( f == OA_PIX_FMT_RGGB8 ) || \
    ( f == OA_PIX_FMT_GRBG8 ) || ( f == OA_PIX_FMT_GBRG8 ) || \
    ( f == OA_PIX_FMT_BGGR16LE ) || ( f == OA_PIX_FMT_BGGR16BE ) || \
    ( f == OA_PIX_FMT_RGGB16LE ) || ( f == OA_PIX_FMT_RGGB16BE ) || \
    ( f == OA_PIX_FMT_GBRG16LE ) || ( f == OA_PIX_FMT_GBRG16BE ) || \
    ( f == OA_PIX_FMT_GRBG16LE ) || ( f == OA_PIX_FMT_GRBG16BE ) || \
    ( f == OA_PIX_FMT_YUYV )))

#define WINDIB_OK(f) (( f == OA_PIX_FMT_GREY8 ) || \
    ( f == OA_PIX_FMT_BGGR8 ) || ( f == OA_PIX_FMT_RGGB8 ) || \
    ( f == OA_PIX_FMT_GRBG8 ) || ( f == OA_PIX_FMT_GBRG8 ))


typedef struct {
  const char*	name;
  const char*	simpleName;
  float		bytesPerPixel;
  unsigned int	bitsPerPixel;
  unsigned int	cfaPattern;
  unsigned int	littleEndian : 1;
  unsigned int	monochrome : 1;
  unsigned int	rawColour : 1;
  unsigned int	useLibraw : 1;
  unsigned int	fullColour : 1;
  unsigned int	lumChrom : 1;
  unsigned int	lossless : 1;
  unsigned int	packed : 1;
  unsigned int	planar : 1;
} frameFormatInfo;

extern frameFormatInfo oaFrameFormats[ OA_PIX_FMT_LAST_P1 ];

#endif	/* OPENASTRO_CAMERA_FORMATS_H */
