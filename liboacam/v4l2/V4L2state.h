/*****************************************************************************
 *
 * V4L2state.h -- V4L2 camera state header
 *
 * Copyright 2013,2014,2015,2018,2019
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

#ifndef OA_V4L2_STATE_H
#define OA_V4L2_STATE_H

#include <sys/types.h>
#include <linux/videodev2.h>
#include <linux/limits.h>

#include "sharedState.h"


typedef struct V4L2_STATE {

#include "sharedDecs.h"

  // connection info
  char			devicePath[ PATH_MAX+1];
  int			fd;
  // video mode settings
  uint32_t		currentFrameFormat;
  uint32_t		currentV4L2Format;
  // buffering for image transfers
  frameBuffer*			buffers;
  struct v4l2_buffer	currentFrame[ OA_CAM_BUFFERS ];
  // camera status
  int			colourDxK;
  int			monoDMK;
  int			isSPC900;
  // have a manual setting for white balance
  uint32_t		haveWhiteBalanceManual;
  // have an "off" setting for auto white balance
  uint32_t		autoWhiteBalanceOff;
  // image settings
  FRAMERATES		frameRates;
  int			frameRateNumerator;
  int			frameRateDenominator;
  // control values
  int			exposureMode;
  int64_t		currentAbsoluteExposure;
  
  // discrete auto exposure menu item ids
  unsigned int		numAutoExposureItems;
  int64_t		autoExposureMenuItems[8];
} V4L2_STATE;

#endif	/* OA_V4L2_STATE_H */
