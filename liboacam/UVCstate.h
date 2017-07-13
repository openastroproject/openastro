/*****************************************************************************
 *
 * UVCstate.h -- UVC camera state header
 *
 * Copyright 2014,2016 James Fidell (james@openastroproject.org)
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

#ifndef OA_UVC_STATE_H
#define OA_UVC_STATE_H

#include <sys/types.h>
#include <libuvc/libuvc.h>

struct UVCbuffer {
  void   *start;
  size_t length;
};

typedef struct UVC_STATE {
  int                   initialised;
  // camera details
  unsigned long         index;
  int			unitId;
  // libuvc connection data
  uvc_context_t*        uvcContext;
  uvc_device_handle_t*  uvcHandle;
  uvc_stream_ctrl_t	streamControl;
  // video mode settings
  const uvc_format_desc_t* videoGrey;
  const uvc_format_desc_t* videoGrey16;
  const uvc_format_desc_t* videoRaw;
  enum uvc_frame_format	videoGreyId;
  enum uvc_frame_format	videoGrey16Id;
  enum uvc_frame_format	videoRawId;
  int                   bytesPerPixel;
  int                   maxBytesPerPixel;
  const uvc_format_desc_t* videoCurrent;
  enum uvc_frame_format	videoCurrentId;
  // buffering for image transfers
  struct UVCbuffer*     buffers;
  int                   configuredBuffers;
  int			nextBuffer;
  int			buffersFree;
  unsigned int          imageBufferLength;
  unsigned int          currentFrameLength;
  // camera status
  unsigned int          isColour;
  unsigned int          haveComponentWhiteBalance;
  uint32_t		xSize;
  uint32_t		ySize;
  // camera settings
  // image settings
  unsigned int          maxResolutionX;
  unsigned int          maxResolutionY;
  FRAMESIZES		frameSizes[2];
  FRAMERATES		frameRates;
  int			frameRateNumerator;
  int			frameRateDenominator;
  // control values
  unsigned int          componentBalance;
  int64_t		currentAbsoluteExposure;
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
  // discrete auto exposure menu item ids
  unsigned int		numAutoExposureItems;
  int64_t		autoExposureMenuItems[8];
} UVC_STATE;

#endif	/* OA_UVC_STATE_H */
