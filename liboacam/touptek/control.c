/*****************************************************************************
 *
 * control.c -- control functions for Touptek-based cameras
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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

#include <pthread.h>
#include <openastro/camera.h>

#include "touptek-conf.h"
#include "oacamprivate.h"
#include "touptekoacam.h"
#include "touptekstate.h"


int
TT_FUNC( oa, CameraTestControl )( oaCamera* camera, int control,
    oaControlValue* valp )
{
  int32_t		val_s32;
  TOUPTEK_STATE*	cameraInfo = camera->_private;

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->OA_CAM_CTRL_TYPE( control ) != valp->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  switch ( control ) {

    case OA_CAM_CTRL_BRIGHTNESS:
      val_s32 = valp->int32;
      if ( val_s32 < TT_DEFINE( BRIGHTNESS_MIN ) ||
          val_s32 > TT_DEFINE( BRIGHTNESS_MAX )) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_CONTRAST:
      val_s32 = valp->int32;
      if ( val_s32 < TT_DEFINE( CONTRAST_MIN ) ||
					val_s32 > TT_DEFINE( CONTRAST_MAX )) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_GAMMA:
      val_s32 = valp->int32;
      if ( val_s32 >= TT_DEFINE( GAMMA_MIN ) ||
					val_s32 <= TT_DEFINE( GAMMA_MAX )) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_HFLIP:
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_VFLIP:
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
      val_s32 = valp->boolean;
      if ( val_s32 == 0 || val_s32 == 1 ) {
        return OA_ERR_NONE;
      }
      return -OA_ERR_OUT_OF_RANGE;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val_s32 = valp->int32;
      if ( val_s32 < cameraInfo->exposureMin ||
          val_s32 > cameraInfo->exposureMax ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;

    case OA_CAM_CTRL_GAIN:
      val_s32 = valp->int32;
      if ( val_s32 < cameraInfo->gainMin || val_s32 > cameraInfo->gainMax ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;

    case OA_CAM_CTRL_SPEED:
      val_s32 = valp->int32;
      if ( val_s32 < 0 || val_s32 > cameraInfo->speedMax ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;

    case OA_CAM_CTRL_HUE:
      val_s32 = valp->int32;
      if ( val_s32 < TT_DEFINE( HUE_MIN ) || val_s32 > TT_DEFINE( HUE_MAX )) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;

    case OA_CAM_CTRL_SATURATION:
      val_s32 = valp->int32;
      if ( val_s32 < TT_DEFINE( SATURATION_MIN ) ||
          val_s32 > TT_DEFINE( SATURATION_MAX )) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;

    case OA_CAM_CTRL_RED_BALANCE:
    case OA_CAM_CTRL_BLUE_BALANCE:
    case OA_CAM_CTRL_GREEN_BALANCE:
      val_s32 = valp->int32;
      if ( val_s32 < TT_DEFINE( WBGAIN_MIN ) ||
					val_s32 > TT_DEFINE( WBGAIN_MAX )) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;

    case OA_CAM_CTRL_BINNING:
      val_s32 = valp->discrete;
      if ( val_s32 < 0 || val_s32 > OA_MAX_BINNING ||
          cameraInfo->frameSizes[ val_s32 ].numSizes < 1 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;
  }

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );
  return -OA_ERR_INVALID_CONTROL;
}


const char*
TT_FUNC( oa, CameraGetMenuString )( oaCamera* camera, int control, int index )
{ 
	switch ( control ) {
		case OA_CAM_CTRL_LED_STATE:
			switch ( index ) { 
				case 1:
					return "On";
					break;
				case 2:
					return "Flash";
					break;
				case 3:
					return "Off";
					break;
			}
			return "Invalid index";
			break;

		case OA_CAM_CTRL_CONVERSION_GAIN:
			switch ( index ) { 
				case 0:
					return "LCG";
					break;
				case 1:
					return "HCG";
					break;
				case 2:
					return "HDR";
					break;
			}
			return "Invalid index";
			break;
	}

  fprintf ( stderr, "%s: control not implemented\n", __FUNCTION__ );
  return "";
}
