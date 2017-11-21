/*****************************************************************************
 *
 * ZWASIstate.h -- ZW ASI camera state header
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

#ifndef OA_ZWASI_STATE_H
#define OA_ZWASI_STATE_H

#include <openastro/util.h>

struct ZWASIbuffer {  
  void   *start; 
  size_t length; 
};


typedef struct ZWASI_STATE {
  int			initialised;
  // camera details
  int			index;
  int			cameraType;
  int			colour;
  long			cameraId;
  int			usb3Cam;
  int			binModes[16];
  // video mode settings
  int			videoRGB24;
  int			videoGrey16;
  int			videoGrey;
  int32_t		videoCurrent;
  uint32_t		FSMState;
  // buffering for image transfers
  struct ZWASIbuffer*	buffers;
  int			configuredBuffers;
  int			nextBuffer;
  int			buffersFree;
  // camera settings
  int			binMode;
  uint32_t		xSize;
  uint32_t		ySize;
  // image settings
  uint32_t		maxResolutionX;
  uint32_t		maxResolutionY;
  int			imageBufferLength;
  FRAMESIZES		frameSizes[ OA_MAX_BINNING+1 ];
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
  uint32_t		autoGain;
  uint32_t		autoBrightness;
  uint32_t		autoExposure;
  uint32_t		autoGamma;
  uint32_t		autoBlueBalance;
  uint32_t		autoRedBalance;
  uint32_t		autoUSBTraffic;
  uint32_t		autoOverclock;
  uint32_t		autoHighSpeed;
  uint32_t		monoBinning;
  uint32_t		fanEnabled;
  uint32_t		patternAdjust;
  uint32_t		dewHeater;
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
} ZWASI_STATE;

#endif	/* OA_ZWASI_STATE_H */
