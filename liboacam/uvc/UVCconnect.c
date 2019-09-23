/*****************************************************************************
 *
 * UVCconnect.c -- Initialise UVC cameras
 *
 * Copyright 2014,2015,2016,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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

#if HAVE_LIBUVC

#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <libuvc/libuvc.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "UVC.h"
#include "UVCoacam.h"
#include "UVCstate.h"
#include "UVCprivate.h"
#include "UVCExtnUnits.h"


// FIX ME -- move this somewhere more sensible
// This is required because the bitmap of known controls in the
// processing units does not match the id of the control.  Gah!!!

struct puCtrl UVCControlData[] = {
  {
    .uvcControl		= UVC_PU_BRIGHTNESS_CONTROL,
    .oaControl		= OA_CAM_CTRL_BRIGHTNESS,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_CONTRAST_CONTROL,
    .oaControl		= OA_CAM_CTRL_CONTRAST,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_HUE_CONTROL,
    .oaControl		= OA_CAM_CTRL_HUE,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_SATURATION_CONTROL,
    .oaControl		= OA_CAM_CTRL_SATURATION,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_SHARPNESS_CONTROL,
    .oaControl		= OA_CAM_CTRL_SHARPNESS,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_GAMMA_CONTROL,
    .oaControl		= OA_CAM_CTRL_GAMMA,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL,
    .oaControl		= OA_CAM_CTRL_WHITE_BALANCE_TEMP,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL,
    .oaControl		= 0, // R&B combined.  Handle this separately
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 4
  }, {
    .uvcControl		= UVC_PU_BACKLIGHT_COMPENSATION_CONTROL,
    .oaControl		= OA_CAM_CTRL_BACKLIGHT_COMPENSATION,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_GAIN_CONTROL,
    .oaControl		= OA_CAM_CTRL_GAIN,
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_POWER_LINE_FREQUENCY_CONTROL,
    .oaControl		= OA_CAM_CTRL_POWER_LINE_FREQ,
    .oaControlType	= OA_CTRL_TYPE_MENU,
    .size		= 1
  }, {
    .uvcControl		= UVC_PU_HUE_AUTO_CONTROL,
    .oaControl		= OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ),
    .oaControlType	= OA_CTRL_TYPE_BOOLEAN,
    .size		= 1
  }, {
    .uvcControl		= UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL,
    .oaControl		= OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE_TEMP ),
    .oaControlType	= OA_CTRL_TYPE_BOOLEAN,
    .size		= 1
  }, {
    .uvcControl		= UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL,
    .oaControl		= OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ),
    .oaControlType	= OA_CTRL_TYPE_BOOLEAN,
    .size		= 1
  }, {
    .uvcControl		= UVC_PU_DIGITAL_MULTIPLIER_CONTROL,
    .oaControl		= 0, // deprecated control
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL,
    .oaControl		= 0, // presumably also deprecated
    .oaControlType	= OA_CTRL_TYPE_INT32,
    .size		= 2
  }, {
    .uvcControl		= UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL,
    .oaControl		= 0, // only for analogue devices
    .oaControlType	= OA_CTRL_TYPE_READONLY,
    .size		= 1
  }, {
    .uvcControl		= UVC_PU_ANALOG_LOCK_STATUS_CONTROL,
    .oaControl		= 0, // only for analogue devices
    .oaControlType	= OA_CTRL_TYPE_READONLY,
    .size		= 1
  }, {
    .uvcControl		= UVC_PU_CONTRAST_AUTO_CONTROL,
    .oaControl		= OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ),
    .oaControlType	= OA_CTRL_TYPE_BOOLEAN,
    .size		= 1
  }
};

unsigned int numPUControls = sizeof ( UVCControlData ) /
		sizeof ( struct puCtrl );

static void _UVCInitFunctionPointers ( oaCamera* );
static void _getUVCControlValues ( oaCamera*, uvc_device_handle_t*,
    uint8_t, int );


/**
 * Initialise a given camera device
 */

