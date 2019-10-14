/*****************************************************************************
 *
 * qhyccdstate.h -- qhyccd camera state header
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

#ifndef OA_QHYCCD_STATE_H
#define OA_QHYCCD_STATE_H

#include <qhyccd/qhyccd.h>
#include <openastro/util.h>

#include "sharedState.h"


typedef struct qhyccdcam_STATE {

#include "sharedDecs.h"

  // connection handle
  char						qhyccdId[64]; // arbitrary size :(
	qhyccd_handle*	handle;

  // video mode settings
  int							colour;
  int							has8Bit;
  int							has16Bit;

  // buffering for image transfers
  frameBuffer*		buffers;

  // camera status
  int			currentBitsPerPixel; // this may be redundant
  int			currentBytesPerPixel;
  int			currentVideoFormat;
  int			binMode;
  unsigned int		currentXResolution;
  unsigned int		currentYResolution;
	int64_t					currentAbsoluteExposure;
} QHYCCD_STATE;

#endif	/* OA_QHYCCD_STATE_H */
