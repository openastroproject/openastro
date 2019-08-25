/*****************************************************************************
 *
 * oacam-features.h -- camera API (sub)header for camera features
 *
 * Copyright 2014,2015,2016,2018,2019
 *   James Fidell (james@openastroproject.org)
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

#ifndef OPENASTRO_CAMERA_FEATURES_H
#define OPENASTRO_CAMERA_FEATURES_H

typedef struct oaCameraFeatures {
  unsigned int hasRawMode : 1;
  unsigned int hasDemosaicMode : 1;
  unsigned int hasBinning : 1;
  unsigned int hasFrameRates : 1;
  unsigned int hasROI : 1;
  unsigned int hasReset : 1;
  unsigned int hasExternalTrigger : 1;
  unsigned int hasStrobeOutput : 1;
  unsigned int hasFixedFrameSizes : 1;
  unsigned int hasReadableControls : 1;
  unsigned int hasFixedReadNoise : 1;
  unsigned int hasStreamingMode : 1;
  unsigned int frameSizeUnknown : 1;
	unsigned int singleShot : 1 ;
  unsigned int pixelSizeX;
  unsigned int pixelSizeY;
	float readNoise;
	float QE;
} oaCameraFeatures;

#endif	/* OPENASTRO_CAMERA_FEATURES_H */
