/*****************************************************************************
 *
 * V4L2getState.c -- state querying for V4L2 cameras
 *
 * Copyright 2013,2014,2015,2017,2018,2021
 *    James Fidell (james@openastroproject.org)
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

#if HAVE_LIBV4L2

#include <openastro/camera.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "V4L2.h"
#include "V4L2oacam.h"
#include "V4L2ioctl.h"
#include "V4L2state.h"


const char* V4L2ControlNames[] = {
	"undefined",
	"V4L2_CID_EXPOSURE_AUTO",
	"V4L2_CID_EXPOSURE_ABSOLUTE",
	"V4L2_CID_EXPOSURE_AUTO_PRIORITY",
	"V4L2_CID_PAN_RELATIVE",
	"V4L2_CID_TILT_RELATIVE",
	"V4L2_CID_PAN_RESET",
	"V4L2_CID_TILT_RESET",
	"V4L2_CID_PAN_ABSOLUTE",
	"V4L2_CID_TILT_ABSOLUTE",
	"V4L2_CID_FOCUS_ABSOLUTE",
	"V4L2_CID_FOCUS_RELATIVE",
	"V4L2_CID_FOCUS_AUTO",
	"V4L2_CID_ZOOM_ABSOLUTE",
	"V4L2_CID_ZOOM_RELATIVE",
	"V4L2_CID_ZOOM_CONTINUOUS",
	"V4L2_CID_PRIVACY",
	"V4L2_CID_IRIS_ABSOLUTE",
	"V4L2_CID_IRIS_RELATIVE",
	"V4L2_CID_AUTO_EXPOSURE_BIAS",
	"V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE",
	"V4L2_CID_WIDE_DYNAMIC_RANGE",
	"V4L2_CID_IMAGE_STABILIZATION",
	"V4L2_CID_ISO_SENSITIVITY",
	"V4L2_CID_ISO_SENSITIVITY_AUTO",
	"V4L2_CID_EXPOSURE_METERING",
	"V4L2_CID_SCENE_MODE",
	"V4L2_CID_3A_LOCK",
	"V4L2_CID_AUTO_FOCUS_START",
	"V4L2_CID_AUTO_FOCUS_STOP",
	"V4L2_CID_AUTO_FOCUS_STATUS",
	"V4L2_CID_AUTO_FOCUS_RANGE",
	"V4L2_CID_PAN_SPEED",
	"V4L2_CID_TILT_SPEED"
};


int
oaV4L2CameraGetControlRange ( oaCamera* camera, int control, int64_t* min,
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


const FRAMESIZES*
oaV4L2CameraGetFrameSizes ( oaCamera* camera )
{
  V4L2_STATE*	cameraInfo = camera->_private;

  return &cameraInfo->frameSizes[1];
}


const FRAMERATES*
oaV4L2CameraGetFrameRates ( oaCamera* camera, int resX, int resY )
{
  V4L2_STATE*	cameraInfo = camera->_private;
  struct	v4l2_frmivalenum fint;
  int		k, numRates = 0;

  k = 0;
  do {
    OA_CLEAR( fint );
    fint.index = k;
    fint.pixel_format = cameraInfo->currentV4L2Format;
    fint.width = resX;
    fint.height = resY;
    if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_ENUM_FRAMEINTERVALS,
        &fint )) { 
      if ( EINVAL != errno) { 
        perror("VIDIOC_ENUM_FRAMEINTERVALS");
      }
      break;
    } 
    if ( V4L2_FRMIVAL_TYPE_DISCRETE == fint.type ) { 
      if (!( cameraInfo->frameRates.rates = realloc (
          cameraInfo->frameRates.rates, ( numRates + 1 ) *
          sizeof ( FRAMERATE )))) {
        fprintf ( stderr, "%s: realloc failed\n", __func__ );
        return 0;
      }
      cameraInfo->frameRates.rates[numRates].numerator =
          fint.discrete.numerator;
      cameraInfo->frameRates.rates[numRates].denominator =
          fint.discrete.denominator;
      numRates++;
    } 
    k++;
  } while ( 1 );

  cameraInfo->frameRates.numRates = numRates;
  return &cameraInfo->frameRates;
}


int
oaV4L2CameraGetFramePixelFormat ( oaCamera* camera )
{
  V4L2_STATE*		cameraInfo = camera->_private;

  // the driver lies for the colour TIS DxK cameras, so we fix it here
  if ( cameraInfo->colourDxK ) {
    return OA_PIX_FMT_GBRG8;
  }

  return cameraInfo->currentFrameFormat;
}


int
oaV4L2CameraGetControlDiscreteSet ( oaCamera* camera, int control,
    int32_t* count, int64_t** values )
{
  V4L2_STATE*    cameraInfo = camera->_private;

  if ( control != OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) &&
      control != OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *count = cameraInfo->numAutoExposureItems;
  *values = cameraInfo->autoExposureMenuItems;
  return OA_ERR_NONE;
}

#endif /* HAVE_LIBV4L2 */
