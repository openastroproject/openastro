/*****************************************************************************
 *
 * GP2connect.c -- Initialise libgphoto2 cameras
 *
 * Copyright 2019
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

#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <gphoto2/gphoto2-camera.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "GP2oacam.h"
#include "GP2private.h"
#include "GP2state.h"


static void		_GP2InitFunctionPointers ( oaCamera* );
static int		_GP2ProcessMenuWidget ( CameraWidget*, const char*,
									CameraWidget**, CameraWidgetType*, int*, const char*,
									const char* camPort );


/**
 * Initialise a given camera device
 */

oaCamera*
oaGP2InitCamera ( oaCameraDevice* device )
{
  oaCamera*					camera;
	CameraList*				cameraList;
	Camera*						gp2camera;
  DEVICE_INFO*			devInfo;
  GP2_STATE*				cameraInfo;
  COMMON_INFO*			commonInfo;
	const char*				camName;
	const char*				camPort;
	int								numCameras, i, ret, found = -1;

  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }
  if (!( cameraInfo = ( GP2_STATE* ) malloc ( sizeof ( GP2_STATE )))) {
    free ( camera );
    perror ( "malloc GP2_STATE failed" );
    return 0;
  }
  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    free ( cameraInfo );
    free ( camera );
    perror ( "malloc COMMON_INFO failed" );
    return 0;
  }
  OA_CLEAR ( *camera );
  OA_CLEAR ( *cameraInfo );
  OA_CLEAR ( *commonInfo );
  camera->_private = cameraInfo;
  camera->_common = commonInfo;

  _oaInitCameraFunctionPointers ( camera );
  _GP2InitFunctionPointers ( camera );

  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  devInfo = device->_private;

  // FIX ME -- This is a bit ugly.  Much of it is repeated from the
  // getCameras function.  I should join the two together somehow.

	// Not clear from the docs what this returns in case of an error, or if
	// an error is even possible
	// FIX ME -- check in source code
	if (!( cameraInfo->ctx = p_gp_context_new())) {
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
		return 0;
	}

	_gp2ConfigureCallbacks ( cameraInfo->ctx );

  if ( p_gp_list_new ( &cameraList ) != GP_OK ) {
    fprintf ( stderr, "gp_list_new failed\n" );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }
  if ( p_gp_list_reset ( cameraList ) != GP_OK ) {
    fprintf ( stderr, "gp_list_reset failed\n" );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

	// gp_camera_autodetect isn't explicitly documented as returning the
	// number of cameras found, but this appears to be the case.
  if (( numCameras = p_gp_camera_autodetect ( cameraList,
			cameraInfo->ctx )) < 0 ) {
    fprintf ( stderr, "gp_camera_autodetect failed: error code %d\n",
				numCameras );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

	if ( numCameras < 1 ) {
    fprintf ( stderr, "Can't see any UVC devices now\n" );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
	}

  for ( i = 0; i < numCameras && found < 0; i++ ) {
		if ( p_gp_list_get_name ( cameraList, i, &camName ) != GP_OK ) {
			fprintf ( stderr, "gp_list_get_name failed\n" );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			free (( void* ) commonInfo );
			free (( void* ) cameraInfo );
			free (( void* ) camera );
			return 0;
		}
		if ( strcmp ( camName, devInfo->deviceId )) {
			continue;
		}
		if ( p_gp_list_get_value ( cameraList, i, &camPort ) != GP_OK ) {
			fprintf ( stderr, "gp_list_get_name failed\n" );
			p_gp_list_unref ( cameraList );
			p_gp_context_unref ( cameraInfo->ctx );
			free (( void* ) commonInfo );
			free (( void* ) cameraInfo );
			free (( void* ) camera );
			return 0;
		}
		if ( !strcmp ( camPort, devInfo->sysPath )) {
			found = i;
		}
	}

	if ( found < 0) {
    fprintf ( stderr, "No matching libgphoto2 device found!\n" );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
	}

	if ( _gp2OpenCamera ( &gp2camera, camName, camPort, cameraInfo->ctx ) !=
			OA_ERR_NONE ) {
		fprintf ( stderr, "Can't open camera '%s' at port '%s'\n", camName,
				camPort );
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
     return 0;
   }

	if ( _gp2GetConfig ( gp2camera, &cameraInfo->rootWidget, cameraInfo->ctx ) !=
			OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get config for camera '%s' at port '%s'\n",
				camName, camPort );
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
  }

	// Now get the widget for imgsettings so it can be used without
	// fetching it again

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "imgsettings",
			&cameraInfo->imgSettings ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get imgsettings widget for camera '%s' "
				"at port '%s'\n", camName, camPort );
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
  }

	// Ditto for capturesettings

	if ( _gp2FindWidget ( cameraInfo->rootWidget, "capturesettings",
			&cameraInfo->captureSettings ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get capturesettings widget for camera '%s' "
				"at port '%s'\n", camName, camPort );
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
  }

	// Now start looking for controls that we can use

	// ISO setting

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->imgSettings, "iso",
			&cameraInfo->iso, &cameraInfo->isoType, &cameraInfo->numIsoOptions,
			camName, camPort )) != OA_ERR_NONE && ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
	}
  if ( ret == OA_ERR_NONE ) {
fprintf ( stderr, "have iso, min = 0, max = %d\n", cameraInfo->numIsoOptions - 1 );
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_ISO ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_ISO ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_ISO ) =
				cameraInfo->numIsoOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_ISO ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_ISO ) = 0;
		// FIX ME -- is there an auto value required here?
	}

	// white balance

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->imgSettings, "whitebalance",
			&cameraInfo->whiteBalance, &cameraInfo->whiteBalanceType,
			&cameraInfo->numWBOptions, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
