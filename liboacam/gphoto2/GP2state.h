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

#include "sharedState.h"


typedef struct GP2_STATE {

#include "sharedDecs.h"

  // camera details
	Camera*							handle;
	GPContext*					ctx;
	int									manufacturer;
	CameraWidget*				rootWidget;
	CameraWidget*				imgSettings;
	CameraWidget*				captureSettings;
	CameraWidget*				settings;
	CameraWidget*				status;
	CameraWidget*				actions;
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
	CameraWidget*				bulbMode;
	CameraWidgetType		bulbModeType;
	int									numBulbModeOptions;
	const char**				bulbModeOptions;
	int									currentFormatOption;
	int									numFormatMenuValues;
	int64_t							formatMenuValues[2];
	int									captureEnabled;
	int									bulbPressOption;
	int									bulbReleaseOption;
	int									bulbExposureTime;
	int									bulbModeEnabled;
  // video mode settings
  int									currentFrameFormat;
  int									bytesPerPixel;
  int									maxBytesPerPixel;
  // buffering for image transfers
  frameBuffer*				buffers;
  unsigned int				currentBufferLength[ OA_CAM_BUFFERS ];
  // handling exposures
  int									exposurePending;
  CALLBACK						exposureCallback;
} GP2_STATE;

#endif	/* OA_GP2_STATE_H */
