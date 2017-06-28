/*****************************************************************************
 *
 * PGEcontroller.c -- Main camera controller thread
 *
 * Copyright 2015,2016 James Fidell (james@openastroproject.org)
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
#include "PGE.h"
#include "PGEoacam.h"
#include "PGEstate.h"


static int	_processSetControl ( PGE_STATE*, OA_COMMAND* );
static int	_processGetControl ( PGE_STATE*, OA_COMMAND* );
static int	_processGetTriggerControl ( PGE_STATE*, OA_COMMAND* );
static int	_processSetTriggerControl ( PGE_STATE*, OA_COMMAND*, int );
static int	_processGetTriggerDelayControl ( PGE_STATE*, OA_COMMAND* );
static int	_processSetTriggerDelayControl ( PGE_STATE*, OA_COMMAND*, int );
static int	_processGetStrobeControl ( PGE_STATE*, OA_COMMAND* );
static int	_processSetStrobeControl ( PGE_STATE*, OA_COMMAND*, int );
static int	_processSetResolution ( PGE_STATE*, OA_COMMAND* );
static int	_processSetROI ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( PGE_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( PGE_STATE*, OA_COMMAND* );
static int	_doStart ( PGE_STATE* );
static int	_doStop ( PGE_STATE* );
static int	_doBitDepth ( PGE_STATE*, int );
//static int	_processSetFrameInterval ( PGE_STATE*, OA_COMMAND* );


void*
oacamPGEcontroller ( void* param )
{
  oaCamera*		camera = param;
  PGE_STATE*		cameraInfo = camera->_private;
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
          case OA_CMD_START:
            resultCode = _processStreamingStart ( cameraInfo, command );
            break;
          case OA_CMD_STOP:
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
_PGEFrameCallback ( fc2Image *frame, void *ptr )
{
  PGE_STATE*    cameraInfo = ptr;
  int           buffersFree, nextBuffer;
  unsigned int  dataLength;

  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  buffersFree = cameraInfo->buffersFree;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

  if ( buffersFree && frame->dataSize ) {
    if (( dataLength = frame->dataSize ) > cameraInfo->imageBufferLength ) {
      dataLength = cameraInfo->imageBufferLength;
    }
    nextBuffer = cameraInfo->nextBuffer;
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
_processSetControl ( PGE_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*val = command->commandData;
  int			control = command->controlId;
  int			found, pgeControl = 0;
  unsigned int		i;
  fc2Property		property;

  for ( i = 0, found = -1; found < 0 && i < numPGEControls; i++ ) {
    if ( pgeControls[i].oaControl == control ||
        pgeControls[i].oaAutoControl == control ) {
      pgeControl = pgeControls[i].pgeControl;
      found = 1;
    }
  }

  if ( OA_CAM_CTRL_EXPOSURE == control || OA_CAM_CTRL_EXPOSURE_ABSOLUTE ==
      control || OA_CAM_CTRL_AUTO_EXPOSURE == control ) {
    pgeControl = FC2_SHUTTER;
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

  if ( oaIsAuto ( control )) {
    uint32_t val_u32;
    if ( OA_CAM_CTRL_AUTO_EXPOSURE == control ) {
      if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u32 = val->boolean;
      if ( val_u32 != OA_EXPOSURE_AUTO && val_u32 != OA_EXPOSURE_MANUAL ) {
        fprintf ( stderr, "%s: control value out of range\n", __FUNCTION__ );
        return -OA_ERR_OUT_OF_RANGE;
      }
      val_u32 = ( OA_EXPOSURE_AUTO == val_u32 ) ? 1 : 0;
    } else {
      // anything here should be a boolean value
      if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u32 = val->boolean;
    }

    // I think the best way to do this is to read the current setting, then
    // change what we need and write it back
    property.type = pgeControl;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get PGE property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    property.autoManualMode = val_u32 ? 1 : 0;
    if (( *p_fc2SetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set PGE property %d\n", pgeControl );
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
      fprintf ( stderr, "Can't get PGE white balance\n" );
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
      fprintf ( stderr, "Can't set PGE white balance\n" );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( found >= 0 || OA_CAM_CTRL_EXPOSURE == control ) {
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
      fprintf ( stderr, "Can't get PGE property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    property.valueA = val_u32;
    if (( *p_fc2SetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set PGE property %d\n", pgeControl );
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
      fprintf ( stderr, "Can't get PGE property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    property.absControl = 1;
    property.absValue = decval;
    if (( *p_fc2SetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't set PGE property %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    cameraInfo->currentAbsoluteExposure = val_s64;
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_BIT_DEPTH == control ) {
    if ( OA_CTRL_TYPE_DISCRETE != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
          __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    return _doBitDepth ( cameraInfo, val->discrete );
  }

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );

  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( PGE_STATE* cameraInfo, OA_COMMAND* command )
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
      fprintf ( stderr, "Can't get PGE white balance\n" );
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

  for ( i = 0, found = 0; !found && i < numPGEControls; i++ ) {
    if ( pgeControls[i].oaControl == control ||
        pgeControls[i].oaAutoControl == control ) {
      pgeControl = pgeControls[i].pgeControl;
      found = 1;
    }
  }

  if ( found ) {

    if ( pgeControl != FC2_SHUTTER || OA_CAM_CTRL_EXPOSURE ==
        control || OA_CAM_CTRL_AUTO_EXPOSURE == control ) {
      property.type = pgeControl;
      if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
          FC2_ERROR_OK ) {
        fprintf ( stderr, "Can't get PGE control %d\n", pgeControl );
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
        if ( OA_CAM_CTRL_AUTO_EXPOSURE == control ) {
          // FIX ME -- should this be DISCRETE?
          val->valueType = OA_CTRL_TYPE_INT32;
          val->int32 = ( property.autoManualMode ) ?  OA_EXPOSURE_AUTO :
              OA_EXPOSURE_MANUAL;
        } else {
          val->valueType = OA_CTRL_TYPE_BOOLEAN;
          val->boolean = property.autoManualMode;
        }
      }
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == control ) {
    // need to handle absolute exposure time separately
    float decval;

    property.type = FC2_SHUTTER;
    if (( *p_fc2GetProperty )( cameraInfo->pgeContext, &property ) !=
        FC2_ERROR_OK ) {
      fprintf ( stderr, "Can't get PGE control %d\n", pgeControl );
      return -OA_ERR_CAMERA_IO;
    }
    val->valueType = OA_CTRL_TYPE_INT64;
    decval = property.absValue;
    val->int64 = decval * 1000.0;
    return OA_ERR_NONE;
  }

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );

  return -OA_ERR_INVALID_CONTROL;
}


static int
_processSetResolution ( PGE_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMESIZE*			size = command->commandData;
  unsigned int			binMode, s, found, mode, restart = 0;
  fc2GigEImageSettings		settings;

  found = -1;
  binMode = cameraInfo->binMode;
  for ( s = 0; s < cameraInfo->frameSizes[ binMode ].numSizes; s++ ) {
    if ( cameraInfo->frameSizes[ binMode ].sizes[ s ].x == size->x &&
        cameraInfo->frameSizes[ binMode ].sizes[ s ].y == size->y ) {
      found = s;
      break;
    }
  }

  if ( found < 0 ) {
    fprintf ( stderr, "resolution not found\n" );
    return -OA_ERR_OUT_OF_RANGE;
  }

  mode = cameraInfo->frameModes[ binMode ][ found ].mode;

  if (( *p_fc2GetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get PGE image settings\n" );
    return -OA_ERR_CAMERA_IO;
  }
  settings.width = size->x;
  settings.height = size->y;
  settings.offsetX = 0;
  settings.offsetY = 0;

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

  if (( *p_fc2SetGigEImageBinningSettings )( cameraInfo->pgeContext, 1, 1 ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set mode %d for PGE GUID\n", mode );
    return -OA_ERR_CAMERA_IO;
  }

  if (( *p_fc2SetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set PGE image settings\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if (( *p_fc2SetGigEImagingMode )( cameraInfo->pgeContext, mode ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set mode %d for PGE GUID\n", mode );
    return -OA_ERR_CAMERA_IO;
  }

  if (( *p_fc2SetGigEImageBinningSettings )( cameraInfo->pgeContext,
      cameraInfo->binMode, cameraInfo->binMode ) != FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set mode %d for PGE GUID\n", mode );
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
  PGE_STATE*			cameraInfo = camera->_private;
  FRAMESIZE*			size = command->commandData;
  unsigned int			x, y;
  fc2GigEImageSettingsInfo	imageInfo;
  fc2GigEImageSettings		settings;
  int				ret, restart = 0;

  if ( !camera->features.ROI ) {
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
_processSetFrameInterval ( PGE_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMERATE*                    rate = command->commandData;

fprintf ( stderr, "implement %s\n", __FUNCTION__ );
  cameraInfo->frameRateNumerator = rate->numerator;
  cameraInfo->frameRateDenominator = rate->denominator;
  return _doCameraConfig ( cameraInfo );
}
*/


