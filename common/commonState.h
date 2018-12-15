/*****************************************************************************
 *
 * commonState.h -- common global application state
 *
 * Copyright 2018
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

#pragma once

#include "camera.h"
#include "filterWheel.h"
#include "timer.h"

#include <QtCore>

typedef struct
{
	void*								localState;

  Camera*							camera;
  FilterWheel*				filterWheel;
  Timer*							timer;

	int									binningValid;
	int									binModeX;
	int									binModeY;

	int									cropMode;
	unsigned int				cropSizeX;
	unsigned int				cropSizeY;

	unsigned int				sensorSizeX;
	unsigned int				sensorSizeY;

	int									gpsValid;
	double							longitude;
	double							latitude;
	double							altitude;

	int									cameraTempValid;
	float								cameraTemp;

  unsigned long long	captureIndex;

	QString							currentDirectory;

	QWidget*						viewerWidget;

	int									generalSettingsIndex;
  int									captureSettingsIndex;
  int									cameraSettingsIndex;
  int									profileSettingsIndex;
  int									filterSettingsIndex;
  int									autorunSettingsIndex;
  int									histogramSettingsIndex;
  int									demosaicSettingsIndex;
  int									fitsSettingsIndex;
  int									timerSettingsIndex;

	/*
  int			histogramOn;
  AdvancedSettings*	advancedSettings;
  FocusOverlay*		focusOverlay;

  int			autorunEnabled;
  int			autorunRemaining;
  unsigned long		autorunStartNext;
  int			pauseEnabled;
  int			captureWasPaused;

  QString		lastRecordedFile;

  unsigned int		needGroupBoxBorders;

  unsigned long		firstFrameTime;
  unsigned long		lastFrameTime;
  double		currentFPS;

  int			preferredExposureControl;

  QString		appPath;
	*/
} COMMON_STATE;

extern COMMON_STATE		commonState;
