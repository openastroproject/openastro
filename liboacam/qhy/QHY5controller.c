/*****************************************************************************
 *
 * QHY5controller.c -- Main camera controller thread
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

#include <openastro/camera.h>
#include <openastro/util.h>
#include <sys/time.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "QHY.h"
#include "QHY5.h"
#include "QHYoacam.h"
#include "QHYstate.h"
#include "QHYusb.h"


static int	_processSetControl ( QHY_STATE*, OA_COMMAND* );
static int	_processGetControl ( QHY_STATE*, OA_COMMAND* );
static int	_processSetResolution ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStop ( QHY_STATE*, OA_COMMAND* );
static int	_doCameraConfig ( QHY_STATE*, OA_COMMAND* );
static int	_doStartExposure ( QHY_STATE* );
static int	_doReadExposure ( QHY_STATE* );
static void	_storeWordBE ( unsigned char*, unsigned int );


void*
oacamQHY5controller ( void* param )
{
  oaCamera*		camera = param;
  QHY_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode, streaming = 0;
  int			maxWaitTime, frameWait;
  int			nextBuffer, buffersFree;
  unsigned int		x, y;
  uint8_t*		s;
  uint8_t*		t;
  uint8_t*		startOfFirstLine;
  uint8_t*		startOfLine;

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
      maxWaitTime = frameWait = cameraInfo->requestedExposure;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
      _doStartExposure ( cameraInfo );
      while ( !exitThread && maxWaitTime > 0 ) {
        usleep ( frameWait );
        maxWaitTime -= frameWait;
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
            startOfFirstLine = cameraInfo->xferBuffer + cameraInfo->xOffset;
            t = cameraInfo->buffers[ nextBuffer ].start;
            startOfLine = startOfFirstLine;
            for ( y = 0; y < cameraInfo->ySize; y++ ) {
              s = startOfLine;
              for ( x = 0; x < cameraInfo->xSize; x++ ) {
                *t++ = *s++;
              }
              startOfLine += QHY5_SENSOR_WIDTH;
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
_processSetControl ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*val = command->commandData;
  int			control = command->controlId;

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentGain = val->int32;
      if ( cameraInfo->isStreaming ) {
        _doCameraConfig ( cameraInfo, command );
      }
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    {
      uint32_t val_u32;
      int32_t  val_s32;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s32 = val->int32;
      if ( val_s32 < 1 ) { val_s32 = 1; }
      val_u32 = ( uint32_t ) val_s32;
      if ( cameraInfo->transferTime > val_u32 ) {
        val_u32 = cameraInfo->transferTime;
      }
      cameraInfo->currentExposure = val_u32;
      cameraInfo->requestedExposure = val_u32 - cameraInfo->transferTime;
      break;
    }
    case OA_CAM_CTRL_DROPPED_RESET:
      // droppedFrames could be mutexed, but it's not the end of the world
      cameraInfo->droppedFrames = 0;
      break;

    case OA_CAM_CTRL_FRAME_FORMAT:
      // Only one format is supported, so there's nothing to do here
      break;

    default:
      fprintf ( stderr, "QHY5: %s not yet implemented for control %d\n",
          __FUNCTION__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  return OA_ERR_NONE;
}


static int
_processGetControl ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentGain;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentExposure;
      break;

    case OA_CAM_CTRL_DROPPED:
      val->valueType = OA_CTRL_TYPE_READONLY;
      val->readonly = cameraInfo->droppedFrames;
      break;

    default:
      fprintf ( stderr,
          "QHY5 %s: Unrecognised control %d\n", __FUNCTION__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


static int
_processSetResolution ( oaCamera* camera, OA_COMMAND* command )
{
  QHY_STATE*	cameraInfo = camera->_private;
  FRAMESIZE*	size = command->commandData;

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;
  cameraInfo->xOffset = QHY5_DARK_WIDTH_X + ( QHY5_IMAGE_WIDTH -
      QHY5_DARK_WIDTH_X - ( int ) cameraInfo->xSize ) / 2;
  cameraInfo->imageBufferLength = cameraInfo->xSize * cameraInfo->ySize;
  cameraInfo->transferTime = QHY5_SENSOR_WIDTH * ( cameraInfo->ySize +
      QHY5_VBLANK ) / QHY5_PIXEL_RATE;
  if ( cameraInfo->isStreaming ) {
    ( void ) _doCameraConfig ( cameraInfo, command );
  }
  return OA_ERR_NONE;
}


static int
_doCameraConfig ( QHY_STATE* cameraInfo, OA_COMMAND* command )
{
  // Weirdness starts here.
  //
  // First, gain settings.  Gain may be set for individual colour planes
  // (not applicable for the mono QHY5, obviously) and globally, but the
  // settings aren't linear.  Everyone seems to go for a lookup table for
  // this, so I guess I'll follow suit.

  static const int	gainLookup[] = {
      0x000,0x004,0x005,0x006,0x007,0x008,0x009,0x00A,0x00B,0x00C,0x00D,0x00E,
      0x00F,0x010,0x011,0x012,0x013,0x014,0x015,0x016,0x017,0x018,0x019,0x01A,
      0x01B,0x01C,0x01D,0x01E,0x01F,0x051,0x052,0x053,0x054,0x055,0x056,0x057,
      0x058,0x059,0x05A,0x05B,0x05C,0x05D,0x05E,0x05F,0x6CE,0x6CF,0x6D0,0x6D1,
      0x6D2,0x6D3,0x6D4,0x6D5,0x6D6,0x6D7,0x6D8,0x6D9,0x6DA,0x6DB,0x6DC,0x6DD,
      0x6DE,0x6DF,0x6E0,0x6E1,0x6E2,0x6E3,0x6E4,0x6E5,0x6E6,0x6E7,0x6FC,0x6FD,
      0x6FE,0x6FF
  };
  static const int	gainTableSize = ( sizeof ( gainLookup ) /
				sizeof ( int ));

  unsigned int		offset, frameSize, frameSizeMSW, frameSizeLSW;
  unsigned int		greenGain, blueGain, redGain, globalGain;
  unsigned char		regs[ QHY5_BUFFER_SIZE ];
  int			gain, height;
  unsigned char		data = 0;
  unsigned int		xferred;

  if ( cameraInfo->firstTimeSetup ) {
    gain = 8; // no idea why this should be
  } else {
    gain = cameraInfo->currentGain;
  }
  height = cameraInfo->ySize;

  if ( gain > gainTableSize ) {
    gain = gainTableSize;
  }
  greenGain = blueGain = redGain = globalGain = gainLookup[ gain - 1 ];

  // height must be a multiple of 4
  height -= height % 4;
  // how far from the top edge of the sensor we start
  offset = ( QHY5_SENSOR_HEIGHT - height ) / 2;
  frameSize = ( QHY5_SENSOR_WIDTH * ( height + QHY5_VBLANK ));
  frameSizeMSW = frameSize >> 16;
  frameSizeLSW = frameSize & 0xffff;

  // The registers we appear to have access to are:
  // 0x01 - first row to read
  // 0x02 - first column to read
  // 0x03 - how many rows to read (less one)
  // 0x04 - how many columns to read (less one)
  // 0x09 - how many rows to integrate (including vblank?)
  // 0x2b - green gain
  // 0x2c - blue gain
  // 0x2d - red gain
  // 0x2e - global gain

  _storeWordBE ( regs, greenGain );        // MT9M001 register 0x2b
  _storeWordBE ( regs + 2, blueGain );     // MT9M001 register 0x2c
  _storeWordBE ( regs + 4, redGain );      // MT9M001 register 0x2d
  _storeWordBE ( regs + 6, globalGain );   // MT9M001 register 0x2e
  _storeWordBE ( regs + 8, offset );       // MT9M001 register 0x01
  _storeWordBE ( regs + 10, 0 );           // MT9M001 register 0x02
  _storeWordBE ( regs + 12, height - 1 );  // MT9M001 register 0x03
  _storeWordBE ( regs + 14, 1313 );        // MT9M001 register 0x04 (why 1314?)
  _storeWordBE ( regs + 16, height + QHY5_VBLANK - 1 ); // MT9M001 reg 0x09
  regs[18] = 0xcc; // Magic number!

  // Lots of magic numbers :(

  if ( cameraInfo->firstTimeSetup ) {
    if ( _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_OUT, &data, 1,
        &xferred, USB2_TIMEOUT )) {
      fprintf ( stderr, "%s: usb bulk transfer failed\n", __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }
  }

  if ( _usbControlMsg ( cameraInfo, QHY_CMD_ENDP_OUT, 0x13, frameSizeLSW,
      frameSizeMSW, regs, QHY5_BUFFER_SIZE, USB2_TIMEOUT ) !=
      QHY5_BUFFER_SIZE ) {
    fprintf ( stderr, "%s: usb control message #1 failed\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  if ( _usbControlMsg ( cameraInfo, QHY_CMD_ENDP_OUT, 0x14, 0x31a5, 0, 0, 0,
      USB2_TIMEOUT )) {
    fprintf ( stderr, "%s: usb control message #2 failed\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  if ( _usbControlMsg ( cameraInfo, QHY_CMD_ENDP_OUT, 0x16,
      cameraInfo->firstTimeSetup, 0, 0, 0, USB2_TIMEOUT )) {
    fprintf ( stderr, "%s: usb control message #3 failed\n", __FUNCTION__ );
    return -OA_ERR_CAMERA_IO;
  }
  cameraInfo->firstTimeSetup = 0;

  return ( height + QHY5_VBLANK );
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

  // this will actually trigger the "firstTimeSetup" step
  cameraInfo->captureHeight = _doCameraConfig ( cameraInfo, command );
  cameraInfo->captureLength = cameraInfo->captureHeight * QHY5_SENSOR_WIDTH;
  _doStartExposure ( cameraInfo );
  usleep ( cameraInfo->requestedExposure );
  _doReadExposure ( cameraInfo );
  cameraInfo->captureHeight = _doCameraConfig ( cameraInfo, command );

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


static void
_storeWordBE ( unsigned char* p, unsigned int val )
{
  *p++ = ( val >> 8 ) & 0xff;
  *p = val & 0xff;
}


static int
_doStartExposure ( QHY_STATE* cameraInfo )
{
  unsigned char		buff[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  unsigned int		exposureMillisecs;

  // weird that a "read" control message is used to kick off the exposure
  // and set the exposure time
  exposureMillisecs = cameraInfo->requestedExposure / 1000;
  _usbControlMsg ( cameraInfo, QHY_CMD_ENDP_IN, 0x12,
      exposureMillisecs & 0xffff, exposureMillisecs >> 16, buff, 2,
      USB2_TIMEOUT );
  return OA_ERR_NONE;
}


static int
_doReadExposure ( QHY_STATE* cameraInfo )
{
  int		ret;
  unsigned int	readSize;

  // references to droppedFrames here could be mutexed, but it's not the
  // end of the world

  ret = _usbBulkTransfer ( cameraInfo, QHY_BULK_ENDP_IN,
      cameraInfo->xferBuffer, cameraInfo->captureLength, &readSize,
      USB2_TIMEOUT );
  if ( ret ) {
    cameraInfo->droppedFrames++;
    return ret;
  }
  if ( readSize != cameraInfo->captureLength ) {
    fprintf ( stderr, "readExposure: USB bulk transfer was short. %d != %d\n",
        readSize, cameraInfo->captureLength );
    cameraInfo->droppedFrames++;
    return -OA_ERR_CAMERA_IO;
  }
  return OA_ERR_NONE;
}
