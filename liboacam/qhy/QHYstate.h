/*****************************************************************************
 *
 * QHYstate.h -- QHY camera state header
 *
 * Copyright 2013,2014,2015,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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

#include "sharedState.h"


typedef struct QHY_STATE {

#include "sharedDecs.h"

  // USB connection data
  libusb_context*       usbContext;
  libusb_device_handle* usbHandle;
  unsigned int		transferPadding;
  // video mode settings
  unsigned int          currentFrameFormat;
  // buffering for image transfers
  frameBuffer*     buffers;
  unsigned int          captureLength;
  struct libusb_transfer* transfers [ QHY_NUM_TRANSFER_BUFS ];
  uint8_t*		transferBuffers [ QHY_NUM_TRANSFER_BUFS ];
  // camera status
  uint64_t		droppedFrames;
  unsigned int          isColour;
  unsigned int		frameSize;
  int			firstTimeSetup;
  struct libusb_transfer* statusTransfer;
  uint8_t		statusBuffer[32];
  unsigned int		receivedBytes;
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
  int			smallFrame;
  // control values
  unsigned int          currentExposure;
  unsigned int          currentGain;
  unsigned int          currentDigitalGain;
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
  pthread_t		eventHandler;
  pthread_mutex_t	videoCallbackMutex;
} QHY_STATE;

#endif	/* OA_QHY_STATE_H */
