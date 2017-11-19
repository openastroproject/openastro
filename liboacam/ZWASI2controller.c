/*****************************************************************************
 *
 * ZWASIcontroller.c -- Main camera controller thread
 *
 * Copyright 2015,2017 James Fidell (james@openastroproject.org)
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
#include <ASICamera2.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "ZWASI.h"
#include "ZWASIoacam.h"
#include "ZWASIstate.h"


static int	_processSetControl ( ZWASI_STATE*, OA_COMMAND* );
static int	_processSetResolution ( ZWASI_STATE*, OA_COMMAND* );
static int	_processStreamingStart ( ZWASI_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( ZWASI_STATE*, OA_COMMAND* );

static void	_doFrameReconfiguration ( ZWASI_STATE* );
static int32_t	_doStateMachine ( ZWASI_STATE*, unsigned int );


void*
oacamZWASI2controller ( void* param )
{
  oaCamera*		camera = param;
  ZWASI_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode, nextBuffer, buffersFree, frameWait;
  int			imageBufferLength, haveFrame;
//int			maxWaitTime;
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
          case OA_CMD_RESOLUTION_SET:
            resultCode = _processSetResolution ( cameraInfo, command );
            break;
          case OA_CMD_START:
            resultCode = _processStreamingStart ( cameraInfo, command );
            break;
          case OA_CMD_STOP:
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
      frameWait = cameraInfo->currentAbsoluteExposure;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

      // convert frameWait from microseconds to milliseconds
      // if it is more than 100ms then set it to 100ms and that
      // is the longest we will wait before seeing the thread
      // killed

      frameWait /= 1000;
//    maxWaitTime = frameWait * 2;
      if ( frameWait > 100 ) {
        frameWait = 100;
      }

      pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
      buffersFree = cameraInfo->buffersFree;
      pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

      if ( buffersFree ) {
        nextBuffer = cameraInfo->nextBuffer;
        haveFrame = 0;
//      do {
          if ( !ASIGetVideoData ( cameraInfo->cameraId,
              cameraInfo->buffers[ nextBuffer ].start, imageBufferLength,
              frameWait )) {
            haveFrame = 1;
          }
//        maxWaitTime -= frameWait;

          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          exitThread = cameraInfo->stopControllerThread;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

          if ( !exitThread && haveFrame ) {
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
//      } while ( !exitThread && !haveFrame && maxWaitTime > 0 );
      }
    }
  } while ( !exitThread );

  return 0;
}


static int
_processSetControl ( ZWASI_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*val = command->commandData;

  switch ( command->controlId ) {

    case OA_CAM_CTRL_BRIGHTNESS:
      ASISetControlValue ( cameraInfo->cameraId, ASI_BRIGHTNESS, val->int32,
          cameraInfo->autoBrightness );
      cameraInfo->currentBrightness = val->int32;
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      ASISetControlValue ( cameraInfo->cameraId, ASI_WB_B, val->int32,
          cameraInfo->autoBlueBalance );
      cameraInfo->currentBlueBalance = val->int32;
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      ASISetControlValue ( cameraInfo->cameraId, ASI_WB_R, val->int32,
          cameraInfo->autoRedBalance );
      cameraInfo->currentRedBalance = val->int32;
      break;

    case OA_CAM_CTRL_GAMMA:
      ASISetControlValue ( cameraInfo->cameraId, ASI_GAMMA, val->int32,
          cameraInfo->autoGamma );
      cameraInfo->currentGamma = val->int32;
      break;

    case OA_CAM_CTRL_GAIN:
      ASISetControlValue ( cameraInfo->cameraId, ASI_GAIN, val->int32,
          cameraInfo->autoGain );
      cameraInfo->currentGain = val->int32;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      ASISetControlValue ( cameraInfo->cameraId, ASI_EXPOSURE, val->int32,
          cameraInfo->autoExposure );
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      cameraInfo->currentAbsoluteExposure = val->int32;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
      break;

    case OA_CAM_CTRL_USBTRAFFIC:
      ASISetControlValue ( cameraInfo->cameraId, ASI_BANDWIDTHOVERLOAD,
          val->int32, cameraInfo->autoUSBTraffic );
      cameraInfo->currentUSBTraffic = val->int32;
      break;

    case OA_CAM_CTRL_OVERCLOCK:
      ASISetControlValue ( cameraInfo->cameraId, ASI_OVERCLOCK, val->int32,
          cameraInfo->autoOverclock );
      cameraInfo->currentOverclock = val->int32;
      break;

    case OA_CAM_CTRL_HIGHSPEED:
      ASISetControlValue ( cameraInfo->cameraId, ASI_HIGH_SPEED_MODE,
          val->boolean, cameraInfo->autoHighSpeed );
      cameraInfo->currentHighSpeed = val->boolean;
      break;

    case OA_CAM_CTRL_BINNING:
      cameraInfo->binMode = val->discrete;
/*
      if ( camera->colour ) {
        if ( val->discrete < OA_BIN_MODE_2x2 &&
            8 == cameraInfo->currentBitDepth ) {
          if ( 3 == cameraInfo->FSMState ) {
            cameraInfo->videoCurrent = ASI_IMG_RAW8;
          } else {
            cameraInfo->videoCurrent = ASI_IMG_RGB24;
          }
        }
      }
*/
      _doFrameReconfiguration ( cameraInfo );
      break;

    case OA_CAM_CTRL_HFLIP:
    case OA_CAM_CTRL_VFLIP:
    {
      int flip = ASI_FLIP_NONE;
      if ( command->controlId == OA_CAM_CTRL_HFLIP ) {
        cameraInfo->currentHFlip = val->boolean;
      } else {
        cameraInfo->currentVFlip = val->boolean;
      }
      if ( cameraInfo->currentHFlip ) {
        flip = ASI_FLIP_HORIZ;
        if ( cameraInfo->currentVFlip ) {
          flip = ASI_FLIP_BOTH;
        }
      } else {
        if ( cameraInfo->currentVFlip ) {
          flip = ASI_FLIP_VERT;
        }
      }
      ASISetControlValue ( cameraInfo->cameraId, ASI_FLIP, flip, 0 );
      break;
    }

    case OA_CAM_CTRL_BIT_DEPTH:
      if ( 16 == val->discrete || 12 == val->discrete ) {
        cameraInfo->currentBitDepth = 16;
        if ( cameraInfo->colour ) {
          cameraInfo->videoCurrent = _doStateMachine ( cameraInfo,
              OA_CAM_CTRL_BIT_DEPTH );
        } else {
          cameraInfo->videoCurrent = ASI_IMG_RAW16;
        }
      } else {
        if ( 8 == val->discrete ) {
          cameraInfo->currentBitDepth = 8;
          if ( cameraInfo->colour ) {
            cameraInfo->videoCurrent = _doStateMachine ( cameraInfo,
              OA_CAM_CTRL_BIT_DEPTH );
          } else {
            cameraInfo->videoCurrent = ASI_IMG_RAW8;
          }
        }
      }
      _doFrameReconfiguration ( cameraInfo );
      break;

    case OA_CAM_CTRL_COLOUR_MODE:
      cameraInfo->videoCurrent = _doStateMachine ( cameraInfo,
            OA_CAM_CTRL_COLOUR_MODE );
      if ( cameraInfo->videoCurrent == ASI_IMG_RAW16 ) {
        cameraInfo->currentBitDepth = 16;
      } else {
        cameraInfo->currentBitDepth = 8;
      }
      _doFrameReconfiguration ( cameraInfo );
      break;

    case OA_CAM_CTRL_COOLER:
      ASISetControlValue ( cameraInfo->cameraId, ASI_COOLER_ON,
          val->boolean, 0 );
      cameraInfo->coolerEnabled = val->boolean;
      break;

    case OA_CAM_CTRL_MONO_BIN_COLOUR:
      ASISetControlValue ( cameraInfo->cameraId, ASI_MONO_BIN,
          val->boolean, 0 );
      cameraInfo->monoBinning = val->boolean;
      break;

    case OA_CAM_CTRL_FAN:
      ASISetControlValue ( cameraInfo->cameraId, ASI_FAN_ON,
          val->boolean, 0 );
      cameraInfo->fanEnabled = val->boolean;
      break;

    case OA_CAM_CTRL_PATTERN_ADJUST:
      ASISetControlValue ( cameraInfo->cameraId, ASI_PATTERN_ADJUST,
          val->boolean, 0 );
      cameraInfo->patternAdjust = val->boolean;
      break;

    case OA_CAM_CTRL_DEW_HEATER:
      ASISetControlValue ( cameraInfo->cameraId, ASI_ANTI_DEW_HEATER,
          val->boolean, 0 );
      cameraInfo->dewHeater = val->boolean;
      break;

    case OA_CAM_CTRL_TEMP_SETPOINT:
      ASISetControlValue ( cameraInfo->cameraId, ASI_TARGET_TEMP, val->int32,
          0 );
      cameraInfo->currentSetPoint = val->int32;
      break;

    case OA_CAM_CTRL_COOLER_POWER:
      ASISetControlValue ( cameraInfo->cameraId, ASI_COOLER_POWER_PERC,
          val->int32, 0 );
      cameraInfo->currentCoolerPower = val->int32;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
      ASISetControlValue ( cameraInfo->cameraId, ASI_GAIN,
          cameraInfo->currentGain, val->boolean );
      cameraInfo->autoGain = val->boolean;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA ):
      ASISetControlValue ( cameraInfo->cameraId, ASI_GAMMA,
          cameraInfo->currentGamma, val->boolean );
      cameraInfo->autoGamma = val->boolean;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ):
      ASISetControlValue ( cameraInfo->cameraId, ASI_BRIGHTNESS,
          cameraInfo->currentBrightness, val->boolean );
      cameraInfo->autoBrightness = val->boolean;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
    {
      int v = ( OA_EXPOSURE_AUTO == val->boolean ? 1 : 0 );
      ASISetControlValue ( cameraInfo->cameraId, ASI_EXPOSURE,
          cameraInfo->currentAbsoluteExposure, v );
      cameraInfo->autoExposure = v;
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_RED_BALANCE ):
      ASISetControlValue ( cameraInfo->cameraId, ASI_WB_R,
          cameraInfo->currentRedBalance, val->boolean );
      cameraInfo->autoRedBalance = val->boolean;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLUE_BALANCE ):
      ASISetControlValue ( cameraInfo->cameraId, ASI_WB_B,
          cameraInfo->currentBlueBalance, val->boolean );
      cameraInfo->autoBlueBalance = val->boolean;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_USBTRAFFIC ):
      ASISetControlValue ( cameraInfo->cameraId, ASI_BANDWIDTHOVERLOAD,
          cameraInfo->currentUSBTraffic, val->boolean );
      cameraInfo->autoUSBTraffic = val->boolean;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_OVERCLOCK ):
      ASISetControlValue ( cameraInfo->cameraId, ASI_OVERCLOCK,
          cameraInfo->currentOverclock, val->boolean );
      cameraInfo->autoOverclock = val->boolean;
      break;

    default:
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


