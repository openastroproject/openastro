/*****************************************************************************
 *
 * dummyController.c -- Main camera controller thread
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
#include <sys/time.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "dummyoacam.h"
#include "dummystate.h"


static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processGetControl ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( DUMMY_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( DUMMY_STATE*, OA_COMMAND* );


void*
oacamDummyController ( void* param )
{
  oaCamera*		camera = param;
  DUMMY_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode, nextBuffer, buffersFree;
  int			imageBufferLength;
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
            resultCode = _processSetControl ( camera, command );
            break;
          case OA_CMD_CONTROL_GET:
            resultCode = _processGetControl ( camera, command );
            break;
          case OA_CMD_START_STREAMING:
            resultCode = _processStreamingStart ( cameraInfo, command );
            break;
          case OA_CMD_STOP_STREAMING:
            resultCode = _processStreamingStop ( cameraInfo, command );
            break;
          default:
            resultCode = -OA_ERR_INVALID_CONTROL;
            break;
        }
        if ( command->callback ) {
//fprintf ( stderr, "CONT: command has callback\n" );
        } else {
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          command->completed = 1;
          command->resultCode = resultCode;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
          pthread_cond_broadcast ( &cameraInfo->commandComplete );
        }
      }
    } while ( command );

    if ( streaming ) {

      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      imageBufferLength = cameraInfo->imageBufferLength;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

      pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
      buffersFree = cameraInfo->buffersFree;
      pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

      if ( buffersFree ) {
        nextBuffer = cameraInfo->nextBuffer;

        pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
        exitThread = cameraInfo->stopControllerThread;
        pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

        if ( !exitThread ) {
          cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
              OA_CALLBACK_NEW_FRAME;
          cameraInfo->frameCallbacks[ nextBuffer ].callback =
              cameraInfo->streamingCallback.callback;
          cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
              cameraInfo->streamingCallback.callbackArg;
          cameraInfo->frameCallbacks[ nextBuffer ].buffer =
              cameraInfo->buffers[ nextBuffer ].start;
          cameraInfo->frameCallbacks[ nextBuffer ].bufferLen =
              imageBufferLength;
          oaDLListAddToTail ( cameraInfo->callbackQueue,
              &cameraInfo->frameCallbacks[ nextBuffer ]);
          pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
          cameraInfo->buffersFree--;
          cameraInfo->nextBuffer = ( nextBuffer + 1 ) %
              cameraInfo->configuredBuffers;
          pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
          pthread_cond_broadcast ( &cameraInfo->callbackQueued );
        }
      }
    }
  } while ( !exitThread );

  return 0;
}


static int
_processSetControl ( oaCamera* camera, OA_COMMAND* command )
{
  DUMMY_STATE*		cameraInfo = camera->_private;
  oaControlValue	*val = command->commandData;

  switch ( command->controlId ) {

    case OA_CAM_CTRL_BRIGHTNESS:
      cameraInfo->currentBrightness = val->int32;
      break;

		/* May return to this one later
    case OA_CAM_CTRL_GAMMA:
      cameraInfo->currentGamma = val->int32;
      break;
			*/

    case OA_CAM_CTRL_GAIN:
      cameraInfo->currentGain = val->int32;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      cameraInfo->currentAbsoluteExposure = val->int32;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
      break;

    case OA_CAM_CTRL_BINNING:
      cameraInfo->binMode = val->discrete;
      break;

    case OA_CAM_CTRL_HFLIP:
      cameraInfo->currentHFlip = val->boolean;
			break;

    case OA_CAM_CTRL_VFLIP:
      cameraInfo->currentVFlip = val->boolean;
      break;

		/* May do this one later
    case OA_CAM_CTRL_TEMP_SETPOINT:
      ASISetControlValue ( cameraInfo->cameraId, ASI_TARGET_TEMP, val->int32,
          0 );
      cameraInfo->currentSetPoint = val->int32;
      break;
			*/

    default:
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


static int
_processGetControl ( oaCamera* camera, OA_COMMAND* command )
{
  DUMMY_STATE*		cameraInfo = camera->_private;
  oaControlValue	*val = command->resultData;

  switch ( command->controlId ) {

    case OA_CAM_CTRL_BRIGHTNESS:
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = cameraInfo->currentBrightness;
      break;

			/* Maybe later
    case OA_CAM_CTRL_GAMMA:
				val->valueType = OA_CTRL_TYPE_INT32;
				val->int32 = cameraInfo->currentGamma;
      break;
			*/

    case OA_CAM_CTRL_GAIN:
				val->valueType = OA_CTRL_TYPE_INT32;
				val->int32 = cameraInfo->currentGain;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
				val->valueType = OA_CTRL_TYPE_INT32;
				val->int32 = cameraInfo->currentAbsoluteExposure;
      break;

    case OA_CAM_CTRL_BINNING:
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = cameraInfo->binMode;
      break;

    case OA_CAM_CTRL_HFLIP:
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->currentHFlip;
			break;

    case OA_CAM_CTRL_VFLIP:
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
      val->boolean = cameraInfo->currentVFlip;
			break;

		/* Maybe later
    case OA_CAM_CTRL_TEMP_SETPOINT:
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = ????;
      break;
			*/

    case OA_CAM_CTRL_TEMPERATURE:
			val->valueType = OA_CTRL_TYPE_INT32;
			// FIX ME -- something more useful?
			val->int32 = -10;
      break;

    default:
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


static int
_processStreamingStart ( DUMMY_STATE* cameraInfo, OA_COMMAND* command )
{
  CALLBACK*	cb = command->commandData;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}


static int
_processStreamingStop ( DUMMY_STATE* cameraInfo, OA_COMMAND* command )
{
  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}
