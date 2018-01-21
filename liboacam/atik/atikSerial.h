/*****************************************************************************
 *
 * atikSerial.h -- header for Atik Serial camera config
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

#ifndef OA_ATIK_SERIAL_H
#define OA_ATIK_SERIAL_H

#define CAPS_FLAGS_LO           0
#define CAPS_FLAGS_HI           1
#define CAPS_TOTAL_PIXELS_X_LO  2
#define CAPS_TOTAL_PIXELS_X_HI  3
#define CAPS_TOTAL_PIXELS_Y_LO  4
#define CAPS_TOTAL_PIXELS_Y_HI  5
#define CAPS_PIXEL_SIZE_X_LO    6
#define CAPS_PIXEL_SIZE_X_HI    7
#define CAPS_PIXEL_SIZE_Y_LO    8
#define CAPS_PIXEL_SIZE_Y_HI    9
#define CAPS_MAX_BIN_X_LO       10
#define CAPS_MAX_BIN_X_HI       11
#define CAPS_MAX_BIN_Y_LO       12
#define CAPS_MAX_BIN_Y_HI       13
#define CAPS_WELL_DEPTH_LO      14
#define CAPS_WELL_DEPTH_HI      15

#define CAM_ARTEMIS		1
#define CAM_MINI_ARTEMIS	2
#define CAM_ATK16		3
#define CAM_ATK16HR		4
#define CAM_ATK16C		5
#define CAM_ATK16HRC		6
#define CAM_ATK16IC		7
#define CAM_ATK16ICC		8
#define CAM_ATK16ICS		9
#define CAM_ATK16ICSC		10

#define	ATIK_FTDI_VENDOR_ID	0x0403

#define	ATIK_CMD_RESET			0
#define	ATIK_CMD_QUERY_CAPS		1
#define	ATIK_CMD_SET_AMP		2
#define	ATIK_CMD_SET_ADC		3
#define	ATIK_CMD_CLEAR_CCD		4
#define	ATIK_CMD_START_EXPOSURE		5
#define	ATIK_CMD_READ_CCD		6

#define	ATIK_CMD_SEND_EXTERNAL		11

#define	ATIK_CMD_CLOCK_CCD		18

#define	ATIK_CMD_DISABLE_GUIDE		24

#define	ATIK_CMD_PING_APP		80

#define	ATIK_CMD_QUERY_FIFO		89

#define	ATIK_CMD_PING			253
#define	ATIK_CMD_QUERY_SERIAL_NO	254

#define ATIK_SERIAL_MAX_SHORT_EXPOSURE	2390

#define	ATIK_SERIAL_FLAGS_HAVE_FIFO	0x01
#define	ATIK_SERIAL_FLAGS_INTERLACED	0x02

#define	ATIK_SERIAL_READ_FLAGS_CTP_1		0x0001
#define	ATIK_SERIAL_READ_FLAGS_CTP_2		0x0002
#define	ATIK_SERIAL_READ_FLAGS_DEINTERLACE	0x0004
#define	ATIK_SERIAL_READ_FLAGS_PRECHARGE	0x0008
#define	ATIK_SERIAL_READ_FLAGS_FOCUS_MODE	0x0010
#define	ATIK_SERIAL_READ_FLAGS_IPCS_MODE	0x0020
#define	ATIK_SERIAL_READ_FLAGS_TIMER_AMP_ON	0x0040
#define	ATIK_SERIAL_READ_FLAGS_USE_FIFO		0x0080
#define	ATIK_SERIAL_READ_FLAGS_PREVIEW		0x0100
#define	ATIK_SERIAL_READ_FLAGS_SUBSAMPLE	0x0200
#define	ATIK_SERIAL_READ_FLAGS_FAST_READ	0x0400
#define	ATIK_SERIAL_READ_FLAGS_AMP_ON		0x0800
#define	ATIK_SERIAL_READ_FLAGS_STREAM		0x1000
#define	ATIK_SERIAL_READ_FLAGS_OVERLAP		0x2000
#define	ATIK_SERIAL_READ_FLAGS_OVERSAMPLE	0x4000
#define	ATIK_SERIAL_READ_FLAGS_EXT_TRIGGER	0x8000

#define	ATIK_SERIAL_READ_FLAGS_CTP_BOTH		0x0003
#define	ATIK_SERIAL_READ_FLAGS_PC_CHECK		0x0028

#define ATIK_SERIAL_CCD_COMMAND			3
#define	ATIK_SERIAL_CCD_EXPOSURE_TIMING		4
#define	ATIK_SERIAL_CCD_X_BINNING		5
#define	ATIK_SERIAL_CCD_Y_BINNING		6
#define	ATIK_SERIAL_CCD_ROI_X_LO		7
#define	ATIK_SERIAL_CCD_ROI_X_HI		8
#define	ATIK_SERIAL_CCD_ROI_Y_LO		9
#define	ATIK_SERIAL_CCD_ROI_Y_HI		10
#define	ATIK_SERIAL_CCD_SIZE_X_LO		11
#define	ATIK_SERIAL_CCD_SIZE_X_HI		12
#define	ATIK_SERIAL_CCD_SIZE_Y_LO		13
#define	ATIK_SERIAL_CCD_SIZE_Y_HI		14
#define	ATIK_SERIAL_CCD_CONFIG_FLAGS_LO		15
#define	ATIK_SERIAL_CCD_CONFIG_FLAGS_HI		16
#define	ATIK_SERIAL_CCD_BUFFER_LENGTH		17

#define	EXPOSURE_TIMING_EXTERNAL	0

#endif	/* OA_ATIK_SERIAL_H */
