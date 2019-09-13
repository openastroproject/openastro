/*****************************************************************************
 *
 * FC2controller.c -- Main camera controller thread
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

#include <oa_common.h>

#include <flycapture/C/FlyCapture2_C.h>
#include <pthread.h>
#include <sys/time.h>

#include <openastro/camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "FC2.h"
#include "FC2oacam.h"
#include "FC2state.h"
#include "FC2private.h"


static int	_processSetControl ( FC2_STATE*, OA_COMMAND* );
static int	_processGetControl ( FC2_STATE*, OA_COMMAND* );
static int	_processGetTriggerControl ( FC2_STATE*, OA_COMMAND* );
static int	_processSetTriggerControl ( FC2_STATE*, OA_COMMAND*, int );
static int	_processGetTriggerDelayControl ( FC2_STATE*, OA_COMMAND* );
static int	_processSetTriggerDelayControl ( FC2_STATE*, OA_COMMAND*, int );
static int	_processGetStrobeControl ( FC2_STATE*, OA_COMMAND* );
static int	_processSetStrobeControl ( FC2_STATE*, OA_COMMAND*, int );
static int	_processSetResolution ( FC2_STATE*, OA_COMMAND* );
static int	_processSetROI ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( FC2_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( FC2_STATE*, OA_COMMAND* );
static int	_doStart ( FC2_STATE* );
static int	_doStop ( FC2_STATE* );
static int	_doFrameFormat ( FC2_STATE*, int );
static int	_doBinning ( FC2_STATE*, int );
//static int	_processSetFrameInterval ( FC2_STATE*, OA_COMMAND* );


void*
oacamFC2controller ( void* param )
{
  oaCamera*		camera = param;
  FC2_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode;
  int			streaming = 0;

  do {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    exitThread = cameraInfo->stopControllerThread;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( exitThread ) {
      break;
    } else {
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      // stop us busy-waiting
      streaming = cameraInfo->isStreaming;
      if ( !streaming && oaDLListIsEmpty ( cameraInfo->commandQueue )) {
        pthread_cond_wait ( &cameraInfo->commandQueued,
            &cameraInfo->commandQueueMutex );
      }
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    }
    do {
      command = oaDLListRemoveFromHead ( cameraInfo->commandQueue );
      if ( command ) {
        switch ( command->commandType ) {
          case OA_CMD_CONTROL_SET:
            resultCode = _processSetControl ( cameraInfo, command );
            break;
          case OA_CMD_CONTROL_GET:
            resultCode = _processGetControl ( cameraInfo, command );
            break;
          case OA_CMD_RESOLUTION_SET:
            resultCode = _processSetResolution ( cameraInfo, command );
            break;
          case OA_CMD_ROI_SET:
            resultCode = _processSetROI ( camera, command );
            break;
          case OA_CMD_START_STREAMING:
            resultCode = _processStreamingStart ( cameraInfo, command );
            break;
          case OA_CMD_STOP_STREAMING:
            resultCode = _processStreamingStop ( cameraInfo, command );
            break;
/*
 * Not sure the Gig-E cams allow this one
          case OA_CMD_FRAME_INTERVAL_SET:
            resultCode = _processSetFrameInterval ( cameraInfo, command );
            break;
 */
          default:
            fprintf ( stderr, "Invalid command type %d in controller\n",
                command->commandType );
            resultCode = -OA_ERR_INVALID_CONTROL;
            break;
        }
        if ( command->callback ) {
        } else {
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          command->completed = 1;
          command->resultCode = resultCode;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
          pthread_cond_broadcast ( &cameraInfo->commandComplete );
        }
      }
    } while ( command );
  } while ( !exitThread );

  return 0;
}