static int
_processStreamingStart ( PGE_STATE* cameraInfo, OA_COMMAND* command )
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
_doStart ( PGE_STATE* cameraInfo )
{
  int			ret;

  if (( ret = ( *p_fc2StartCaptureCallback )( cameraInfo->pgeContext,
      _PGEFrameCallback, cameraInfo )) != FC2_ERROR_OK ) {
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
_processStreamingStop ( PGE_STATE* cameraInfo, OA_COMMAND* command )
{
  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  return _doStop ( cameraInfo );
}


static int
_doStop ( PGE_STATE* cameraInfo )
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
_processGetTriggerControl ( PGE_STATE* cameraInfo, OA_COMMAND* command )
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
_processSetTriggerControl ( PGE_STATE* cameraInfo, OA_COMMAND* command,
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
      fprintf ( stderr, "Can't set PGE trigger mode\n" );
      return -OA_ERR_CAMERA_IO;
    }
  }

  return -OA_ERR_NONE;
}


static int
_processGetTriggerDelayControl ( PGE_STATE* cameraInfo, OA_COMMAND* command )
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
_processSetTriggerDelayControl ( PGE_STATE* cameraInfo, OA_COMMAND* command,
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
      fprintf ( stderr, "Can't set PGE trigger delay\n" );
      return -OA_ERR_CAMERA_IO;
    }
  }

  return -OA_ERR_NONE;
}