fprintf ( stderr, "have wb, min = 0, max = %d\n", cameraInfo->numWBOptions - 1 );
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_WHITE_BALANCE ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_WHITE_BALANCE ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_WHITE_BALANCE ) =
				cameraInfo->numWBOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_WHITE_BALANCE ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_WHITE_BALANCE ) = 0;
	}

	// shutter speed

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->captureSettings,
			"shutterspeed", &cameraInfo->shutterSpeed, &cameraInfo->shutterSpeedType,
			&cameraInfo->numShutterSpeedOptions, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
fprintf ( stderr, "have shutter speed, min = 0, max = %d\n", cameraInfo->numShutterSpeedOptions - 1 );
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHUTTER_SPEED ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHUTTER_SPEED ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHUTTER_SPEED ) =
				cameraInfo->numShutterSpeedOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHUTTER_SPEED ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHUTTER_SPEED ) = 0;
	}

	// sharpening

	if (( ret = _GP2ProcessMenuWidget ( cameraInfo->captureSettings, "sharpening",
			&cameraInfo->sharpening, &cameraInfo->sharpeningType,
			&cameraInfo->numSharpeningOptions, camName, camPort )) != OA_ERR_NONE &&
			ret != -OA_ERR_INVALID_COMMAND ) {
		_gp2CloseCamera ( gp2camera, cameraInfo->ctx );
		// FIX ME -- free rootWidget?
		p_gp_list_unref ( cameraList );
		p_gp_context_unref ( cameraInfo->ctx );
		free (( void* ) commonInfo );
		free (( void* ) cameraInfo );
		free (( void* ) camera );
    return 0;
	}
	if ( ret == OA_ERR_NONE ) {
fprintf ( stderr, "have sharpening, min = 0, max = %d\n", cameraInfo->numSharpeningOptions - 1 );
		camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_SHARPNESS ) = OA_CTRL_TYPE_MENU;
		commonInfo->OA_CAM_CTRL_MIN( OA_CAM_CTRL_SHARPNESS ) = 0;
		commonInfo->OA_CAM_CTRL_MAX( OA_CAM_CTRL_SHARPNESS ) =
				cameraInfo->numSharpeningOptions - 1;
		commonInfo->OA_CAM_CTRL_STEP( OA_CAM_CTRL_SHARPNESS ) = 1;
		commonInfo->OA_CAM_CTRL_DEF( OA_CAM_CTRL_SHARPNESS ) = 0;
	}

