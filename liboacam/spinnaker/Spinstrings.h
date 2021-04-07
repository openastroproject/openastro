/*****************************************************************************
 *
 * Spinstrings.h -- definitions of string values
 *
 * Copyright 2018,2021
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

#ifndef OA_SPINNAKER_SPINSTRINGS_H
#define OA_SPINNAKER_SPINSTRINGS_H

extern const char*	analogueFeatures[];
extern const char*	deviceFeatures[];
extern const char*	acquisitionFeatures[];
extern const char*	formatFeatures[];
extern const char*	nodeTypes[];

#define	ANALOGUE_GAIN_SELECTOR		0
#define	ANALOGUE_GAIN_AUTO		1
#define	ANALOGUE_GAIN			2
#define	ANALOGUE_AUTO_GAIN_LOWER_LIMIT	3
#define	ANALOGUE_AUTO_GAIN_UPPER_LIMIT	4
#define	ANALOGUE_BLACK_LEVEL		5
#define	ANALOGUE_BLACK_LEVEL_ENABLED	6
#define	ANALOGUE_UNKNOWN_7		7
#define	ANALOGUE_UNKNOWN_8		8
#define	ANALOGUE_UNKNOWN_9		9
#define	ANALOGUE_UNKNOWN_10		10
#define	ANALOGUE_GAMMA			11
#define	ANALOGUE_GAMMA_ENABLED		12
#define	ANALOGUE_SHARPNESS		13
#define	ANALOGUE_SHARPNESS_ENABLED	14
#define	ANALOGUE_SHARPNESS_AUTO		15
#define	ANALOGUE_MAX_FEATURES		( ANALOGUE_SHARPNESS_AUTO + 1 )

#define DEVICE_INDICATOR_MODE		0
#define DEVICE_VENDOR_NAME		1
#define DEVICE_SENSOR_DESC		2
#define DEVICE_MODEL_NAME		3
#define DEVICE_VERSION			4
#define DEVICE_SVN_VERSION		5
#define DEVICE_FW_VERSION		6
#define DEVICE_ID			7
#define DEVICE_SERIAL_NO		8
#define DEVICE_USER_ID			9
#define DEVICE_SCAN_TYPE		10
#define DEVICE_TEMPERATURE		11
#define DEVICE_RESET			12
#define DEVICE_UPTIME			13
#define DEVICE_AUTO_FUNC_AOIS_CONTROL	14
#define DEVICE_UNKNOWN_15		15
#define DEVICE_UNKNOWN_16		16
#define DEVICE_UNKNOWN_17		17
#define DEVICE_UNKNOWN_18		18
#define DEVICE_POWER_SUPPLY_SELECTOR	19
#define DEVICE_UNKNOWN_20		20
#define DEVICE_UNKNOWN_21		21
#define DEVICE_POWER_SUPPLY_VOLTAGE	22
#define DEVICE_POWER_SUPPLY_CURRENT	23
#define DEVICE_MAX_THROUGHPUT		24
#define DEVICE_LINK_THROUGHPUT_LIMIT	25
#define	DEVICE_MAX_FEATURES		( DEVICE_LINK_THROUGHPUT_LIMIT + 1 )

#define	AQUISITION_TRIGGER_SELECTOR	0
#define	AQUISITION_TRIGGER_MODE		1
#define	AQUISITION_TRIGGER_SOFTWARE		2
#define	AQUISITION_TRIGGER_SOURCE	3
#define	AQUISITION_TRIGGER_ACTIVATION	4
#define	AQUISITION_OVERLAP		5
#define	AQUISITION_TRIGGER_DELAY	6
#define	AQUISITION_TRIGGER_DELAY_ENABLED	7
#define	AQUISITION_EXPOSURE_MODE	8
#define	AQUISITION_EXPOSURE_AUTO	9
#define	AQUISITION_EXPOSURE_TIME	10
#define	AQUISITION_EXPOSURE_TIME_ABS	11
#define	AQUISITION_AUTO_EXP_LOWER_LIMIT	12
#define	AQUISITION_AUTO_EXP_UPPER_LIMIT	13
#define	AQUISITION_EXPOSURE_COMPENSATION_AUTO	14
#define	AQUISITION_EXPOSURE_COMPENSATION	15
#define	AQUISITION_AUTO_EC_LOWER_LIMIT	16
#define	AQUISITION_AUTO_EC_UPPER_LIMIT	17
#define	AQUISITION_MODE			18
#define	AQUISITION_START		19
#define	AQUISITION_STOP			20
#define	AQUISITION_FRAME_RATE_AUTO	21
#define	AQUISITION_FR_CONTROL_ENABLED	22
#define	AQUISITION_FRAME_RATE		23

#define	FORMAT_PIXEL_FORMAT		0
#define	FORMAT_UNKNOWN_1		1
#define	FORMAT_WIDTH			2
#define	FORMAT_HEIGHT			3
#define	FORMAT_OFFSET_X			4
#define	FORMAT_OFFSET_Y			5
#define	FORMAT_SENSOR_WIDTH		6
#define	FORMAT_SENSOR_HEIGHT		7
#define	FORMAT_MAX_WIDTH		8
#define	FORMAT_MAX_HEIGHT		9
#define	FORMAT_VIDEO_MODE		10
#define	FORMAT_BINNING_HORIZONTAL	11
#define	FORMAT_BINNING_VERTICAL		12
#define	FORMAT_UNKNOWN_13		13
#define	FORMAT_UNKNOWN_14		14
#define	FORMAT_REVERSE_X		15
#define	FORMAT_PIXEL_BIG_ENDIAN		16
#define	FORMAT_PIXEL_CODING		17
#define	FORMAT_PIXEL_SIZE		18
#define	FORMAT_COLOUR_FILTER		19
#define	FORMAT_PIXEL_DYNAMIC_RANGE_MIN	20
#define	FORMAT_PIXEL_DYNAMIC_RANGE_MAX	21
#define	FORMAT_TEST_IMAGE_SELECTOR	22
#define	FORMAT_TEST_PATTERN		23

#endif /* OA_SPINNAKER_SPINSTRINGS_H */