static int
_processGetStrobeControl ( PGE_STATE* cameraInfo, OA_COMMAND* command )
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
_processSetStrobeControl ( PGE_STATE* cameraInfo, OA_COMMAND* command,
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
      fprintf ( stderr, "Can't set PGE strobe control\n" );
      return -OA_ERR_CAMERA_IO;
    }
  }

  return -OA_ERR_NONE;
}


static int
_doBitDepth ( PGE_STATE* cameraInfo, int bitDepth )
{
  fc2GigEImageSettings	settings;
  int			bpp = 1, restart;

  // Affected values here are MONO8, MONO16, MONO12, RAW8 and RAW16

  // FIX ME -- handle MONO12 rather more nicely here

  if (( *p_fc2GetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't get PGE image settings\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if ( 8 == bitDepth ) {
    switch ( cameraInfo->currentVideoFormat ) {

      case FC2_PIXEL_FORMAT_MONO16:
      case FC2_PIXEL_FORMAT_MONO12:
        if ( cameraInfo->pixelFormats & FC2_PIXEL_FORMAT_MONO8 ) {
          settings.pixelFormat = FC2_PIXEL_FORMAT_MONO8;
          bpp = 1;
        } else {
          fprintf ( stderr, "%s: no 8-bit mono format available\n",
              __FUNCTION__ );
          return -OA_ERR_OUT_OF_RANGE;
        }
        break;

      case FC2_PIXEL_FORMAT_RAW16:
        if ( cameraInfo->pixelFormats & FC2_PIXEL_FORMAT_RAW8 ) {
          settings.pixelFormat = FC2_PIXEL_FORMAT_RAW8;
          bpp = 1;
        } else {
          fprintf ( stderr, "%s: no 8-bit raw format available\n",
              __FUNCTION__ );
          return -OA_ERR_OUT_OF_RANGE;
        }
        break;

      default:
        fprintf ( stderr, "%s: can't handle switch to 8-bit from %d\n",
            __FUNCTION__, cameraInfo->currentVideoFormat );
        return -OA_ERR_OUT_OF_RANGE;
        break;
    }
  }

  if ( 16 == bitDepth ) {

    switch ( cameraInfo->currentVideoFormat ) {
      case FC2_PIXEL_FORMAT_MONO8:
        if ( cameraInfo->pixelFormats & FC2_PIXEL_FORMAT_MONO16 ) {
          settings.pixelFormat = FC2_PIXEL_FORMAT_MONO16;
          bpp = 2;
        } else {
          if ( cameraInfo->pixelFormats & FC2_PIXEL_FORMAT_MONO12 ) {
            settings.pixelFormat = FC2_PIXEL_FORMAT_MONO12;
          bpp = 2;
          } else {
            fprintf ( stderr, "%s: no 16-bit mono format available\n",
                __FUNCTION__ );
            return -OA_ERR_OUT_OF_RANGE;
          }
        }
        break;

      case FC2_PIXEL_FORMAT_RAW8:
        if ( cameraInfo->pixelFormats & FC2_PIXEL_FORMAT_RAW16 ) {
          settings.pixelFormat = FC2_PIXEL_FORMAT_RAW16;
          bpp = 2;
        } else {
          fprintf ( stderr, "%s: no 16-bit raw format available\n",
              __FUNCTION__ );
          return -OA_ERR_OUT_OF_RANGE;
        }
        break;

      default:
        fprintf ( stderr, "%s: can't handle switch to 16-bit from %d\n",
            __FUNCTION__, cameraInfo->currentVideoFormat );
        return -OA_ERR_OUT_OF_RANGE;
        break;
    }
  }

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

  if (( *p_fc2SetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
      FC2_ERROR_OK ) {
    fprintf ( stderr, "Can't set PGE image settings\n" );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->currentVideoFormat = settings.pixelFormat;
  cameraInfo->currentBytesPerPixel = bpp;
  cameraInfo->imageBufferLength = cameraInfo->xSize * cameraInfo->ySize * bpp;

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}
