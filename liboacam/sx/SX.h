/*****************************************************************************
 *
 * SX.h -- header for Starlight Xpress camera API
 *
 * Copyright 2015,2018 James Fidell (james@openastroproject.org)
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

#ifndef OA_SX_H
#define OA_SX_H

#define CAM_SXVF_M5	1
#define CAM_SXVF_M5C	2
#define CAM_SXVF_M7	3
#define CAM_SXVF_M7C	4
#define CAM_SXVF_M8C	5
#define CAM_SXVF_M9	6
#define CAM_SXVR_M25C	7
#define CAM_SXVR_M26C	8
#define CAM_SXVR_H18	9
#define CAM_SXVR_H16	10
#define CAM_SXVR_H35	11
#define CAM_SXVR_H36	12
#define CAM_SXVR_H9	13
#define CAM_SXVR_H9C	14
#define CAM_LODESTAR	15
#define CAM_LODESTAR_C	16
#define CAM_COSTAR	17
#define CAM_SUPERSTAR	18
#define CAM_MX		19

// fields in request block

#define SXUSB_REQ_CMD_TYPE          0
#define SXUSB_REQ_CMD               1
#define SXUSB_REQ_VALUE_L           2
#define SXUSB_REQ_VALUE_H           3
#define SXUSB_REQ_INDEX_L           4
#define SXUSB_REQ_INDEX_H           5
#define SXUSB_REQ_LENGTH_L          6
#define SXUSB_REQ_LENGTH_H          7
#define SXUSB_REQ_DATA              8

#define SXUSB_CMD_SEND              0x40
#define SXUSB_CMD_REQUEST           0xc0

// commands from the SX USB programming reference

#define SXUSB_GET_FIRMWARE_VERSION  255
#define SXUSB_ECHO                  0
#define SXUSB_CLEAR_PIXELS          1
#define SXUSB_READ_PIXELS_DELAYED   2
#define SXUSB_READ_PIXELS           3
#define SXUSB_SET_TIMER             4
#define SXUSB_GET_TIMER             5
#define SXUSB_RESET                 6
#define SXUSB_SET_CCD               7
#define SXUSB_GET_CCD               8
#define SXUSB_SET_STAR2K            9
#define SXUSB_WRITE_SERIAL_PORT     10
#define SXUSB_READ_SERIAL_PORT      11
#define SXUSB_SET_SERIAL            12
#define SXUSB_GET_SERIAL            13
#define SXUSB_CAMERA_MODEL          14
#define SXUSB_LOAD_EEPROM           15
#define SXUSB_SET_A2D               16
#define SXUSB_READ_A2D              17
#define SXUSB_READ_PIXELS_GATED     18
#define SXUSB_BUILD_NUMBER          19

#define SXUSB_COOLER                30
#define SXUSB_COOLER_TEMPERATURE    31
#define SXUSB_SHUTTER               32
#define SXUSB_READ_I2CPORT          33

// buffer sizes for commands and responses

#define SXUSB_REQUEST_BUFSIZE       8
#define SXUSB_GET_CCD_BUFSIZE       17
#define SXUSB_CAMERA_MODEL_BUFSIZE  2
#define SXUSB_TEMPERATURE_BUFSIZE   2
#define SXUSB_READ_BUFSIZE          18
#define SXUSB_TIMER_BUFSIZE         12

// endpoints for bulk transfers

#define SXUSB_BULK_ENDP_OUT         0x01
#define SXUSB_BULK_ENDP_IN          0x82


// command flags
// low byte
#define CCD_EXP_FLAGS_FIELD_ODD         0x01
#define CCD_EXP_FLAGS_FIELD_EVEN        0x02
#define CCD_EXP_FLAGS_FIELD_BOTH        0x03
#define CCD_EXP_FLAGS_SPARE2            0x04
#define CCD_EXP_FLAGS_NOWIPE_FRAME      0x08
#define CCD_EXP_FLAGS_SPARE4            0x10
#define CCD_EXP_FLAGS_TDI               0x20
#define CCD_EXP_FLAGS_NOCLEAR_FRAME     0x40
#define CCD_EXP_FLAGS_NOCLEAR_REGISTER  0x80
// high byte
#define CCD_EXP_FLAGS_SPARE8            0x01
#define CCD_EXP_FLAGS_SPARE9            0x02
#define CCD_EXP_FLAGS_SPARE10           0x04
#define CCD_EXP_FLAGS_SPARE11           0x08
#define CCD_EXP_FLAGS_SPARE12           0x10
#define CCD_EXP_FLAGS_SHUTTER_MANUAL    0x20
#define CCD_EXP_FLAGS_SHUTTER_OPEN      0x40
#define CCD_EXP_FLAGS_SHUTTER_CLOSE     0x80

// timeout

#define SXUSB_TIMEOUT               500
#define SXUSB_FRAME_TIMEOUT         10000

// capabilities

#define SXCCD_CAPS_STAR2K           0x01
#define SXCCD_CAPS_COMPRESS         0x02
#define SXCCD_CAPS_EEPROM           0x04
#define SXCCD_CAPS_GUIDER           0x08
#define SXUSB_CAPS_COOLER           0x10
#define SXUSB_CAPS_SHUTTER          0x20

// colour matrix values

#define SX_COLOUR_MATRIX_PACKED_RGB		0x8000
#define SX_COLOUR_MATRIX_PACKED_BGR		0x4000
#define SX_COLOUR_MATRIX_PACKED_RED_SIZE	0x0f00
#define SX_COLOUR_MATRIX_PACKED_GREEN_SIZE	0x00f0
#define SX_COLOUR_MATRIX_PACKED_BLUE_SIZE	0x000f
#define SX_COLOUR_MATRIX_ALT_EVEN		0x2000
#define SX_COLOUR_MATRIX_ALT_ODD		0x1000
#define SX_COLOUR_MATRIX_2X2			0x0000
#define SX_COLOUR_MATRIX_RED_MASK		0x0f00
#define SX_COLOUR_MATRIX_GREEN_MASK		0x00f0
#define SX_COLOUR_MATRIX_BLUE_MASK		0x000f
#define SX_COLOUR_MATRIX_MONOCHROME		0x0fff

// masks for the model features

#define SX_MODEL_MASK_COLOUR      0x80
#define SX_MODEL_MASK_INTERLACE   0x40
#define SX_MODEL_MASK_MODEL       0x1f

#endif	/* OA_SX_H */
