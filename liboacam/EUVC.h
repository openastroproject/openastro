/*****************************************************************************
 *
 * EUVC.h -- header for EUVC camera API
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

#ifndef OA_EUVC_H
#define OA_EUVC_H

#define	TIS_VENDOR_ID	0x199e

#define	EUVC_CAM_UNKNOWN	1
#define	EUVC_CAM_DFK22		2
#define	EUVC_CAM_DFK61		3
#define	EUVC_CAM_DFK41		4
#define	EUVC_CAM_DFx51		5
#define	EUVC_CAM_DFx41		6
#define	EUVC_CAM_DFK72		7
#define	EUVC_CAM_DFK42		8

#define VS_PROBE_CONTROL	0x01
#define VS_COMMIT_CONTROL	0x02

#define EUVC_VC_DESCRIPTOR_UNDEFINED	0x00
#define EUVC_VC_HEADER			0x01
#define EUVC_VC_INPUT_TERMINAL		0x02
#define EUVC_VC_OUTPUT_TERMINAL		0x03
#define EUVC_VC_SELECTOR_UNIT		0x04
#define EUVC_VC_PROCESSING_UNIT		0x05
#define EUVC_VC_EXTENSION_UNIT		0x06

#define EUVC_VS_INPUT_HEADER		0x01
#define EUVC_VS_OUTPUT_HEADER		0x02
#define EUVC_VS_STILL_IMAGE_FRAME	0x03
#define EUVC_VS_FORMAT_UNCOMPRESSED	0x04
#define EUVC_VS_FRAME_UNCOMPRESSED	0x05
#define EUVC_VS_FORMAT_MJPEG		0x06
#define EUVC_VS_FRAME_MJPEG		0x07
#define EUVC_VS_FORMAT_MPEG2TS		0x0a
#define EUVC_VS_FORMAT_DV		0x0c
#define EUVC_VS_COLORFORMAT		0x0d
#define EUVC_VS_FORMAT_FRAME_BASED	0x10
#define EUVC_VS_FRAME_FRAME_BASED	0x11
#define EUVC_VS_FORMAT_STREAM_BASED	0x12

#define EUVC_CT_SCANNING_MODE_CONTROL		0x01
#define EUVC_CT_AE_MODE_CONTROL			0x02
#define EUVC_CT_AE_PRIORITY_CONTROL		0x03
#define EUVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL	0x04
#define EUVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL	0x05
#define EUVC_CT_FOCUS_ABSOLUTE_CONTROL		0x06
#define EUVC_CT_FOCUS_RELATIVE_CONTROL		0x07
#define EUVC_CT_FOCUS_AUTO_CONTROL		0x08
#define EUVC_CT_IRIS_ABSOLUTE_CONTROL		0x09
#define EUVC_CT_IRIS_RELATIVE_CONTROL		0x0a
#define EUVC_CT_ZOOM_ABSOLUTE_CONTROL		0x0b
#define EUVC_CT_ZOOM_RELATIVE_CONTROL		0x0c
#define EUVC_CT_PANTILT_ABSOLUTE_CONTROL	0x0d
#define EUVC_CT_PANTILT_RELATIVE_CONTROL	0x0e
#define EUVC_CT_ROLL_ABSOLUTE_CONTROL		0x0f
#define EUVC_CT_ROLL_RELATIVE_CONTROL		0x10
#define EUVC_CT_PRIVACY_CONTROL			0x11
#define EUVC_CT_FOCUS_SIMPLE_CONTROL		0x12
#define EUVC_CT_DIGITAL_WINDOW_CONTROL		0x13
#define EUVC_CT_REGION_OF_INTEREST_CONTROL	0x14
#define	EUVC_CT_CAPABILITY			0x20
#define EUVC_CT_PIXEL_CLOCK			0x24
#define EUVC_CT_PARTIAL_SCAN_WIDTH		0x25
#define EUVC_CT_PARTIAL_SCAN_HEIGHT		0x26
#define EUVC_CT_BLANKING_INFO			0x27
#define EUVC_CT_PARTIAL_SCAN_X			0x28
#define EUVC_CT_PARTIAL_SCAN_Y			0x29
#define EUVC_CT_BINNING				0x2a
#define EUVC_CT_SOFTWARE_TRIGGER		0x2b
#define EUVC_CT_SENSOR_RESET			0x2c
#define EUVC_CT_FIRMWARE_REVISION		0x2d
#define EUVC_CT_GPOUT				0x2e
#define EUVC_CT_HDR_ENABLE			0x2f
#define EUVC_CT_HDR_SHUTTER_1			0x30
#define EUVC_CT_HDR_SHUTTER_2			0x31
#define EUVC_CT_HDR_VSTEP_1			0x32
#define EUVC_CT_HDR_VSTEP_2			0x33
#define EUVC_CT_HDR_VSTEP_3			0x34
#define EUVC_CT_HDR_VSTEP_4			0x35
#define EUVC_CT_UNKNOWN_CMD_1			0x38
#define EUVC_CT_UNKNOWN_CMD_2			0x39
#define EUVC_CT_UART				0x41

#define EUVC_CT_CAPABILITY_PIXEL_CLOCK         (1<<4)
#define EUVC_CT_CAPABILITY_PARTIAL_SCAN_WIDTH  (1<<5)
#define EUVC_CT_CAPABILITY_PARTIAL_SCAN_HEIGHT (1<<6)
#define EUVC_CT_CAPABILITY_BLANKING_INFO       (1<<7)
#define EUVC_CT_CAPABILITY_BINNING             (1<<8)
#define EUVC_CT_CAPABILITY_SOFTWARE_TRIGGER    (1<<9)
#define EUVC_CT_CAPABILITY_FIRMWARE_REVISION   (1<<11)
#define EUVC_CT_CAPABILITY_GPIO                (1<<12)
#define EUVC_CT_CAPABILITY_UART                (1<<14)

#define	EUVC_PU_BACKLIGHT_COMPENSATION_CONTROL		0x01
#define	EUVC_PU_BRIGHTNESS_CONTROL			0x02
#define EUVC_PU_CONTRAST_CONTROL			0x03
#define EUVC_PU_GAIN_CONTROL				0x04
#define EUVC_PU_POWER_LINE_FREQUENCY_CONTROL		0x05
#define EUVC_PU_HUE_CONTROL				0x06
#define EUVC_PU_SATURATION_CONTROL			0x07
#define EUVC_PU_SHARPNESS_CONTROL			0x08
#define EUVC_PU_GAMMA_CONTROL				0x09
#define EUVC_PU_GAMMA_CONTROL				0x09
#define EUVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL	0x0a
#define EUVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL	0x0b
#define EUVC_PU_WHITE_BALANCE_COMPONENT_CONTROL		0x0c
#define EUVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL	0x0d
#define EUVC_PU_DIGITAL_MULTIPLIER_CONTROL		0x0e
#define EUVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL	0x0f
#define EUVC_PU_HUE_AUTO_CONTROL			0x10
#define EUVC_PU_ANALOG_VIDEO_STANDARD_CONTROL		0x11
#define EUVC_PU_ANALOG_LOCK_STATUS_CONTROL		0x12
#define EUVC_PU_CONTRAST_AUTO_CONTROL			0x13
#define EUVC_PU_CAPABILITY				0x20
#define EUVC_PU_COLOR_FORMAT				0x21

#define EUVC_SET_CUR	0x01
#define EUVC_GET_CUR	0x81
#define EUVC_GET_MIN	0x82
#define EUVC_GET_MAX	0x83
#define EUVC_GET_RES	0x84
#define EUVC_GET_LEN	0x85
#define EUVC_GET_INFO	0x86
#define EUVC_GET_DEF	0x87

#define EUVC_PU_CAPABILITY_COLOUR_FORMAT	(1<<1)

#define	EUVC_COLOUR_FORMAT_MONO8		(1<<0)
#define	EUVC_COLOUR_FORMAT_RGGB			(1<<3)
#define	EUVC_COLOUR_FORMAT_GRBG			(1<<4)
#define	EUVC_COLOUR_FORMAT_BGGR			(1<<5)
#define	EUVC_COLOUR_FORMAT_GBRG			(1<<6)

#define EUVC_CAM_TERMINAL	1

#define	EUVC_REG_CAMERA_TYPE	0x1a
#define	EUVC_REG_FRAME_RATE	0x3a

#define EUVC_NUM_TRANSFER_BUFS	100


struct euvccam {
  unsigned int	productId;
  const char*	name;
  unsigned int	devType;
};

extern struct euvccam EUVCCameraList[];

struct euvcFormat {
  unsigned int	formatId;
  unsigned int	frameId;
  unsigned int	xSize;
  unsigned int	ySize;
  unsigned int	frameFormat;
  FRAMERATES	frameIntervals;
  unsigned int*	frameIntervalIds;
};

struct puCtrl {
  uint8_t       euvcControl;
  int           oaControl;
  int           oaControlType;
  int           size;
};

struct frameExtras {
  uint8_t	formatId;
  uint8_t	frameId;
  unsigned int	maxBufferSize;
};

typedef struct {
  uint16_t	bmHint;
  uint8_t	bFormatIndex;
  uint8_t	bFrameIndex;
  uint32_t	wFrameInterval;
/*
  uint16_t	wKeyFrameRate;
  uint16_t	wPFrameRate;
  uint16_t	wCompQuality;
  uint16_t	wCompWindowSize;
  uint16_t	wDelay;
  uint32_t	wMaxVideoFrameSize;
  uint32_t	wMaxPayloadTransferSize;
  uint32_t	wClockFrequency;
  uint8_t	bmFramingInfo;
  uint8_t	bPreferedVersion;
  uint8_t	bMinVersion;
  uint8_t	bMaxVersion;
*/
} PROBE_BLOCK;


extern struct puCtrl	EUVCControlData[];
extern unsigned int	numPUEUVCControls;

#endif	/* OA_EUVC_H */
