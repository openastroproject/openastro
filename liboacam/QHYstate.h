/*****************************************************************************
 *
 * QHYstate.h -- QHY camera state header
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

#ifndef OA_QHY_STATE_H
#define OA_QHY_STATE_H

#include <sys/types.h>
#include <libusb-1.0/libusb.h>
#include <pthread.h>

struct QHYbuffer {
  void   *start;
  size_t length;
};


typedef struct QHY_STATE {
  int                   initialised;
  // camera details
  unsigned long         index;
  int                   cameraType;
  // USB connection data
  libusb_context*       usbContext;
  libusb_device_handle* usbHandle;
  unsigned int		transferPadding;
  // video mode settings
  int                   videoRGB24;
  int                   videoYUYV;
  int                   videoGrey;
  int                   videoGrey16;
  unsigned int          videoCurrent;
  // buffering for image transfers
  struct QHYbuffer*     buffers;
  unsigned int          imageBufferLength;
  unsigned char*        xferBuffer;
  unsigned int          captureLength;
  int                   configuredBuffers;
  int			nextBuffer;
  int                   buffersFree;
  // camera status
  uint64_t		droppedFrames;
  unsigned int          isColour;
  unsigned int          xSize;
  unsigned int          ySize;
  unsigned int		frameSize;
  int			firstTimeSetup;
  // camera settings
  int	                xOffset;
  unsigned int		topOffset;
  unsigned int		bottomOffset;
  unsigned int		binMode;
  unsigned int		horizontalBinMode;
  unsigned int		verticalBinMode;
  unsigned int		longExposureMode;
  unsigned int		CMOSClock;
  double		PLLRatio;
  unsigned int		transferTime;
  unsigned int		requestedExposure;
  unsigned int		captureHeight;
  // image settings
  unsigned int          maxResolutionX;
  unsigned int          maxResolutionY;
  FRAMESIZES		frameSizes[3];
  // control values
  unsigned int          currentExposure;
  unsigned int          currentGain;
  unsigned int		currentAmpMode;
  unsigned int          currentHighSpeed;
  unsigned int          currentBitDepth;
  unsigned int          currentUSBTraffic;
  unsigned int          currentHDR;
  unsigned int          currentRedBalance;
  unsigned int          currentGreenBalance;
  unsigned int          currentBlueBalance;
  unsigned int		requestedAmpMode;
  unsigned int          correctedExposureTime;
  // thread management
  pthread_mutex_t       usbMutex;
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
} QHY_STATE;

#endif	/* OA_QHY_STATE_H */