static int
_processSetResolution ( ZWASI_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMESIZE*	size = command->commandData;

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;
  _doFrameReconfiguration ( cameraInfo );
  return OA_ERR_NONE;
}


static void
_doFrameReconfiguration ( ZWASI_STATE* cameraInfo )
{
  int		multiplier;
  int		restartStreaming = 0;
  unsigned int	actualX, actualY;

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  if ( cameraInfo->isStreaming ) {
    cameraInfo->isStreaming = 0;
    restartStreaming = 1;
  }
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  if ( restartStreaming ) {
    ASIStopVideoCapture ( cameraInfo->cameraId );
  }
  actualX = cameraInfo->xSize;
  actualY = cameraInfo->ySize;
  if (( actualX * cameraInfo->binMode ) > cameraInfo->maxResolutionX ) {
    actualX = cameraInfo->maxResolutionX / cameraInfo->binMode;
  }
  if (( actualY * cameraInfo->binMode ) > cameraInfo->maxResolutionY ) {
    actualY = cameraInfo->maxResolutionY / cameraInfo->binMode;
  }
  ASISetROIFormat ( cameraInfo->cameraId, cameraInfo->xSize,
      cameraInfo->ySize, cameraInfo->binMode, cameraInfo->videoCurrent );
  if ( OA_BIN_MODE_NONE == cameraInfo->binMode &&
      ( cameraInfo->xSize != cameraInfo->maxResolutionX ||
      cameraInfo->ySize != cameraInfo->maxResolutionY )) {
    ASISetStartPos ( cameraInfo->cameraId,
        ( cameraInfo->maxResolutionX - cameraInfo->xSize ) / 2,
        ( cameraInfo->maxResolutionY - cameraInfo->ySize ) / 2 );
  }

  // RGB colour is 3 bytes per pixel, mono one for 8-bit, two for 16-bit,
  // RAW is one for 8-bit, 2 for 16-bit
  multiplier = ( ASI_IMG_RGB24 == cameraInfo->videoCurrent ) ? 3 :
      ( ASI_IMG_RAW16 == cameraInfo->videoCurrent ) ? 2 : 1;
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->imageBufferLength = actualX * actualY * multiplier;
  if ( restartStreaming ) {
    usleep ( 300000 );
    ASIStartVideoCapture ( cameraInfo->cameraId );
    cameraInfo->isStreaming = 1;
  }
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
}


