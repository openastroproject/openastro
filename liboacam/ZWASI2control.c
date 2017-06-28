/*****************************************************************************
 *
 * ZWASI2control.c -- control functions for ZW ASI cameras, APIv2
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

#include <oa_common.h>
#include <openastro/camera.h>

#include <ASICamera2.h>
#include <pthread.h>

#include "oacamprivate.h"
#include "ZWASI.h"
#include "ZWASIoacam.h"
#include "ZWASI2oacam.h"
#include "ZWASIstate.h"


int
oaZWASI2CameraReadControl ( oaCamera* camera, int control,
    oaControlValue* val )
{
  ZWASI_STATE*	cameraInfo = camera->_private;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "%s: ( %d )\n", __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_BRIGHTNESS:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentBrightness;
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentBlueBalance;
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentRedBalance;
      break;

    case OA_CAM_CTRL_GAMMA:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentGamma;
      break;

    case OA_CAM_CTRL_GAIN:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentGain;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentAbsoluteExposure;
      break;

    case OA_CAM_CTRL_USBTRAFFIC:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentUSBTraffic;
      break;

    case OA_CAM_CTRL_OVERCLOCK:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentOverclock;
      break;

    case OA_CAM_CTRL_HIGHSPEED:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->currentHighSpeed;
      break;

    case OA_CAM_CTRL_BINNING:
      val->valueType = OA_CTRL_TYPE_DISCRETE;
      val->discrete = cameraInfo->binMode;
      break;

    case OA_CAM_CTRL_HFLIP:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->currentHFlip;
      break;

    case OA_CAM_CTRL_VFLIP:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->currentVFlip;
      break;

    case OA_CAM_CTRL_BIT_DEPTH:
      val->valueType = OA_CTRL_TYPE_DISCRETE;
      val->discrete = cameraInfo->currentBitDepth;
      break;

    case OA_CAM_CTRL_COLOUR_MODE:
      val->valueType = OA_CTRL_TYPE_DISCRETE;
      if ( ASI_IMG_RAW16 == cameraInfo->videoCurrent || ASI_IMG_RAW8 ==
          cameraInfo->videoCurrent ) {
        val->discrete = OA_COLOUR_MODE_RAW;
      }
      val->discrete = OA_COLOUR_MODE_NONRAW;
      break;

    case OA_CAM_CTRL_TEMPERATURE:
    {
      ASI_BOOL dummy;
      long temp;

      val->valueType = OA_CTRL_TYPE_READONLY;
      ASIGetControlValue ( cameraInfo->cameraId, ASI_TEMPERATURE, &temp,
          &dummy );
      val->readonly = temp;
      break;
    }

    case OA_CAM_CTRL_AUTO_GAIN:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoGain;
      break;

    case OA_CAM_CTRL_AUTO_GAMMA:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoGamma;
      break;

    case OA_CAM_CTRL_AUTO_BRIGHTNESS:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoBrightness;
      break;

    case OA_CAM_CTRL_AUTO_EXPOSURE:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoExposure;
      break;

    case OA_CAM_CTRL_AUTO_RED_BALANCE:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoRedBalance;
      break;

    case OA_CAM_CTRL_AUTO_BLUE_BALANCE:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoBlueBalance;
      break;

    case OA_CAM_CTRL_AUTO_USBTRAFFIC:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoUSBTraffic;
      break;

    case OA_CAM_CTRL_AUTO_OVERCLOCK:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoOverclock;
      break;

    case OA_CAM_CTRL_DROPPED:
    {
      int drops;
      ASIGetDroppedFrames ( cameraInfo->cameraId, &drops );
      val->valueType = OA_CTRL_TYPE_READONLY;
      val->readonly = drops;
      break;
    }
    default:
      fprintf ( stderr,
          "Unrecognised control %d in %s\n", control, __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}