void
_FC2FrameCallback ( fc2Image *frame, void *ptr )
{
  FC2_STATE*				cameraInfo = ptr;
  int								buffersFree, nextBuffer;
  unsigned int			dataLength;
	fc2ImageMetadata	metadata;

  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  buffersFree = cameraInfo->buffersFree;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

  if ( buffersFree && frame->dataSize ) {
    if (( dataLength = frame->dataSize ) > cameraInfo->imageBufferLength ) {
      dataLength = cameraInfo->imageBufferLength;
    }
    nextBuffer = cameraInfo->nextBuffer;
		if ( !cameraInfo->haveFrameCounter || p_fc2GetImageMetadata ( frame,
				&metadata ) != FC2_ERROR_OK ) {
			cameraInfo->metadataBuffers[ nextBuffer ].frameCounterValid = 0;
		} else {
			cameraInfo->metadataBuffers[ nextBuffer ].frameCounter =
					metadata.embeddedFrameCounter;
			cameraInfo->metadataBuffers[ nextBuffer ].frameCounterValid = 1;
		}
    ( void ) memcpy ( cameraInfo->buffers[ nextBuffer ].start, frame->pData,
        dataLength );
    cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
        OA_CALLBACK_NEW_FRAME;
    cameraInfo->frameCallbacks[ nextBuffer ].callback =
        cameraInfo->streamingCallback.callback;
    cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
        cameraInfo->streamingCallback.callbackArg;
    cameraInfo->frameCallbacks[ nextBuffer ].buffer =
        cameraInfo->buffers[ nextBuffer ].start;
    cameraInfo->frameCallbacks[ nextBuffer ].metadata =
        &( cameraInfo->metadataBuffers[ nextBuffer ]);
    cameraInfo->frameCallbacks[ nextBuffer ].bufferLen = dataLength;
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    oaDLListAddToTail ( cameraInfo->callbackQueue,
        &cameraInfo->frameCallbacks[ nextBuffer ]);
    cameraInfo->buffersFree--;
    cameraInfo->nextBuffer = ( nextBuffer + 1 ) % cameraInfo->configuredBuffers;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
  }
}


