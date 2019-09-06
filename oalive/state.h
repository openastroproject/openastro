/*****************************************************************************
 *
 * state.h -- global application state datastructures
 *
 * Copyright 2015,2017,2018,2019 James Fidell (james@openastroproject.org)
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

#include "viewWidget.h"
#include "advancedSettings.h"
#include "mainWindow.h"
#include "cameraControls.h"
#include "settingsWidget.h"
#include "focusOverlay.h"

typedef struct
{
  MainWindow*		mainWindow;
  ViewWidget*		viewWidget;
  ControlsWidget*	controlsWidget;
  CameraControls*	cameraControls;
  ProcessingControls*	processingControls;

  SettingsWidget*	settingsWidget;
  AdvancedSettings*	advancedSettings;
  FocusOverlay*		focusOverlay;

  QString		lastRecordedFile;

  int			stackingMethod;

  int                   preferredExposureControl;
	int			histogramCCSignalConnected;
	int			histogramProcessingSignalConnected;
	int			cameraRunning;

  QString               appPath;
} STATE;

extern STATE		state;

#define TOP_WIDGET ( state.settingsWidget ? state.settingsWidget->getTabset() :\
    ( state.mainWindow ? ( QWidget* ) state.mainWindow : ( QWidget* ) this ))
