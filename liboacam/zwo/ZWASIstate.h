/*****************************************************************************
 *
 * ZWASIstate.h -- ZW ASI camera state header
 *
 * Copyright 2013,2014,2015,2017,2019,2020
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

#ifndef OA_ZWASI_STATE_H
#define OA_ZWASI_STATE_H

#include <openastro/util.h>

#include "sharedState.h"


typedef struct ZWASI_STATE {

#include "sharedDecs.h"

  // camera details
  int			colour;
  long			cameraId;
  int			usb3Cam;
  int			binModes[16];
  // video mode settings
  int32_t		currentFormat;
  int32_t		currentMode;
  int32_t		maxBitDepth;
  int32_t		greyscaleMode;
  // buffering for image transfers
  frameBuffer*	buffers;
  // camera settings
  int			binMode;
  // control values
  uint32_t		currentBrightness;
  uint32_t		currentGain;
  uint32_t		currentAbsoluteExposure;
  uint32_t		currentGamma;
  uint32_t		currentRedBalance;
  uint32_t		currentBlueBalance;
  uint32_t		currentUSBTraffic;
  uint32_t		currentOverclock;
  uint32_t		currentHighSpeed;
  uint32_t		currentHFlip;
  uint32_t		currentVFlip;
  uint32_t		currentBitDepth;
  uint32_t		coolerEnabled;
  uint32_t		currentSetPoint;
  uint32_t		currentCoolerPower;
  int32_t		autoGain;
  int32_t		autoBrightness;
  int32_t		autoExposure;
  int32_t		autoGamma;
  int32_t		autoBlueBalance;
  int32_t		autoRedBalance;
  int32_t		autoUSBTraffic;
  int32_t		autoOverclock;
  int32_t		autoHighSpeed;
  uint32_t		monoBinning;
  uint32_t		fanEnabled;
  uint32_t		patternAdjust;
  uint32_t		dewHeater;
} ZWASI_STATE;

#endif	/* OA_ZWASI_STATE_H */
