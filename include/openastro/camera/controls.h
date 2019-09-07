/*****************************************************************************
 *
 * oacam-controls.h -- camera API (sub)header for camera controls
 *
 * Copyright 2014,2015,2016,2017,2018,2019
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

#ifndef OPENASTRO_CAMERA_CONTROLS_H
#define OPENASTRO_CAMERA_CONTROLS_H

#define	OA_CAM_CTRL_BRIGHTNESS								1
#define	OA_CAM_CTRL_CONTRAST									2
#define	OA_CAM_CTRL_SATURATION								3
#define	OA_CAM_CTRL_HUE												4
#define	OA_CAM_CTRL_WHITE_BALANCE							5
#define	OA_CAM_CTRL_BLUE_BALANCE							6
#define	OA_CAM_CTRL_RED_BALANCE								7
#define	OA_CAM_CTRL_GAMMA											8
#define	OA_CAM_CTRL_EXPOSURE_UNSCALED					9
#define	OA_CAM_CTRL_GAIN											10
#define	OA_CAM_CTRL_HFLIP											11
#define	OA_CAM_CTRL_VFLIP											12
#define	OA_CAM_CTRL_POWER_LINE_FREQ						13
#define	OA_CAM_CTRL_WHITE_BALANCE_TEMP				14
#define	OA_CAM_CTRL_SHARPNESS									15
#define	OA_CAM_CTRL_BACKLIGHT_COMPENSATION		16
#define	OA_CAM_CTRL_CHROMA_AGC								17
#define	OA_CAM_CTRL_COLOUR_KILLER							18
#define	OA_CAM_CTRL_COLOR_KILLER							OA_CAM_CTRL_COLOUR_KILLER
#define	OA_CAM_CTRL_COLOURFX									19
#define	OA_CAM_CTRL_COLORFX										OA_CAM_CTRL_COLOURFX
#define	OA_CAM_CTRL_BAND_STOP_FILTER					20
#define	OA_CAM_CTRL_ROTATE										21
#define	OA_CAM_CTRL_BG_COLOUR									22
#define	OA_CAM_CTRL_BG_COLOR									OA_CAM_CTRL_BG_COLOUR
#define	OA_CAM_CTRL_CHROMA_GAIN								23
#define	OA_CAM_CTRL_MIN_BUFFERS_FOR_CAPTURE		24
#define	OA_CAM_CTRL_ALPHA_COMPONENT						25
#define	OA_CAM_CTRL_COLOURFX_CBCR							26
#define	OA_CAM_CTRL_COLORFX_CBCR							OA_CAM_CTRL_COLOURFX_CBCR
#define OA_CAM_CTRL_EXPOSURE_ABSOLUTE					27
#define	OA_CAM_CTRL_PAN_RELATIVE							28
#define	OA_CAM_CTRL_TILT_RELATIVE							29
#define OA_CAM_CTRL_PAN_RESET									30
#define OA_CAM_CTRL_TILT_RESET								31
#define OA_CAM_CTRL_PAN_ABSOLUTE							32
#define OA_CAM_CTRL_TILT_ABSOLUTE							33
#define OA_CAM_CTRL_ZOOM_ABSOLUTE							34
#define	OA_CAM_CTRL_BACKLIGHT									35
#define	OA_CAM_CTRL_BLACKLEVEL								36
#define	OA_CAM_CTRL_GAIN2X										37
#define	OA_CAM_CTRL_GAINBOOST									38
#define	OA_CAM_CTRL_HDR												39
#define	OA_CAM_CTRL_HPC												40
#define	OA_CAM_CTRL_HIGHSPEED									41
#define	OA_CAM_CTRL_LOWNOISE									42
#define	OA_CAM_CTRL_PIXELCLOCK								43
//Redundant
//#define	OA_CAM_CTRL_COLOUR_MODE							44
#define	OA_CAM_CTRL_ROLLING_SHUTTER						45
#define	OA_CAM_CTRL_SHUTTER										46
#define	OA_CAM_CTRL_SIGNAL_BOOST							47
#define	OA_CAM_CTRL_SUBS_VOLTAGE							48
#define	OA_CAM_CTRL_TEMP_SETPOINT							49
#define	OA_CAM_CTRL_USBTRAFFIC								50
//Redundant
//#define	OA_CAM_CTRL_BIT_DEPTH								51
#define	OA_CAM_CTRL_BINNING										52
#define	OA_CAM_CTRL_TEMPERATURE								53
#define	OA_CAM_CTRL_GREEN_BALANCE							54
#define OA_CAM_CTRL_CONTOUR										55
#define OA_CAM_CTRL_NOISE_REDUCTION						56
#define OA_CAM_CTRL_SAVE_USER									57
#define OA_CAM_CTRL_RESTORE_USER							58
#define OA_CAM_CTRL_RESTORE_FACTORY						59
#define OA_CAM_CTRL_DROPPED										60
#define OA_CAM_CTRL_DROPPED_RESET							61
#define	OA_CAM_CTRL_OVERCLOCK									62
#define	OA_CAM_CTRL_COOLER										63
#define	OA_CAM_CTRL_COOLER_POWER							64
#define	OA_CAM_CTRL_TRIGGER_ENABLE						65
#define	OA_CAM_CTRL_TRIGGER_MODE							66
#define	OA_CAM_CTRL_TRIGGER_SOURCE						67
#define	OA_CAM_CTRL_TRIGGER_POLARITY					68
#define	OA_CAM_CTRL_TRIGGER_DELAY_ENABLE			69
#define	OA_CAM_CTRL_TRIGGER_DELAY							70
#define OA_CAM_CTRL_STROBE_ENABLE							71
#define	OA_CAM_CTRL_STROBE_POLARITY						72
#define OA_CAM_CTRL_STROBE_SOURCE							73
#define	OA_CAM_CTRL_STROBE_DELAY							74
#define	OA_CAM_CTRL_STROBE_DURATION						75
#define	OA_CAM_CTRL_SPEED											76
#define	OA_CAM_CTRL_FAN												77
#define	OA_CAM_CTRL_PATTERN_ADJUST						78
#define	OA_CAM_CTRL_MONO_BIN_COLOUR						79
#define	OA_CAM_CTRL_DEW_HEATER								80
#define OA_CAM_CTRL_AUTO_WHITE_BALANCE_SPEED	81
#define OA_CAM_CTRL_AUTO_WHITE_BALANCE_DELAY	82
#define	OA_CAM_CTRL_AUTO_WHITE_BALANCE_TEMP		83
#define	OA_CAM_CTRL_MAX_AUTO_EXPOSURE					84
#define	OA_CAM_CTRL_MAX_AUTO_GAIN							85
#define	OA_CAM_CTRL_INTERLACE_ENABLE					86
#define	OA_CAM_CTRL_FOCUS_ABSOLUTE						87
#define	OA_CAM_CTRL_IRIS_ABSOLUTE							88
#define	OA_CAM_CTRL_ROLL_ABSOLUTE							89
#define	OA_CAM_CTRL_PRIVACY_ENABLE						90
#define	OA_CAM_CTRL_FOCUS_SIMPLE							91
#define	OA_CAM_CTRL_FOCUS_RELATIVE						92
#define	OA_CAM_CTRL_FOCUS_RELATIVE_SPEED			93
#define	OA_CAM_CTRL_IRIS_RELATIVE							94
#define	OA_CAM_CTRL_ZOOM_RELATIVE							95
#define	OA_CAM_CTRL_ZOOM_RELATIVE_SPEED				96
#define	OA_CAM_CTRL_DIGITAL_ZOOM_ENABLE				97
#define	OA_CAM_CTRL_PAN_RELATIVE_SPEED				98
#define	OA_CAM_CTRL_TILT_RELATIVE_SPEED				99
#define	OA_CAM_CTRL_ROLL_RELATIVE							100
#define	OA_CAM_CTRL_ROLL_RELATIVE_SPEED				101
#define	OA_CAM_CTRL_WHITE_SHADING							102
#define	OA_CAM_CTRL_LED_STATE									103
#define	OA_CAM_CTRL_LED_PERIOD								104
#define OA_CAM_CTRL_AUTO_EXPOSURE_PRIORITY		105
#define OA_CAM_CTRL_EXPOSURE_VALUE						106
#define OA_CAM_CTRL_WHITE_BALANCE_PRESET			107
#define OA_CAM_CTRL_DIGITAL_GAIN							108
#define OA_CAM_CTRL_DIGITAL_GAIN_RED					109
#define OA_CAM_CTRL_DIGITAL_GAIN_GREEN				110
#define OA_CAM_CTRL_DIGITAL_GAIN_BLUE					111
#define OA_CAM_CTRL_FRAME_FORMAT							112
#define	OA_CAM_CTRL_ISO												113
#define	OA_CAM_CTRL_SHUTTER_SPEED							114
#define	OA_CAM_CTRL_MIRROR_LOCKUP							115
#define	OA_CAM_CTRL_POWER_SOURCE							116
#define	OA_CAM_CTRL_BATTERY_LEVEL							117
// Adding more items here may require updating liboacam/control.c
#define	OA_CAM_CTRL_LAST_P1										OA_CAM_CTRL_BATTERY_LEVEL+1

// Adding more here will need camera.h and oacamprivate.h changing to make
// the array bigger and require the the OA_CAM_CTRL_MODIFIER define
// is change to handle going from a bitmask to an integer sequence

#define OA_CAM_CTRL_MODIFIER_STD	0
#define OA_CAM_CTRL_MODIFIER_AUTO	1
#define OA_CAM_CTRL_MODIFIER_ON_OFF	2
#define OA_CAM_CTRL_MODIFIERS_P1	(OA_CAM_CTRL_MODIFIER_ON_OFF+1)

#define OA_CAM_CTRL_MODIFIER_BASE_MASK		0xff
#define OA_CAM_CTRL_MODIFIER_AUTO_MASK		0x100
#define OA_CAM_CTRL_MODIFIER_ON_OFF_MASK	0x200

#define OA_CAM_CTRL_MODIFIER(x)		(x >> 8)

#define OA_CAM_CTRL_MODE_NONAUTO(x)	( x & OA_CAM_CTRL_MODIFIER_BASE_MASK )
#define OA_CAM_CTRL_MODE_AUTO(x)	( x | OA_CAM_CTRL_MODIFIER_AUTO_MASK )
#define OA_CAM_CTRL_MODE_ON_OFF(x)	( x | OA_CAM_CTRL_MODIFIER_ON_OFF_MASK )
#define OA_CAM_CTRL_MODE_BASE(x)	( x & 0xff )

#define OA_CAM_CTRL_IS_AUTO(x)		( x & OA_CAM_CTRL_MODIFIER_AUTO_MASK )
#define OA_CAM_CTRL_IS_ON_OFF(x)	( x & OA_CAM_CTRL_MODIFIER_ON_OFF_MASK )

#define OA_CAM_CTRL_TYPE(x)	controlType[OA_CAM_CTRL_MODIFIER(x)][OA_CAM_CTRL_MODE_BASE(x)]

#define OA_CAM_CTRL_AUTO_TYPE(x)	controlType[OA_CAM_CTRL_MODIFIER_AUTO][OA_CAM_CTRL_MODE_BASE(x)]


// white balance ranges

#define OA_WHITE_BALANCE_MANUAL		0
#define OA_WHITE_BALANCE_AUTO		1
#define OA_WHITE_BALANCE_INCANDESCENT	2
#define OA_WHITE_BALANCE_FLUORESCENT	3
#define OA_WHITE_BALANCE_FLUORESCENT_H	4
#define OA_WHITE_BALANCE_HORIZON	5
#define OA_WHITE_BALANCE_DAYLIGHT	6
#define OA_WHITE_BALANCE_FLASH		7
#define OA_WHITE_BALANCE_CLOUDY		8
#define OA_WHITE_BALANCE_SHADE		9
#define OA_AWB_PRESET_LAST_P1		OA_WHITE_BALANCE_SHADE+1

// auto exposure types

#define OA_EXPOSURE_AUTO		0
#define OA_EXPOSURE_MANUAL		1
#define OA_EXPOSURE_SHUTTER_PRIORITY	2
#define OA_EXPOSURE_APERTURE_PRIORITY	3
#define OA_EXPOSURE_TYPE_LAST_P1	OA_EXPOSURE_APERTURE_PRIORITY+1

// powerline frequency values

#define OA_POWER_LINE_FREQ_OFF		0
#define OA_POWER_LINE_FREQ_50HZ		1
#define OA_POWER_LINE_FREQ_60HZ		2
#define OA_POWER_LINE_FREQ_AUTO		3

#define OA_TRIGGER_EXTERNAL		0
#define OA_TRIGGER_BULB_SHUTTER		1
#define OA_TRIGGER_PULSE_COUNT		2
#define OA_TRIGGER_SKIP_FRAMES		3
#define OA_TRIGGER_MULTI_PRESET		4
#define OA_TRIGGER_MULTI_PULSE_WIDTH    5
#define OA_TRIGGER_LOW_SMEAR		13
#define OA_TRIGGER_VENDOR_1		14
#define OA_TRIGGER_VENDOR_2		15

#define OA_TRIGGER_SOURCE_0		0
#define OA_TRIGGER_SOURCE_1		1
#define OA_TRIGGER_SOURCE_2		2
#define OA_TRIGGER_SOURCE_3		3

#define OA_TRIGGER_POLARITY_ACTIVE_LOW	0
#define OA_TRIGGER_POLARITY_ACTIVE_HIGH	1

#define	OA_AC_POWER				0
#define OA_BATTERY_POWER	1

extern const char* oaCameraControlLabel[ OA_CAM_CTRL_LAST_P1 ];
extern const char* oaCameraControlModifierPrefix[ OA_CAM_CTRL_MODIFIERS_P1 ];
extern const char* oaCameraPresetAWBLabel[ OA_AWB_PRESET_LAST_P1 ];
extern const char* oaCameraAutoExposureLabel[ OA_EXPOSURE_TYPE_LAST_P1 ];

#endif	/* OPENASTRO_CAMERA_CONTROLS_H */