oaCamera*
oaUVCInitCamera ( oaCameraDevice* device )
{
  oaCamera*				camera;
  unsigned int                          matched, haveCamera, haveUnit;
  unsigned int                          numPUControls, oaControl, i, j;
  unsigned int				numUVCDevices, mask;
  int					k, deviceAddr, deviceBus;
  uint64_t				flags;
  uvc_device_t**                        devlist;
  uvc_device_t*                         uvcDevice;
  uvc_device_descriptor_t*              desc;
  uvc_device_handle_t*			uvcHandle;
  const uvc_input_terminal_t*		inputTerminals;
  const uvc_input_terminal_t*		cameraTerminal;
  enum uvc_ct_ctrl_selector		control;
  enum uvc_pu_ctrl_selector		uvcControl;
  const uvc_processing_unit_t*		processingUnits;
  const uvc_processing_unit_t*		unit;
  const uvc_format_desc_t*		formatDescs;
  const uvc_format_desc_t*		format;
  const uvc_frame_desc_t*		frame;
  const uvc_extension_unit_t*		extensionUnits;
  const uvc_extension_unit_t*		extn;
  int					allFramesHaveFixedRates;
  DEVICE_INFO*				devInfo;
  UVC_STATE*				cameraInfo;
  COMMON_INFO*				commonInfo;
	void*							tmpPtr;

	if ( _oaInitCameraStructs ( &camera, ( void* ) &cameraInfo,
			sizeof ( UVC_STATE ), &commonInfo ) != OA_ERR_NONE ) {
		return 0;
	}

  _UVCInitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  cameraInfo->index = -1;
  devInfo = device->_private;

  // FIX ME -- This is a bit ugly.  Much of it is repeated from the
  // getCameras function.  I should join the two together somehow.

  if ( p_uvc_init ( &cameraInfo->uvcContext, 0 ) != UVC_SUCCESS ) {
    fprintf ( stderr, "uvc_init failed\n" );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if ( p_uvc_get_device_list ( cameraInfo->uvcContext, &devlist ) !=
      UVC_SUCCESS ) {
    fprintf ( stderr, "uvc_get_device_list failed\n" );
    FREE_DATA_STRUCTS;
    return 0;
  }
  numUVCDevices = 0;
  while ( devlist[numUVCDevices] ) { numUVCDevices++; }
  if ( numUVCDevices < 1 ) {
    p_uvc_free_device_list ( devlist, 1 );
    p_uvc_exit ( cameraInfo->uvcContext );
    if ( numUVCDevices ) {
      fprintf ( stderr, "Can't see any UVC devices now (list returns -1)\n" );
      FREE_DATA_STRUCTS;
      return 0;
    }
    fprintf ( stderr, "Can't see any UVC devices now\n" );
    FREE_DATA_STRUCTS;
    return 0;
  }

  matched = 0;
  deviceAddr = devInfo->devIndex & 0xff;
  deviceBus = devInfo->devIndex >> 8;

  for ( i = 0; i < numUVCDevices && !matched; i++ ) {
    uvcDevice = devlist[i];
    if ( p_uvc_get_device_descriptor ( uvcDevice, &desc )) {
      p_uvc_free_device_list ( devlist, 1 );
      p_uvc_exit ( cameraInfo->uvcContext );
      fprintf ( stderr, "get UVC device descriptor failed\n" );
      FREE_DATA_STRUCTS;
      return 0;
    }
    if ( p_uvc_get_bus_number ( uvcDevice ) == deviceBus &&
        p_uvc_get_device_address ( uvcDevice ) == deviceAddr ) {
      // this looks like the one!
      if ( p_uvc_open ( uvcDevice, &uvcHandle ) != UVC_SUCCESS ) {
        p_uvc_free_device_list ( devlist, 1 );
        p_uvc_exit ( cameraInfo->uvcContext );
        fprintf ( stderr, "open of UVC device failed\n" );
        FREE_DATA_STRUCTS;
        return 0;
      }
      matched = 1;
    }
  }
  p_uvc_free_device_list ( devlist, 1 );

  if ( !matched ) {
    fprintf ( stderr, "No matching UVC device found!\n" );
    p_uvc_exit ( cameraInfo->uvcContext );
    FREE_DATA_STRUCTS;
    return 0;
  }
  if ( !uvcHandle ) {
    fprintf ( stderr, "Unable to open UVC device!\n" );
    p_uvc_exit ( cameraInfo->uvcContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  if (!( inputTerminals = p_uvc_get_input_terminals ( uvcHandle ))) {
    fprintf ( stderr, "No input terminals found!\n" );
    p_uvc_exit ( cameraInfo->uvcContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  cameraTerminal = inputTerminals;
  haveCamera = 0;
  do {
    // fprintf ( stderr, "terminal id: %d, type %d, fmax %d, fmin %d, f %d, flags %llx\n", cameraTerminal->bTerminalID, cameraTerminal->wTerminalType, cameraTerminal->wObjectiveFocalLengthMin, cameraTerminal->wObjectiveFocalLengthMax, cameraTerminal->wOcularFocalLength, cameraTerminal->bmControls );
    if ( UVC_ITT_CAMERA == cameraTerminal->wTerminalType ) {
      haveCamera = 1;
    } else {
      cameraTerminal = cameraTerminal->next;
    }
  } while ( !haveCamera && cameraTerminal );

  if ( !haveCamera ) {
    p_uvc_close ( uvcHandle );
    fprintf ( stderr, "Device doesn't actually look like a camera\n" );
    p_uvc_exit ( cameraInfo->uvcContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  // These are so we can get the auto focus stuff right later.
  int   autoFocusType = 0;
  uint8_t autoFocusMax, autoFocusMin, autoFocusDef, autoFocusStep;

  control = 1;
  flags = cameraTerminal->bmControls;
  while ( flags ) {
    if ( flags & 1 ) {

      // FIX ME -- need to know what the units are for all of these

      switch ( control ) {

        case UVC_CT_SCANNING_MODE_CONTROL:
        {
          uint32_t val_u32;

          if ( p_uvc_get_exposure_abs ( uvcHandle, &val_u32, UVC_GET_DEF ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get default for scanning mode\n" );
          }

          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_INTERLACE_ENABLE ) =
              OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_INTERLACE_ENABLE ) = 0;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_INTERLACE_ENABLE ) = 1;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_INTERLACE_ENABLE ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_INTERLACE_ENABLE ) =
            val_u32 ? 1 : 0;
          break;
        }
        case UVC_CT_AE_MODE_CONTROL:
        {
          // UVC auto exposure mode is messy -- a bitfield of:
          // 1 = manual, 2 = auto, 4 = shutter priority, 8 = aperture priority
          // fortunately the exponents of the bit values correspond to the
          // menu values we're using

          uint8_t uvcdef, modes, minSet;

          if ( p_uvc_get_ae_mode ( uvcHandle, &modes, UVC_GET_RES ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get modes for autoexp setting\n" );
          }

          cameraInfo->numAutoExposureItems = 0;
					minSet = 0;
          for ( k = 0, mask = 1; k < 4; k++, mask <<= 1 ) {
            if ( modes & mask ) {
              cameraInfo->autoExposureMenuItems[
                  cameraInfo->numAutoExposureItems++ ] = mask;
							if ( !minSet ) {
								commonInfo->OA_CAM_CTRL_AUTO_MIN(
										OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = mask;
								minSet = 1;
							}
							commonInfo->OA_CAM_CTRL_AUTO_MAX(
									OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) = mask;
            }
          }

          camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              OA_CTRL_TYPE_DISC_MENU;
          if ( p_uvc_get_ae_mode ( uvcHandle, &uvcdef, UVC_GET_DEF ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get default for autoexp setting\n" );
          }
          commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              uvcdef;
          break;
        }
        case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
        {
          uint32_t val_u32;
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              OA_CTRL_TYPE_INT64;
          if ( p_uvc_get_exposure_abs ( uvcHandle, &val_u32, UVC_GET_MIN ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get min value for exposure\n" );
          }
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
							val_u32;
          if ( p_uvc_get_exposure_abs ( uvcHandle, &val_u32, UVC_GET_MAX ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get max value for exposure\n" );
          }
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              val_u32;
          if ( p_uvc_get_exposure_abs ( uvcHandle, &val_u32, UVC_GET_RES ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get resolution for exposure\n" );
          }
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              val_u32;
          if ( p_uvc_get_exposure_abs ( uvcHandle, &val_u32, UVC_GET_DEF ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get default for exposure\n" );
          }
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) =
              val_u32;

          // now convert the values from 100usec to usec.

          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) *= 100;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) *= 100;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) *= 100;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) *= 100;

          break;
        }
        case UVC_CT_ZOOM_ABSOLUTE_CONTROL:
        {
          uint16_t val_u16;
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_ZOOM_ABSOLUTE ) =
              OA_CTRL_TYPE_INT32;
          if ( p_uvc_get_zoom_abs ( uvcHandle, &val_u16, UVC_GET_MIN ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get min value for zoom abs\n" );
          }
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_ZOOM_ABSOLUTE ) = val_u16;
          if ( p_uvc_get_zoom_abs ( uvcHandle, &val_u16, UVC_GET_MAX ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get max value for zoom abs\n" );
          }
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_ZOOM_ABSOLUTE ) =
              val_u16;
          if ( p_uvc_get_zoom_abs ( uvcHandle, &val_u16, UVC_GET_RES ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get resolution for zoom abs\n" );
          }
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_ZOOM_ABSOLUTE ) =
              val_u16;
          if ( p_uvc_get_zoom_abs ( uvcHandle, &val_u16, UVC_GET_DEF ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get default for zoom abs\n" );
          }
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_ZOOM_ABSOLUTE ) =
              val_u16;
          break;
        }

        case UVC_CT_AE_PRIORITY_CONTROL:
          // The values specified here are from the UVC 1.1 spec.
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_AUTO_EXPOSURE_PRIORITY ) =
              OA_CTRL_TYPE_MENU;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_AUTO_EXPOSURE_PRIORITY ) = 0;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_AUTO_EXPOSURE_PRIORITY ) = 1;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_AUTO_EXPOSURE_PRIORITY ) =
              1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_AUTO_EXPOSURE_PRIORITY ) = 0;
          break;

        case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
        {
          uint16_t val_u16;
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
              OA_CTRL_TYPE_INT32;
          if ( p_uvc_get_focus_abs ( uvcHandle, &val_u16, UVC_GET_MIN ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get min value for AE setting\n" );
          }
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_FOCUS_ABSOLUTE ) = val_u16;
          if ( p_uvc_get_focus_abs ( uvcHandle, &val_u16, UVC_GET_MAX ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get max value for AE setting\n" );
          }
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
              val_u16;
          if ( p_uvc_get_focus_abs ( uvcHandle, &val_u16, UVC_GET_RES ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get resolution for AE setting\n" );
          }
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
              val_u16;
          if ( p_uvc_get_focus_abs ( uvcHandle, &val_u16, UVC_GET_DEF ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get default for AE setting\n" );
          }
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
              val_u16;
          break;
        }

        case UVC_CT_FOCUS_AUTO_CONTROL:
        {
          // This might allow an autofocus option for "focus absolute",
          // "focus relative" or "focus simple", so we need to remember
          // the settings and handle this later when we know which focus
          // options exist

          autoFocusType = OA_CTRL_TYPE_BOOLEAN;
          if ( p_uvc_get_focus_auto ( uvcHandle, &autoFocusMin, UVC_GET_MIN )
              != UVC_SUCCESS ) { 
            fprintf ( stderr, "failed to get min value for autofocus\n" );
          }
          if ( p_uvc_get_focus_auto ( uvcHandle, &autoFocusMax, UVC_GET_MAX )
              != UVC_SUCCESS ) { 
            fprintf ( stderr, "failed to get max value for autofocus\n" );
          }
          if ( p_uvc_get_focus_auto ( uvcHandle, &autoFocusStep, UVC_GET_RES )
              != UVC_SUCCESS ) { 
            fprintf ( stderr, "failed to get resolution for autofocus\n" );
          }
          if ( p_uvc_get_focus_auto ( uvcHandle, &autoFocusDef, UVC_GET_DEF )
              != UVC_SUCCESS ) { 
            fprintf ( stderr, "failed to get default for autofocus\n" );
          }
          break;
        }

        case UVC_CT_IRIS_ABSOLUTE_CONTROL:
        {
          uint16_t val_u16;
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_IRIS_ABSOLUTE ) =
              OA_CTRL_TYPE_INT32;
          if ( p_uvc_get_iris_abs ( uvcHandle, &val_u16, UVC_GET_MIN ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get min value for iris abs\n" );
          }
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_IRIS_ABSOLUTE ) = val_u16;
          if ( p_uvc_get_iris_abs ( uvcHandle, &val_u16, UVC_GET_MAX ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get max value for iris abs\n" );
          }
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_IRIS_ABSOLUTE ) =
              val_u16;
          if ( p_uvc_get_iris_abs ( uvcHandle, &val_u16, UVC_GET_RES ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get resolution for iris abs\n" );
          }
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_IRIS_ABSOLUTE ) =
              val_u16;
          if ( p_uvc_get_iris_abs ( uvcHandle, &val_u16, UVC_GET_DEF ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get default for iris abs\n" );
          }
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_IRIS_ABSOLUTE ) =
              val_u16;
          break;
        }

        case UVC_CT_PANTILT_ABSOLUTE_CONTROL:
        {
          int32_t defPan, defTilt;
          // we have two controls combined here
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_PAN_ABSOLUTE ) =
              camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TILT_ABSOLUTE ) =
              OA_CTRL_TYPE_INT32; 

          // units are arcseconds, and the spec defines the min and max
          // values as -180*3600 to 180*3600

          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_PAN_ABSOLUTE ) =
              commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_TILT_ABSOLUTE ) =
              -180 * 3600;

          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_PAN_ABSOLUTE ) =
              commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_TILT_ABSOLUTE ) =
              180 * 3600;

          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_PAN_ABSOLUTE ) =
              commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_TILT_ABSOLUTE ) = 1;

          if ( p_uvc_get_pantilt_abs ( uvcHandle, &defPan, &defTilt,
              UVC_GET_DEF ) != UVC_SUCCESS ) { 
            fprintf ( stderr, "failed to get default for pan/tilt default\n" );
          }
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PAN_ABSOLUTE ) = defPan;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_TILT_ABSOLUTE ) = defTilt;
          break;
        }

        case UVC_CT_ROLL_ABSOLUTE_CONTROL:
        {
          int16_t val_s16;

          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_ROLL_ABSOLUTE ) =
              OA_CTRL_TYPE_INT32;

          // units are degrees, and the spec defines the min and max
          // values as -180 to 180

          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_ROLL_ABSOLUTE ) = -180;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_ROLL_ABSOLUTE ) = 180;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_ROLL_ABSOLUTE ) = 1;

          if ( p_uvc_get_roll_abs ( uvcHandle, &val_s16, UVC_GET_DEF ) !=
              UVC_SUCCESS ) { 
            fprintf ( stderr, "failed to get default for roll default\n" );
          }
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_ROLL_ABSOLUTE ) = val_s16;
          break;
        }

        case UVC_CT_PRIVACY_CONTROL:
        {
          uint8_t val_u8;

          if ( p_uvc_get_privacy ( uvcHandle, &val_u8, UVC_GET_DEF ) !=
              UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get default for privacy mode\n" );
          }

          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_PRIVACY_ENABLE ) =
              OA_CTRL_TYPE_BOOLEAN;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_PRIVACY_ENABLE ) = 0;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_PRIVACY_ENABLE ) = 1;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_PRIVACY_ENABLE ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_PRIVACY_ENABLE ) =
            val_u8 ? 1 : 0;
          break;
        }

        case UVC_CT_FOCUS_SIMPLE_CONTROL:
        {
          uint8_t val_u8;

          if ( p_uvc_get_focus_simple_range ( uvcHandle, &val_u8,
              UVC_GET_DEF ) != UVC_SUCCESS ) {
            fprintf ( stderr, "failed to get default for simple focus\n" );
          }

          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FOCUS_SIMPLE ) =
              OA_CTRL_TYPE_MENU;
          commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_FOCUS_SIMPLE ) = 0;
          commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_FOCUS_SIMPLE ) = 3;
          commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_FOCUS_SIMPLE ) = 1;
          commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_FOCUS_SIMPLE ) = val_u8;
          break;
        }

        case UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL:
          // FIX ME -- this needs a new control type to provide increment
          // and decrement functions

        case UVC_CT_FOCUS_RELATIVE_CONTROL:
          // FIX ME -- this needs to be split into two controls, one to set
          // the speed and a new type for in/stop/out

        case UVC_CT_IRIS_RELATIVE_CONTROL:
          // FIX ME -- needs a new control type (as for FOCUS), but for the
          // options open 1 step/default/close 1 step

        case UVC_CT_ZOOM_RELATIVE_CONTROL:
          // FIX ME -- really not sure about this one, specifically with
          // reference to the digital zoom mode.  How do we know if it
          // exists?
          // Perhaps this should be three controls -- zoom in/stop/zoom out,
          // digital zoom enabled and zoom speed

        case UVC_CT_PANTILT_RELATIVE_CONTROL:
        case UVC_CT_ROLL_RELATIVE_CONTROL:
          // FIX ME -- again, new controls as for focus relative

        case UVC_CT_DIGITAL_WINDOW_CONTROL:
          // FIX ME -- I'm really not sure I understand this one

        case UVC_CT_REGION_OF_INTEREST_CONTROL:
          // FIX ME -- I want to use this, but there's a lot of complexity
          // around how the auto controls are affected.  See the UVC1.5
          // spec, section 4.2.2.1.20
          fprintf ( stderr, "Unsupported UVC control %d\n", control );
          break;

        default:
          fprintf ( stderr, "unknown UVC control %d\n", control );
          break;
      }
    }
    flags >>= 1;
    control++;
  }

  // FIX ME -- what if we have auto focus, but none of the three focus
  // modes?
  if ( autoFocusType ) {
    if ( camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FOCUS_ABSOLUTE )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
          autoFocusType;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
          autoFocusMin;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
          autoFocusMax;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
          autoFocusStep;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_FOCUS_ABSOLUTE ) =
          autoFocusDef;
    }
    if ( camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FOCUS_RELATIVE )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_FOCUS_RELATIVE ) =
          autoFocusType;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_FOCUS_RELATIVE ) =
          autoFocusMin;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_FOCUS_RELATIVE ) =
          autoFocusMax;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_FOCUS_RELATIVE ) =
          autoFocusStep;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_FOCUS_RELATIVE ) =
          autoFocusDef;
    }
    if ( camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FOCUS_SIMPLE )) {
      camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_FOCUS_SIMPLE ) =
          autoFocusType;
      commonInfo->OA_CAM_CTRL_AUTO_MIN( OA_CAM_CTRL_FOCUS_SIMPLE ) =
          autoFocusMin;
      commonInfo->OA_CAM_CTRL_AUTO_MAX( OA_CAM_CTRL_FOCUS_SIMPLE ) =
          autoFocusMax;
      commonInfo->OA_CAM_CTRL_AUTO_STEP( OA_CAM_CTRL_FOCUS_SIMPLE ) =
          autoFocusStep;
      commonInfo->OA_CAM_CTRL_AUTO_DEF( OA_CAM_CTRL_FOCUS_SIMPLE ) =
          autoFocusDef;
    }
  }

  if (!( processingUnits = p_uvc_get_processing_units ( uvcHandle ))) {
    fprintf ( stderr, "No processing units found!\n" );
    p_uvc_exit ( cameraInfo->uvcContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  unit = processingUnits;
  haveUnit = 0;
  do {
    // FIX ME -- find out how this should actually work
    // if ( cameraTerminal->bTerminalID == unit->bSourceID ) {
      haveUnit = 1;
    // } else {
    //   unit = unit->next;
    // }
  } while ( !haveUnit && unit );

  if ( !haveUnit ) {
    fprintf ( stderr, "Can't find any processing units for the camera\n" );
    p_uvc_close ( uvcHandle );
    p_uvc_exit ( cameraInfo->uvcContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  flags = unit->bmControls;
  cameraInfo->haveComponentWhiteBalance = 0;
  numPUControls = sizeof ( UVCControlData ) / sizeof ( struct puCtrl );
  for ( i = 0; i < numPUControls; i++ ) {
    if ( flags & 1 ) {
      // FIX ME -- remove these two temp variables
      uvcControl = UVCControlData[ i ].uvcControl;
      oaControl = UVCControlData[ i ].oaControl;
      // if oaControl == 0 we don't support this yet, but white balance
      // component is a special case because it is red and blue balance
      // combined
      if ( UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL == uvcControl ) {
        cameraInfo->haveComponentWhiteBalance = 1;
      }
      if ( oaControl || ( UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL ==
          uvcControl )) {
        // FIX ME -- need to know what the default units are for all of these
        _getUVCControlValues ( camera, uvcHandle, unit->bUnitID, i );
      } else {
        fprintf ( stderr, "Unsupported UVC processing unit control %d = %d\n",
            i, uvcControl );
      }
    }
    flags >>= 1;
  }

  if ( flags ) {
    fprintf ( stderr, "unknown UVC processing unit controls are present\n" );
  }

  if (( extensionUnits = p_uvc_get_extension_units ( uvcHandle ))) {
    fprintf ( stderr, "Extension units found\n" );
    extn = extensionUnits;
    do {
      int i, j, mismatch, done;
      done = i = 0;
      while ( UVCExtensionMap[i].handler && !done ) {
        mismatch = 0;
        for ( j = 0; j < 16 && !mismatch; j++ ) {
          if ( UVCExtensionMap[i].guid[j] != extn->guidExtensionCode[j] ) {
            mismatch = 1;
          }
        }
        if ( !mismatch ) {
          done = 1;
          UVCExtensionMap[i].handler ( camera, commonInfo, extn->bmControls );
        }
        i++;
      }
      if ( !done ) {
        int i;
        fprintf ( stderr, "extn unit: %d, controls: %08lx, guid: ",
            extn->bUnitID, ( long unsigned int ) extn->bmControls );
        for ( i = 0; i < 16; i++ ) {
          fprintf ( stderr, "%02x", extn->guidExtensionCode[i] );
          if ( i == 3 || i == 5 || i == 7 || i == 9 ) {
            fprintf ( stderr, "-" );
          }
        }
        fprintf ( stderr, "\n" );
      }
      extn = extn->next;
    } while ( extn );
  }

  // Now process the format descriptions...

  cameraInfo->currentUVCFormat = 0;
  cameraInfo->currentUVCFormatId = 0;
  cameraInfo->currentFrameFormat = 0;
  cameraInfo->bytesPerPixel = 0;
  cameraInfo->isColour = 0;
  camera->features.flags |= OA_CAM_FEATURE_RESET;
  camera->features.flags |= OA_CAM_FEATURE_READABLE_CONTROLS;
  camera->features.flags |= OA_CAM_FEATURE_STREAMING;

  /*
   * For the time being we know that libuvc knows about:
   * YUY2, UYVY, Y800, Y16, BY8, BA81, GRBG, GBRG, RGGB, BGGR, MJPG
   *
   * BY8 is in fact GBRG and BA81 is BGGR
   */

  formatDescs = p_uvc_get_format_descs ( uvcHandle );
  format = formatDescs;
  cameraInfo->maxBytesPerPixel = cameraInfo->bytesPerPixel = 1;
  do {
    switch ( format->bDescriptorSubtype ) {

      case UVC_VS_FORMAT_FRAME_BASED:

        if ( !memcmp ( format->fourccFormat, "BY8 ", 4 )) {
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
          camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_GBRG8 ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_GBRG8 ] =
              UVC_FRAME_FORMAT_BY8;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_BY8;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_GBRG8;
          }
          cameraInfo->isColour = 1;
        }

        if ( !memcmp ( format->fourccFormat, "BA81", 4 )) {
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
          camera->frameFormats[ OA_PIX_FMT_BGGR8 ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_BGGR8 ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_BGGR8 ] =
              UVC_FRAME_FORMAT_BA81;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_BA81;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_BGGR8;
          }
          cameraInfo->isColour = 1;
        }

        if ( !memcmp ( format->fourccFormat, "GRBG", 4 )) {
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
          camera->frameFormats[ OA_PIX_FMT_GRBG8 ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_GRBG8 ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_GRBG8 ] =
              UVC_FRAME_FORMAT_SGRBG8;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_SGRBG8;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_GRBG8;
          }
          cameraInfo->isColour = 1;
        }

        if ( !memcmp ( format->fourccFormat, "GBRG", 4 )) {
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
          camera->frameFormats[ OA_PIX_FMT_GBRG8 ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_GBRG8 ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_GBRG8 ] =
              UVC_FRAME_FORMAT_SGBRG8;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_SGBRG8;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_GBRG8;
          }
          cameraInfo->isColour = 1;
        }

        if ( !memcmp ( format->fourccFormat, "RGGB", 4 )) {
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
          camera->frameFormats[ OA_PIX_FMT_RGGB8 ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_RGGB8 ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_RGGB8 ] =
              UVC_FRAME_FORMAT_SRGGB8;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_SRGGB8;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_RGGB8;
          }
          cameraInfo->isColour = 1;
        }

        if ( !memcmp ( format->fourccFormat, "BGGR", 4 )) {
					camera->features.flags |= OA_CAM_FEATURE_RAW_MODE;
          camera->frameFormats[ OA_PIX_FMT_BGGR8 ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_BGGR8 ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_BGGR8 ] =
              UVC_FRAME_FORMAT_SBGGR8;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_SBGGR8;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_BGGR8;
          }
          cameraInfo->isColour = 1;
        }

        if ( !memcmp ( format->fourccFormat, "Y800", 4 )) {
          camera->frameFormats[ OA_PIX_FMT_GREY8 ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_GREY8 ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_GREY8 ] =
              UVC_FRAME_FORMAT_GRAY8;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_GRAY8;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY8;
          }
        }

        if ( !memcmp ( format->fourccFormat, "Y16 ", 4 )) {
          cameraInfo->maxBytesPerPixel = 2;
          // this is a guess until someone can tell me definitively what it is
          camera->frameFormats[ OA_PIX_FMT_GREY16LE ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_GREY16LE ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_GREY16LE ] =
              UVC_FRAME_FORMAT_GRAY16;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_GRAY16;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_GREY16LE;
          }
        }
        break;

        if ( !cameraInfo->currentUVCFormatId ) {
          fprintf ( stderr, "unrecognised frame format '%4s'\n",
              format->fourccFormat );
        }

      case UVC_VS_FORMAT_UNCOMPRESSED:

        if ( !memcmp ( format->fourccFormat, "YUY2", 4 )) {
          camera->frameFormats[ OA_PIX_FMT_YUYV ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_YUYV ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_YUYV ] =
              UVC_FRAME_FORMAT_YUYV;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_YUYV;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_YUYV;
          }
          cameraInfo->isColour = 1;
          cameraInfo->bytesPerPixel = 2;
          cameraInfo->maxBytesPerPixel = 2;
        }

        if ( !memcmp ( format->fourccFormat, "UYVY", 4 )) {
          camera->frameFormats[ OA_PIX_FMT_UYVY ] = 1;
          cameraInfo->frameFormatMap[ OA_PIX_FMT_UYVY ] = format;
          cameraInfo->frameFormatIdMap[ OA_PIX_FMT_UYVY ] =
              UVC_FRAME_FORMAT_UYVY;
          if ( !cameraInfo->currentFrameFormat ) {
            cameraInfo->currentUVCFormat = format;
            cameraInfo->currentUVCFormatId = UVC_FRAME_FORMAT_UYVY;
            cameraInfo->currentFrameFormat = OA_PIX_FMT_UYVY;
          }
          cameraInfo->isColour = 1;
          cameraInfo->bytesPerPixel = 2;
          cameraInfo->maxBytesPerPixel = 2;
        }

        if ( !cameraInfo->currentFrameFormat ) {
          fprintf ( stderr, "unrecognised uncompressed format '%4s'\n",
              format->fourccFormat );
        }

        break;

      default:
        fprintf ( stderr, "non frame-based format %d ('%4s') found\n",
            format->bDescriptorSubtype, format->fourccFormat );
        break;
    }
    format = format->next;
  } while ( format );

  if ( !cameraInfo->currentFrameFormat ) {
    fprintf ( stderr, "No suitable video format found on %s\n",
      camera->deviceName );
    p_uvc_close ( uvcHandle );
    p_uvc_exit ( cameraInfo->uvcContext );
    FREE_DATA_STRUCTS;
    return 0;
  }

  camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_FRAME_FORMAT ) = OA_CTRL_TYPE_DISCRETE;

  cameraInfo->frameSizes[1].numSizes = 0;
  cameraInfo->frameSizes[1].sizes = 0;

  cameraInfo->maxResolutionX = cameraInfo->maxResolutionY = 0;
  frame = cameraInfo->currentUVCFormat->frame_descs;
  allFramesHaveFixedRates = 1;
  i = 0;
  do {
    if (!(  tmpPtr = realloc ( cameraInfo->frameSizes[1].sizes,
				( i+1 ) * sizeof ( FRAMESIZE )))) {
      p_uvc_close ( uvcHandle );
      p_uvc_exit ( cameraInfo->uvcContext );
      fprintf ( stderr, "realloc of frameSizes failed\n" );
      FREE_DATA_STRUCTS;
      return 0;
    }
		cameraInfo->frameSizes[1].sizes = tmpPtr;

    if (( cameraInfo->frameSizes[1].sizes[i].x = frame->wWidth ) >
        cameraInfo->maxResolutionX ) {
      cameraInfo->maxResolutionX = cameraInfo->frameSizes[1].sizes[i].x;
    }
    if (( cameraInfo->frameSizes[1].sizes[i].y = frame->wHeight ) >
        cameraInfo->maxResolutionY ) {
      cameraInfo->maxResolutionY = cameraInfo->frameSizes[1].sizes[i].y;
    }
    if ( !frame->bFrameIntervalType ) {
      allFramesHaveFixedRates = 0;
    }
    i++;
    frame = frame->next;
  } while ( frame );
  cameraInfo->frameSizes[1].numSizes = i;

  if ( allFramesHaveFixedRates ) {
		camera->features.flags |= OA_CAM_FEATURE_FRAME_RATES;
	}
	camera->features.flags |= OA_CAM_FEATURE_FIXED_FRAME_SIZES;
  cameraInfo->frameRates.numRates = 0;

  camera->interface = device->interface;
  cameraInfo->uvcHandle = uvcHandle;
  cameraInfo->index = devInfo->devIndex;
  cameraInfo->unitId = unit->bUnitID;

  // Save a local copy for the values of red and blue balance here to
  // save having to read the combined value every time we want to
  // change it.

  if ( cameraInfo->isColour  && cameraInfo->haveComponentWhiteBalance ) {
    cameraInfo->componentBalance = getUVCControl ( cameraInfo->uvcHandle,
        cameraInfo->unitId, UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL, 4,
        UVC_GET_CUR );
  }

  // The largest buffer size we should need

  cameraInfo->buffers = 0;
  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * cameraInfo->maxBytesPerPixel;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( struct UVCbuffer ));
  for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
    void* m = malloc ( cameraInfo->imageBufferLength );
    if ( m ) {
      cameraInfo->buffers[i].start = m;
      cameraInfo->configuredBuffers++;
    } else {
      fprintf ( stderr, "%s malloc failed\n", __FUNCTION__ );
      if ( i ) {
        for ( j = 0; j < i; j++ ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
      p_uvc_close ( uvcHandle );
      p_uvc_exit ( cameraInfo->uvcContext );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      free (( void* ) cameraInfo->buffers );
      FREE_DATA_STRUCTS;
      return 0;
    }
  }

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();
  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = OA_CAM_BUFFERS;
  cameraInfo->buffersFree = OA_CAM_BUFFERS;

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamUVCcontroller, ( void* ) camera )) {
    p_uvc_close ( uvcHandle );
    p_uvc_exit ( cameraInfo->uvcContext );
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->buffers );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    fprintf ( stderr, "controller thread creation failed\n" );
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamUVCcallbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    p_uvc_close ( uvcHandle );
    p_uvc_exit ( cameraInfo->uvcContext );
    for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
      free (( void* ) cameraInfo->buffers[j].start );
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->buffers );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    FREE_DATA_STRUCTS;
    fprintf ( stderr, "callback thread creation failed\n" );
    return 0;
  }

  return camera;
}


