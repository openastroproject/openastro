/*****************************************************************************
 *
 * atikSerialcontroller.c -- Main camera controller thread
 *
 * Copyright 2015,2016,2018,2019 James Fidell (james@openastroproject.org)
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
#include <libusb-1.0/libusb.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <sys/time.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "atikSerial.h"
#include "atikSerialoacam.h"
#include "atikSerialstate.h"


static int	_processSetControl ( AtikSerial_STATE*, OA_COMMAND* );
static int	_processGetControl ( AtikSerial_STATE*, OA_COMMAND* );
static int	_processSetResolution ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStop ( AtikSerial_STATE*, OA_COMMAND* );
static int	_doStartExposure ( AtikSerial_STATE* );
static int	_doReadExposure ( AtikSerial_STATE* );


void*
oacamAtikSerialcontroller ( void* param )
{
  oaCamera*		camera = param;
  AtikSerial_STATE*	cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode;
  int			streaming = 0;
  int			maxWaitTime, frameWait;
  int			nextBuffer, buffersFree;
  unsigned char		ampOnCmd[5] = { 'C', 'M', 'D', ATIK_CMD_SET_AMP, 1 };

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
      maxWaitTime = frameWait = cameraInfo->currentExposure;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

      // ATIK_SERIAL_MAX_SHORT_EXPOSURE is the maximum exposure that can
      // be timed on-camera.  If we're doing an exposure timed by the
      // camera hardware then there's no point doing the stuff in the
      // _startExposure() function because the exposure doesn't actually
      // take place until we get to  _readExposure() where we revert some of
      // the stuff done in _startExposure()

      if ( frameWait > ATIK_SERIAL_MAX_SHORT_EXPOSURE ) {
        // FIX ME -- does _startExposure actually need to turn off guiding, or
        // could it be done here as it happens for both exposures timed on-
        // camera and externally?
        _doStartExposure ( cameraInfo );
        // 300ms is some sort of magic number here
        maxWaitTime = frameWait = frameWait - 300000;
        while ( !exitThread && maxWaitTime > 0 ) {
          usleep ( frameWait );
          maxWaitTime -= frameWait;
        }
        if ( cameraInfo->write ( cameraInfo, ampOnCmd, 4 )) {
          fprintf ( stderr, "%s: write error on amp on\n", __FUNCTION__ );
        }
        usleep ( 100000 );
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
            memcpy ( cameraInfo->buffers[ nextBuffer ].start,
                cameraInfo->xferBuffer, cameraInfo->imageBufferLength );
            cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
                OA_CALLBACK_NEW_FRAME;
            cameraInfo->frameCallbacks[ nextBuffer ].callback =
                cameraInfo->streamingCallback.callback;
            cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
                cameraInfo->streamingCallback.callbackArg;
            cameraInfo->frameCallbacks[ nextBuffer ].buffer =
                cameraInfo->buffers[ nextBuffer ].start;
            cameraInfo->frameCallbacks[ nextBuffer ].bufferLen =
                cameraInfo->imageBufferLength;
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
_processSetControl ( AtikSerial_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->commandData;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "atikSerial: control: %s ( %d, ? )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      if ( val->valueType != OA_CTRL_TYPE_INT64 ) {
        fprintf ( stderr, "%s: invalid control type %d where int64 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentExposure = val->int64;
      break;

    case OA_CAM_CTRL_BINNING:
      if ( val->valueType != OA_CTRL_TYPE_DISCRETE ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      if ( val->discrete > OA_BIN_MODE_2x2 ) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->binMode = val->discrete;
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
_processGetControl ( AtikSerial_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "atikSerial: control: %s ( %d )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val->valueType = OA_CTRL_TYPE_INT64;
      val->int64 = cameraInfo->currentExposure;
      break;

    case OA_CAM_CTRL_BINNING:
      val->valueType = OA_CTRL_TYPE_DISCRETE;
      val->discrete = cameraInfo->binMode;
      break;

    case OA_CAM_CTRL_DROPPED:
      val->valueType = OA_CTRL_TYPE_READONLY;
      val->readonly = cameraInfo->droppedFrames;
      break;

    default:
      fprintf ( stderr,
          "Unrecognised control %d in %s\n", control, __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  return OA_ERR_NONE;
}


static int
_processSetResolution ( oaCamera* camera, OA_COMMAND* command )
{
  AtikSerial_STATE*	cameraInfo = camera->_private;
  FRAMESIZE*		size = command->commandData;

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;

  cameraInfo->imageBufferLength = size->x * size->y * 2 / cameraInfo->binMode;

  return OA_ERR_NONE;
}


static int
_processStreamingStart ( oaCamera* camera, OA_COMMAND* command )
{
  AtikSerial_STATE*	cameraInfo = camera->_private;
  CALLBACK*		cb = command->commandData;

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
_processStreamingStop ( AtikSerial_STATE* cameraInfo, OA_COMMAND* command )
{
  int		queueEmpty;

  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  // We wait here until the callback queue has drained otherwise a
  // future close of the camera  could rip the image frame out from
  // underneath the callback

  queueEmpty = 0;
  do {
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    queueEmpty = ( OA_CAM_BUFFERS == cameraInfo->buffersFree ) ? 1 : 0;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
    if ( !queueEmpty ) {
      usleep ( 100 );
    }
  } while ( !queueEmpty );

  return OA_ERR_NONE;
}


static int
_doStartExposure ( AtikSerial_STATE* cameraInfo )
{
  unsigned char		guideOffCmd[4] = { 'C', 'M', 'D',
			    ATIK_CMD_DISABLE_GUIDE };
  unsigned char		ampOffCmd[5] = { 'C', 'M', 'D', ATIK_CMD_SET_AMP, 0 };
  unsigned char		clearCCDCmd[4] = { 'C', 'M', 'D', ATIK_CMD_CLEAR_CCD };
  unsigned char		shutterCmd[4] = { 'C', 'M', 'D',
			    ATIK_CMD_START_EXPOSURE };

  if ( cameraInfo->write ( cameraInfo, ampOffCmd, 5 )) {
    fprintf ( stderr, "%s: write error on amp off\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  usleep ( 100000 );

  if ( cameraInfo->write ( cameraInfo, clearCCDCmd, 4 )) {
    fprintf ( stderr, "%s: write error on clear CCD\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  usleep ( 100000 );

  if ( cameraInfo->write ( cameraInfo, shutterCmd, 4 )) {
    fprintf ( stderr, "%s: write error on shutter\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  usleep ( 100000 );

  if ( cameraInfo->write ( cameraInfo, guideOffCmd, 4 )) {
    fprintf ( stderr, "%s: write error on guide off\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  usleep ( 100000 );
  return OA_ERR_NONE;
}


static int
_doReadExposure ( AtikSerial_STATE* cameraInfo )
{
  unsigned char         guideOffCmd[4] = { 'C', 'M', 'D',
                            ATIK_CMD_DISABLE_GUIDE };
  unsigned char		ampOffCmd[5] = { 'C', 'M', 'D', ATIK_CMD_SET_AMP, 0 };
  unsigned char         readCCDCmd[20] = { 'C', 'M', 'D', ATIK_CMD_READ_CCD };
  unsigned char*	p = cameraInfo->xferBuffer;
  unsigned int		bytesToRead = cameraInfo->imageBufferLength;
  unsigned int		exposureMillisecs = cameraInfo->currentExposure / 1000;
  int			allowedEmptyReads, numRead;

  if ( cameraInfo->write ( cameraInfo, guideOffCmd, 4 )) {
    fprintf ( stderr, "%s: write error on guide off\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  usleep ( 100000 );

  if ( exposureMillisecs < 10 ) {
    readCCDCmd[ ATIK_SERIAL_CCD_EXPOSURE_TIMING ] = exposureMillisecs;
    // This may need to be the other way around for some of the cameras
    // cameraInfo->ccdReadFlags |= ATIK_SERIAL_READ_FLAGS_TIMER_AMP_ON;
    cameraInfo->ccdReadFlags &= ~ATIK_SERIAL_READ_FLAGS_TIMER_AMP_ON;
  } else {
    if ( exposureMillisecs <= ATIK_SERIAL_MAX_SHORT_EXPOSURE ) {
      readCCDCmd[ ATIK_SERIAL_CCD_EXPOSURE_TIMING ] =
          exposureMillisecs / 10 + 9;
      // This may need to be the other way around for some of the cameras
      // cameraInfo->ccdReadFlags |= ATIK_SERIAL_READ_FLAGS_TIMER_AMP_ON;
      cameraInfo->ccdReadFlags &= ~ATIK_SERIAL_READ_FLAGS_TIMER_AMP_ON;
    } else {
      readCCDCmd[ ATIK_SERIAL_CCD_EXPOSURE_TIMING ] = EXPOSURE_TIMING_EXTERNAL;
      // This may need to be the other way around for some of the cameras
      // cameraInfo->ccdReadFlags &= ~ATIK_SERIAL_READ_FLAGS_TIMER_AMP_ON;
      cameraInfo->ccdReadFlags |= ATIK_SERIAL_READ_FLAGS_TIMER_AMP_ON;
    }
  }

  readCCDCmd[ ATIK_SERIAL_CCD_X_BINNING ] = cameraInfo->binMode;
  readCCDCmd[ ATIK_SERIAL_CCD_Y_BINNING ] = cameraInfo->binMode;
  readCCDCmd[ ATIK_SERIAL_CCD_ROI_X_LO ] = 0;
  readCCDCmd[ ATIK_SERIAL_CCD_ROI_X_HI ] = 0;
  readCCDCmd[ ATIK_SERIAL_CCD_ROI_Y_LO ] = 0;
  readCCDCmd[ ATIK_SERIAL_CCD_ROI_Y_HI ] = 0;
  readCCDCmd[ ATIK_SERIAL_CCD_SIZE_X_LO ] = cameraInfo->xSize & 0xff;
  readCCDCmd[ ATIK_SERIAL_CCD_SIZE_X_HI ] = cameraInfo->xSize >> 8;
  readCCDCmd[ ATIK_SERIAL_CCD_SIZE_Y_LO ] = cameraInfo->ySize & 0xff;
  readCCDCmd[ ATIK_SERIAL_CCD_SIZE_Y_HI ] = cameraInfo->ySize >> 8;

  if ( cameraInfo->ccdReadFlags & ATIK_SERIAL_READ_FLAGS_DEINTERLACE ) {
    fprintf ( stderr, "%s: Help!  can't handle interlaced camera yet.  Add code\n",
        __FUNCTION__ );
  }

  readCCDCmd[ ATIK_SERIAL_CCD_CONFIG_FLAGS_LO ] =
      cameraInfo->ccdReadFlags & 0xff;
  readCCDCmd[ ATIK_SERIAL_CCD_CONFIG_FLAGS_HI ] =
      cameraInfo->ccdReadFlags >> 8;

  if ( cameraInfo->write ( cameraInfo, readCCDCmd,
      ATIK_SERIAL_CCD_BUFFER_LENGTH )) {
    fprintf ( stderr, "%s: write error on read cmd\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  usleep ( 100000 );

  allowedEmptyReads = NULL_READS;
  while ( bytesToRead && allowedEmptyReads ) {
    numRead = cameraInfo->readBlock ( cameraInfo, p,
        bytesToRead >= OPTIMAL_READ_SIZE ? OPTIMAL_READ_SIZE : bytesToRead );
    if ( !numRead ) {
      allowedEmptyReads--;
    } else {
      if ( numRead > 0 ) {
        p += numRead;
        bytesToRead -= numRead;
        allowedEmptyReads = NULL_READS;
      }
    }
    // I determined this delay experimentally to allow something close to
    // OPTIMAL_READ_SIZE byte to be available at the time of the above read
    usleep ( 20000 );
  }

  if ( bytesToRead ) {
    fprintf ( stderr, "short read.  Expected %d, got %d\n",
        cameraInfo->imageBufferLength, ( int ) ( p - cameraInfo->xferBuffer ));
    cameraInfo->droppedFrames++;
  }

  if ( cameraInfo->write ( cameraInfo, ampOffCmd, 5 )) {
    fprintf ( stderr, "%s: write error on amp off\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  usleep ( 100000 );

  return OA_ERR_NONE;
}
