/*****************************************************************************
 *
 * profile.h -- declaration of data structures for configuration profiles
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

#if HAVE_LIMITS_H
#include <limits.h>
#endif

extern "C" {
#include <openastro/camera.h>
#include <openastro/filterwheel.h>
#include <openastro/userConfig.h>
}


typedef struct {
  QString	filterName;
  int		controls[OA_CAM_CTRL_MODIFIERS_P1][ OA_CAM_CTRL_LAST_P1 ];
  int		intervalMenuOption;
} FILTER_PROFILE;

typedef struct {
  QString       profileName;
  int           binning2x2;
  int           colourise;
  int           useROI;
  unsigned int  imageSizeX;
  unsigned int  imageSizeY;
  QList<FILTER_PROFILE> filterProfiles;
  int           frameRateNumerator;
  int           frameRateDenominator;
  int           fileTypeOption;
  int           filterOption;
  int           limitEnabled;
  int           secondsLimitValue;
  int           framesLimitValue;
  int           limitType;
  QString       fileNameTemplate;
  QString       frameFileNameTemplate;
  QString       processedFileNameTemplate;
  int		target;
} PROFILE;


// overkill, but i may want to expand this later
typedef struct {
  QString	filterName;
} FILTER;


#define	SET_PROFILE_CONTROL(c,v) if ( config.profileOption >= 0 ) config.profiles[ config.profileOption ].filterProfiles[ config.filterOption ].controls[OA_CAM_CTRL_MODIFIER(c)][OA_CAM_CTRL_MODE_BASE(c)] = v

#define	SET_PROFILE_INTERVAL(v) if ( config.profileOption >= 0 ) config.profiles[ config.profileOption ].filterProfiles[ config.filterOption ].intervalMenuOption = v

#define SET_PROFILE_CONFIG(n,v) if ( config.profileOption >= 0 ) config.profiles[config.profileOption].n = v
