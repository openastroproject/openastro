/*****************************************************************************
 *
 * openastro.h -- Main Open Astro Project API header
 *
 * Copyright 2014,2015,2016,2017,2019
 *   James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_H
#define OPENASTRO_H

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_LIBFLYCAPTURE2
#include <flycapture/C/FlyCapture2_C.h>
#endif

#include <openastro/errno.h>
#include <openastro/debug.h>
#include <openastro/controlTypes.h>

typedef struct {
  const int		interfaceType;
  const char*		name;
  const char*		shortName;
  int			(*enumerate)();
  int			flags;
  const unsigned int	userConfigFlags;
} oaInterface;

typedef struct {
  unsigned long         devIndex;
  unsigned long         devType;
  unsigned short        vendorId;
  unsigned short        productId;
  unsigned long         misc;
  uint64_t              guid;
  uint64_t              ipAddress;
  uint8_t               unit;
  unsigned int          colour;
  unsigned int          cfaPattern;
  char                  sysPath[ PATH_MAX+1 ];
#ifdef HAVE_LIBFLYCAPTURE2
  fc2PGRGuid		pgeGuid;
#endif
  char			deviceId[ 256 ]; // FIX ME -- magic no. spinnaker probably biggest
  uint32_t		majorVersion;
  uint32_t		minorVersion;
} DEVICE_INFO;

typedef struct OA_CONTROL_VALUE {
  unsigned char		valueType;
  union {
    int32_t		int32;
    int64_t		int64;
    int8_t		boolean;
    int8_t		menu;
    int8_t		discrete;
    int64_t		readonly;
    const char*		string;
  };
} oaControlValue;

#define	OA_DEVICE_CAMERA	1
#define	OA_DEVICE_FILTERWHEEL	2
#define	OA_DEVICE_PTR		3

#define OA_MAX_NAME_LEN		80
#define OA_MAX_DEVICES		32

#endif	/* OPENASTRO_H */
