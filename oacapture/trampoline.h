/*****************************************************************************
 *
 * trampoline.h -- functions redirecting to class function calls
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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

#include <QtGui>

#include <oa_common.h>

typedef struct {
  int ( *getCurrentGain )( void );
  int ( *getCurrentExposure )( void );
  int ( *getCurrentTargetId )( void );
  QString ( *getCurrentFilterName )( void );
  QString ( *getCurrentProfileName )( void );
	void ( *setFilterSlotCount )( int );
	void ( *reloadFilters )( void );
	void ( *updateHistogramLayout )( void );
	void ( *resetAutorun )( void );
	void ( *updateControlCheckbox )( int, int );
	int ( *getSpinboxMinimum )( int );
	int ( *getSpinboxMaximum )( int );
	int ( *getSpinboxStep )( int );
	int ( *getSpinboxValue )( int );
	void ( *updateSpinbox )( int, int );
	QStringList ( *getFrameRates )( void );
	int ( *getFrameRateIndex )( void );
	void ( *updateFrameRate )( int );
	void ( *setFlipX )( int );
	void ( *setFlipY )( int );
	void ( *updateForceFrameFormat )( unsigned int, unsigned int );
	void ( *reloadProfiles )( void );
	void ( *resetTemperatureLabel )( void );
	void ( *setDisplayFPS )( int );
	void ( *enableTIFFCapture )( int );
	void ( *enableMOVCapture )( int );
	void ( *enablePNGCapture )( int );
	void ( *setVideoFramePixelFormat )( int );
} trampolineFuncs;

extern trampolineFuncs trampolines;
