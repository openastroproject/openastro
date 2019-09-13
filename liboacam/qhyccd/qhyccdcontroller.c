/*****************************************************************************
 *
 * qhyccdcontroller.c -- Main camera controller thread
 *
 * Copyright 2019  James Fidell (james@openastroproject.org)
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
#include <sys/time.h>

#include <openastro/camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "qhyccdprivate.h"
#include "qhyccdoacam.h"
#include "qhyccdstate.h"


static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processGetControl ( QHYCCD_STATE*, OA_COMMAND* );
static int	_processSetResolution ( QHYCCD_STATE*, OA_COMMAND* );
static int	_processSetROI ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( QHYCCD_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( QHYCCD_STATE*, OA_COMMAND* );
static int	_doStart ( QHYCCD_STATE* );
static int	_doStop ( QHYCCD_STATE* );
static int	_setBinning ( QHYCCD_STATE*, int );
static int	_setFrameFormat ( QHYCCD_STATE*, int );


void*
oacamQHYCCDcontroller ( void* param )
{
  oaCamera*		camera = param;
  QHYCCD_STATE*	cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode;
  int			streaming = 0;
	int			imageBufferLength, haveFrame, frameWait;
	int			buffersFree, nextBuffer;

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
      imageBufferLength = cameraInfo->imageBufferLength;
      frameWait = cameraInfo->currentAbsoluteExposure;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

      // convert frameWait from microseconds to milliseconds
      // if it is more than 100ms then set it to 100ms and that
      // is the longest we will wait before seeing the thread
      // killed

      frameWait /= 1000;
      if ( frameWait > 100 ) {
        frameWait = 100;
      }

      pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
      buffersFree = cameraInfo->buffersFree;
      pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

      if ( buffersFree ) {
				uint32_t	w, h, bpp, channels;

        nextBuffer = cameraInfo->nextBuffer;
        haveFrame = 0;
        if ( p_GetQHYCCDLiveFrame ( cameraInfo->handle, &w, &h, &bpp,
						&channels, (uint8_t*) cameraInfo->buffers[ nextBuffer ].start ) ==
						QHYCCD_SUCCESS ) {
          haveFrame = 1;
        }

        pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
        exitThread = cameraInfo->stopControllerThread;
        pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

        if ( !exitThread ) {
					if ( haveFrame ) {
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
					} else {
						usleep ( frameWait );
					}
        }
      }
    }
  } while ( !exitThread );

  return 0;
}


static int
_processSetControl ( oaCamera* camera, OA_COMMAND* command )
{
  QHYCCD_STATE*	cameraInfo = camera->_private;
  oaControlValue	*valp = command->commandData;
  int			control = command->controlId;
	int								val;
	unsigned int			found, i;
	float		val_s32, val_s64;

	if ( OA_CAM_CTRL_FRAME_FORMAT == control ) {
		if ( valp->valueType != OA_CTRL_TYPE_DISCRETE ) {
			fprintf ( stderr, "%s: invalid control type %d where discrete "
					"expected\n", __FUNCTION__, valp->valueType );
			return -OA_ERR_INVALID_CONTROL_TYPE;
		}
		val = valp->discrete;
		if ( !camera->frameFormats[ val ] ) {
			return -OA_ERR_OUT_OF_RANGE;
		}
		return _setFrameFormat ( cameraInfo, val );
	}

	if ( OA_CAM_CTRL_BINNING == control ) {
		if ( valp->valueType != OA_CTRL_TYPE_DISCRETE ) {
			fprintf ( stderr, "%s: invalid control type %d where discrete "
					"expected\n", __FUNCTION__, valp->valueType );
			return -OA_ERR_INVALID_CONTROL_TYPE;
		}
		val = valp->discrete;
		return _setBinning ( cameraInfo, val );
	}

	found = 0;
  for ( i = 0; i < numQHYControls && !found; i++ ) {
		if ( QHYControlData[i].oaControl == control ) {
			found = 1;
			if ( valp->valueType != QHYControlData[i].oaControlType ) {
				fprintf ( stderr, "%s: invalid control type %d where %d expected "
						"for control %d\n", __FUNCTION__, valp->valueType,
						QHYControlData[i].oaControlType, control );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}

			switch ( QHYControlData[i].oaControlType ) {
				case OA_CTRL_TYPE_INT32:
					val_s32 = valp->int32;
					val_s32 /= QHYControlData[i].multiplier;
					if ( p_SetQHYCCDParam ( cameraInfo->handle,
							QHYControlData[i].qhyControl, val_s32 ) != QHYCCD_SUCCESS ) {
						fprintf ( stderr, "QHYCCD: Set control %d to %f failed\n",
								QHYControlData[i].qhyControl, val_s32 );
						return -OA_ERR_CAMERA_IO;
					}
					return OA_ERR_NONE;
					break;
				case OA_CTRL_TYPE_INT64:
					val_s64 = valp->int64;
					val_s64 /= QHYControlData[i].multiplier;
					if ( p_SetQHYCCDParam ( cameraInfo->handle,
							QHYControlData[i].qhyControl, val_s64 ) != QHYCCD_SUCCESS ) {
						fprintf ( stderr, "QHYCCD: Set control %d to %f failed\n",
								QHYControlData[i].qhyControl, val_s64 );
						return -OA_ERR_CAMERA_IO;
					}
					if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == control ) {
						cameraInfo->currentAbsoluteExposure = valp->int64;
					}
					return OA_ERR_NONE;
					break;
			}
		}
	}

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );
  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( QHYCCD_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*valp = command->resultData;
  int			control = command->controlId;
	float		qhyccdval;
	unsigned int			i, found;

	found = 0;
  for ( i = 0; i < numQHYControls && !found; i++ ) {
		if ( QHYControlData[i].oaControl == control ) {
			found = 1;
			valp->valueType = QHYControlData[i].oaControlType;
			qhyccdval = p_GetQHYCCDParam ( cameraInfo->handle,
					QHYControlData[i].qhyControl );
			switch ( QHYControlData[ found ].oaControlType ) {
				case OA_CTRL_TYPE_INT32:
					valp->int32 = qhyccdval * QHYControlData[ found ].multiplier;
					return OA_ERR_NONE;
					break;
				case OA_CTRL_TYPE_INT64:
					valp->int64 = qhyccdval * QHYControlData[ found ].multiplier;
					return OA_ERR_NONE;
					break;
			}
		}
	}

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );
  return -OA_ERR_INVALID_CONTROL;
}


static int
_processSetResolution ( QHYCCD_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMESIZE*			size = command->commandData;
  unsigned int			s, restart = 0;
  int				found;

  found = -1;
  for ( s = 0; s < cameraInfo->frameSizes[ cameraInfo->binMode ].numSizes;
      s++ ) {
    if ( cameraInfo->frameSizes[ cameraInfo->binMode ].sizes[ s ].x == size->x
        && cameraInfo->frameSizes[ cameraInfo->binMode ].sizes[ s ].y ==
        size->y ) {
      found = s;
      break;
    }
  }

  if ( found < 0 ) {
    return -OA_ERR_OUT_OF_RANGE;
  }

  if ( cameraInfo->isStreaming ) {
    restart = 1;
		_doStop ( cameraInfo );
  }

  // Reset the ROI

	if ( p_SetQHYCCDResolution ( cameraInfo->handle, 0, 0, size->x, size->y ) !=
			QHYCCD_SUCCESS ) {
    fprintf ( stderr, "Can't set QHYCCD frame size %dx%d\n", size->x,
      size->y );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->currentXSize = cameraInfo->currentXResolution = size->x;
  cameraInfo->currentYSize = cameraInfo->currentYResolution = size->y;
  cameraInfo->imageBufferLength = cameraInfo->currentXSize *
      cameraInfo->currentYSize * cameraInfo->currentBytesPerPixel;

  if ( restart ) {
		return _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}


static int
_processSetROI ( oaCamera* camera, OA_COMMAND* command )
{
  QHYCCD_STATE*		cameraInfo = camera->_private;
  FRAMESIZE*			size = command->commandData;
  unsigned int		offsetX, offsetY, x, y, restart;

  if (!( camera->features.flags & OA_CAM_FEATURE_ROI )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  x = size->x;
  y = size->y;

  if (( x % cameraInfo->binMode ) != 0 || ( y % cameraInfo->binMode ) != 0 ) {
    return -OA_ERR_OUT_OF_RANGE;
  }

  offsetX = ( cameraInfo->currentXResolution - x ) / 2;
  offsetY = ( cameraInfo->currentYResolution - y ) / 2;

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

	if ( p_SetQHYCCDResolution ( cameraInfo->handle, 0, 0, size->x, size->y ) !=
			QHYCCD_SUCCESS ) {
    fprintf ( stderr, "Can't set QHYCCD ROI ( %d, %d, %d, %d\n", offsetX,
				offsetY, size->x, size->y );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->currentXSize = x;
  cameraInfo->currentYSize = y;
  cameraInfo->imageBufferLength = cameraInfo->currentXSize *
      cameraInfo->currentYSize * cameraInfo->currentBytesPerPixel;

  if ( restart ) {
		return _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}


static int
_processStreamingStart ( QHYCCD_STATE* cameraInfo, OA_COMMAND* command )
{
  CALLBACK*		cb = command->commandData;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

  cameraInfo->imageBufferLength = cameraInfo->currentXSize *
      cameraInfo->currentYSize * cameraInfo->currentBytesPerPixel;

  return _doStart ( cameraInfo );
}


static int
_doStart ( QHYCCD_STATE* cameraInfo )
{
  int			ret;

  if (( ret = p_BeginQHYCCDLive ( cameraInfo->handle )) != QHYCCD_SUCCESS ) {
    fprintf ( stderr, "%s: BeginQHYCCDLive failed: %d\n", __FUNCTION__, ret );
    return -OA_ERR_CAMERA_IO;
	}

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}


static int
_processStreamingStop ( QHYCCD_STATE* cameraInfo, OA_COMMAND* command )
{
  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  return _doStop ( cameraInfo );
}


static int
_doStop ( QHYCCD_STATE* cameraInfo )
{
  int		ret;

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  if (( ret = p_StopQHYCCDLive ( cameraInfo->handle )) != QHYCCD_SUCCESS ) {
    fprintf ( stderr, "%s: StopQHYCCDLive failed: %d\n", __FUNCTION__, ret );
    return -OA_ERR_CAMERA_IO;
  }
  return OA_ERR_NONE;
}


static int
_setBinning ( QHYCCD_STATE* cameraInfo, int binMode )
{
  int		x, y;

  if ( binMode < 0 || binMode > OA_MAX_BINNING ||
      cameraInfo->frameSizes[ binMode ].numSizes < 1 ) {
    return -OA_ERR_OUT_OF_RANGE;
  }

  x = cameraInfo->frameSizes[ binMode ].sizes[0].x;
  y = cameraInfo->frameSizes[ binMode ].sizes[0].y;
	if ( p_SetQHYCCDBinMode ( cameraInfo->handle, binMode, binMode ) !=
			QHYCCD_SUCCESS ) {
    fprintf ( stderr, "Can't set bin mode %d\n", binMode );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->binMode = binMode;
  cameraInfo->currentXSize = cameraInfo->currentXResolution = x;
  cameraInfo->currentYSize = cameraInfo->currentYResolution = y;

  return OA_ERR_NONE;
}


static int
_setFrameFormat ( QHYCCD_STATE* cameraInfo, int format )
{
  int           bitspp;

	// Handle change of bit depth
	bitspp = oaFrameFormats[ format ].bitsPerPixel;
   if ( p_SetQHYCCDBitsMode ( cameraInfo->handle, bitspp ) != QHYCCD_SUCCESS ) {
     fprintf ( stderr, "SetQHYCCDBitsMode ( transferbit, %d ) fails\n",
				bitspp );
		return -OA_ERR_CAMERA_IO;
	}

	if ( cameraInfo->colour ) {
		// Colour can also switch between raw and RGB
    if ( p_SetQHYCCDDebayerOnOff ( cameraInfo->handle,
					oaFrameFormats[ format ].rawColour ? 0 : 1 ) != QHYCCD_SUCCESS ) {
      fprintf ( stderr, "p_SetQHYCCDDebayerOnOff ( %d ) returns error\n",
          cameraInfo->colour );
			return -OA_ERR_CAMERA_IO;
		}
  }

  cameraInfo->currentVideoFormat = format;
  cameraInfo->currentBitsPerPixel = bitspp;
  // This converts from float, but should be ok for these cameras
  cameraInfo->currentBytesPerPixel = oaFrameFormats[ format ].bytesPerPixel;
  cameraInfo->imageBufferLength = cameraInfo->currentXSize *
      cameraInfo->currentYSize * cameraInfo->currentBytesPerPixel;

  return OA_ERR_NONE;
}
