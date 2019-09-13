/*****************************************************************************
 *
 * GP2state.h -- libgphoto2 camera state header
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

#ifndef OA_GP2_STATE_H
#define OA_GP2_STATE_H

#include <sys/types.h>
#include <gphoto2/gphoto2-camera.h>


struct GP2buffer {
  void   *start;
  size_t length;
};

typedef struct GP2_STATE {
  int									initialised;
  // camera details
	Camera*							handle;
	GPContext*					ctx;
	int									manufacturer;
	CameraWidget*				rootWidget;
	CameraWidget*				imgSettings;
	CameraWidget*				captureSettings;
	CameraWidget*				settings;
	CameraWidget*				status;
	CameraWidget*				iso;
	CameraWidgetType		isoType;
	int									numIsoOptions;
	const char**				isoOptions;
	CameraWidget*				whiteBalance;
	CameraWidgetType		whiteBalanceType;
	int									numWBOptions;
	const char**				whiteBalanceOptions;
	CameraWidget*				shutterSpeed;
	CameraWidgetType		shutterSpeedType;
	int									numShutterSpeedOptions;
	const char**				shutterSpeedOptions;
	CameraWidget*				sharpening;
	CameraWidgetType		sharpeningType;
	int									numSharpeningOptions;
	const char**				sharpeningOptions;
	CameraWidget*				customfuncex;
	char*								customFuncStr;
	int									mirrorLockupPos;
	CameraWidget*				frameFormat;
	CameraWidgetType		frameFormatType;
	int									numFrameFormatOptions;
	const char**				frameFormatOptions;
	CameraWidget*				acpower;
	CameraWidgetType		acpowerType;
	int									numACPowerOptions;
	CameraWidget*				batteryLevel;
	int									jpegOption;
	int									rawOption;
	CameraWidget*				capture;
	int									currentFormatOption;
	int									numFormatMenuValues;
	int64_t							formatMenuValues[2];
	int									captureEnabled;
  // video mode settings
  int									currentFrameFormat;
  int									bytesPerPixel;
  int									maxBytesPerPixel;
  // buffering for image transfers
  struct GP2buffer*		buffers;
  int									configuredBuffers;
  int									nextBuffer;
  int									buffersFree;
  unsigned int				currentBufferLength[ OA_CAM_BUFFERS ];
  // camera status
  uint32_t						xSize;
  uint32_t						ySize;
  // image settings
  unsigned int				maxResolutionX;
  unsigned int				maxResolutionY;
  // control values
  // thread management
  pthread_t						controllerThread;
  pthread_mutex_t			commandQueueMutex;
  pthread_cond_t			commandComplete;
  pthread_cond_t			commandQueued;
  int									stopControllerThread;

  pthread_t						callbackThread;
  pthread_mutex_t			callbackQueueMutex;
  pthread_cond_t			callbackQueued;
  CALLBACK						frameCallbacks[ OA_CAM_BUFFERS ];
  int									stopCallbackThread;
  // queues for controls and callbacks
  DL_LIST							commandQueue;
  DL_LIST							callbackQueue;
  // handling exposures
  int									exposurePending;
  int									exposureInProgress;
  int									abortExposure;
	time_t							exposureStartTime;
  CALLBACK						exposureCallback;
} GP2_STATE;

#endif	/* OA_GP2_STATE_H */
