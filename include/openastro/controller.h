/*****************************************************************************
 *
 * controller.h -- common definitions for device controller queues
 *
 * Copyright 2015,2016,2017,2018,2019
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

#ifndef OPENASTRO_CONTROLLER_H
#define OPENASTRO_CONTROLLER_H

typedef struct {
  unsigned int          commandType;
  unsigned int          completed;
  unsigned int          controlId;
  int                   resultCode;
  void*                 commandArgs;
  void*                 commandData;
  void*                 resultData;
  void*                 callback;
} OA_COMMAND;

typedef struct {
  unsigned int          callbackType;
  void*                 callback;
  void*                 callbackArg;
  void*                 buffer;
  void*                 metadata;
  int                   bufferLen;
} CALLBACK;

#define OA_CMD_CONTROL_SET              0x01
#define OA_CMD_CONTROL_GET              0x02
#define OA_CMD_RESOLUTION_SET           0x03
#define OA_CMD_RESOLUTION_GET           0x04
#define OA_CMD_START                    0x05
#define OA_CMD_STOP                     0x06
#define OA_CMD_FRAME_INTERVAL_SET       0x07
#define OA_CMD_MENU_ITEM_GET            0x08
#define OA_CMD_WARM_RESET               0x09
#define	OA_CMD_RESET			OA_CMD_WARM_RESET
#define OA_CMD_COLD_RESET               0x0a
#define OA_CMD_ROI_SET                  0x0b
#define OA_CMD_ROI_GET                  0x0c
#define OA_CMD_DATA_GET			0x0d
#define OA_CMD_GPS_GET			0x0e
#define OA_CMD_GPS_CACHE_GET		0x0f
#define OA_CMD_START_EXPOSURE						0x10
#define OA_CMD_START_STREAMING          0x11
#define OA_CMD_STOP_STREAMING						0x12
#define OA_CMD_ABORT_EXPOSURE						0x13

#define OA_CALLBACK_NEW_FRAME           0x01

#define OA_CAM_BUFFERS                  8


#endif	/* OPENASTRO_CONTROLLER_H */
