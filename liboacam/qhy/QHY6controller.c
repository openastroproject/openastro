/*****************************************************************************
 *
 * QHY6controller.c -- Main camera controller thread
 *
 * Copyright 2015,2018,2019 James Fidell (james@openastroproject.org)
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
#include <sys/time.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "QHY.h"
#include "QHYoacam.h"
#include "QHYstate.h"
#include "QHY6.h"
#include "QHYusb.h"


static int	_processSetControl ( QHY_STATE*, OA_COMMAND* );
static int	_processGetControl ( QHY_STATE*, OA_COMMAND* );
static int	_processSetResolution ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStop ( QHY_STATE*, OA_COMMAND* );
static int	_doConfigCamera ( QHY_STATE* );
static int	_doSetResolution ( QHY_STATE*, int, int );
static int	_doStartExposure ( QHY_STATE* );
static int	_doReadExposure ( QHY_STATE* );


void*
oacamQHY6controller ( void* param )
{
  oaCamera*		camera = param;
  QHY_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode, streaming = 0;
  int			maxWaitTime, frameWait;
  int			nextBuffer, buffersFree, rowBytes;
  unsigned int		i, reorderFrame;
  unsigned char*	evenSrc;
  unsigned char*	oddSrc;
  unsigned char*	t;
  unsigned char*	s1;
  unsigned char*	s2;

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
            resultCode = _processSetResolution ( camera, command );
            break;
          case OA_CMD_START_STREAMING:
            resultCode = _processStreamingStart ( camera, command );
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

    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    streaming = cameraInfo->isStreaming;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

    if ( streaming ) {
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      evenSrc = cameraInfo->xferBuffer;
      oddSrc = cameraInfo->xferBuffer + cameraInfo->frameSize / 2;
      rowBytes = cameraInfo->xSize * 2;
      reorderFrame = ( OA_BIN_MODE_2x2 == cameraInfo->binMode ) ? 0 : 1;
      maxWaitTime = frameWait = cameraInfo->currentExposure;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
      _doStartExposure ( cameraInfo );
      if ( frameWait > 1000 ) {
        frameWait = 1000;
      }
      while ( !exitThread && maxWaitTime > 0 ) {
        usleep ( frameWait );
        maxWaitTime -= frameWait;
        pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
        exitThread = cameraInfo->stopControllerThread;
        pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
      }
      if ( !exitThread ) {
        if ( !_doReadExposure ( cameraInfo )) {
          pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
          buffersFree = cameraInfo->buffersFree;
          pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          streaming = cameraInfo->isStreaming;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
          if ( buffersFree && streaming ) {

            nextBuffer = cameraInfo->nextBuffer;

            // If the frame is unbinned it now has to be unpacked.  There
            // are two "half-frames" in the buffer, one of odd-numbered
            // scanlines and one of even

            t = cameraInfo->buffers[ nextBuffer ].start;
            if ( reorderFrame ) {
              s1 = evenSrc;
              s2 = oddSrc;
              for ( i = 0; i < cameraInfo->ySize / 2; i++ ) {
                memcpy ( t, s1, rowBytes );
                t += rowBytes;
                s1 += rowBytes;
                memcpy ( t, s2, rowBytes );
                t += rowBytes;
                s2 += rowBytes;
              }
            } else {
              memcpy ( t, cameraInfo->xferBuffer, cameraInfo->frameSize );
            }

            cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
                OA_CALLBACK_NEW_FRAME;
            cameraInfo->frameCallbacks[ nextBuffer ].callback =
                cameraInfo->streamingCallback.callback;
            cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
                cameraInfo->streamingCallback.callbackArg;
            cameraInfo->frameCallbacks[ nextBuffer ].buffer =
                cameraInfo->buffers[ nextBuffer ].start;
            cameraInfo->frameCallbacks[ nextBuffer ].bufferLen =
                cameraInfo->frameSize;
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
    }
  } while ( !exitThread );

  return 0;
}


static int
_processSetControl ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*val = command->commandData;
  int			control = command->controlId;
  int32_t		val_s64;
  int			updateSettings = 0;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY6: control: %s ( %d, ? )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentGain = val->int64;
      updateSettings = 1;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    {
      uint64_t val_u64;

      if ( val->valueType != OA_CTRL_TYPE_INT64 ) {
        fprintf ( stderr, "%s: invalid control type %d where int64 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s64 = val->int64;
      if ( val_s64 < 1 ) { val_s64 = 1; }
      val_u64 = ( uint64_t ) val_s64;
      cameraInfo->currentExposure = val_u64;
      cameraInfo->correctedExposureTime = val_u64 - val_u64 / 10;
      if ( cameraInfo->correctedExposureTime > 550000 &&
          QHY6_AMP_MODE_AUTO == cameraInfo->requestedAmpMode ) {
        cameraInfo->currentAmpMode = 1;
      } else {
        cameraInfo->currentAmpMode = cameraInfo->requestedAmpMode ? 0 : 1;
      }
      updateSettings = 1;
      break;
    }

    case OA_CAM_CTRL_BINNING:
      if ( val->valueType != OA_CTRL_TYPE_DISCRETE ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      switch ( val->discrete ) {
        case OA_BIN_MODE_NONE:
        case OA_BIN_MODE_2x2:
          cameraInfo->binMode = cameraInfo->horizontalBinMode =
              cameraInfo->verticalBinMode = val->discrete;
          // _recalculateSizes ( camera );
          // don't need to update the settings here as a resolution
          // change should be forthcoming
          break;
        default:
          return -OA_ERR_OUT_OF_RANGE;
          break;
      }
      break;

    case OA_CAM_CTRL_HIGHSPEED:
      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentHighSpeed = val->boolean;
      updateSettings = 1;
      break;

    case OA_CAM_CTRL_DROPPED_RESET:
      // droppedFrames could be mutexed, but it's not the end of the world
      cameraInfo->droppedFrames = 0;
      break;

    case OA_CAM_CTRL_FRAME_FORMAT:
      // nothing to do here
      break;

    default:
      fprintf ( stderr, "QHY6: %s not yet implemented for control %d\n",
          __FUNCTION__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  if ( updateSettings ) {
    _doConfigCamera ( cameraInfo );
  }
  return OA_ERR_NONE;
}


static int
_doConfigCamera ( QHY_STATE* cameraInfo )
{
  unsigned char buff[64];
  unsigned int exposureMillisecs;

  exposureMillisecs = cameraInfo->correctedExposureTime / 1000;
  OA_CLEAR ( buff );
  buff[0] = cameraInfo->currentGain;
  buff[1] = cameraInfo->xOffset;
  buff[2] = ( exposureMillisecs >> 16 ) & 0xff;
  buff[3] = ( exposureMillisecs >> 8 ) & 0xff;
  buff[4] = exposureMillisecs & 0xff;
  buff[5] = cameraInfo->horizontalBinMode;
  buff[6] = cameraInfo->verticalBinMode;
  buff[7] = ( cameraInfo->xSize >> 8 ) & 0xff;
  buff[8] = cameraInfo->xSize & 0xff;
  buff[9] = ( cameraInfo->ySize >> 8 ) & 0xff;
  buff[10] = cameraInfo->ySize & 0xff;
  buff[11] = ( cameraInfo->topOffset >> 8 ) & 0xff;
  buff[12] = cameraInfo->topOffset & 0xff;
  buff[13] = ( cameraInfo->bottomOffset >> 8 ) & 0xff;
  buff[14] = cameraInfo->bottomOffset & 0xff;

  buff[17] = ( cameraInfo->transferPadding >> 8 ) & 0xff;
  buff[18] = cameraInfo->transferPadding & 0xff;

  buff[32] = cameraInfo->currentAmpMode;
  buff[33] = cameraInfo->currentHighSpeed;

  buff[46] = 30;

  buff[58] = 100;
  _usbControlMsg ( cameraInfo, QHY_CMD_CLEAR_FEATURE, QHY_REQ_SET_REGISTERS,
      0, 0, buff, sizeof ( buff ), USB1_CTRL_TIMEOUT );
  return OA_ERR_NONE;
}


static int
_processSetResolution ( oaCamera* camera, OA_COMMAND* command )
{
  QHY_STATE*	cameraInfo = camera->_private;
  FRAMESIZE*	size = command->commandData;

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;

  _doSetResolution ( cameraInfo, cameraInfo->xSize, cameraInfo->ySize );
  return OA_ERR_NONE;
}


static int
_doSetResolution ( QHY_STATE* cameraInfo, int x, int y )
{
  oaQHY6RecalculateSizes ( cameraInfo );
  _doConfigCamera ( cameraInfo );
  cameraInfo->imageBufferLength = cameraInfo->frameSize;
  return OA_ERR_NONE;
}


void
oaQHY6RecalculateSizes ( QHY_STATE* cameraInfo )
{
  unsigned int	transferBlockSize;

  if ( cameraInfo->binMode == OA_BIN_MODE_2x2 ) {
    transferBlockSize = 234 * 1024;
  } else {
    transferBlockSize = 932 * 1024;
  }

  cameraInfo->frameSize = cameraInfo->xSize * cameraInfo->ySize * 2;
  cameraInfo->captureLength = cameraInfo->frameSize;
  cameraInfo->transferPadding = transferBlockSize -
      ( cameraInfo->frameSize % transferBlockSize );
}


static int
_processGetControl ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY5L-II: control: %s ( %d )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentGain;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val->valueType = OA_CTRL_TYPE_INT64;
      val->int64 = cameraInfo->currentExposure;
      break;

    case OA_CAM_CTRL_DROPPED:
      val->valueType = OA_CTRL_TYPE_READONLY;
      val->readonly = cameraInfo->droppedFrames;
      break;

    default:
      fprintf ( stderr,
          "Unrecognised control %d in QHY6:%s\n", control, __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


static int
_processStreamingStart ( oaCamera* camera, OA_COMMAND* command )
{
  QHY_STATE*	cameraInfo = camera->_private;
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
_processStreamingStop ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  int		queueEmpty;

  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  // We wait here until the callback queue has drained otherwise a future
  // close of the camera could rip the image frame out from underneath the
  // callback

  queueEmpty = 0;
  do {
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    queueEmpty = ( OA_CAM_BUFFERS == cameraInfo->buffersFree ) ? 1 : 0;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
    if ( !queueEmpty ) {
      usleep ( 10000 );
    }
  } while ( !queueEmpty );

  return OA_ERR_NONE;
}


static int
_doStartExposure ( QHY_STATE* cameraInfo )
{
  unsigned char buff[8] = "DEADBEEF";
  // FIX ME -- more magic numbers
  buff[0] = 0;
  buff[1] = 100;
  _usbControlMsg ( cameraInfo, QHY_CMD_CLEAR_FEATURE, QHY_REQ_BEGIN_VIDEO,
      0, 0, buff, 2, USB1_CTRL_TIMEOUT );
  return OA_ERR_NONE;
}


static int
_doReadExposure ( QHY_STATE* cameraInfo )
{
  int ret;
  unsigned int readSize, expectedSize;

  expectedSize = cameraInfo->captureLength - 256; // Why?
  ret = _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_IN,
      cameraInfo->xferBuffer, expectedSize, &readSize, USB1_BULK_TIMEOUT );
  if ( ret ) {
    fprintf ( stderr, "readExposure: USB bulk transfer failed, err = %d\n",
        ret );
    cameraInfo->droppedFrames++;
    return -OA_ERR_CAMERA_IO;
  }
  if ( readSize != expectedSize ) {
    fprintf ( stderr, "readExposure: USB bulk transfer was short. %d != %d\n",
        readSize, expectedSize );
    cameraInfo->droppedFrames++;
    return -OA_ERR_CAMERA_IO;
  }
  return OA_ERR_NONE;
}
