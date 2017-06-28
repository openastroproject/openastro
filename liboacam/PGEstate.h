/*****************************************************************************
 *
 * PGEstate.h -- Point Grey Gig-E camera state header
 *
 * Copyright 2015,2016 James Fidell (james@openastroproject.org)
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

#ifndef OA_PGE_STATE_H
#define OA_PGE_STATE_H

#include <flycapture/C/FlyCapture2_C.h>
#include <openastro/util.h>

struct PGEbuffer {
  void   *start;
  size_t length;
};

typedef struct PGE_STATE {
  int			initialised;
  // libdc1394 connection data
  fc2Context*		pgeContext;
  // video mode settings
  int			videoRGB24;
  int			videoGrey;
  int			videoGrey16;
  int			videoGrey12;
  int			videoRaw;
  int			bytesPerPixel;
  unsigned int		pixelFormats;
  int			bigEndian;
  // buffering for image transfers
  struct PGEbuffer*	buffers;
  int			configuredBuffers;
  int			nextBuffer;
  int			buffersFree;
  unsigned int		imageBufferLength;
//dc1394video_frame_t*	currentFrame;
  // camera status
  uint32_t		xSize;
  uint32_t		ySize;
  uint32_t		maxResolutionX;
  uint32_t		maxResolutionY;
  int			colour;
  int			cfaPattern;
  // image settings
  FRAMESIZES		frameSizes[ OA_MAX_BINNING+1 ];
  struct modeInfo*	frameModes[ OA_MAX_BINNING+1 ];
  FRAMERATES		frameRates;
  int			frameRateNumerator;
  int			frameRateDenominator;
  fc2PixelFormat	currentVideoFormat;
  int			currentMode;
  int			currentBytesPerPixel;
  unsigned int		binMode;
  // control values
  int64_t		currentAbsoluteExposure;
  int32_t		currentRedBalance;
  int32_t		currentBlueBalance;
  // trigger/strobe data
  int32_t		triggerModeCount;
  int64_t*		triggerModes;
  int8_t		triggerEnable;
  int16_t		modeMask;
  int8_t		triggerEnabled;
  int8_t		triggerGPIO;
  int8_t		triggerCurrentMode;
  int8_t		triggerCurrentPolarity;
  int8_t		triggerDelayEnable;
  int8_t		triggerDelayEnabled;
  int64_t		triggerCurrentDelay;
  int8_t		strobeEnable;
  int8_t		strobeEnabled;
  int8_t		strobeGPIO;
  int8_t		strobeCurrentPolarity;
  int64_t		strobeCurrentDelay;
  int64_t		strobeCurrentDuration;
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

  // pointers to shared library functions so we can use them if they are
  // present

  void*                 p_fc2Connect;
  void*                 p_fc2CreateGigEContext;
  void*                 p_fc2DestroyContext;
  void*                 p_fc2DiscoverGigECameras;
  void*                 p_fc2GetCameraFromIndex;
  void*                 p_fc2GetCameraInfo;
  void*                 p_fc2GetGigEImageBinningSettings;
  void*                 p_fc2GetGigEImageSettings;
  void*                 p_fc2GetGigEImageSettingsInfo;
  void*                 p_fc2GetInterfaceTypeFromGuid;
  void*                 p_fc2GetNumOfCameras;
  void*                 p_fc2GetProperty;
  void*                 p_fc2GetPropertyInfo;
  void*                 p_fc2GetStrobe;
  void*                 p_fc2GetStrobeInfo;
  void*                 p_fc2GetTriggerDelay;
  void*                 p_fc2GetTriggerDelayInfo;
  void*                 p_fc2GetTriggerMode;
  void*                 p_fc2GetTriggerModeInfo;
  void*                 p_fc2QueryGigEImagingMode;
  void*                 p_fc2ReadRegister;
  void*                 p_fc2SetGigEImageBinningSettings;
  void*                 p_fc2SetGigEImageSettings;
  void*                 p_fc2SetGigEImagingMode;
  void*                 p_fc2SetProperty;
  void*                 p_fc2SetStrobe;
  void*                 p_fc2SetTriggerDelay;
  void*                 p_fc2SetTriggerMode;
  void*                 p_fc2StartCaptureCallback;
  void*                 p_fc2StopCapture;

} PGE_STATE;

#endif	/* OA_PGE_STATE_H */
