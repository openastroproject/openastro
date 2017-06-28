/*****************************************************************************
 *
 * Mallincamstate.h -- Mallincam camera state header
 *
 * Copyright 2016 James Fidell (james@openastroproject.org)
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

#ifndef OA_MALLINCAM_STATE_H
#define OA_MALLINCAM_STATE_H

#include <toupcam.h>
#include <openastro/util.h>

struct Mallincambuffer {
  void   *start;
  size_t length;
};

typedef struct Mallincam_STATE {
  int			initialised;

  // connection handle
  HToupCam		handle;
  // video mode settings
  int			videoRGB24;
  int			videoGrey;
  int			videoGrey16;
  int			videoGrey12;
  int			videoRaw;
  int			bytesPerPixel;
  int			currentBytesPerPixel;
  int			currentVideoFormat;
  // buffering for image transfers
  struct Mallincambuffer*	buffers;
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
  int			cfaPattern;
  int32_t		exposureMin;
  int32_t		exposureMax;
  int32_t		gainMin;
  int32_t		gainMax;
  int32_t		speedMax;
  int			binMode;
  int			maxBitDepth;
  int			currentBitsPerPixel;
  // image settings
  FRAMESIZES		frameSizes[ OA_MAX_BINNING+1 ];
  // control values
//  int32_t		currentColourBalance[3];
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
/*
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
*/
} MALLINCAM_STATE;

#endif	/* OA_MALLINCAM_STATE_H */