static void
_UVCInitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaUVCInitCamera;
  camera->funcs.closeCamera = oaUVCCloseCamera;

  camera->funcs.setControl = oaUVCCameraSetControl;
  camera->funcs.readControl = oaUVCCameraReadControl;
  camera->funcs.testControl = oaUVCCameraTestControl;
  camera->funcs.getControlRange = oaUVCCameraGetControlRange;
  camera->funcs.getControlDiscreteSet = oaUVCCameraGetControlDiscreteSet;

  camera->funcs.startStreaming = oaUVCCameraStartStreaming;
  camera->funcs.stopStreaming = oaUVCCameraStopStreaming;
  camera->funcs.isStreaming = oaUVCCameraIsStreaming;

  camera->funcs.setResolution = oaUVCCameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaUVCCameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaUVCCameraGetFramePixelFormat;

  camera->funcs.enumerateFrameRates = oaUVCCameraGetFrameRates;
  camera->funcs.setFrameInterval = oaUVCCameraSetFrameInterval;

  camera->funcs.getMenuString = oaUVCCameraGetMenuString;
}


static void
_getUVCControlValues ( oaCamera* camera, uvc_device_handle_t* uvcHandle,
    uint8_t unitId, int index )
{
  int				len;
  unsigned int			oaControl;
  enum uvc_pu_ctrl_selector	uvcControl;
  COMMON_INFO*			commonInfo = camera->_common;

  uvcControl = UVCControlData[ index ].uvcControl;
  oaControl = UVCControlData[ index ].oaControl;
  len = UVCControlData[ index ].size;

  // special case for white component control
  if ( UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL == uvcControl ) {
    int val;

    if (( val = getUVCControl ( uvcHandle, unitId,
        UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL, len, UVC_GET_MIN )) < 0 ) {
      return;
    }
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_BLUE_BALANCE ) = val & 0xffff;
    commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_RED_BALANCE ) = val >> 16;

    if (( val = getUVCControl ( uvcHandle, unitId,
        UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL, len, UVC_GET_MAX )) < 0 ) {
      return;
    }
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_BLUE_BALANCE ) = val & 0xffff;
    commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_RED_BALANCE ) = val >> 16;

    if (( val = getUVCControl ( uvcHandle, unitId,
        UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL, len, UVC_GET_RES )) < 0 ) {
      return;
    }
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_BLUE_BALANCE ) = val & 0xffff;
    commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_RED_BALANCE ) = val >> 16;

    if (( val = getUVCControl ( uvcHandle, unitId,
        UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL, len, UVC_GET_DEF )) < 0 ) {
      return;
    }
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_BLUE_BALANCE ) = val & 0xffff;
    commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_RED_BALANCE ) = val >> 16;

    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_BLUE_BALANCE ) = OA_CTRL_TYPE_INT32;
    camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_RED_BALANCE ) = OA_CTRL_TYPE_INT32;

  } else {

    int val;

    camera->OA_CAM_CTRL_TYPE( oaControl ) =
				UVCControlData[ index ].oaControlType;

    switch ( camera->OA_CAM_CTRL_TYPE( oaControl )) {
      case OA_CTRL_TYPE_INT32:
      case OA_CTRL_TYPE_MENU:
        if (( val = getUVCControl ( uvcHandle, unitId, uvcControl, len,
            UVC_GET_MIN )) < 0 ) {
          return;
        }
        commonInfo->OA_CAM_CTRL_MIN( oaControl ) = val;
        if (( val = getUVCControl ( uvcHandle, unitId, uvcControl, len,
            UVC_GET_MAX )) < 0 ) {
          return;
        }
        commonInfo->OA_CAM_CTRL_MAX( oaControl ) = val;
        if (( val = getUVCControl ( uvcHandle, unitId, uvcControl, len,
            UVC_GET_RES )) < 0 ) {
          return;
        }
        commonInfo->OA_CAM_CTRL_STEP( oaControl ) = val;
        if (( commonInfo->OA_CAM_CTRL_DEF( oaControl ) =
            getUVCControl ( uvcHandle, unitId,
            uvcControl, len, UVC_GET_DEF )) < 0 ) {
          return;
        }
        break;

      case OA_CTRL_TYPE_BOOLEAN:
        commonInfo->OA_CAM_CTRL_MIN( oaControl ) = 0;
        commonInfo->OA_CAM_CTRL_MAX( oaControl ) = 1;
        commonInfo->OA_CAM_CTRL_STEP( oaControl ) = 1;
        if (( val = getUVCControl ( uvcHandle, unitId, uvcControl, len,
            UVC_GET_DEF )) < 0 ) {
          return;
        }
        commonInfo->OA_CAM_CTRL_DEF( oaControl ) = val;
        break;

      case OA_CTRL_TYPE_READONLY:
        break;

      default:
        fprintf ( stderr, "unhandled control type %d in %s\n",
            camera->OA_CAM_CTRL_TYPE( oaControl ), __FUNCTION__ );
        break;
    }
  }
}


int
oaUVCCloseCamera ( oaCamera* camera )
{
  int		j;
  void*		dummy;
  UVC_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    p_uvc_close ( cameraInfo->uvcHandle );

    if ( cameraInfo->buffers ) {
      for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
        if ( cameraInfo->buffers[j].start ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
    }

    if ( cameraInfo->frameRates.numRates ) {
     free (( void* ) cameraInfo->frameRates.rates );
    }
    free (( void* ) cameraInfo->frameSizes[1].sizes );

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo );
    free (( void* ) camera->_common );
    free (( void* ) camera );

    // p_uvc_unref_device ( device );

  } else {
   return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}

#endif /* HAVE_LIBUVC */
