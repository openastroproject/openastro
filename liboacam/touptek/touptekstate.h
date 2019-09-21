/*****************************************************************************
 *
 * state.h -- Touptek camera state header
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

#ifndef OA_TOUPTEK_STATE_H
#define OA_TOUPTEK_STATE_H

#include <openastro/util.h>

#include "touptek-conf.h"

struct Touptekbuffer {
  void   *start;
  size_t length;
};

typedef struct Touptek_STATE {
  int			initialised;

  // connection handle
  TT_HANDLE		handle;
  // video mode settings
  int			maxBytesPerPixel;
  float			currentBytesPerPixel;
  int			currentVideoFormat;
  // buffering for image transfers
  struct Touptekbuffer*	buffers;
  int			configuredBuffers;
  int			nextBuffer;
  int			buffersFree;
  unsigned int		imageBufferLength;
  // camera status
  int			currentXSize;
  int			currentYSize;
  unsigned int		currentXResolution;
  unsigned int		currentYResolution;
  int			maxResolutionX;
  int			maxResolutionY;
  int			colour;
  int32_t		exposureMin;
  int32_t		exposureMax;
  int32_t		gainMin;
  int32_t		gainMax;
  int32_t		speedMax;
  int			binMode;
  int			maxBitDepth;
  int			currentBitsPerPixel;
  int32_t		ledState;
  int32_t		ledPeriod;
  // image settings
  FRAMESIZES		frameSizes[ OA_MAX_BINNING+1 ];
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
  int			exposureInProgress;
  int			abortExposure;
  CALLBACK		streamingCallback;

} TOUPTEK_STATE;

#endif	/* OA_TOUPTEK_STATE_H */
