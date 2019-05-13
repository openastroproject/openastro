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

struct dummyBuffer {  
  void   *start; 
  size_t length; 
};


typedef struct DUMMY_STATE {
  int			initialised;
  // camera details
  int			cameraType;
	int			index;
	int			binModes[16];
  // buffering for image transfers
  struct dummyBuffer*	buffers;
  int			configuredBuffers;
  int			nextBuffer;
  int			buffersFree;
  // camera settings
  int			binMode;
  uint32_t		xSize;
  uint32_t		ySize;
  uint32_t		currentBrightness;
  uint32_t		currentGain;
  uint32_t		currentAbsoluteExposure;
  uint32_t		currentHFlip;
  uint32_t		currentVFlip;
  // image settings
  uint32_t		maxResolutionX;
  uint32_t		maxResolutionY;
  FRAMESIZES		frameSizes[ OA_MAX_BINNING+1 ];
	uint32_t		imageBufferLength;
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
} DUMMY_STATE;

#endif	/* OA_DUMMY_STATE_H */
