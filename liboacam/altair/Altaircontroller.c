/*****************************************************************************
 *
 * Altaircontroller.c -- Main camera controller thread
 *
 * Copyright 2016,2017,2018,2019
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

#include <pthread.h>
#include <sys/time.h>

#include <openastro/camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Altairprivate.h"
#include "Altairoacam.h"
#include "Altairstate.h"


static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processGetControl ( ALTAIRCAM_STATE*, OA_COMMAND* );
static int	_processSetResolution ( ALTAIRCAM_STATE*, OA_COMMAND* );
static int	_processSetROI ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( ALTAIRCAM_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( ALTAIRCAM_STATE*, OA_COMMAND* );
static int	_processExposureStart ( ALTAIRCAM_STATE*, OA_COMMAND* );
static int	_processAbortExposure ( ALTAIRCAM_STATE* );
static int	_doStart ( ALTAIRCAM_STATE* );
static int	_doStop ( ALTAIRCAM_STATE* );
static int	_setBinning ( ALTAIRCAM_STATE*, int );
static int	_setFrameFormat ( ALTAIRCAM_STATE*, int );
static void _AltairPullCallback ( unsigned int, void* );
/*
static int	_setColourMode ( ALTAIRCAM_STATE*, int );
static int	_setBitDepth ( ALTAIRCAM_STATE*, int );
*/


void*
oacamAltaircontroller ( void* param )
{
  oaCamera*		camera = param;
  ALTAIRCAM_STATE*	cameraInfo = camera->_private;
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
					case OA_CMD_START_EXPOSURE:
						resultCode = _processExposureStart ( cameraInfo, command );
						break;
					case OA_CMD_ABORT_EXPOSURE:
						resultCode = _processAbortExposure ( cameraInfo );
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
  } while ( !exitThread );

  return 0;
}


