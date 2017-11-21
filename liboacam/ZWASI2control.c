/*****************************************************************************
 *
 * ZWASI2control.c -- control functions for ZW ASI cameras, APIv2
 *
 * Copyright 2015,2017 James Fidell (james@openastroproject.org)
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

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoGain;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA ):
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoGamma;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ):
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoBrightness;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoExposure;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_RED_BALANCE ):
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoRedBalance;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLUE_BALANCE ):
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->autoBlueBalance;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_USBTRAFFIC ):
      val->boolean = cameraInfo->autoUSBTraffic;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_OVERCLOCK ):
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


int
oaZWASI2CameraTestControl ( oaCamera* camera, int control,
    oaControlValue* val )
{
  int32_t	val_s32;
  ZWASI_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "%s: ( %d, ? )\n", __FUNCTION__, control );

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->OA_CAM_CTRL_TYPE( control ) != val->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  switch ( control ) {
    case OA_CAM_CTRL_BRIGHTNESS:
    case OA_CAM_CTRL_BLUE_BALANCE:
    case OA_CAM_CTRL_RED_BALANCE:
    case OA_CAM_CTRL_GAMMA:
    case OA_CAM_CTRL_GAIN:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    case OA_CAM_CTRL_USBTRAFFIC:
    case OA_CAM_CTRL_OVERCLOCK:
      val_s32 = val->int32;
      if ( val_s32 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val_s32 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val_s32 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_BINNING:
    {
      int i = 0;

      val_s32 = val->discrete;
      while ( cameraInfo->binModes[i] && i < 16 ) {
        if ( val_s32 == cameraInfo->binModes[i] ) {
          return OA_ERR_NONE;
        }
        i++;
      }
      break;
    }
    case OA_CAM_CTRL_BIT_DEPTH:
      val_s32 = val->discrete;
      // This may be a bit of an ugly assumption, but I think it
      // should hold for the time being
      if ( camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BIT_DEPTH )) {
        if ( 16 == val_s32 || 12 == val_s32 || 8 == val_s32 ) {
          return OA_ERR_NONE;
        }
      }
      if ( 8 == val_s32 ) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_COLOUR_MODE:
      val_s32 = val->discrete;
      if (( camera->features.rawMode && OA_COLOUR_MODE_RAW == val_s32 ) ||
          ( camera->features.demosaicMode && OA_COLOUR_MODE_NONRAW ==
          val_s32 )) {
        return OA_ERR_NONE;
      }
      break;

    // This lot are all boolean, so we'll take any value
    case OA_CAM_CTRL_HIGHSPEED:
    case OA_CAM_CTRL_HFLIP:
    case OA_CAM_CTRL_VFLIP:
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_RED_BALANCE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLUE_BALANCE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_USBTRAFFIC ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_OVERCLOCK ):
      return OA_ERR_NONE;
      break;

    default:
      // If we reach here it's because we don't recognise the control
      fprintf ( stderr, "Unrecognised control %d in oaZWASICameraSetControl\n",
          control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  // And if we reach here it's because the value wasn't valid
  return -OA_ERR_OUT_OF_RANGE;
}
