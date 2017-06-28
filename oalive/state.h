/*****************************************************************************
 *
 * state.h -- global application state datastructures
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
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

#pragma once

#include "camera.h"
#include "filterWheel.h"
#include "viewWidget.h"
#include "advancedSettings.h"
#include "mainWindow.h"
#include "cameraControls.h"
/*
#include "controlWidget.h"
#include "captureWidget.h"
#include "imageWidget.h"
#include "zoomWidget.h"
#include "histogramWidget.h"
*/
#include "settingsWidget.h"
#include "focusOverlay.h"

typedef struct
{
  MainWindow*		mainWindow;
  ViewWidget*		viewWidget;
  ControlsWidget*	controlsWidget;
  CameraControls*	cameraControls;
/*
  ControlWidget*	controlWidget;
  CaptureWidget*	captureWidget;
  ImageWidget*		imageWidget;
  ZoomWidget*		zoomWidget;
  CameraWidget*		cameraWidget;
*/
  int			libavStarted; // has libav* been initialised?
  Camera*		camera;
  FilterWheel*		filterWheel;

  SettingsWidget*	settingsWidget;
  AdvancedSettings*	advancedSettings;
  FocusOverlay*		focusOverlay;

  int			generalSettingsIndex;
  int			captureSettingsIndex;
  int			profileSettingsIndex;
  int			filterSettingsIndex;
  int			demosaicSettingsIndex;
/*
  int			cameraSettingsIndex;
  int			autorunSettingsIndex;
  int			histogramSettingsIndex;

*/
  QString		lastRecordedFile;
  QString		currentDirectory;

  unsigned int		captureIndex;

  int			stackingMethod;

} STATE;

extern STATE		state;

#define	RETICLE_CIRCLE		1
#define	RETICLE_CROSS		2
#define	RETICLE_TRAMLINES	3

#define QUICKTIME_OK(f)	(( f == OA_PIX_FMT_RGB24 ) || \
    ( f == OA_PIX_FMT_BGR24 ) || ( f == OA_PIX_FMT_GREY16BE ) || \
    ( f == OA_PIX_FMT_RGB48BE ) || ( f == OA_PIX_FMT_GREY8 ))

#define UTVIDEO_OK(f) (!(( f == OA_PIX_FMT_GREY8 ) || \
    ( f == OA_PIX_FMT_GREY16LE ) || ( f == OA_PIX_FMT_GREY16BE ) || \
    ( f == OA_PIX_FMT_BGGR8 ) || ( f == OA_PIX_FMT_RGGB8 ) || \
    ( f == OA_PIX_FMT_GRBG8 ) || ( f == OA_PIX_FMT_GBRG8 ) || \
    ( f == OA_PIX_FMT_BGGR16LE ) || ( f == OA_PIX_FMT_BGGR16BE ) || \
    ( f == OA_PIX_FMT_RGGB16LE ) || ( f == OA_PIX_FMT_RGGB16BE ) || \
    ( f == OA_PIX_FMT_GBRG16LE ) || ( f == OA_PIX_FMT_GBRG16BE ) || \
    ( f == OA_PIX_FMT_GRBG16LE ) || ( f == OA_PIX_FMT_GRBG16BE ) || \
    ( f == OA_PIX_FMT_YUYV )))

#define WINDIB_OK(f) (( f == OA_PIX_FMT_GREY8 ) || \
    ( f == OA_PIX_FMT_BGGR8 ) || ( f == OA_PIX_FMT_RGGB8 ) || \
    ( f == OA_PIX_FMT_GRBG8 ) || ( f == OA_PIX_FMT_GBRG8 ))