void
_AltairFrameCallback ( const void *frame, const AltaircamFrameInfoV2*
    frameInfo, int bSnap, void *ptr )
{
  ALTAIRCAM_STATE*	cameraInfo = ptr;
  int			buffersFree, nextBuffer, shiftBits, bitsPerPixel;
  unsigned int		dataLength;

  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  buffersFree = cameraInfo->buffersFree;
  bitsPerPixel = cameraInfo->currentBitsPerPixel;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

  if ( frame && buffersFree ) {
    dataLength = cameraInfo->imageBufferLength;
    nextBuffer = cameraInfo->nextBuffer;

    // Now here's the fun...
    //
    // In 12-bit (and presumably 10- and 14-bit) mode, Altair cameras
    // appear to return little-endian data, but right-aligned rather than
    // left-aligned as many other cameras do.  So if we have such an image we
    // try to fix it here.
    //
    // FIX ME -- I'm not sure this is the right place to be doing this.
    // Perhaps there should be a flag to tell the user whether the data is
    // left-or right-aligned and they can sort it out.

    if ( bitsPerPixel > 8 && bitsPerPixel < 16 ) {
      shiftBits = 16 - bitsPerPixel;

      if ( shiftBits ) {
        const uint16_t	*s = frame;
        uint16_t	*t = cameraInfo->buffers[ nextBuffer ].start;
        uint16_t	v;
        unsigned int	i;

        for ( i = 0; i < dataLength; i += 2 ) {
          v = *s++;
          v <<= shiftBits;
          *t++ = v;
        }
      }
    } else {
      ( void ) memcpy ( cameraInfo->buffers[ nextBuffer ].start, frame,
          dataLength );
    }

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
_processSetControl ( oaCamera* camera, OA_COMMAND* command )
{
  ALTAIRCAM_STATE*	cameraInfo = camera->_private;
  oaControlValue	*valp = command->commandData;
  int			control = command->controlId, val = 0;

  switch ( control ) {

    case OA_CAM_CTRL_BRIGHTNESS:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= ALTAIRCAM_BRIGHTNESS_MIN &&
					val <= ALTAIRCAM_BRIGHTNESS_MAX ) {
        if ((( p_Altaircam_put_Brightness )( cameraInfo->handle, val )) < 0 ) {
          fprintf ( stderr, "Altaircam_put_Brightness ( %d ) failed\n", val );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_CONTRAST:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= ALTAIRCAM_CONTRAST_MIN && val <= ALTAIRCAM_CONTRAST_MAX ) {
        if ((( p_Altaircam_put_Contrast )( cameraInfo->handle, val )) < 0 ) {
          fprintf ( stderr, "Altaircam_put_Contrast ( %d ) failed\n", val );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_GAMMA:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= ALTAIRCAM_GAMMA_MIN && val <= ALTAIRCAM_GAMMA_MAX ) {
        if ((( p_Altaircam_put_Gamma )( cameraInfo->handle, val )) < 0 ) {
          fprintf ( stderr, "Altaircam_put_Gamma ( %d ) failed\n", val );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_HFLIP:
      if ( OA_CTRL_TYPE_BOOLEAN != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where boolean expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->boolean ? 1 : 0;
      if ((( p_Altaircam_put_HFlip )( cameraInfo->handle, val )) < 0 ) {
        fprintf ( stderr, "Altaircam_put_HFlip ( %d ) failed\n", val );
        return -OA_ERR_CAMERA_IO;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_VFLIP:
      if ( OA_CTRL_TYPE_BOOLEAN != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where boolean expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->boolean ? 1 : 0;
      if ((( p_Altaircam_put_VFlip )( cameraInfo->handle, val )) < 0 ) {
        fprintf ( stderr, "Altaircam_put_VFlip ( %d ) failed\n", val );
        return -OA_ERR_CAMERA_IO;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
      if ( OA_CTRL_TYPE_BOOLEAN != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where boolean expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      if ((( p_Altaircam_put_AutoExpoEnable )( cameraInfo->handle,
						valp->boolean )) < 0) {
        fprintf ( stderr, "Altaircam_put_AutoExpoEnable ( %d ) failed\n",
						valp->boolean );
        return -OA_ERR_CAMERA_IO;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= cameraInfo->exposureMin && val <= cameraInfo->exposureMax ) {
        if ((( p_Altaircam_put_ExpoTime )( cameraInfo->handle, val )) < 0 ) {
          fprintf ( stderr, "Altaircam_put_ExpoTime ( %d ) failed\n", val );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_GAIN:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= cameraInfo->gainMin && val <= cameraInfo->gainMax ) {
        if ((( p_Altaircam_put_ExpoAGain )( cameraInfo->handle, val )) < 0 ) {
          fprintf ( stderr, "Altaircam_put_ExpoAGain ( %d ) failed\n", val );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_SPEED:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= 0 && val <= cameraInfo->speedMax ) {
        if ((( p_Altaircam_put_Speed )( cameraInfo->handle, val )) < 0 ) {
          fprintf ( stderr, "Altaircam_put_Speed ( %d ) failed\n", val );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_HUE:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= ALTAIRCAM_HUE_MIN && val <= ALTAIRCAM_HUE_MAX ) {
        if ((( p_Altaircam_put_Hue )( cameraInfo->handle, val )) < 0 ) {
          fprintf ( stderr, "Altaircam_put_Hue ( %d ) failed\n", val );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_SATURATION:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= ALTAIRCAM_SATURATION_MIN &&
					val <= ALTAIRCAM_SATURATION_MAX ) {
        if ((( p_Altaircam_put_Saturation )( cameraInfo->handle, val )) < 0 ) {
          fprintf ( stderr, "Altaircam_put_Saturation ( %d ) failed\n", val );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_RED_BALANCE:
    case OA_CAM_CTRL_BLUE_BALANCE:
    case OA_CAM_CTRL_GREEN_BALANCE:
      if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->int32;
      if ( val >= ALTAIRCAM_WBGAIN_MIN && val <= ALTAIRCAM_WBGAIN_MAX ) {
        int gain[3];
        if ((( p_Altaircam_get_WhiteBalanceGain )( cameraInfo->handle,
            gain )) < 0 ) {
          fprintf ( stderr,
							"Altaircam_get_WhiteBalanceGain (gain[3]) failed\n" );
          return -OA_ERR_CAMERA_IO;
        }
        switch ( control ) {
          case OA_CAM_CTRL_RED_BALANCE:
            gain[0] = val;
            break;
          case OA_CAM_CTRL_BLUE_BALANCE:
            gain[2] = val;
            break;
          case OA_CAM_CTRL_GREEN_BALANCE:
            gain[1] = val;
            break;
        }
        if ((( p_Altaircam_put_WhiteBalanceGain )( cameraInfo->handle,
            gain )) < 0 ) {
          fprintf ( stderr,
							"Altaircam_put_WhiteBalanceGain (gain[3]) failed\n" );
          return -OA_ERR_CAMERA_IO;
        }
      } else {
        return -OA_ERR_OUT_OF_RANGE;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_BINNING:
      if ( OA_CTRL_TYPE_DISCRETE != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->discrete;
      return _setBinning ( cameraInfo, val );
      break;

    case OA_CAM_CTRL_COOLER:
      if ( OA_CTRL_TYPE_BOOLEAN != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where boolean expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->boolean ? 0 : 1;
      if ((( p_Altaircam_put_Option )( cameraInfo->handle,
          ALTAIRCAM_OPTION_TEC, 1 )) < 0 ) {
        fprintf ( stderr, "Altaircam_put_Option ( cooler, %d ) failed\n", val );
        return -OA_ERR_CAMERA_IO;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_FAN:
      if ( OA_CTRL_TYPE_BOOLEAN != valp->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where boolean expected "
            "for control %d\n", __FUNCTION__, valp->valueType, control );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val = valp->boolean ? 0 : 1;
      if ((( p_Altaircam_put_Option )( cameraInfo->handle,
          ALTAIRCAM_OPTION_FAN, 1 )) < 0 ) {
        fprintf ( stderr, "Altaircam_put_Option ( fan, %d ) failed\n", val );
        return -OA_ERR_CAMERA_IO;
      }
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_FRAME_FORMAT:
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
      break;

    case OA_CAM_CTRL_LED_STATE:
    case OA_CAM_CTRL_LED_PERIOD:
      if ( control == OA_CAM_CTRL_LED_STATE ) {
        if ( OA_CTRL_TYPE_DISC_MENU != valp->valueType ) {
          fprintf ( stderr, "%s: invalid control type %d where menu expected "
              "for control %d\n", __FUNCTION__, valp->valueType, control );
          return -OA_ERR_INVALID_CONTROL_TYPE;
        }
        cameraInfo->ledState = valp->menu;
      } else {
        if ( OA_CTRL_TYPE_INT32 != valp->valueType ) {
          fprintf ( stderr, "%s: invalid control type %d where int32 expected "
              "for control %d\n", __FUNCTION__, valp->valueType, control );
          return -OA_ERR_INVALID_CONTROL_TYPE;
        }
        cameraInfo->ledPeriod = valp->int32;
      }
      if ((( p_Altaircam_put_LEDState )( cameraInfo->handle, 0,
          cameraInfo->ledState, cameraInfo->ledPeriod )) < 0 ) {
        fprintf ( stderr, "Altaircam_put_LEDState ( 0, %d, %d ) failed\n",
            cameraInfo->ledState, cameraInfo->ledPeriod );
        return -OA_ERR_CAMERA_IO;
      }
      return OA_ERR_NONE;
      break;
  }

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );
  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( ALTAIRCAM_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*valp = command->resultData;
  int			control = command->controlId;
  int32_t		val_s32;
  uint32_t		val_u32;
  unsigned short	val_u16;
  short			val_s16;

  switch ( control ) {

    case OA_CAM_CTRL_BRIGHTNESS:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_Brightness )( cameraInfo->handle,
          &val_s32 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_Brightness failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_s32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_CONTRAST:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_Contrast )( cameraInfo->handle, &val_s32 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_Contrast failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_s32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_GAMMA:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_Gamma )( cameraInfo->handle, &val_s32 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_Gamma failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_s32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_HFLIP:
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      if ((( p_Altaircam_get_HFlip )( cameraInfo->handle, &val_s32 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_HFlip failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->boolean = val_s32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_VFLIP:
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      if ((( p_Altaircam_get_VFlip )( cameraInfo->handle, &val_s32 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_VFlip failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->boolean = val_s32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      if ((( p_Altaircam_get_AutoExpoEnable )( cameraInfo->handle,
          &val_s32 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_AutoExpoEnable failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->boolean = val_s32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_ExpoTime )( cameraInfo->handle, &val_u32 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_ExpoTime failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_u32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_GAIN:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_ExpoAGain )( cameraInfo->handle,
          &val_u16 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_ExpoAGain failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_u16;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_SPEED:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_Speed )( cameraInfo->handle, &val_u16 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_Speed failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_u16;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_HUE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_Hue )( cameraInfo->handle, &val_s32 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_Hue failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_s32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_SATURATION:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_Saturation )( cameraInfo->handle,
          &val_s32 )) < 0 ){
        fprintf ( stderr, "Altaircam_get_Saturation failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_s32;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_RED_BALANCE:
    case OA_CAM_CTRL_BLUE_BALANCE:
    case OA_CAM_CTRL_GREEN_BALANCE:
    {
      int gain[3];
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_WhiteBalanceGain )( cameraInfo->handle,
          gain )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_WhiteBalanceGain (gain[3]) failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      switch ( control ) {
        case OA_CAM_CTRL_RED_BALANCE:
          val_s32 = gain[0];
          break;
        case OA_CAM_CTRL_BLUE_BALANCE:
          val_s32 = gain[2];
          break;
        case OA_CAM_CTRL_GREEN_BALANCE:
          val_s32 = gain[1];
          break;
      }
      valp->int32 = val_s32;
      return OA_ERR_NONE;
      break;
    }

    case OA_CAM_CTRL_TEMPERATURE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      if ((( p_Altaircam_get_Temperature )( cameraInfo->handle,
          &val_s16 )) < 0 ) {
        fprintf ( stderr, "Altaircam_get_Temperature failed\n" );
        return -OA_ERR_CAMERA_IO;
      }
      valp->int32 = val_s16;
      return OA_ERR_NONE;
      break;

    case OA_CAM_CTRL_BINNING:
      // FIX ME
      fprintf ( stderr, "%s: Need to code binning control for Altaircam\n",
          __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;

    case OA_CAM_CTRL_COOLER:
    case OA_CAM_CTRL_FAN:
      fprintf ( stderr, "%s: unimplemented control\n", __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );
  return -OA_ERR_INVALID_CONTROL;
}


static int
_processSetResolution ( ALTAIRCAM_STATE* cameraInfo, OA_COMMAND* command )
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

  if ((( p_Altaircam_put_Roi )( cameraInfo->handle, 0, 0, 0, 0 )) < 0 ) {
    fprintf ( stderr, "Can't clear Altaircam ROI\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if ((( p_Altaircam_put_Size )( cameraInfo->handle, size->x, size->y )) < 0 ) {
    fprintf ( stderr, "Can't set Altaircam frame size %dx%d\n", size->x,
      size->y );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->currentXSize = cameraInfo->currentXResolution = size->x;
  cameraInfo->currentYSize = cameraInfo->currentYResolution = size->y;
  cameraInfo->imageBufferLength = cameraInfo->currentXSize *
      cameraInfo->currentYSize * cameraInfo->currentBytesPerPixel;

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}


static int
_processSetROI ( oaCamera* camera, OA_COMMAND* command )
{
  ALTAIRCAM_STATE*		cameraInfo = camera->_private;
  FRAMESIZE*			size = command->commandData;
  unsigned int			offsetX, offsetY, x, y;

  if (!( camera->features.flags & OA_CAM_FEATURE_ROI )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  x = size->x;
  y = size->y;

  if ( x < 16 || x > cameraInfo->currentXResolution || ( x % 2 ) || y < 16 ||
     y > cameraInfo->currentYResolution || ( y % 2 )) {
    return -OA_ERR_OUT_OF_RANGE;
  }

  // ROI position must be even co-ordinate values

  offsetX = (( cameraInfo->currentXResolution - x ) / 2 ) & ~1;
  offsetY = (( cameraInfo->currentYResolution - y ) / 2 ) & ~1;

  if ((( p_Altaircam_put_Roi )( cameraInfo->handle, offsetX, offsetY, x,
      y )) < 0 ) {
    fprintf ( stderr, "Can't set Altaircam ROI ( %d, %d, %d, %d )\n",
        offsetX, offsetY, x, y );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->currentXSize = x;
  cameraInfo->currentYSize = y;
  cameraInfo->imageBufferLength = cameraInfo->currentXSize *
      cameraInfo->currentYSize * cameraInfo->currentBytesPerPixel;

  return OA_ERR_NONE;
}


static int
_processStreamingStart ( ALTAIRCAM_STATE* cameraInfo, OA_COMMAND* command )
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
_doStart ( ALTAIRCAM_STATE* cameraInfo )
{
  int			ret;

  if (( ret = ( p_Altaircam_StartPushModeV2 )( cameraInfo->handle,
      _AltairFrameCallback, cameraInfo )) < 0 ) {
    fprintf ( stderr, "%s: Altaircam_StartPushModeV2 failed: 0x%x\n",
        __FUNCTION__, ret );
    return -OA_ERR_CAMERA_IO;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}


static int
_processStreamingStop ( ALTAIRCAM_STATE* cameraInfo, OA_COMMAND* command )
{
  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  return _doStop ( cameraInfo );
}


static int
_doStop ( ALTAIRCAM_STATE* cameraInfo )
{
  int		ret;

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  if (( ret = ( p_Altaircam_Stop )( cameraInfo->handle )) < 0 ) {
    fprintf ( stderr, "%s: Altaircam_Stop failed: %d\n", __FUNCTION__, ret );
    return -OA_ERR_CAMERA_IO;
  }
  return OA_ERR_NONE;
}


static int
_setBinning ( ALTAIRCAM_STATE* cameraInfo, int binMode )
{
  int		restart = 0, x, y;

  if ( binMode < 0 || binMode > OA_MAX_BINNING ||
      cameraInfo->frameSizes[ binMode ].numSizes < 1 ) {
    return -OA_ERR_OUT_OF_RANGE;
  }

  // Reset the ROI

  if ((( p_Altaircam_put_Roi )( cameraInfo->handle, 0, 0, 0, 0 )) < 0 ) {
    fprintf ( stderr, "Can't clear Altaircam ROI\n" );
    return -OA_ERR_CAMERA_IO;
  }

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

  x = cameraInfo->frameSizes[ binMode ].sizes[0].x;
  y = cameraInfo->frameSizes[ binMode ].sizes[0].y;
  if ((( p_Altaircam_put_Size )( cameraInfo->handle, x, y )) < 0 ) {
    fprintf ( stderr, "Can't set Altaircam frame size\n" );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->binMode = binMode;
  cameraInfo->currentXSize = cameraInfo->currentXResolution = x;
  cameraInfo->currentYSize = cameraInfo->currentYResolution = y;

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}


static int
_setFrameFormat ( ALTAIRCAM_STATE* cameraInfo, int format )
{
  int           restart = 0;
  int           raw = 0, bitspp;

  // Only need to do this if we're dealing with a colour camera

  if ( !oaFrameFormats[ format ].monochrome ) {

    // FIX ME -- could make this more effcient by doing nothing here unless
    // we need to change it

    if ( cameraInfo->isStreaming ) {
      restart = 1;
      _doStop ( cameraInfo );
    }

    raw = oaFrameFormats[ format ].rawColour ? 1 : 0;
    if ((( p_Altaircam_put_Option )( cameraInfo->handle, ALTAIRCAM_OPTION_RAW,
        raw  )) < 0 ) {
      fprintf ( stderr, "Altaircam_put_Option ( raw, %d ) failed\n", raw );
      return -OA_ERR_CAMERA_IO;
    }

    if ((( p_Altaircam_put_Option )( cameraInfo->handle, ALTAIRCAM_OPTION_RGB,
        format == OA_PIX_FMT_RGB48LE ? 1 : 0 )) < 0 ) {
      fprintf ( stderr, "Altaircam_put_Option ( raw, %d ) failed\n", raw );
      return -OA_ERR_CAMERA_IO;
    }
    if ( restart ) {
      _doStart ( cameraInfo );
    }
  }

  // FIX ME -- don't do this if we don't need to
  // And now change the bit depth

  bitspp = oaFrameFormats[ format ].bitsPerPixel;
  if ((( p_Altaircam_put_Option )( cameraInfo->handle,
			ALTAIRCAM_OPTION_BITDEPTH, ( bitspp > 8 ) ? 1 : 0  )) < 0 ) {
    fprintf ( stderr, "Altaircam_put_Option ( depth, %d ) failed\n",
        bitspp > 8 ? 1 : 0 );
    return -OA_ERR_CAMERA_IO;
  }

  cameraInfo->currentVideoFormat = format;
  cameraInfo->currentBitsPerPixel = bitspp;
  // This converts from float, but should be ok for these cameras
  cameraInfo->currentBytesPerPixel = oaFrameFormats[ format ].bytesPerPixel;
  cameraInfo->imageBufferLength = cameraInfo->currentXSize *
      cameraInfo->currentYSize * cameraInfo->currentBytesPerPixel;

  return OA_ERR_NONE;
}


static int
_processExposureStart ( ALTAIRCAM_STATE* cameraInfo, OA_COMMAND* command )
{
  CALLBACK*		cb = command->commandData;
	int					ret;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

	if ( cameraInfo->exposureInProgress ) {
		return OA_ERR_NONE;
	}

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

  cameraInfo->imageBufferLength = cameraInfo->currentXSize *
      cameraInfo->currentYSize * cameraInfo->currentBytesPerPixel;

  if (( ret = ( p_Altaircam_StartPullModeWithCallback )( cameraInfo->handle,
      _AltairPullCallback, cameraInfo )) < 0 ) {
    fprintf ( stderr, "%s: Altaircam_StartPullModeWithCallback failed: 0x%x\n",
        __FUNCTION__, ret );
    return -OA_ERR_CAMERA_IO;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->exposureInProgress = 1;
  cameraInfo->abortExposure = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}


static void
_AltairPullCallback ( unsigned int event, void* ptr )
{
	ALTAIRCAM_STATE*			cameraInfo = ptr;
	AltaircamFrameInfoV2	frameInfo;
  int										buffersFree, nextBuffer, shiftBits, bitsPerPixel;
	int										bytesPerPixel, ret, abort;
  unsigned int					dataLength;

	// FIX ME -- this is very similar to the "push" callback, but not quite
	// It should be possible to combine the two

  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  buffersFree = cameraInfo->buffersFree;
  bitsPerPixel = cameraInfo->currentBitsPerPixel;
  bytesPerPixel = cameraInfo->currentBytesPerPixel;
	abort = cameraInfo->abortExposure;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

  if ( !abort && buffersFree && event == ALTAIRCAM_EVENT_IMAGE ) {
    dataLength = cameraInfo->imageBufferLength;
    nextBuffer = cameraInfo->nextBuffer;

		if (( ret = ( p_Altaircam_PullImageV2 )( cameraInfo->handle,
				cameraInfo->buffers[ nextBuffer ].start, bytesPerPixel * 8,
				&frameInfo )) < 0 ) {
			fprintf ( stderr, "%s: Altaircam_PullImageV2 failed: 0x%x\n",
					__FUNCTION__, ret );
			return;
		}

		// Now here's the fun...
		//
		// In 12-bit (and presumably 10- and 14-bit) mode, Altair cameras
		// appear to return little-endian data, but right-aligned rather than
		// left-aligned as many other cameras do.  So if we have such an image we
		// try to fix it here.
		//
		// FIX ME -- I'm not sure this is the right place to be doing this.
		// Perhaps there should be a flag to tell the user whether the data is
		// left-or right-aligned and they can sort it out.

		if ( bitsPerPixel > 8 && bitsPerPixel < 16 ) {
			shiftBits = 16 - bitsPerPixel;

			if ( shiftBits ) {
				uint16_t	*s = cameraInfo->buffers[ nextBuffer ].start;
				uint16_t	v;
				unsigned int	i;

				for ( i = 0; i < dataLength; i += 2 ) {
					v = *s;
					v <<= shiftBits;
					*s++ = v;
				}
			}
		}

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
		cameraInfo->nextBuffer = ( nextBuffer + 1 ) %
				cameraInfo->configuredBuffers;
		pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
		pthread_cond_broadcast ( &cameraInfo->callbackQueued );
	}
}


static int
_processAbortExposure ( ALTAIRCAM_STATE* cameraInfo )
{
	int			ret;

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->abortExposure = 1;
  cameraInfo->exposureInProgress = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  if (( ret = ( p_Altaircam_Stop )( cameraInfo->handle )) < 0 ) {
    fprintf ( stderr, "%s: Altaircam_Stop failed: %d\n", __FUNCTION__, ret );
    return -OA_ERR_CAMERA_IO;
  }

  return OA_ERR_NONE;
}
