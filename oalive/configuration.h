/*****************************************************************************
 *
 * config.h -- declaration of data structures for configuration data
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

#if HAVE_LIMITS_H
#include <limits.h>
#endif

extern "C" {
#include <openastro/camera.h>
#include <openastro/filterwheel.h>
#include <openastro/userConfig.h>
}

#define	CONFIG_VERSION	1

typedef struct {
  QString	filterName;
  int		controls[ OA_CAM_CTRL_LAST_P1 ];
} FILTER_PROFILE;

typedef struct {
  QString       profileName;
  int           sixteenBit;
  int           binning2x2;
  int           colourise;
  int           useROI;
  unsigned int  imageSizeX;
  unsigned int  imageSizeY;
  QList<FILTER_PROFILE> filterProfiles;
  int		frameRateNumerator;
  int		frameRateDenominator;
  int           fileTypeOption;
  int           filterOption;
  QString       frameFileNameTemplate;
  QString       processedFileNameTemplate;
  int		target;
} PROFILE;


// overkill, but i may want to expand this later
typedef struct {
  QString	filterName;
} FILTER;

typedef QList<userDeviceConfig> userConfigList;

typedef struct
{
  // general
  int			saveSettings;
  int			tempsInC;
  int			connectSoleCamera;

  // settings from device menu
  int			cameraDevice;

  // options menu
  int			showHistogram;
  int			showReticle;
  int			showFocusAid;
  int			derotate;
  int			flipX;
  int			flipY;
  int			demosaic;

  // FITS keyword data
  QString		fitsObserver;
  QString		fitsTelescope;
  QString		fitsInstrument;
  QString		fitsObject;
  QString		fitsComment;

  // camera config
  int			sixteenBit;
  int			binning2x2;
  int			rawMode;

  // image config
  int			useROI;
  unsigned int		imageSizeX;
  unsigned int		imageSizeY;
  int			colourise;
  QColor		currentColouriseColour;
  int			numCustomColours;
  QList<QColor>		customColours;

  // zoom config
  int			zoomButton1Option;
  int			zoomButton2Option;
  int			zoomButton3Option;
  int			zoomValue;

  // control config
  int64_t		controlValues[ OA_CAM_CTRL_LAST_P1 ];
  int			exposureMenuOption;
  int			frameRateNumerator;
  int			frameRateDenominator;
  int			selectableControl[2];

  // capture config
  int			profileOption;
  int			filterOption;
  int			fileTypeOption;
  QString		frameFileNameTemplate;
  QString		processedFileNameTemplate;
  QString		captureDirectory;
  int			saveEachFrame;
  int			saveProcessedImage;
  int			saveCaptureSettings;
  int			windowsCompatibleAVI;

  // reticle config
  int			reticleStyle;

  // demosaic config
  int			cfaPattern;
  int			demosaicMethod;

  // saved profiles
  int			numProfiles;
  QList<PROFILE>	profiles;

  // filters
  int			numFilters;
  QList<FILTER>		filters;
  int			filterSlots[MAX_FILTER_SLOTS];
  int			promptForFilterChange;
  int			interFilterDelay;

  // advanced user configuration

  QList<userConfigList>	filterWheelConfig;
  int			experimentalASI2;

} CONFIG;

extern CONFIG		config;

#define	SET_PROFILE_CONTROL(c,v) if ( config.profileOption >= 0 ) config.profiles[ config.profileOption ].filterProfiles[ config.filterOption ].controls[ c ] = v
