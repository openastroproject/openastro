/*****************************************************************************
 *
 * SXstate.h -- Starlight Xpress state header
 *
 * Copyright 2014,2015,2018,2019 James Fidell (james@openastroproject.org)
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

#ifndef OA_SX_STATE_H
#define OA_SX_STATE_H

#include <sys/types.h>
#include <libusb-1.0/libusb.h>
#include <pthread.h>

#include <openastro/util.h>

#include "sharedState.h"


typedef struct SX_STATE {

#include "sharedDecs.h"

  // USB connection data
  libusb_context*       usbContext;
  libusb_device_handle* usbHandle;
  // video mode settings
  unsigned int          currentFrameFormat;
  uint32_t		horizontalFrontPorch;
  uint32_t		horizontalBackPorch;
  uint32_t		verticalFrontPorch;
  uint32_t		verticalBackPorch;
  // buffering for image transfers
  frameBuffer*		buffers;
  unsigned int          actualImageLength;
  // camera status
  unsigned int          isColour;
  unsigned int          isInterlaced;
  uint32_t		colourMatrix;
  uint64_t		droppedFrames;
  // camera settings
  unsigned int          xSubframeSize;
  unsigned int          ySubframeSize;
  unsigned int          xSubframeOffset;
  unsigned int          ySubframeOffset;
  unsigned int          xImageSize;
  unsigned int          yImageSize;
  unsigned int          binMode;
  // image settings
  unsigned int          bytesPerPixel;
  // control values
  unsigned int          currentExposure;
  uint32_t		bitDepth;
} SX_STATE;

#endif	/* OA_SX_STATE_H */
