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

struct qhyccdbuffer {
  void   *start;
  size_t length;
};

typedef struct qhyccdcam_STATE {
  int							initialised;

  // connection handle
  char						qhyccdId[64]; // arbitrary size :(
	qhyccd_handle*	handle;

  // video mode settings
  int							colour;
  int							has8Bit;
  int							has16Bit;
  int							maxResolutionX;
  int							maxResolutionY;
  FRAMESIZES			frameSizes[ OA_MAX_BINNING+1 ];

  // buffering for image transfers
  struct qhyccdbuffer*	buffers;
  unsigned int		imageBufferLength;
  int							configuredBuffers;
  int							nextBuffer;
  int							buffersFree;

  // camera status
  int			currentBitsPerPixel; // this may be redundant
  int			currentBytesPerPixel;
  int			currentVideoFormat;
  int			binMode;
  int			currentXSize;
  int			currentYSize;
  unsigned int		currentXResolution;
  unsigned int		currentYResolution;
	int64_t					currentAbsoluteExposure;
/*
  int32_t		exposureMin;
  int32_t		exposureMax;
  int32_t		gainMin;
  int32_t		gainMax;
  int32_t		speedMax;
  int			maxBitDepth;
  // image settings
	*/
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

} QHYCCD_STATE;

#endif	/* OA_QHYCCD_STATE_H */