/*

  // Now process the format descriptions...

  cameraInfo->currentUVCFormat = 0;
  cameraInfo->currentUVCFormatId = 0;
  cameraInfo->currentFrameFormat = 0;
  cameraInfo->bytesPerPixel = 0;
  cameraInfo->isColour = 0;
  camera->features.rawMode = camera->features.demosaicMode = 0;
  camera->features.hasReset = 1;
  camera->features.readableControls = 1;

  formatDescs = p_uvc_get_format_descs ( uvcHandle );
  format = formatDescs;
  cameraInfo->maxBytesPerPixel = cameraInfo->bytesPerPixel = 1;
  do {
    switch ( format->bDescriptorSubtype ) {

      case UVC_VS_FORMAT_FRAME_BASED:

        if ( !memcmp ( format->fourccFormat, "BY8 ", 4 )) {
          camera->features.rawMode = 1;
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
          camera->features.rawMode = 1;
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
          camera->features.rawMode = 1;
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
          camera->features.rawMode = 1;
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
          camera->features.rawMode = 1;
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
          camera->features.rawMode = 1;
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
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
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
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
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

  camera->features.frameRates = allFramesHaveFixedRates;
  camera->features.fixedFrameSizes = 1;
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
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
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
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
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
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    fprintf ( stderr, "callback thread creation failed\n" );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
*/
  return camera;
}


static void
_GP2InitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaGP2InitCamera;
/*
  camera->funcs.closeCamera = oaGP2CloseCamera;

  camera->funcs.setControl = oaGP2CameraSetControl;
  camera->funcs.readControl = oaGP2CameraReadControl;
  camera->funcs.testControl = oaGP2CameraTestControl;
*/
  camera->funcs.getControlRange = oaGP2CameraGetControlRange;
/*
  camera->funcs.getControlDiscreteSet = oaGP2CameraGetControlDiscreteSet;

  camera->funcs.startStreaming = oaGP2CameraStartStreaming;
  camera->funcs.stopStreaming = oaGP2CameraStopStreaming;
  camera->funcs.isStreaming = oaGP2CameraIsStreaming;

  camera->funcs.setResolution = oaGP2CameraSetResolution;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;

  camera->funcs.enumerateFrameSizes = oaGP2CameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaGP2CameraGetFramePixelFormat;

  camera->funcs.enumerateFrameRates = oaGP2CameraGetFrameRates;
  camera->funcs.setFrameInterval = oaGP2CameraSetFrameInterval;

  camera->funcs.getMenuString = oaGP2CameraGetMenuString;
*/
}


static int
_GP2ProcessMenuWidget ( CameraWidget* parent, const char* name,
		CameraWidget** ptarget, CameraWidgetType* ptargetType, int* numVals,
		const char* camName, const char* camPort )
{
	int				ret;

	if (( ret = _gp2FindWidget ( parent, name, ptarget )) != OA_ERR_NONE ) {
		if ( ret != -OA_ERR_INVALID_COMMAND ) {
			fprintf ( stderr, "Can't get %s widget for camera '%s' "
					"at port '%s'\n", name, camName, camPort );
		}
    return ret;
	}

  if ( _gp2GetWidgetType ( *ptarget, ptargetType ) != OA_ERR_NONE ) {
		fprintf ( stderr, "Can't get type for %s widget for camera "
				"'%s' at port '%s'\n", name, camName, camPort );
    return -OA_ERR_SYSTEM_ERROR;
	}

	// We'll accept RADIO and MENU types for menus
	if ( *ptargetType != GP_WIDGET_RADIO && *ptargetType != GP_WIDGET_MENU ) {
		fprintf ( stderr, "Unexpected type %d for %s widget for "
				"camera '%s' at port '%s'\n", *ptargetType, name , camName, camPort );
    return -OA_ERR_SYSTEM_ERROR;
	}

	// By the looks of it, radio and menu widgets have sets of options that
	// always have a starting value of zero and step up in units, so just
	// counting the number of children for this widget should be sufficient
	// to set the min and max values for this command.  I'll assume zero is
	// the default.

	if (( *numVals = p_gp_widget_count_choices ( *ptarget )) < GP_OK ) {
		fprintf ( stderr, "Can't get number of choices for %s "
				"widget for camera '%s' at port '%s'\n", name, camName, camPort );
    return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}
