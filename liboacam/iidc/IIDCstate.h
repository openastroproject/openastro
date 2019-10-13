/*****************************************************************************
 *
 * IIDCstate.h -- IEEE1394/IIDC camera state header
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

#ifndef OA_IIDC_STATE_H
#define OA_IIDC_STATE_H

#include <dc1394/dc1394.h>
#include <openastro/util.h>

#include "sharedState.h"


typedef struct IIDC_STATE {
	// Data common to all interfaces comes first, so it can be shared across
	// a union of all state structures
  int								initialised;
  // camera details
  unsigned long			index;
  int								cameraType;
  // thread management
  pthread_t					controllerThread;
  pthread_mutex_t		commandQueueMutex;
  pthread_cond_t		commandComplete;
  pthread_cond_t		commandQueued;
  int								stopControllerThread;
  pthread_t					callbackThread;
  pthread_mutex_t		callbackQueueMutex;
  pthread_cond_t		callbackQueued;
  CALLBACK					frameCallbacks[ OA_CAM_BUFFERS ];
  int								stopCallbackThread;
	pthread_t					timerThread;
	pthread_cond_t		timerState;
	pthread_mutex_t		timerMutex;
  // queues for controls and callbacks
  DL_LIST						commandQueue;
  DL_LIST						callbackQueue;
  // streaming
  int								isStreaming;
  CALLBACK					streamingCallback;
	int								exposureInProgress;
	int								abortExposure;
	// shared buffer config
  int								configuredBuffers;
  unsigned char*		xferBuffer;
  unsigned int			imageBufferLength;
  int								nextBuffer;
  int								buffersFree;
	// common image config
  unsigned int			maxResolutionX;
  unsigned int			maxResolutionY;
  FRAMESIZES				frameSizes[ OA_MAX_BINNING+1 ];
	// common camera settings
  unsigned int			xSize;
  unsigned int			ySize;

	// END OF COMMON DATA

  // libdc1394 connection data
  dc1394camera_t*	iidcHandle;
  // video mode settings
  unsigned int		currentFrameFormat;
  dc1394video_mode_t	currentIIDCMode;
  dc1394color_coding_t	currentCodec;
  // buffering for image transfers
  dc1394video_frame_t*	currentFrame;
  // camera status
  int			haveFormat7;
  int			isTISColour;
  uint8_t		absoluteSupported[ DC1394_FEATURE_NUM ];
  uint8_t		haveSetpointCooling;
  int32_t		triggerMode;
  int32_t		triggerEnable;
  int32_t		triggerPolarity;
  int32_t		triggerDelay;
  int32_t		triggerDelayEnable;
  // image settings
  FRAMERATES		frameRates;
  int			frameRateNumerator;
  int			frameRateDenominator;
  // control values
  int64_t		currentAbsoluteExposure;
  uint32_t		currentRedBalance;
  uint32_t		currentBlueBalance;
} IIDC_STATE;

#endif	/* OA_IIDC_STATE_H */
