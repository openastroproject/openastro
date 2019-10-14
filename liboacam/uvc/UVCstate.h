/*****************************************************************************
 *
 * UVCstate.h -- UVC camera state header
 *
 * Copyright 2014,2016,2018,2019
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

#ifndef OA_UVC_STATE_H
#define OA_UVC_STATE_H

#include <sys/types.h>
#include <libuvc/libuvc.h>

#include "sharedState.h"


typedef struct UVC_STATE {

#include "sharedDecs.h"

  // camera details
  int			unitId;
  // libuvc connection data
  uvc_context_t*        uvcContext;
  uvc_device_handle_t*  uvcHandle;
  uvc_stream_ctrl_t	streamControl;
  // video mode settings
  const uvc_format_desc_t* currentUVCFormat;
  enum uvc_frame_format	currentUVCFormatId;
  int			currentFrameFormat;
  int                   bytesPerPixel;
  int                   maxBytesPerPixel;
  const uvc_format_desc_t* frameFormatMap[ OA_PIX_FMT_LAST_P1 ];
  enum uvc_frame_format	frameFormatIdMap[ OA_PIX_FMT_LAST_P1 ];
  // buffering for image transfers
  frameBuffer*			    buffers;
  unsigned int          currentFrameLength;
  // camera status
  unsigned int          isColour;
  unsigned int          haveComponentWhiteBalance;
  // camera settings
  int32_t		currentPan;
  int32_t		currentTilt;
  // image settings
  FRAMERATES		frameRates;
  int			frameRateNumerator;
  int			frameRateDenominator;
  // control values
  unsigned int          componentBalance;
  int64_t		currentAbsoluteExposure;
  // discrete auto exposure menu item ids
  unsigned int		numAutoExposureItems;
  int64_t		autoExposureMenuItems[8];
} UVC_STATE;

#endif	/* OA_UVC_STATE_H */
