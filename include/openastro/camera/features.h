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
	unsigned long flags;
  unsigned int pixelSizeX;
  unsigned int pixelSizeY;
	float readNoise;
	float QE;
} oaCameraFeatures;

#define OA_CAM_FEATURE_NONE										0x00000000
#define OA_CAM_FEATURE_RAW_MODE								0x00000001
#define OA_CAM_FEATURE_DEMOSAIC_MODE					0x00000002
#define OA_CAM_FEATURE_BINNING								0x00000004
#define OA_CAM_FEATURE_FRAME_RATES						0x00000008
#define OA_CAM_FEATURE_ROI										0x00000010
#define OA_CAM_FEATURE_RESET									0x00000020
#define OA_CAM_FEATURE_EXTERNAL_TRIGGER				0x00000040
#define OA_CAM_FEATURE_STROBE_OUTPUT					0x00000080
#define OA_CAM_FEATURE_FIXED_FRAME_SIZES			0x00000100
#define OA_CAM_FEATURE_READABLE_CONTROLS			0x00000200
#define OA_CAM_FEATURE_FIXED_READ_NOISE				0x00000400
#define OA_CAM_FEATURE_STREAMING							0x00000800
#define OA_CAM_FEATURE_SINGLE_SHOT						0x00001000
#define OA_CAM_FEATURE_FRAME_SIZE_UNKNOWN			0x00002000

#endif	/* OPENASTRO_CAMERA_FEATURES_H */
