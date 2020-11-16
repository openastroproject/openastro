/*****************************************************************************
 *
 * controller.c -- Main camera controller thread
 *
 * Copyright 2020
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

#include <pylonc/PylonC.h>
#include <pthread.h>
#include <sys/time.h>

#include <openastro/camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "oacam.h"
#include "state.h"
#include "private.h"


static int	_processSetControl ( PYLON_STATE*, OA_COMMAND* );
static int	_processGetControl ( PYLON_STATE*, OA_COMMAND* );
static int	_processSetROI ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( PYLON_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( PYLON_STATE*, OA_COMMAND* );
static int	_doBinning ( PYLON_STATE*, int );
static int	_doFrameFormat ( PYLON_STATE*, int );
static int	_doStop ( PYLON_STATE* );
static int	_doStart ( PYLON_STATE* );

void*
oacamPylonController ( void* param )
{
  oaCamera*							camera = param;
  PYLON_STATE*					cameraInfo = camera->_private;
  OA_COMMAND*						command;
  int										exitThread = 0;
  int										resultCode, nextBuffer, buffersFree;
  int										streaming = 0;
	PylonGrabResult_t			grab;
	_Bool									haveFrame;
	unsigned int					frameWait, bufferIdx = 0;
	const unsigned char*	frame = 0;

  do {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    exitThread = cameraInfo->stopControllerThread;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( exitThread ) {
      break;
    } else {
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      // stop us busy-waiting
      streaming = ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) ? 1 : 0;
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
          case OA_CMD_ROI_SET:
            resultCode = _processSetROI ( camera, command );
            break;
          case OA_CMD_START_STREAMING:
            resultCode = _processStreamingStart ( cameraInfo, command );
            break;
          case OA_CMD_STOP_STREAMING:
            resultCode = _processStreamingStop ( cameraInfo, command );
            break;
					case OA_CMD_RESOLUTION_SET:
						resultCode = OA_ERR_NONE;
						break;
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

		if ( streaming ) {

			pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
			frameWait = cameraInfo->currentAbsoluteExposure;
			pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

			if ( frameWait > 100 ) {
				frameWait = 100;
			}

			pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
			buffersFree = cameraInfo->buffersFree;
			pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

			if ( buffersFree ) {
				nextBuffer = cameraInfo->nextBuffer;
				haveFrame = 0;

				if ( p_PylonWaitObjectWait ( cameraInfo->waitHandle, frameWait,
						&haveFrame ) == GENAPI_E_OK ) {
					if ( haveFrame ) {
						if ( p_PylonStreamGrabberRetrieveResult ( cameraInfo->grabberHandle,
								&grab, &haveFrame ) == GENAPI_E_OK ) {
							if ( haveFrame ) {
								if ( grab.Status == Grabbed ) {
									frame = grab.pBuffer;
									bufferIdx = *(( unsigned int* ) grab.Context );
								} else {
									fprintf ( stderr, "grab status was not Grabbed\n" );
								}
							} else {
								fprintf ( stderr, "no frame when a frame should be ready?\n" );
							}
						} else {
							fprintf ( stderr, "PylonStreamGrabberRetrieveResult failed\n" );
						}
					}
				} else {
					fprintf ( stderr, "PylonWaitObjectWait failed\n" );
				}

				if ( !exitThread && haveFrame ) {
					cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
							OA_CALLBACK_NEW_FRAME;
					cameraInfo->frameCallbacks[ nextBuffer ].callback =
							cameraInfo->streamingCallback.callback;
					cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
							cameraInfo->streamingCallback.callbackArg;
					cameraInfo->frameCallbacks[ nextBuffer ].buffer = ( void* ) frame;
					cameraInfo->frameCallbacks[ nextBuffer ].bufferLen =
							cameraInfo->imageBufferLength;
					cameraInfo->frameCallbacks[ nextBuffer ].bufferIdx = bufferIdx;
					pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
					oaDLListAddToTail ( cameraInfo->callbackQueue,
							&cameraInfo->frameCallbacks[ nextBuffer ]);
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
_processSetControl ( PYLON_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*val = command->commandData;
  int							control = command->controlId;

	// At present we have to worry about:
	//   binning
	//   hflip, vflip
	//   gain +auto
	//   exposure time (abs/unscaled) +auto
	//   frame format

	switch ( control ) {

		case OA_CAM_CTRL_BINNING:
			if ( OA_CTRL_TYPE_INT32 != val->valueType ) {
				fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
						__FUNCTION__, val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			return _doBinning ( cameraInfo, val->int32 );
			break;

		case OA_CAM_CTRL_HFLIP:
			if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
				fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
						__FUNCTION__, val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			if ( p_PylonDeviceSetBooleanFeature ( cameraInfo->deviceHandle,
					"ReverseX", val->boolean ) != GENAPI_E_OK ) {
				fprintf ( stderr, "HFLIP failed\n" );
			}
			break;

		case OA_CAM_CTRL_VFLIP:
			if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
				fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
						__FUNCTION__, val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			if ( p_PylonDeviceSetBooleanFeature ( cameraInfo->deviceHandle,
					"ReverseY", val->boolean ) != GENAPI_E_OK ) {
				fprintf ( stderr, "VFLIP failed\n" );
			}
			break;

		case OA_CAM_CTRL_GAIN:
			if ( cameraInfo->gainIsFloat ) {
				fprintf ( stderr, "float gain value not currently supported\n" );
			} else {
				if ( OA_CTRL_TYPE_INT32 != val->valueType ) {
					fprintf ( stderr,
						"%s: invalid control type %d where int32 expected\n",
						__FUNCTION__, val->valueType );
					return -OA_ERR_INVALID_CONTROL_TYPE;
				}
				if ( p_PylonDeviceSetIntegerFeature ( cameraInfo->deviceHandle,
						"GainRaw", val->int32 ) != GENAPI_E_OK ) {
					fprintf ( stderr, "set GainRaw failed\n" );
				}
			}
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
			if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
				fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
						__FUNCTION__, val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			if ( p_PylonDeviceFeatureFromString ( cameraInfo->deviceHandle,
					"GainAuto", val->boolean ? "Continuous" : "Off" ) != GENAPI_E_OK ) {
				fprintf ( stderr, "set GainAuto failed\n" );
			}
			break;

		case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
			if ( OA_CTRL_TYPE_INT64 != val->valueType ) {
				fprintf ( stderr, "%s: invalid control type %d where int64 expected\n",
						__FUNCTION__, val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			if ( p_PylonDeviceSetFloatFeature ( cameraInfo->deviceHandle,
					cameraInfo->exposureTimeName, ( double ) val->int64 ) !=
					GENAPI_E_OK ) {
				fprintf ( stderr, "set %s failed\n", cameraInfo->exposureTimeName );
			} else {
				cameraInfo->currentAbsoluteExposure = val->int64;
			}
			break;

		case OA_CAM_CTRL_EXPOSURE_UNSCALED:
			if ( OA_CTRL_TYPE_INT64 != val->valueType ) {
				fprintf ( stderr, "%s: invalid control type %d where int64 expected\n",
						__FUNCTION__, val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			if ( p_PylonDeviceSetIntegerFeature ( cameraInfo->deviceHandle,
					"ExposureTimeRaw", val->int64 ) != GENAPI_E_OK ) {
				fprintf ( stderr, "set ExposureTimeRaw failed\n" );
			}
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ):
			if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
				fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
						__FUNCTION__, val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			if ( p_PylonDeviceFeatureFromString ( cameraInfo->deviceHandle,
					"ExposureAuto", val->boolean ? "Continuous" : "Off" ) !=
					GENAPI_E_OK ) {
				fprintf ( stderr, "set ExposureAuto failed\n" );
			}
			break;

		case OA_CAM_CTRL_FRAME_FORMAT:
			if ( OA_CTRL_TYPE_DISCRETE != val->valueType ) {
				fprintf ( stderr,
						"%s: invalid control type %d where discrete expected\n",
          __FUNCTION__, val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			return _doFrameFormat ( cameraInfo, val->discrete );
			break;

		default:
			fprintf ( stderr, "Unrecognised control %d in %s\n", control,
					__FUNCTION__ );
			return -OA_ERR_INVALID_CONTROL;
			break;
  }

	return OA_ERR_NONE;
}


static int
_processGetControl ( PYLON_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

	switch ( control ) {

		case OA_CAM_CTRL_BINNING:
		{
			int64_t		curr;

			val->valueType = OA_CTRL_TYPE_INT32;
			// Just get the horizontal value here as we don't currently support
			// binning differently in each direction
			if (( p_PylonDeviceGetIntegerFeature )( cameraInfo->deviceHandle,
					"BinningHorizontal", &curr ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Get binning failed\n" );
			}
			val->int32 = curr;
			break;
		}
		case OA_CAM_CTRL_HFLIP:
		{
			_Bool			curr;

			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			if ( p_PylonDeviceGetBooleanFeature ( cameraInfo->deviceHandle,
					"ReverseX", &curr ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Get HFLIP failed\n" );
			}
			val->boolean = curr ? 1 : 0;
			break;
		}
		case OA_CAM_CTRL_VFLIP:
		{
			_Bool			curr;

			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			if ( p_PylonDeviceGetBooleanFeature ( cameraInfo->deviceHandle,
					"ReverseY", &curr ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Get VFLIP failed\n" );
			}
			val->boolean = curr ? 1 : 0;
			break;
		}
		case OA_CAM_CTRL_GAIN:
		{
			int64_t			curr;

			val->valueType = OA_CTRL_TYPE_INT32;
			if ( cameraInfo->gainIsFloat ) {
				fprintf ( stderr, "float gain value not currently supported\n" );
			} else {
				if ( OA_CTRL_TYPE_INT32 != val->valueType ) {
					fprintf ( stderr,
						"%s: invalid control type %d where int32 expected\n",
						__FUNCTION__, val->valueType );
					return -OA_ERR_INVALID_CONTROL_TYPE;
				}
				if ( p_PylonDeviceGetIntegerFeature ( cameraInfo->deviceHandle,
						"GainRaw", &curr ) != GENAPI_E_OK ) {
					fprintf ( stderr, "get GainRaw failed\n" );
				}
				val->int32 = curr;
			}
			break;
		}
		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
		{
			_Bool			curr;

			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			if ( p_PylonDeviceGetBooleanFeature ( cameraInfo->deviceHandle,
					"GainAuto", &curr ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Get GainAuto failed\n" );
			}
			val->boolean = curr ? 1 : 0;
			break;
		}
		case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
		{
			double		curr;

			val->valueType = OA_CTRL_TYPE_INT64;
			if ( p_PylonDeviceGetFloatFeature ( cameraInfo->deviceHandle,
					cameraInfo->exposureTimeName, &curr ) != GENAPI_E_OK ) {
				fprintf ( stderr, "get %s failed\n", cameraInfo->exposureTimeName );
			}
			val->int64 = curr;
			break;
		}
		case OA_CAM_CTRL_EXPOSURE_UNSCALED:
		{
			int64_t		curr;

			val->valueType = OA_CTRL_TYPE_INT64;
			if ( p_PylonDeviceGetIntegerFeature ( cameraInfo->deviceHandle,
					"ExposureTimeRaw", &curr ) != GENAPI_E_OK ) {
				fprintf ( stderr, "get %s failed\n", cameraInfo->exposureTimeName );
			}
			val->int64 = curr;
			break;
		}
		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ):
		{
			_Bool			curr;

			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			if ( p_PylonDeviceGetBooleanFeature ( cameraInfo->deviceHandle,
					"ExposureAuto", &curr ) != GENAPI_E_OK ) {
				fprintf ( stderr, "Get ExposureAuto failed\n" );
			}
			val->boolean = curr ? 1 : 0;
			break;
		}
		default:
			fprintf ( stderr, "Unrecognised control %d in %s\n", control,
					__FUNCTION__ );
			return -OA_ERR_INVALID_CONTROL;
			break;
  }

  return OA_ERR_NONE;
}


static int
_processSetROI ( oaCamera* camera, OA_COMMAND* command )
{
	PYLON_STATE*	cameraInfo = camera->_private;
  FRAMESIZE*		size = command->commandData;
	unsigned int	maxX, maxY, offsetX, offsetY;
  unsigned int	restart = 0;

	if (!( camera->features.flags & OA_CAM_FEATURE_ROI )) {
		return -OA_ERR_INVALID_CONTROL;
	}

  if ( size->x == cameraInfo->xSize && size->y == cameraInfo->ySize ) {
    return OA_ERR_NONE;
  }

	if (( size->x - cameraInfo->minResolutionX ) % cameraInfo->xSizeStep != 0 ) {
		return -OA_ERR_OUT_OF_RANGE;
  }
	if (( size->y - cameraInfo->minResolutionY ) % cameraInfo->ySizeStep != 0 ) {
		return -OA_ERR_OUT_OF_RANGE;
  }

	maxX = cameraInfo->maxResolutionX / cameraInfo->binMode;
	maxY = cameraInfo->maxResolutionY / cameraInfo->binMode;
	if ( size->x > maxX || size->y > maxY ) {
		return -OA_ERR_OUT_OF_RANGE;
	}
	offsetX = ( cameraInfo->maxResolutionX - size->x ) / 2;
	offsetY = ( cameraInfo->maxResolutionY - size->y ) / 2;

  if ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

	if (( p_PylonDeviceSetIntegerFeature )( cameraInfo->deviceHandle,
			"Width", size->x ) != GENAPI_E_OK ) {
		fprintf ( stderr, "set Width failed\n" );
	}
	if (( p_PylonDeviceSetIntegerFeature )( cameraInfo->deviceHandle,
			"Height", size->y ) != GENAPI_E_OK ) {
		fprintf ( stderr, "set Height failed\n" );
	}
	if (( p_PylonDeviceSetIntegerFeature )( cameraInfo->deviceHandle,
			"OffsetX", offsetX ) != GENAPI_E_OK ) {
		fprintf ( stderr, "set OffsetX failed\n" );
	}
	if (( p_PylonDeviceSetIntegerFeature )( cameraInfo->deviceHandle,
			"OffsetY", offsetY ) != GENAPI_E_OK ) {
		fprintf ( stderr, "set OffsetY failed\n" );
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
_processStreamingStart ( PYLON_STATE* cameraInfo, OA_COMMAND* command )
{
  CALLBACK*		cb = command->commandData;
	size_t			numStreams;

  if ( cameraInfo->runMode != CAM_RUN_MODE_STOPPED ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

	if ( p_PylonDeviceFeatureFromString ( cameraInfo->deviceHandle,
			"AcquisitionMode", "Continuous" ) != GENAPI_E_OK ) {
		fprintf ( stderr, "set AcquisitionMode failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( p_PylonDeviceGetNumStreamGrabberChannels ( cameraInfo->deviceHandle,
			&numStreams ) != GENAPI_E_OK ) {
		fprintf ( stderr, "PylonDeviceGetNumStreamGrabberChannels failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( numStreams < 1 ) {
		fprintf ( stderr, "PylonDeviceGetNumStreamGrabberChannels returns %d\n",
				( int ) numStreams );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if ( p_PylonDeviceGetStreamGrabber ( cameraInfo->deviceHandle, 0,
			&cameraInfo->grabberHandle ) != GENAPI_E_OK ) {
		fprintf ( stderr, "PylonDeviceGetStreamGrabber failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( p_PylonStreamGrabberOpen ( cameraInfo->grabberHandle ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "PylonStreamGrabberOpen failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( p_PylonStreamGrabberGetWaitObject ( cameraInfo->grabberHandle,
			&cameraInfo->waitHandle ) != GENAPI_E_OK ) {
		fprintf ( stderr, "PylonStreamGrabberGetWaitObject failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}

  return _doStart ( cameraInfo );
}


static int
_doStart ( PYLON_STATE* cameraInfo )
{
	size_t					payloadSize, i;
	GENAPIC_RESULT	res;

	if (( res = p_PylonStreamGrabberGetPayloadSize ( cameraInfo->deviceHandle,
			cameraInfo->grabberHandle, &payloadSize )) != GENAPI_E_OK ) {
		unsigned int r = res;
		fprintf ( stderr, "PylonStreamGrabberGetPayloadSize failed: %08x\n", r );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( cameraInfo->imageBufferLength != payloadSize ) {
		if ( cameraInfo->imageBufferLength < payloadSize ) {
			for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
				if ( cameraInfo->buffers[i].start ) {
					cameraInfo->buffers[i].start = 0;
				}
				// FIX ME -- check for errors
				cameraInfo->buffers[i].start = realloc (
						cameraInfo->buffers[i].start, payloadSize );
			}
		}
		if ( p_PylonStreamGrabberSetMaxNumBuffer ( cameraInfo->grabberHandle,
				OA_CAM_BUFFERS ) != GENAPI_E_OK ) {
			fprintf ( stderr, "PylonStreamGrabberSetMaxNumBuffer failed\n" );
			// free buffers?
			return -OA_ERR_SYSTEM_ERROR;
		}
		if (( res = p_PylonStreamGrabberSetMaxBufferSize ( 
				cameraInfo->grabberHandle, payloadSize )) != GENAPI_E_OK ) {
			unsigned int r = res;
			fprintf ( stderr, "PylonStreamGrabberSetMaxBufferSize failed: %08x\n",
					r );
			// free buffers?
			return -OA_ERR_SYSTEM_ERROR;
		}
		cameraInfo->imageBufferLength = payloadSize;
	}

	if ( p_PylonStreamGrabberPrepareGrab ( cameraInfo->grabberHandle ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "PylonStreamGrabberPrepareGrab failed\n" );
		// free buffers?
		return -OA_ERR_SYSTEM_ERROR;
	}

	for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
		if (( res = p_PylonStreamGrabberRegisterBuffer ( cameraInfo->grabberHandle,
				cameraInfo->buffers[i].start, payloadSize,
				&( cameraInfo->bufferHandle[i] ))) != GENAPI_E_OK ) {
			unsigned int r = res;
			fprintf ( stderr, "PylonStreamGrabberRegisterBuffer failed: %08x\n", r );
			// free buffers, deregister buffers?
			return -OA_ERR_SYSTEM_ERROR;
		}
		cameraInfo->ctx[i] = i;
		if ( p_PylonStreamGrabberQueueBuffer ( cameraInfo->grabberHandle,
				cameraInfo->bufferHandle[i],
				( void* ) &( cameraInfo->ctx[i] )) != GENAPI_E_OK ) {
			fprintf ( stderr, "PylonStreamGrabberQueueBuffer failed\n" );
			// free buffers, deregister buffers, dequeue buffers?
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

	if ( p_PylonStreamGrabberStartStreamingIfMandatory (
			cameraInfo->grabberHandle ) != GENAPI_E_OK ) {
		fprintf ( stderr, "PylonStreamGrabberStartStreamingIfMandatory failed\n" );
		// free buffers, deregister buffers, dequeue buffers?
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( p_PylonDeviceExecuteCommandFeature ( cameraInfo->deviceHandle,
			"AcquisitionStart" ) != GENAPI_E_OK ) {
		fprintf ( stderr, "PylonDeviceExecuteCommandFeature failed\n" );
		// free buffers, deregister buffers, dequeue buffers?
		return -OA_ERR_SYSTEM_ERROR;
	}

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->runMode = CAM_RUN_MODE_STREAMING;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  return OA_ERR_NONE;
}


static int
_processStreamingStop ( PYLON_STATE* cameraInfo, OA_COMMAND* command )
{
  if ( cameraInfo->runMode != CAM_RUN_MODE_STREAMING ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  ( void ) _doStop ( cameraInfo );
	if ( p_PylonStreamGrabberClose ( cameraInfo->grabberHandle ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "PylonStreamGrabberClose failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}


static int
_doStop ( PYLON_STATE* cameraInfo )
{
	PylonGrabResult_t			grabResult;
	_Bool									ready;
	size_t								i;

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->runMode = CAM_RUN_MODE_STOPPED;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

	if ( p_PylonDeviceExecuteCommandFeature ( cameraInfo->deviceHandle,
			"AcquisitionStop" ) != GENAPI_E_OK ) {
		fprintf ( stderr, "PylonDeviceExecuteCommandFeature failed\n" );
		// free buffers, deregister buffers, dequeue buffers?
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( p_PylonStreamGrabberStopStreamingIfMandatory (
			cameraInfo->grabberHandle ) != GENAPI_E_OK ) {
		fprintf ( stderr, "PylonStreamGrabberStopStreamingIfMandatory failed\n" );
		// free buffers, deregister buffers, dequeue buffers?
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( p_PylonStreamGrabberFlushBuffersToOutput (
			cameraInfo->grabberHandle ) != GENAPI_E_OK ) {
		fprintf ( stderr, "PylonStreamGrabberFlushBuffersToOutput failed\n" );
		// free buffers, deregister buffers, dequeue buffers?
		return -OA_ERR_SYSTEM_ERROR;
	}

	do {
		if ( p_PylonStreamGrabberRetrieveResult ( cameraInfo->grabberHandle,
					&grabResult, &ready ) != GENAPI_E_OK ) {
			fprintf ( stderr, "PylonStreamGrabberRetrieveResult failed\n" );
			// free buffers, deregister buffers, dequeue buffers?
			return -OA_ERR_SYSTEM_ERROR;
		}
	} while ( ready );

	for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
		if ( p_PylonStreamGrabberDeregisterBuffer ( cameraInfo->grabberHandle,
					cameraInfo->bufferHandle[i] ) != GENAPI_E_OK ) {
			fprintf ( stderr, "PylonStreamGrabberDeregisterBuffer failed\n" );
			// free buffers, deregister buffers, dequeue buffers?
			return -OA_ERR_SYSTEM_ERROR;
		}
	}

	if ( p_PylonStreamGrabberFinishGrab ( cameraInfo->grabberHandle ) !=
			GENAPI_E_OK ) {
		fprintf ( stderr, "PylonStreamGrabberFinishGrab failed\n" );
		// free buffers, deregister buffers, dequeue buffers?
		return -OA_ERR_SYSTEM_ERROR;
	}

  return OA_ERR_NONE;
}


static int
_doBinning ( PYLON_STATE* cameraInfo, int binMode )
{
	int				oldBinMode = cameraInfo->binMode;
	int				restart = 0, err = OA_ERR_NONE;
	int64_t		val;

	if ( binMode > cameraInfo->maxBinning ) {
		return -OA_ERR_OUT_OF_RANGE;
	}

  if ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

	if ( p_PylonDeviceSetIntegerFeature ( cameraInfo->deviceHandle,
			"BinningHorizontal", binMode ) != GENAPI_E_OK ) {
		fprintf ( stderr, "set BinningHorizontal failed\n" );
		err = OA_ERR_SYSTEM_ERROR;
	} else {
		if ( p_PylonDeviceSetIntegerFeature ( cameraInfo->deviceHandle,
				"BinningVertical", binMode ) != GENAPI_E_OK ) {
			fprintf ( stderr, "set BinningVertical failed\n" );
			( void ) p_PylonDeviceSetIntegerFeature ( cameraInfo->deviceHandle,
					"BinningHorizontal", oldBinMode );
			err = OA_ERR_SYSTEM_ERROR;
		} else {
			( void ) p_PylonDeviceGetIntegerFeature ( cameraInfo->deviceHandle,
				"Width", &val );
			cameraInfo->xSize = val;
			( void ) p_PylonDeviceGetIntegerFeature ( cameraInfo->deviceHandle,
				"Height", &val );
			cameraInfo->ySize = val;
			cameraInfo->binMode = binMode;
		}
	}

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return -err;
}


static int
_doFrameFormat ( PYLON_STATE* cameraInfo, int format )
{
/*
  fc2GigEImageSettings	settings;
  int			restart = 0;

  if (( p_fc2GetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
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

  if ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

  if (( p_fc2SetGigEImageSettings )( cameraInfo->pgeContext, &settings ) !=
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
*/
  return OA_ERR_NONE;
}
