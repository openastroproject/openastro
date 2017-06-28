/*****************************************************************************
 *
 * SER.h -- SER API header
 *
 * Copyright 2013,2014,2016 James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_SER_H
#define OPENASTRO_SER_H

#include <stdint.h>
#include <sys/types.h>

#define OA_SER_MONO		0
#define OA_SER_BAYER_RGGB	8
#define OA_SER_BAYER_GRBG	9
#define OA_SER_BAYER_GBRG	10
#define OA_SER_BAYER_BGGR	11
#define OA_SER_BAYER_CYYM	16
#define OA_SER_BAYER_YCMY	17
#define OA_SER_BAYER_YMCY	18
#define OA_SER_BAYER_MYYC	19
#define OA_SER_RGB		100
#define OA_SER_BGR		101

#define OA_SER_MAX_STRING_LEN	40

typedef struct {
  uint8_t   version;
  char      FileID[15];
  uint32_t  LuID;
  uint32_t  ColorID;
  uint32_t  LittleEndian;
  uint32_t  ImageWidth;
  uint32_t  ImageHeight;
  uint32_t  PixelDepth;
  uint32_t  FrameCount;
  char      Observer[OA_SER_MAX_STRING_LEN+1];
  char      Instrument[OA_SER_MAX_STRING_LEN+1];
  char      Telescope[OA_SER_MAX_STRING_LEN+1];
  int64_t   DateTime;
  int64_t   DateTimeUTC;
} oaSERHeader;

typedef struct {
  int       SERfd;
  int64_t*  timestampBuffer;
  int64_t*  nextTimestamp;
  uint32_t  bufferSize;
  uint32_t  frames;
  uint32_t  framesLeft;
  uint32_t  frameSize;
  int       shiftBits;
  uint32_t  pixelDepth;
  void*     transformBuffer;
} oaSERContext;


extern int  oaSEROpen ( const char*, oaSERContext* );
extern int  oaSERWriteHeader ( oaSERContext*, oaSERHeader* );
extern int  oaSERWriteFrame ( oaSERContext*, void*, const char* );
extern int  oaSERWriteTrailer ( oaSERContext* );
extern int  oaSERClose ( oaSERContext* );

#endif	/* OPENASTRO_SER_H */
