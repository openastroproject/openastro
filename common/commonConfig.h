/*****************************************************************************
 *
 * commonConfig.h -- common configuration data
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

#include <oa_common.h>

#if HAVE_LIMITS_H
#include <limits.h>
#endif

extern "C" {
#include <openastro/camera.h>
#include <openastro/filterwheel.h>
#include <openastro/userConfig.h>
}

#include <QtCore>

typedef QList<userDeviceConfig> userConfigList;

typedef struct
{
  // capture config
  int							profileOption;
  int							filterOption;
  int							fileTypeOption;
  int							limitEnabled;
  int							secondsLimitValue;
  int							framesLimitValue;
  int							limitType;

	// options
	int							demosaic;

	// camera config
	int							binning2x2;
	int							colourise;

	// image config
	int							useROI;
	unsigned int		imageSizeX;
	unsigned int		imageSizeY;

	// control config
	int							frameRateNumerator;
	int							frameRateDenominator;
	QString					fileNameTemplate;
	QString					frameFileNameTemplate;
	QString					captureDirectory;

	// advanced user configuration
	QList<userConfigList>		filterWheelConfig;
	QList<userConfigList>		timerConfig;

} COMMON_CONFIG;

extern COMMON_CONFIG		commonConfig;

#define CONTROL_VALUE(c)	controlValues[OA_CAM_CTRL_MODIFIER(c)][OA_CAM_CTRL_MODE_BASE(c)]

#define	SET_PROFILE_CONTROL(c,v) if ( commonConfig.profileOption >= 0 ) profileConf.profiles[ commonConfig.profileOption ].filterProfiles[ commonConfig.filterOption ].controls[OA_CAM_CTRL_MODIFIER(c)][OA_CAM_CTRL_MODE_BASE(c)] = v

#define	SET_PROFILE_INTERVAL(v) if ( commonConfig.profileOption >= 0 ) profileConf.profiles[ commonConfig.profileOption ].filterProfiles[ commonConfig.filterOption ].intervalMenuOption = v

#define SET_PROFILE_CONFIG(n,v) if ( commonConfig.profileOption >= 0 ) profileConf.profiles[commonConfig.profileOption].n = v
