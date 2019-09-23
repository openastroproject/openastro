/*****************************************************************************
 *
 * UVCcontroller.c -- Main camera controller thread
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

#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/util.h>
#include <sys/time.h>
#include <libuvc/libuvc.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "UVC.h"
#include "UVCoacam.h"
#include "UVCstate.h"
#include "UVCprivate.h"


static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processGetControl ( oaCamera*, OA_COMMAND* );
static int	_processSetResolution ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStop ( UVC_STATE*, OA_COMMAND* );
static int	_processSetFrameInterval ( oaCamera*, OA_COMMAND* );
static int	_doStart ( oaCamera* );
static int	_doCameraConfig ( oaCamera*, OA_COMMAND* );
int		_doSetUVCControl ( uvc_device_handle_t*, uint8_t, uint8_t,
			int, int );


void*
oacamUVCcontroller ( void* param )
{
  oaCamera*		camera = param;
  UVC_STATE*		cameraInfo = camera->_private;
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
            resultCode = _processGetControl ( camera, command );
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
          case OA_CMD_FRAME_INTERVAL_SET:
            resultCode = _processSetFrameInterval ( camera, command );
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
  } while ( !exitThread );

  return 0;
}


static void
_UVCFrameCallback ( uvc_frame_t *frame, void *ptr )
{
  oaCamera*	camera = ptr;
  UVC_STATE*	cameraInfo = camera->_private;
  int		buffersFree, nextBuffer;
  unsigned int	dataLength;

  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  buffersFree = cameraInfo->buffersFree;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

  if ( buffersFree && frame->data_bytes ) {
    if (( dataLength = frame->data_bytes ) > cameraInfo->currentFrameLength ) {
      dataLength = cameraInfo->currentFrameLength;
    }
    nextBuffer = cameraInfo->nextBuffer;
    ( void ) memcpy ( cameraInfo->buffers[ nextBuffer ].start, frame->data,
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
_processSetControl ( oaCamera* camera, OA_COMMAND* command )
{
  oaControlValue	*valp = command->commandData;
  int			control = command->controlId;
  int			found, len, err;
  uint8_t		uvcControl;
  unsigned int		i;
  int32_t		val_s32;
  int64_t		val_s64;
  UVC_STATE*		cameraInfo = camera->_private;

  for ( i = 0, found = 0; !found && i < numPUControls; i++ ) {
    if ( UVCControlData[i].oaControl == control ) {
      uvcControl = UVCControlData[i].uvcControl;
      len = UVCControlData[i].size;
      found = 1;
      // make all non-zero boolean values 1
      if ( OA_CTRL_TYPE_BOOLEAN == UVCControlData[i].oaControlType ) {
        val_s32 = valp->boolean ? 1 : 0;
      } else {
        val_s32 = valp->int32;
      }
    }
  }

  if ( found ) {
    switch ( control ) {

      case OA_CAM_CTRL_BRIGHTNESS:
      case OA_CAM_CTRL_CONTRAST:
      case OA_CAM_CTRL_SATURATION:
      case OA_CAM_CTRL_HUE:
      case OA_CAM_CTRL_GAMMA:
      case OA_CAM_CTRL_GAIN:
      case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ):
      case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
      case OA_CAM_CTRL_SHARPNESS:
      case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
      case OA_CAM_CTRL_BACKLIGHT_COMPENSATION:
      case OA_CAM_CTRL_POWER_LINE_FREQ:
      case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE_TEMP ):
      case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ):
        return _doSetUVCControl ( cameraInfo->uvcHandle, cameraInfo->unitId,
            uvcControl, len, val_s32 );
        break;

      default:
        fprintf ( stderr, "Unrecognised control %d in %s\n", control,
            __FUNCTION__ );
        return -OA_ERR_INVALID_CONTROL;
        break;
    }
  }

  // Now handle the ones that are not in a processing unit, and white
  // component which is something of a special case

  switch ( control ) {

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val_s64 = valp->int64 / 100;
      val_s32 = val_s64 & 0xffffffff;
      if (( err = p_uvc_set_exposure_abs ( cameraInfo->uvcHandle, val_s32 ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_exposure_abs() failed in %s, err = %d\n",
            __FUNCTION__, err );
      }
      cameraInfo->currentAbsoluteExposure = val_s32;
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
			if (( err = p_uvc_set_ae_mode ( cameraInfo->uvcHandle, valp->menu ))
					!= UVC_SUCCESS ) {
				fprintf ( stderr, "uvc_set_ae_mode( %d ) failed in %s, err %d\n",
						valp->menu, __FUNCTION__, err );
			}
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      val_s32 = valp->int32;
      cameraInfo->componentBalance = ( cameraInfo->componentBalance &
          ~0xffff ) | ( val_s32 & 0xffff );
        return _doSetUVCControl ( cameraInfo->uvcHandle, cameraInfo->unitId,
            UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL, 4,
            cameraInfo->componentBalance );

    case OA_CAM_CTRL_RED_BALANCE:
      val_s32 = valp->int32;
      cameraInfo->componentBalance = ( cameraInfo->componentBalance &
          0xffff ) | (( val_s32 & 0xffff ) <<  16 );
        return _doSetUVCControl ( cameraInfo->uvcHandle, cameraInfo->unitId,
            UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL, 4,
            cameraInfo->componentBalance );

    case OA_CAM_CTRL_FRAME_FORMAT:
      if ( valp->valueType != OA_CTRL_TYPE_DISCRETE ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "expected\n", __FUNCTION__, valp->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s32 = valp->discrete;
      if ( !cameraInfo->frameFormatMap[ val_s32 ]) {
        return -OA_ERR_OUT_OF_RANGE;
      }
      cameraInfo->currentFrameFormat = val_s32;
      cameraInfo->currentUVCFormat = cameraInfo->frameFormatMap[ val_s32 ];
      cameraInfo->currentUVCFormatId = cameraInfo->frameFormatIdMap[ val_s32 ];
      return _doCameraConfig ( camera, command );
      break;

    case OA_CAM_CTRL_INTERLACE_ENABLE:
    {
      uint8_t data = valp->boolean ? 1 : 0;
      if (( err = p_uvc_set_scanning_mode ( cameraInfo->uvcHandle, data ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_scanning_mode ( %d ) failed in %s, err %d\n",
            data, __FUNCTION__, err );
      }
      break;
    }

    case OA_CAM_CTRL_ZOOM_ABSOLUTE:
    {
      uint16_t data = valp->int32;
      if (( err = p_uvc_set_zoom_abs ( cameraInfo->uvcHandle, data ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_zoom_abs ( %d ) failed in %s, err %d\n",
            data, __FUNCTION__, err );
      }
      break;
    }

    case OA_CAM_CTRL_FOCUS_ABSOLUTE:
    {
      uint16_t data = valp->int32;
      if (( err = p_uvc_set_focus_abs ( cameraInfo->uvcHandle, data ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_focus_abs ( %d ) failed in %s, err %d\n",
            data, __FUNCTION__, err );
      }
      break;
    }

    case OA_CAM_CTRL_IRIS_ABSOLUTE:
    {
      uint16_t data = valp->int32;
      if (( err = p_uvc_set_iris_abs ( cameraInfo->uvcHandle, data ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_iris_abs ( %d ) failed in %s, err %d\n",
            data, __FUNCTION__, err );
      }
      break;
    }

    case OA_CAM_CTRL_PAN_ABSOLUTE:
    case OA_CAM_CTRL_TILT_ABSOLUTE:
    {
      int32_t data = valp->int32;

      if ( OA_CAM_CTRL_PAN_ABSOLUTE == control ) {
        cameraInfo->currentPan = data;
      } else {
        cameraInfo->currentTilt = data;
      }
      if (( err = p_uvc_set_pantilt_abs ( cameraInfo->uvcHandle,
          cameraInfo->currentPan, cameraInfo->currentTilt )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_pantilt_abs ( %d, %d ) failed in %s,"
            " err %d\n", cameraInfo->currentPan, cameraInfo->currentTilt,
            __FUNCTION__, err );
      }
      break;
    }

    case OA_CAM_CTRL_ROLL_ABSOLUTE:
    {
      int16_t data = valp->int32;
      if (( err = p_uvc_set_roll_abs ( cameraInfo->uvcHandle, data ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_iris_abs ( %d ) failed in %s, err %d\n",
            data, __FUNCTION__, err );
      }
      break;
    }


    case OA_CAM_CTRL_PRIVACY_ENABLE:
    {
      uint8_t data = valp->boolean ? 1 : 0;
      if (( err = p_uvc_set_privacy ( cameraInfo->uvcHandle, data ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_privacy ( %d ) failed in %s, err %d\n",
            data, __FUNCTION__, err );
      }
      break;
    }

    case OA_CAM_CTRL_FOCUS_SIMPLE:
    {
      // FIX ME -- more error checking might be good?
      uint8_t data = valp->menu & 0xff;

      if (( err = p_uvc_set_focus_simple_range ( cameraInfo->uvcHandle, data ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_focus_simple_range( %d ) failed in %s,"
            " err %d\n", data, __FUNCTION__, err );
      }
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_ABSOLUTE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_RELATIVE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_SIMPLE ):
    {
      uint8_t data = valp->boolean ? 1 : 0;
      if (( err = p_uvc_set_focus_auto ( cameraInfo->uvcHandle, data ))
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_set_focus_auto ( %d ) failed in %s, err %d\n",
            data, __FUNCTION__, err );
      }
      break;
    }

    default:
      fprintf ( stderr, "Unrecognised control %d in %s\n", control,
          __FUNCTION__ );
      break;
  }

  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( oaCamera* camera, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	valp = command->resultData;
  int			found, len, err;
  uint8_t		uvcControl;
  unsigned int		i;
  UVC_STATE*		cameraInfo = camera->_private;

  for ( i = 0, found = 0; !found && i < numPUControls; i++ ) {
    if ( UVCControlData[i].oaControl == control ) {
      uvcControl = UVCControlData[i].uvcControl;
      len = UVCControlData[i].size;
      found = 1;
    }
  }

  if ( found ) {
    switch ( control ) {

      case OA_CAM_CTRL_BRIGHTNESS:
      case OA_CAM_CTRL_CONTRAST:
      case OA_CAM_CTRL_SATURATION:
      case OA_CAM_CTRL_HUE:
      case OA_CAM_CTRL_GAMMA:
      case OA_CAM_CTRL_GAIN:
      case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
      case OA_CAM_CTRL_SHARPNESS:
      case OA_CAM_CTRL_BACKLIGHT_COMPENSATION:
        valp->valueType = OA_CTRL_TYPE_INT32;
        valp->int32 = getUVCControl ( cameraInfo->uvcHandle,
            cameraInfo->unitId, uvcControl, len, UVC_GET_CUR );
        break;

      case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ):
      case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
      case OA_CAM_CTRL_AUTO_WHITE_BALANCE_TEMP:
      case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE_TEMP ):
      case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTRAST ):
        valp->valueType = OA_CTRL_TYPE_BOOLEAN;
        valp->boolean = getUVCControl ( cameraInfo->uvcHandle,
            cameraInfo->unitId, uvcControl, len, UVC_GET_CUR );
        break;

      case OA_CAM_CTRL_POWER_LINE_FREQ:
        valp->valueType = OA_CTRL_TYPE_MENU;
        valp->menu = getUVCControl ( cameraInfo->uvcHandle,
            cameraInfo->unitId, uvcControl, len, UVC_GET_CUR );
        break;

      default:
        fprintf ( stderr, "Unrecognised control %d in %s\n", control,
            __FUNCTION__ );
        return -OA_ERR_INVALID_CONTROL;
        break;
    }
    return OA_ERR_NONE;
  }

  // Now handle the ones that are not in a processing unit...

  switch ( control ) {

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    {
      uint32_t data32;
      int64_t data64;

      if ( p_uvc_get_exposure_abs ( cameraInfo->uvcHandle, &data32,
					UVC_GET_CUR ) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_exposure_abs() failed in %s\n",
            __FUNCTION__ );
      }
      data64 = data32;
      data64 *= 1000; // convert msec to usec
      valp->valueType = OA_CTRL_TYPE_INT64;
      valp->int64 = data64;
      break;
    }
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
    {
      uint8_t data;

      if ( p_uvc_get_ae_mode ( cameraInfo->uvcHandle, &data, UVC_GET_CUR )
          != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_ae_mode() failed in %s\n",
            __FUNCTION__ );
      }
      valp->valueType = OA_CTRL_TYPE_MENU;
      valp->menu = data;
			break;
    }
    case OA_CAM_CTRL_BLUE_BALANCE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = cameraInfo->componentBalance & 0xffff;
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = cameraInfo->componentBalance >> 16;
      break;

    case OA_CAM_CTRL_INTERLACE_ENABLE:
    {
      uint8_t data;

      if (( err = p_uvc_get_scanning_mode ( cameraInfo->uvcHandle, &data,
          UVC_GET_CUR )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_scanning_mode() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      valp->boolean = data ? 1 : 0;
      break;
    }

    case OA_CAM_CTRL_ZOOM_ABSOLUTE:
    {
      uint16_t data;

      if (( err = p_uvc_get_zoom_abs ( cameraInfo->uvcHandle, &data,
          UVC_GET_CUR )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_zoom_abs() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = data;
      break;
    }

    case OA_CAM_CTRL_FOCUS_ABSOLUTE:
    {
      uint16_t data;

      if (( err = p_uvc_get_focus_abs ( cameraInfo->uvcHandle, &data,
          UVC_GET_CUR )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_focus_abs() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = data;
      break;
    }

    case OA_CAM_CTRL_IRIS_ABSOLUTE:
    {
      uint16_t data;

      if (( err = p_uvc_get_iris_abs ( cameraInfo->uvcHandle, &data,
          UVC_GET_CUR )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_iris_abs() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = data;
      break;
    }

    case OA_CAM_CTRL_PAN_ABSOLUTE:
    case OA_CAM_CTRL_TILT_ABSOLUTE:

      if (( err = p_uvc_get_pantilt_abs ( cameraInfo->uvcHandle,
          &cameraInfo->currentPan, &cameraInfo->currentTilt, UVC_GET_CUR )) !=
          UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_pantilt_abs() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = OA_CAM_CTRL_PAN_ABSOLUTE == control ?
          cameraInfo->currentPan : cameraInfo->currentTilt;
      break;

    case OA_CAM_CTRL_ROLL_ABSOLUTE:
    {
      int16_t data;

      if (( err = p_uvc_get_roll_abs ( cameraInfo->uvcHandle, &data,
          UVC_GET_CUR )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_iris_abs() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      break;
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = data;
    }


    case OA_CAM_CTRL_PRIVACY_ENABLE:
    {
      uint8_t data;

      if (( err = p_uvc_get_privacy ( cameraInfo->uvcHandle, &data,
          UVC_GET_CUR )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_privacy() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      valp->boolean = data ? 1 : 0;
      break;
    }

    case OA_CAM_CTRL_FOCUS_SIMPLE:
    {
      uint8_t data;

      if (( err = p_uvc_get_focus_simple_range ( cameraInfo->uvcHandle, &data,
          UVC_GET_CUR )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_focus_simple_range() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      valp->valueType = OA_CTRL_TYPE_MENU;
      valp->menu = data;
      break;
    }

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_ABSOLUTE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_RELATIVE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_SIMPLE ):
    {
      uint8_t data;

      if (( err = p_uvc_get_focus_auto ( cameraInfo->uvcHandle, &data,
          UVC_GET_CUR )) != UVC_SUCCESS ) {
        fprintf ( stderr, "uvc_get_focus_auto() failed in %s, err %d\n",
            __FUNCTION__, err );
      }
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      valp->boolean = data ? 1 : 0;
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


int
getUVCControl ( uvc_device_handle_t* uvcHandle, uint8_t unitId, uint8_t ctrl,
    int len, enum uvc_req_code req )
{
  int		ret, i;
  uint8_t	data[4] = { 0xde, 0xad, 0xbe, 0xef };
  unsigned int	val;

  if (( ret = p_uvc_get_ctrl ( uvcHandle, unitId, ctrl, data, len, req )) !=
      len ) {
    fprintf ( stderr, "%s requested %d for control %d, got %d\n",
        __FUNCTION__, len, ctrl, ret );
    return ret;
  }

  val = 0;
  i = len - 1;
  do {
    val <<= 8;
    val += data[i--];
  } while ( i >= 0 );

  return val;
}


static int
_processSetResolution ( oaCamera* camera, OA_COMMAND* command )
{
  UVC_STATE*	cameraInfo = camera->_private;
  FRAMESIZE*	size = command->commandData;

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;
  return _doCameraConfig ( camera, command );
}


static int
_processSetFrameInterval ( oaCamera* camera, OA_COMMAND* command )
{
  UVC_STATE*	cameraInfo = camera->_private;
  FRAMERATE*	rate = command->commandData;

  cameraInfo->frameRateNumerator = rate->numerator;
  cameraInfo->frameRateDenominator = rate->denominator;
  return _doCameraConfig ( camera, command );
}


int
_doSetUVCControl ( uvc_device_handle_t* uvcHandle, uint8_t unitId,
    uint8_t ctrl, int len, int value )
{
  int		ret, i;
  uint8_t	data[4] = { 0x00, 0x00, 0x00, 0x00 };

  for ( i = 0; i < len; i++ ) {
    data[i] = value & 0xff;
    value >>= 8;
  }

  if (( ret = p_uvc_set_ctrl ( uvcHandle, unitId, ctrl, data, len )) !=
      len ) {
    fprintf ( stderr, "%s requested %d for control %d, got %d\n",
        __FUNCTION__, len, ctrl, ret );
    return ret;
  }
  return ret;
}


static int
_doCameraConfig ( oaCamera* camera, OA_COMMAND* command )
{
  int		r;
  UVC_STATE*	cameraInfo = camera->_private;

  if (( r = _processStreamingStop ( cameraInfo, command ))) {
    return r;
  }
  return _doStart ( camera );
}


static int
_processStreamingStart ( oaCamera* camera, OA_COMMAND* command )
{
  UVC_STATE*	cameraInfo = camera->_private;
  CALLBACK*	cb = command->commandData;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

  return _doStart ( camera );
}


static int
_doStart ( oaCamera* camera )
{
  UVC_STATE*			cameraInfo = camera->_private;
  const uvc_frame_desc_t*	frame;
  int				matched, res, multiplier;

  // Need to pick appropriate resolution out of current frame format
  // entry and kick off streaming with that.

  frame = cameraInfo->currentUVCFormat->frame_descs;
  matched = 0;
  do {
    if ( frame->wWidth == cameraInfo->xSize && frame->wHeight ==
        cameraInfo->ySize ) {
      matched = 1;
    } else {
      frame = frame->next;
    }
  } while ( !matched && frame );

  if ( !matched ) {
    fprintf ( stderr, "Requested image size %dx%d not available\n",
        cameraInfo->xSize, cameraInfo->ySize );
    return -OA_ERR_OUT_OF_RANGE;
  }

  if ( frame->bFrameIntervalType ) {
		camera->features.flags |= OA_CAM_FEATURE_FRAME_RATES;
	} else {
		camera->features.flags &= ~OA_CAM_FEATURE_FRAME_RATES;
	}
//cameraInfo->frameInterval = frame->dwDefaultFrameInterval;

  multiplier = oaFrameFormats[ cameraInfo->currentFrameFormat ].bytesPerPixel;
  cameraInfo->currentFrameLength = cameraInfo->xSize * cameraInfo->ySize *
      multiplier;
  res = p_uvc_get_stream_ctrl_format_size ( cameraInfo->uvcHandle,
      &cameraInfo->streamControl, cameraInfo->currentUVCFormatId,
      cameraInfo->xSize, cameraInfo->ySize,
      cameraInfo->frameRateDenominator / cameraInfo->frameRateNumerator );
  if ( UVC_ERROR_INVALID_MODE == res ) {
    // have another go with whatever frame rate we can find
    res = p_uvc_get_stream_ctrl_format_size ( cameraInfo->uvcHandle,
        &cameraInfo->streamControl, cameraInfo->currentUVCFormatId,
        cameraInfo->xSize, cameraInfo->ySize, 0 );
  }
  if ( res < 0 ) {
    fprintf ( stderr, "uvc_get_stream_ctrl_format_size returned %d\n", res );
    return -OA_ERR_CAMERA_IO;
  }

  if (( res = p_uvc_start_streaming ( cameraInfo->uvcHandle,
      &cameraInfo->streamControl, _UVCFrameCallback, camera, 0 )) < 0 ) {
    fprintf ( stderr, "uvc_start_streaming returned %d\n", res );
    return -OA_ERR_CAMERA_IO;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  return OA_ERR_NONE;
}


static int
_processStreamingStop ( UVC_STATE* cameraInfo, OA_COMMAND* command )
{
  int		queueEmpty;

  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  p_uvc_stop_streaming ( cameraInfo->uvcHandle );

  // We wait here until the callback queue has drained otherwise a future
  // close of the camera could rip the image frame out from underneath the
  // callback

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
