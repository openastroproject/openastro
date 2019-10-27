/*****************************************************************************
 *
 * state.h -- Touptek camera state header
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#ifndef OA_TOUPTEK_STATE_H
#define OA_TOUPTEK_STATE_H

#include <openastro/util.h>

#include "touptek-conf.h"
#include "sharedState.h"


typedef struct Touptek_STATE {

#include "sharedDecs.h"

	unsigned int	libMajorVersion;
	unsigned int	libMinorVersion;
  // connection handle
  TT_HANDLE		handle;
  // video mode settings
  int			maxBytesPerPixel;
  float			currentBytesPerPixel;
  int			currentVideoFormat;
  // buffering for image transfers
  frameBuffer*	buffers;
  // camera status
  unsigned int		currentXResolution;
  unsigned int		currentYResolution;
  int			colour;
  int32_t		exposureMin;
  int32_t		exposureMax;
  int32_t		exposureTime;
  int32_t		gainMin;
  int32_t		gainMax;
  int32_t		speedMax;
  int			binMode;
  int			maxBitDepth;
  int			currentBitsPerPixel;
  int32_t		ledState;
  int32_t		ledPeriod;
  int32_t		fanSpeedMax;
	int				haveTEC;
} TOUPTEK_STATE;

#endif	/* OA_TOUPTEK_STATE_H */