static int32_t
_doStateMachine ( ZWASI_STATE* cameraInfo, unsigned int control )
{
  int32_t	newMode = 0;

  /*
   * Switching between 8-bit and 16-bit modes combined with raw colour
   * and RGB colour is messy because there's no 16-bit RGB mode, so to
   * return to the most sensible state when a box is checked/unchecked
   * we need to know how we got into the state we're in now.  The neat
   * solution to this appears to be a finite state machine, as initialised
   * when the camera was connected.  The states are thus:
   *
   * state 0 (RGB24): 16-bit on goes to state 1, raw mode goes to state 3
   * state 1 (RAW16): 16-bit off goes to state 0, raw goes to state 2
   * state 2 (RAW16): 16-bit off goes to state 3, raw off goes to state 1
   * state 3 (RAW8) : 16-bit on goes to state 4, raw off goes to state 0
   * state 4 (RAW16): 16-bit off goes to state 3, raw off goes to state 5
   * state 5 (RAW16): 16-bit off goes to state 0, raw on goes to state 4
   *
   * The state machine means that we don't actually need to know how
   * a control was changed to get the next result, merely to know which
   * control has been changed.
   */

  switch ( cameraInfo->FSMState ) {
    case 0:
      if ( OA_CAM_CTRL_BIT_DEPTH == control ) {
        cameraInfo->FSMState = 1;
        newMode = ASI_IMG_RAW16;
      } else { // raw
        cameraInfo->FSMState = 3;
        newMode = ASI_IMG_RAW8;
      }
      break;

    case 1:
      if ( OA_CAM_CTRL_BIT_DEPTH == control ) {
        cameraInfo->FSMState = 0;
        newMode = ASI_IMG_RGB24;
      } else { // raw
        cameraInfo->FSMState = 2;
        newMode = ASI_IMG_RAW16;
      }
      break;

    case 2:
      if ( OA_CAM_CTRL_BIT_DEPTH == control ) {
        cameraInfo->FSMState = 3;
        newMode = ASI_IMG_RAW8;
      } else { // raw
        cameraInfo->FSMState = 1;
        newMode = ASI_IMG_RAW16;
      }
      break;

    case 3:
      if ( OA_CAM_CTRL_BIT_DEPTH == control ) {
        cameraInfo->FSMState = 4;
        newMode = ASI_IMG_RAW16;
      } else { // raw
        cameraInfo->FSMState = 0;
        newMode = ASI_IMG_RGB24;
      }
      break;

    case 4:
      if ( OA_CAM_CTRL_BIT_DEPTH == control ) {
        cameraInfo->FSMState = 3;
        newMode = ASI_IMG_RAW8;
      } else { // raw
        cameraInfo->FSMState = 5;
        newMode = ASI_IMG_RAW16;
      }
      break;

    case 5:
      if ( OA_CAM_CTRL_BIT_DEPTH == control ) {
        cameraInfo->FSMState = 0;
        newMode = ASI_IMG_RGB24;
      } else { // raw
        cameraInfo->FSMState = 4;
        newMode = ASI_IMG_RAW16;
      }
      break;
  }

  return newMode;
}


