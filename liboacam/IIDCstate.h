/*****************************************************************************
 *
 * IIDCstate.h -- IEEE1394/IIDC camera state header
 *
 * Copyright 2013,2014,2015 James Fidell (james@openastroproject.org)
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

#ifndef OA_IIDC_STATE_H
#define OA_IIDC_STATE_H

#include <dc1394/dc1394.h>
#include <openastro/util.h>

typedef struct IIDC_STATE {
  int			initialised;
  // libdc1394 connection data
  dc1394camera_t*	iidcHandle;
  // video mode settings
  int			videoRGB24;
  int			videoGrey;
  int			videoGrey16;
  int			videoRaw;
  unsigned int		videoCurrent;
  dc1394color_coding_t	currentCodec;
  int			mosaicFormat;
  // buffering for image transfers
  int			configuredBuffers;
  int			nextBuffer;
  int			buffersFree;
  dc1394video_frame_t*	currentFrame;
  // camera status
  int			haveFormat7;
  int			isTISColour;
  uint32_t		xSize;
  uint32_t		ySize;
  uint8_t		absoluteSupported[ DC1394_FEATURE_NUM ];
  uint8_t		haveSetpointCooling;
  int32_t		triggerMode;
  int32_t		triggerEnable;
  int32_t		triggerPolarity;
  int32_t		triggerDelay;
  int32_t		triggerDelayEnable;
  // image settings
  FRAMESIZES		frameSizes[2];
  FRAMERATES		frameRates;
  int			frameRateNumerator;
  int			frameRateDenominator;
  // control values
  int64_t		currentAbsoluteExposure;
  uint32_t		currentRedBalance;
  uint32_t		currentBlueBalance;
  // thread management
  pthread_t		controllerThread;
  pthread_mutex_t	commandQueueMutex;
  pthread_cond_t	commandComplete;
  pthread_cond_t	commandQueued;
  int			stopControllerThread;

  pthread_t		callbackThread;
  pthread_mutex_t	callbackQueueMutex;
  pthread_cond_t	callbackQueued;
  CALLBACK		frameCallbacks[ OA_CAM_BUFFERS ];
  int			stopCallbackThread;
  // queues for controls and callbacks
  DL_LIST		commandQueue;
  DL_LIST		callbackQueue;
  // streaming
  int			isStreaming;
  CALLBACK		streamingCallback;
} IIDC_STATE;

#endif	/* OA_IIDC_STATE_H */
