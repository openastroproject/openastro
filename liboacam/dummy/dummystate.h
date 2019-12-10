/*****************************************************************************
 *
 * dummystate.h -- dummy camera state header
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

#ifndef OA_DUMMY_STATE_H
#define OA_DUMMY_STATE_H

#include <openastro/util.h>

#include "sharedState.h"


typedef struct DUMMY_STATE {

#include "sharedDecs.h"

	int					numIsoOptions;
	int					numShutterSpeedOptions;

  // buffering for image transfers
  frameBuffer*			buffers;
  // camera settings
  int			binMode;
  uint32_t		currentBrightness;
  uint32_t		currentGain;
  uint32_t		currentAbsoluteExposure;
  uint32_t		currentHFlip;
  uint32_t		currentVFlip;
  // image settings
	int			binModes[16];
} DUMMY_STATE;

#endif	/* OA_DUMMY_STATE_H */