static int
_processStreamingStart ( ZWASI_STATE* cameraInfo, OA_COMMAND* command )
{
  CALLBACK*	cb = command->commandData;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  /*
   * This is now done by assigning the largest possible buffers when
   * the camera is initialised, but could be changed back to here to
   * use meory more efficiently.
   *
  cameraInfo->configuredBuffers = 0;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( struct ZWASIbuffer ));
  for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
    void* m = malloc ( cameraInfo->imageBufferLength );
    if ( m ) {
      cameraInfo->buffers[i].start = m;
      cameraInfo->configuredBuffers++;
    } else {
      fprintf ( stderr, "oaZWASICameraStart malloc failed\n" );
      if ( i ) {
        for ( j = 0; i < i; j++ ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
      return -OA_ERR_MEM_ALLOC;
    }
  }
   */

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;
  ASIStartVideoCapture ( cameraInfo->cameraId );
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}


static int
_processStreamingStop ( ZWASI_STATE* cameraInfo, OA_COMMAND* command )
{
  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  /*
   * This will be needed if the buffer assignment is restored in
   * _processStreamingStart()
   *
  for ( i = 0; i < cameraInfo->configuredBuffers; i++ ) {
    free (( void* ) cameraInfo->buffers[i].start );
  }
  cameraInfo->configuredBuffers = 0;
  cameraInfo->lastUsedBuffer = -1;
  free (( void* ) cameraInfo->buffers );
  cameraInfo->buffers = 0;
   */

  ASIStopVideoCapture ( cameraInfo->cameraId );
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}
