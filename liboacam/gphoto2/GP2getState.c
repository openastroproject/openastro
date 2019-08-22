/*****************************************************************************
 *
 * GP2getState.c -- state querying for libgphoto2 cameras
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

#include <openastro/camera.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "GP2oacam.h"
#include "GP2state.h"
#include "GP2private.h"


int
oaGP2CameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
    int64_t* max, int64_t* step, int64_t* def )
{
  COMMON_INFO*	commonInfo = camera->_common;

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL; 
  }

  *min = commonInfo->OA_CAM_CTRL_MIN( control );
  *max = commonInfo->OA_CAM_CTRL_MAX( control );
  *step = commonInfo->OA_CAM_CTRL_STEP( control );
  *def = commonInfo->OA_CAM_CTRL_DEF( control );
  return OA_ERR_NONE;
}


const char*
oaGP2CameraGetMenuString ( oaCamera* camera, int control, int index )
{
	GP2_STATE*		cameraInfo = camera->_private;
	CameraWidget*	widget;
	int						numOptions, i;
	const char**	options;
	const char***	poptions;

	switch ( control ) {
		case OA_CAM_CTRL_ISO:
			widget = cameraInfo->iso;
			numOptions = cameraInfo->numIsoOptions;
			options = cameraInfo->isoOptions;
			poptions = &cameraInfo->isoOptions;
			break;
		case OA_CAM_CTRL_WHITE_BALANCE:
			widget = cameraInfo->whiteBalance;
			numOptions = cameraInfo->numWBOptions;
			options = cameraInfo->whiteBalanceOptions;
			poptions = &cameraInfo->whiteBalanceOptions;
			break;
		case OA_CAM_CTRL_SHUTTER_SPEED:
			widget = cameraInfo->shutterSpeed;
			numOptions = cameraInfo->numShutterSpeedOptions;
			options = cameraInfo->shutterSpeedOptions;
			poptions = &cameraInfo->shutterSpeedOptions;
			break;
		case OA_CAM_CTRL_SHARPNESS:
			widget = cameraInfo->sharpening;
			numOptions = cameraInfo->numSharpeningOptions;
			options = cameraInfo->sharpeningOptions;
			poptions = &cameraInfo->sharpeningOptions;
			break;
		case OA_CAM_CTRL_FRAME_FORMAT:
			widget = cameraInfo->frameFormat;
			numOptions = cameraInfo->numFrameFormatOptions;
			options = cameraInfo->frameFormatOptions;
			poptions = &cameraInfo->frameFormatOptions;
			break;
		default:
			return "Invalid control";
			break;
	}

	if ( !widget || !numOptions ) {
		return "Invalid control";
	}

	if ( options ) {
		return options[ index ];
	}

	if (!( *poptions = calloc ( sizeof ( char* ), numOptions ))) {
		return "Memory allocation failed";
	}

	options = *poptions;
	for ( i = 0; i < numOptions; i++ ) {
		if ( p_gp_widget_get_choice ( widget, i, &options[i] ) != GP_OK ) {
			options[i] = "unknown";
		}
	}

	return options[ index ];
}

int
oaGP2CameraGetFramePixelFormat ( oaCamera* camera )
{
  GP2_STATE*	cameraInfo = camera->_private;

	if ( cameraInfo->currentFormatOption == cameraInfo->jpegOption ) {
		return OA_PIX_FMT_JPEG8;
	}

	if ( cameraInfo->manufacturer == CAMERA_MANUF_CANON ) {
fprintf ( stderr, "Returning CR2 format which may be CRW or CR3\n" );
		return OA_PIX_FMT_CANON_CR2;
	}

	if ( cameraInfo->manufacturer == CAMERA_MANUF_NIKON ) {
		return OA_PIX_FMT_NIKON_NEF;
	}

	return 0;
}


int
oaGP2CameraGetControlDiscreteSet ( oaCamera* camera, int control,
    int32_t* count, int64_t** values )
{
  GP2_STATE*    cameraInfo = camera->_private;

  if ( control != OA_CAM_CTRL_FRAME_FORMAT ) {
    return -OA_ERR_INVALID_CONTROL;
  }

	*count = cameraInfo->numFormatMenuValues;
	*values = cameraInfo->formatMenuValues;
  return OA_ERR_NONE;
}
