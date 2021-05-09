/*****************************************************************************
 *
 * IIDCcontrol.c -- control functions for IIDC cameras
 *
 * Copyright 2014,2015,2017,2018,2019,2021
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

#include <oa_common.h>

#if HAVE_LIBDC1394

#include <pthread.h>
#include <openastro/camera.h>

#include "oacamprivate.h"
#include "IIDCoacam.h"
#include "IIDCstate.h"


int
oaIIDCCameraTestControl ( oaCamera* camera, int control, oaControlValue* val )
{
  uint32_t	val_u32;
  int64_t	val_s64;
  COMMON_INFO*	commonInfo = camera->_common;

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->OA_CAM_CTRL_TYPE( control ) != val->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  switch ( control ) {

    case OA_CAM_CTRL_BRIGHTNESS:
    case OA_CAM_CTRL_CONTRAST:
    case OA_CAM_CTRL_SATURATION:
    case OA_CAM_CTRL_HUE:
    case OA_CAM_CTRL_SHARPNESS:
    case OA_CAM_CTRL_GAMMA:
    case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
    case OA_CAM_CTRL_GAIN:
      // This should be unsigned 32-bit
      val_s64 = val->int64;
      if ( val_s64 < 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      val_u32 = val_s64 & 0xffffffff;
      if ( val_u32 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val_u32 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val_u32 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val_s64 = val->int64;
      if ( val_s64 <= 0 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
    case OA_CAM_CTRL_AUTO_WHITE_BALANCE_TEMP:
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ):
      // These just need to be boolean and we've already checked that
      return OA_ERR_NONE;
      break;

			/*
			 * This is not yet supported
			 *
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
      if ( val->int32 != OA_EXPOSURE_AUTO && val->int32 !=
          OA_EXPOSURE_MANUAL ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;     
			 */

    case OA_CAM_CTRL_BINNING:
      return -OA_ERR_INVALID_CONTROL;
      break;

    default:
      // If we reach here it's because we don't recognise the control
      oaLogError ( OA_LOG_CAMERA, "%s: Unrecognised control %d", __func__,
					control, __func__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  // And if we reach here it's because the value wasn't valid
  return -OA_ERR_OUT_OF_RANGE;
}


const char*
oaIIDCCameraGetMenuString ( oaCamera* camera, int control, int index )
{
  if ( OA_CAM_CTRL_TRIGGER_POLARITY == control ||
      OA_CAM_CTRL_STROBE_POLARITY == control ) {

    switch ( index ) {
      case 0:
        return "Active on falling edge";
        break;
      case 1:
        return "Active on rising edge";
        break;
    }
    return "Invalid index";

  }

  if ( OA_CAM_CTRL_TRIGGER_MODE == control ) {
    switch ( index ) {
      case 0:
        return "External Trigger";
        break;
      case 1:
        return "Bulb Shutter Trigger";
        break;
      case 2:
        return "Pulse Count Trigger";
        break;
      case 3:
        return "Skip Frames Trigger";
        break;
      case 4:
        return "Multiple Preset Trigger";
        break;
      case 5:
        return "Multiple Pulse Width Trigger";
        break;
    }
    return "Invalid index";
  }

  oaLogError ( OA_LOG_CAMERA, "%s: control not implemented", __func__ );
  return "";
}

#endif /* HAVE_LIBDC1394 */
