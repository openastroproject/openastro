/*****************************************************************************
 *
 * configuration.h -- declaration of data structures for configuration data
 *
 * Copyright 2013,2014,2015,2016,2017,2018,2019
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

#if HAVE_CLIMITS
#include <climits>
#endif

extern "C" {
#include <openastro/camera.h>
#include <openastro/filterwheel.h>
#include <openastro/userConfig.h>
}

#define	CONFIG_VERSION	2

#include "profile.h"

typedef struct
{
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

  // camera config
  unsigned int		inputFrameFormat;

  // image config
  QColor		currentColouriseColour;
  int			numCustomColours;
  QList<QColor>		customColours;

  // zoom config
  int			zoomButton1Option;
  int			zoomButton2Option;
  int			zoomButton3Option;
  int			zoomValue;

  // control config
  int			exposureMenuOption;
  int			frameRateNumerator;
  int			frameRateDenominator;
  int			selectableControl[2];
  int			intervalMenuOption;

  // capture config
  QString		frameFileNameTemplate;
  QString		processedFileNameTemplate;
  QString		captureDirectory;
  int			saveEachFrame;
  int			saveProcessedImage;
  int			saveCaptureSettings;
  int			indexDigits;

	// processing config
	double		stackKappa;

} CONFIG;

extern CONFIG		config;