static int
_processSetControl ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*val = command->commandData;
  int			control = command->controlId;
  int			found, pgeControl = 0;
  unsigned int		i;
  fc2Property		property;

  // do this first as it's not a recognised control in itself
  if ( OA_CAM_CTRL_BINNING == control ) {
    if ( OA_CTRL_TYPE_INT32 != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
          __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    return _doBinning ( cameraInfo, val->int32 );
  }

  for ( i = 0, found = -1; found < 0 && i < numFC2Controls; i++ ) {
    if ( pgeControls[i].oaControl == control ||
        pgeControls[i].oaAutoControl == control ||
        ( OA_CAM_CTRL_IS_ON_OFF( control ) && pgeControls[i].oaControl ==
        OA_CAM_CTRL_MODE_BASE ( control ))) {
      pgeControl = pgeControls[i].pgeControl;
      found = OA_CAM_CTRL_IS_ON_OFF( control ) ? 3 :
          ( OA_CAM_CTRL_IS_AUTO( control ) ? 2 : 1 );
    }
  }

  if ( OA_CAM_CTRL_EXPOSURE_UNSCALED == control ||
      OA_CAM_CTRL_EXPOSURE_ABSOLUTE == control ||
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ) == control ||
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) == control ||
      OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_EXPOSURE_UNSCALED ) == control ||
      OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) == control ) {
    pgeControl = FC2_SHUTTER;
    found = OA_CAM_CTRL_IS_ON_OFF( control ) ? 3 :
        ( OA_CAM_CTRL_IS_AUTO( control ) ? 2 : 1 );
  }

  switch ( control ) {
    case OA_CAM_CTRL_TRIGGER_ENABLE:
    case OA_CAM_CTRL_TRIGGER_MODE:
    case OA_CAM_CTRL_TRIGGER_SOURCE:
    case OA_CAM_CTRL_TRIGGER_POLARITY:
      return _processSetTriggerControl ( cameraInfo, command, control );
      break;
    case OA_CAM_CTRL_TRIGGER_DELAY_ENABLE:
    case OA_CAM_CTRL_TRIGGER_DELAY:
      return _processSetTriggerDelayControl ( cameraInfo, command, control );
      break;
    case OA_CAM_CTRL_STROBE_ENABLE:
    case OA_CAM_CTRL_STROBE_POLARITY:
    case OA_CAM_CTRL_STROBE_SOURCE:
    case OA_CAM_CTRL_STROBE_DELAY:
    case OA_CAM_CTRL_STROBE_DURATION:
      return _processSetStrobeControl ( cameraInfo, command, control );
      break;
  }

  OA_CLEAR ( property );

  if ( found == 2 || found == 3 ) { // auto or on/off
    uint32_t val_u32;
    if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
          __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    val_u32 = val->boolean;
    if ( val_u32 > 1 ) {
      fprintf ( stderr, "%s: control value out of range\n", __FUNCTION__ );
      return -OA_ERR_OUT_OF_RANGE;
    }

    // I think the best way to do this is to read the current setting, then
    // change what we need and write it back
    property.type = pgeControl;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get FC2 property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    if ( found == 2 ) {
      property.autoManualMode = val_u32;
    } else {
      property.onOff = val_u32;
    }
    if (( *p_fc2SetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set FC2 property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_BLUE_BALANCE == control || OA_CAM_CTRL_RED_BALANCE ==
      control ) {
    uint32_t val_u32;

    val_u32 = val->int32;
    property.type = FC2_WHITE_BALANCE;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get FC2 white balance\n" );
      return -OA_ERR_CAMERA_IO;
    }
    if ( OA_CAM_CTRL_BLUE_BALANCE == control ) {
      property.valueB = val_u32;
      cameraInfo->currentBlueBalance = val_u32;
    } else {
      property.valueA = val_u32;
      cameraInfo->currentRedBalance = val_u32;
    }
    if (( *p_fc2SetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set FC2 white balance\n" );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  // need to handle absolute exposure time separately
  if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == control ) {
    float decval;
    int64_t val_s64;
    // We should also have a 64-bit value here and it can't be negative.
    // Probably shouldn't be non-positive, really.
    if ( OA_CTRL_TYPE_INT64 != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where int64 expected "
          "for OA_CAM_CTRL_EXPOSURE_ABSOLUTE\n", __FUNCTION__,
          val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    val_s64 = val->int64;
    decval = val_s64 / 1000.0;
    property.type = pgeControl;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get FC2 property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    property.absControl = 1;
    property.absValue = decval;
    if (( *p_fc2SetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set FC2 property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    cameraInfo->currentAbsoluteExposure = val_s64;
    return OA_ERR_NONE;
  }

  if ( found >= 0 || OA_CAM_CTRL_EXPOSURE_UNSCALED == control ) {
    uint32_t val_u32;
    if ( OA_CTRL_TYPE_INT32 != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where int32 expected "
          "for control %d\n", __FUNCTION__, val->valueType, control );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    val_u32 = val->int32;
    property.type = pgeControl;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get FC2 property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    property.valueA = val_u32;
    if (( *p_fc2SetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set FC2 property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_FRAME_FORMAT == control ) {
    if ( OA_CTRL_TYPE_DISCRETE != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where discrete expected\n",
          __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    return _doFrameFormat ( cameraInfo, val->discrete );
  }

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );

  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  int			found, pgeControl = 0;
  unsigned int		i;
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;
  fc2Property		property;

  switch ( control ) {
    case OA_CAM_CTRL_TRIGGER_ENABLE:
    case OA_CAM_CTRL_TRIGGER_MODE:
    case OA_CAM_CTRL_TRIGGER_SOURCE:
    case OA_CAM_CTRL_TRIGGER_POLARITY:
      return _processGetTriggerControl ( cameraInfo, command );
      break;
    case OA_CAM_CTRL_TRIGGER_DELAY_ENABLE:
    case OA_CAM_CTRL_TRIGGER_DELAY:
      return _processGetTriggerDelayControl ( cameraInfo, command );
      break;
    case OA_CAM_CTRL_STROBE_ENABLE:
    case OA_CAM_CTRL_STROBE_POLARITY:
    case OA_CAM_CTRL_STROBE_SOURCE:
    case OA_CAM_CTRL_STROBE_DELAY:
    case OA_CAM_CTRL_STROBE_DURATION:
      return _processGetStrobeControl ( cameraInfo, command );
      break;
  }

  // We can handle these first as a special case
  if ( OA_CAM_CTRL_BLUE_BALANCE == control || OA_CAM_CTRL_RED_BALANCE ==
      control ) {
    property.type = FC2_WHITE_BALANCE;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get FC2 white balance\n" );
      return -OA_ERR_CAMERA_IO;
    }
    val->valueType = OA_CTRL_TYPE_INT32;
    if ( OA_CAM_CTRL_BLUE_BALANCE == control ) {
      val->int32 = property.valueB;
    } else {
      val->int32 = property.valueA;
    }
    return OA_ERR_NONE;
  }

  for ( i = 0, found = 0; !found && i < numFC2Controls; i++ ) {
    if ( pgeControls[i].oaControl == control ||
        pgeControls[i].oaAutoControl == control ) {
      pgeControl = pgeControls[i].pgeControl;
      found = 1;
    }
  }

	if ( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ) == control ||
			OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) == control ||
			OA_CAM_CTRL_EXPOSURE_UNSCALED == control ) {
		found = 1;
		pgeControl = FC2_SHUTTER;
	}

  if ( found ) {
    property.type = pgeControl;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get FC2 control %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    if ( !oaIsAuto ( control )) {
      if ( pgeControl != FC2_TEMPERATURE ) {
        val->valueType = OA_CTRL_TYPE_INT32;
        val->int32 = property.valueA;
      } else {
        val->valueType = OA_CTRL_TYPE_READONLY;
        val->int32 = property.valueA / 10;
      }
    } else {
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = property.autoManualMode ? 1 : 0;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == control ) {
    // need to handle absolute exposure time separately
    float decval;

    property.type = FC2_SHUTTER;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get FC2 control %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    val->valueType = OA_CTRL_TYPE_INT64;
    decval = property.absValue;
    val->int64 = decval * 1000.0;
    return OA_ERR_NONE;
  }

	if ( OA_CAM_CTRL_BINNING == control ) {
		int		xbin, ybin;

	  if (( *p_fc2GetGigEImageBinningSettings )( cameraInfo->pgeContext, &xbin,
				&ybin ) != FC2_ERROR_OK ) {
			fprintf ( stderr, "Can't get binning state\n" );
			return -OA_ERR_CAMERA_IO;
		}

		val->valueType = OA_CTRL_TYPE_INT32;
		val->int32 = xbin;
		return OA_ERR_NONE;
  }

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );

  return -OA_ERR_INVALID_CONTROL;
}


static int
_processSetResolution ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMESIZE*			size = command->commandData;
  unsigned int			binMode, s, mode, restart = 0;
	fc2GigEImageSettingsInfo	imageInfo;
  fc2GigEImageSettings		settings;
  int				found;

  if ( size->x == cameraInfo->xSize && size->y == cameraInfo->ySize ) {
    return OA_ERR_NONE;
  }

  found = -1;
  binMode = cameraInfo->binMode;
  for ( s = 0; s < cameraInfo->frameSizes[ binMode ].numSizes && found < 0;
			s++ ) {
    if ( cameraInfo->frameSizes[ binMode ].sizes[ s ].x >= size->x &&
        cameraInfo->frameSizes[ binMode ].sizes[ s ].y >= size->y ) {
      found = s;
      break;
    }
  }

  if ( found < 0 ) {
    fprintf ( stderr, "resolution not found\n" );
    return -OA_ERR_OUT_OF_RANGE;
  }

  mode = cameraInfo->frameModes[ binMode ][ found ].mode;

  if (( *p_fc2GetGigEImageSettingsInfo )( cameraInfo->pgeContext,
      &imageInfo ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get image settings info\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if (( *p_fc2GetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get FC2 image settings\n" );
    return -OA_ERR_CAMERA_IO;
  }
  settings.width = size->x;
  settings.height = size->y;
  settings.offsetX = ( imageInfo.maxWidth - size->x ) / 2;
  settings.offsetY = ( imageInfo.maxHeight - size->y ) / 2;

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

/*
  if (( *p_fc2SetGigEImageBinningSettings )( cameraInfo->pgeContext, 1, 1 ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set mode %d for FC2 GUID\n", mode );
    return -OA_ERR_CAMERA_IO;
  }
*/

  if (( *p_fc2SetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set FC2 image settings\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if (( *p_fc2SetGigEImagingMode )( cameraInfo->pgeContext, mode ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set mode %d for FC2 GUID\n", mode );
    return -OA_ERR_CAMERA_IO;
  }

  if (( *p_fc2SetGigEImageBinningSettings )( cameraInfo->pgeContext,
      cameraInfo->binMode, cameraInfo->binMode ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set mode %d for FC2 GUID\n", mode );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;
  cameraInfo->imageBufferLength = size->x * size->y *
      cameraInfo->currentBytesPerPixel;

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}


static int
_processSetROI ( oaCamera* camera, OA_COMMAND* command )
{
  FC2_STATE*			cameraInfo = camera->_private;
  FRAMESIZE*			size = command->commandData;
  unsigned int			x, y;
  fc2GigEImageSettingsInfo	imageInfo;
  fc2GigEImageSettings		settings;
  int				ret, restart = 0;

  if (!( camera->features.flags & OA_CAM_FEATURE_ROI )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  x = size->x;
  y = size->y;

  if (( *p_fc2GetGigEImageSettingsInfo )( cameraInfo->pgeContext,
      &imageInfo ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get image settings info\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if ( x > imageInfo.maxWidth || y > imageInfo.maxHeight ||
      (( x % imageInfo.imageHStepSize ) != 0 ) ||
      (( y % imageInfo.imageVStepSize ) != 0 )) {
    fprintf ( stderr, "Requested image size out of range\n" );
    return -OA_ERR_OUT_OF_RANGE;
  }

  // Set the ROI and ROI position

  settings.width = x;
  settings.height = y;
  settings.offsetX = ( imageInfo.maxWidth - x ) / 2;
  settings.offsetY = ( imageInfo.maxHeight - y ) / 2;

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    _doStop ( cameraInfo );
  }
  if (( ret = ( *p_fc2SetGigEImageSettings )( cameraInfo->pgeContext,
      &settings )) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set image settings, error %d\n", ret );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->xSize = x;
  cameraInfo->ySize = y;

  cameraInfo->imageBufferLength = x * y * cameraInfo->currentBytesPerPixel;

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}


/*
static int
_processSetFrameInterval ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMERATE*                    rate = command->commandData;

fprintf ( stderr, "implement %s\n", __FUNCTION__ );
  cameraInfo->frameRateNumerator = rate->numerator;
  cameraInfo->frameRateDenominator = rate->denominator;
  return _doCameraConfig ( cameraInfo );
}
*/


static int
_processStreamingStart ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  CALLBACK*		cb = command->commandData;
  fc2GigEImageSettings  settings;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

  if (( *p_fc2GetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get image info\n" );
    return -OA_ERR_CAMERA_IO;
  }   
      
  switch ( settings.pixelFormat ) {
    case FC2_PIXEL_FORMAT_MONO8:
    case FC2_PIXEL_FORMAT_RAW8:
      cameraInfo->currentBytesPerPixel = 1;
      break;
    case FC2_PIXEL_FORMAT_MONO12:
      cameraInfo->currentBytesPerPixel = 1.5;
      break;
    case FC2_PIXEL_FORMAT_MONO16:
      cameraInfo->currentBytesPerPixel = 2;
      break;
    case FC2_PIXEL_FORMAT_RGB8:
    case FC2_PIXEL_FORMAT_BGR:
      cameraInfo->currentBytesPerPixel = 3;
      break;
    default:
      fprintf ( stderr, "Can't handle pixel depth calculation in %s\n",
          __FUNCTION__ );
      return -OA_ERR_OUT_OF_RANGE;
      break;
  }

  cameraInfo->imageBufferLength = cameraInfo->xSize * cameraInfo->ySize *
      cameraInfo->currentBytesPerPixel;

  return _doStart ( cameraInfo );
}


static int
_doStart ( FC2_STATE* cameraInfo )
{
  int			ret;

  if (( ret = ( *p_fc2StartCaptureCallback )( cameraInfo->pgeContext,
      _FC2FrameCallback, cameraInfo )) != FC2_ERROR_OK ) {
    fprintf ( stderr, "%s: fc2StartCaptureCallback failed: %d\n", __FUNCTION__,
        ret );
    return -OA_ERR_CAMERA_IO;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  return OA_ERR_NONE;
}


static int
_processStreamingStop ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  return _doStop ( cameraInfo );
}


static int
_doStop ( FC2_STATE* cameraInfo )
{
  int		ret;

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  if (( ret = ( *p_fc2StopCapture )( cameraInfo->pgeContext )) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "%s: fc2StopCapture failed: %d\n", __FUNCTION__,
        ret );
    return -OA_ERR_CAMERA_IO;
  }

  return OA_ERR_NONE;
}


static int
_processGetTriggerControl ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

  switch ( control ) {
    case OA_CAM_CTRL_TRIGGER_ENABLE:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->triggerEnabled;
      break;

    case OA_CAM_CTRL_TRIGGER_MODE:
      val->valueType = OA_CTRL_TYPE_DISC_MENU;
      val->menu = cameraInfo->triggerCurrentMode;
      break;

    case OA_CAM_CTRL_TRIGGER_SOURCE:
      val->valueType = OA_CTRL_TYPE_MENU;
      val->menu = cameraInfo->triggerGPIO;
      break;

    case OA_CAM_CTRL_TRIGGER_POLARITY:
      val->valueType = OA_CTRL_TYPE_MENU;
      val->menu = cameraInfo->triggerCurrentPolarity;
      break;

    default:
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  return -OA_ERR_NONE;
}


static int
_processSetTriggerControl ( FC2_STATE* cameraInfo, OA_COMMAND* command,
    int control )
{
  oaControlValue	*val = command->commandData;
  fc2TriggerMode	triggerMode;

  switch ( control ) {
    case OA_CAM_CTRL_TRIGGER_ENABLE:
      if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->triggerEnabled = val->boolean;
      break;

    case OA_CAM_CTRL_TRIGGER_MODE:
      if ( OA_CTRL_TYPE_DISC_MENU != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "menu expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->triggerCurrentMode = val->menu;
      break;

    case OA_CAM_CTRL_TRIGGER_SOURCE:
      if ( OA_CTRL_TYPE_MENU != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where "
            "menu expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->triggerGPIO = val->menu;
      break;

    case OA_CAM_CTRL_TRIGGER_POLARITY:
      if ( OA_CTRL_TYPE_MENU != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where "
            "menu expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->triggerCurrentPolarity = val->menu;
      break;
  }
  if ( OA_CAM_CTRL_TRIGGER_ENABLE == control || cameraInfo->triggerEnabled ||
      !cameraInfo->triggerEnable ) {
    OA_CLEAR ( triggerMode );
    triggerMode.onOff = cameraInfo->triggerEnabled;
    triggerMode.polarity = cameraInfo->triggerCurrentPolarity;
    triggerMode.source = cameraInfo->triggerGPIO;
    triggerMode.mode = cameraInfo->triggerCurrentMode;

    if (( *p_fc2SetTriggerMode )( cameraInfo->pgeContext, &triggerMode ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set FC2 trigger mode\n" );
      return -OA_ERR_CAMERA_IO;
    }
  }

  return -OA_ERR_NONE;
}


static int
_processGetTriggerDelayControl ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

  switch ( control ) {
    case OA_CAM_CTRL_TRIGGER_DELAY_ENABLE:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->triggerDelayEnabled;
      break;

    case OA_CAM_CTRL_TRIGGER_DELAY:
      val->valueType = OA_CTRL_TYPE_INT64;
      val->boolean = cameraInfo->triggerCurrentDelay;
      break;

    default:
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  return -OA_ERR_NONE;
}


static int
_processSetTriggerDelayControl ( FC2_STATE* cameraInfo, OA_COMMAND* command,
    int control )
{
  oaControlValue	*val = command->commandData;
  fc2TriggerDelay	triggerDelay;

  switch ( control ) {
    case OA_CAM_CTRL_TRIGGER_DELAY_ENABLE:
      if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->triggerDelayEnabled = val->boolean;
      break;

    case OA_CAM_CTRL_TRIGGER_DELAY:
      if ( OA_CTRL_TYPE_INT64 != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "menu expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->triggerCurrentDelay = val->int64;
      break;
  }

  if ( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE == control ||
      cameraInfo->triggerDelayEnabled ||
      !cameraInfo->triggerDelayEnable ) {
    OA_CLEAR ( triggerDelay );
    triggerDelay.onOff = cameraInfo->triggerDelayEnabled;
    triggerDelay.valueA = cameraInfo->triggerCurrentDelay;

    if (( *p_fc2SetTriggerDelay )( cameraInfo->pgeContext, &triggerDelay ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set FC2 trigger delay\n" );
      return -OA_ERR_CAMERA_IO;
    }
  }

  return -OA_ERR_NONE;
}


static int
_processGetStrobeControl ( FC2_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

  switch ( control ) {
    case OA_CAM_CTRL_STROBE_ENABLE:
      val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->strobeEnabled;
      break;

    case OA_CAM_CTRL_STROBE_POLARITY:
      val->valueType = OA_CTRL_TYPE_MENU;
      val->boolean = cameraInfo->strobeCurrentPolarity;
      break;

    case OA_CAM_CTRL_STROBE_DELAY:
      val->valueType = OA_CTRL_TYPE_INT64;
      val->boolean = cameraInfo->strobeCurrentDelay;
      break;

    case OA_CAM_CTRL_STROBE_DURATION:
      val->valueType = OA_CTRL_TYPE_INT64;
      val->boolean = cameraInfo->strobeCurrentDuration;
      break;

    default:
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  return -OA_ERR_NONE;
}


static int
_processSetStrobeControl ( FC2_STATE* cameraInfo, OA_COMMAND* command,
    int control )
{
  oaControlValue	*val = command->commandData;
  fc2StrobeControl	strobeControl;

  switch ( control ) {
    case OA_CAM_CTRL_STROBE_ENABLE:
      if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->strobeEnabled = val->boolean;
      break;

    case OA_CAM_CTRL_STROBE_POLARITY:
      if ( OA_CTRL_TYPE_MENU != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where "
            "menu expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->strobeCurrentPolarity = val->menu;
      break;

    case OA_CAM_CTRL_STROBE_DELAY:
      if ( OA_CTRL_TYPE_INT64 != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int64 "
            "expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->strobeCurrentDelay = val->int64;
      break;

    case OA_CAM_CTRL_STROBE_DURATION:
      if ( OA_CTRL_TYPE_INT64 != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int64 "
            "expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->strobeCurrentDuration = val->int64;
      break;
  }

	// strobeEnable means the strobe can be switched on and off
	// strobeEnabled means we want to switch it on
  if ( OA_CAM_CTRL_STROBE_ENABLE == control || cameraInfo->strobeEnabled ||
      !cameraInfo->strobeEnable ) {
    OA_CLEAR ( strobeControl );
    strobeControl.source = cameraInfo->strobeGPIO;
    strobeControl.onOff = cameraInfo->strobeEnabled;
    strobeControl.polarity = cameraInfo->strobeCurrentPolarity;
    strobeControl.delay = ( float ) cameraInfo->strobeCurrentDelay / 1000000;
    strobeControl.duration = ( float ) cameraInfo->strobeCurrentDuration /
        1000000;

    if (( *p_fc2SetStrobe )( cameraInfo->pgeContext, &strobeControl ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set FC2 strobe control\n" );
      return -OA_ERR_CAMERA_IO;
    }
  }

  return -OA_ERR_NONE;
}


static int
_doBinning ( FC2_STATE* cameraInfo, int binMode )
{
  unsigned int			s, mode, restart = 0;
  unsigned int			oldX, oldY, newX, newY;
  int				found;

  if (!( cameraInfo->availableBinModes & ( 1 << ( binMode - 1 )))) {
    return -OA_ERR_OUT_OF_RANGE;
  }

  oldX = cameraInfo->xSize * cameraInfo->binMode;
  oldY = cameraInfo->ySize * cameraInfo->binMode;
  newX = oldX / binMode;
  newY = oldY / binMode;

  found = -1;
  for ( s = 0; s < cameraInfo->frameSizes[ binMode ].numSizes; s++ ) {
    if ( cameraInfo->frameSizes[ binMode ].sizes[ s ].x >= newX &&
        cameraInfo->frameSizes[ binMode ].sizes[ s ].y >= newY ) {
      found = s;
      break;
    }
  }

  if ( found < 0 ) {
    fprintf ( stderr, "resolution not found\n" );
    return -OA_ERR_OUT_OF_RANGE;
  }

  mode = cameraInfo->frameModes[ binMode ][ found ].mode;

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

/*
  if (( *p_fc2SetGigEImagingMode )( cameraInfo->pgeContext, mode ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set mode %d\n", mode );
    return -OA_ERR_CAMERA_IO;
  }
*/

  if (( *p_fc2SetGigEImageBinningSettings )( cameraInfo->pgeContext,
      binMode, binMode ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set binning %d for mode %d\n", binMode, mode );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->binMode = binMode;
  cameraInfo->xSize = newX;
  cameraInfo->ySize = newY;
  cameraInfo->imageBufferLength = newX * newY *
      cameraInfo->currentBytesPerPixel;

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}


static int
_doFrameFormat ( FC2_STATE* cameraInfo, int format )
{
  fc2GigEImageSettings	settings;
  int			restart = 0;

  if (( *p_fc2GetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get FC2 image settings\n" );
    return -OA_ERR_CAMERA_IO;
  }

  switch ( format ) {
    case OA_PIX_FMT_GREY8:
      settings.pixelFormat = FC2_PIXEL_FORMAT_MONO8;
      break;
    case OA_PIX_FMT_YUV411:
      settings.pixelFormat = FC2_PIXEL_FORMAT_411YUV8;
      break;
    case OA_PIX_FMT_YUV422:
      settings.pixelFormat = FC2_PIXEL_FORMAT_422YUV8;
      break;
    case OA_PIX_FMT_YUV444:
      settings.pixelFormat = FC2_PIXEL_FORMAT_444YUV8;
      break;
    case OA_PIX_FMT_RGB24:
      settings.pixelFormat = FC2_PIXEL_FORMAT_RGB8;
      break;
    case OA_PIX_FMT_GREY16BE:
    case OA_PIX_FMT_GREY16LE:
      settings.pixelFormat = FC2_PIXEL_FORMAT_MONO16;
      break;
    case OA_PIX_FMT_RGB48BE:
    case OA_PIX_FMT_RGB48LE:
      settings.pixelFormat = FC2_PIXEL_FORMAT_RGB16;
      break;
    case OA_PIX_FMT_RGGB8:
    case OA_PIX_FMT_BGGR8:
    case OA_PIX_FMT_GRBG8:
    case OA_PIX_FMT_GBRG8:
      settings.pixelFormat = FC2_PIXEL_FORMAT_RAW8;
      break;
    case OA_PIX_FMT_RGGB16BE:
    case OA_PIX_FMT_RGGB16LE:
    case OA_PIX_FMT_BGGR16BE:
    case OA_PIX_FMT_BGGR16LE:
    case OA_PIX_FMT_GRBG16BE:
    case OA_PIX_FMT_GRBG16LE:
    case OA_PIX_FMT_GBRG16BE:
    case OA_PIX_FMT_GBRG16LE:
      settings.pixelFormat = FC2_PIXEL_FORMAT_RAW16;
      break;
    case OA_PIX_FMT_GREY12:
      settings.pixelFormat = FC2_PIXEL_FORMAT_MONO12;
      break;
    case OA_PIX_FMT_RGGB12:
    case OA_PIX_FMT_BGGR12:
    case OA_PIX_FMT_GRBG12:
    case OA_PIX_FMT_GBRG12:
      settings.pixelFormat = FC2_PIXEL_FORMAT_RAW12;
      break;
    case OA_PIX_FMT_BGR24:
      settings.pixelFormat = FC2_PIXEL_FORMAT_BGR;
      break;
    case OA_PIX_FMT_BGR48BE:
    case OA_PIX_FMT_BGR48LE:
      settings.pixelFormat = FC2_PIXEL_FORMAT_BGR16;
      break;
  }

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

  if (( *p_fc2SetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set FC2 image settings\n" );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->currentVideoFormat = settings.pixelFormat;
  cameraInfo->currentFrameFormat = format;
  cameraInfo->currentBytesPerPixel = oaFrameFormats[ format ].bytesPerPixel;
  cameraInfo->imageBufferLength = cameraInfo->xSize * cameraInfo->ySize *
    cameraInfo->currentBytesPerPixel;

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}


