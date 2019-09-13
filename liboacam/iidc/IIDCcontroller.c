/*****************************************************************************
 *
 * IIDCcontroller.c -- Main camera controller thread
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
#include <sys/time.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "IIDC.h"
#include "IIDCoacam.h"
#include "IIDCstate.h"
#include "IIDCprivate.h"


static int	_processSetControl ( IIDC_STATE*, OA_COMMAND* );
static int	_processGetControl ( IIDC_STATE*, OA_COMMAND* );
static int	_processSetResolution ( IIDC_STATE*, OA_COMMAND* );
static int	_processStreamingStart ( IIDC_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( IIDC_STATE*, OA_COMMAND* );
static int	_processSetFrameInterval ( IIDC_STATE*, OA_COMMAND* );
static int	_doCameraConfig ( IIDC_STATE* );


void*
oacamIIDCcontroller ( void* param )
{
  oaCamera*		camera = param;
  IIDC_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode, nextBuffer, buffersFree, frameWait;
  int			haveFrame;
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
          case OA_CMD_CONTROL_GET:
            resultCode = _processGetControl ( cameraInfo, command );
            break;
          case OA_CMD_RESOLUTION_SET:
            resultCode = _processSetResolution ( cameraInfo, command );
            break;
          case OA_CMD_START_STREAMING:
            resultCode = _processStreamingStart ( cameraInfo, command );
            break;
          case OA_CMD_STOP_STREAMING:
            resultCode = _processStreamingStop ( cameraInfo, command );
            break;
          case OA_CMD_FRAME_INTERVAL_SET:
            resultCode = _processSetFrameInterval ( cameraInfo, command );
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

    if ( streaming ) {

      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      frameWait = cameraInfo->currentAbsoluteExposure;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

      // pause between tests for frame here is no more than 1ms
//    maxWaitTime = frameWait * 2;
      if ( frameWait > 1000 ) {
        frameWait = 1000;
      }

      pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
      buffersFree = cameraInfo->buffersFree;
      pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

      if ( buffersFree ) {
        nextBuffer = cameraInfo->nextBuffer;
        haveFrame = 0;
//      do {
          // This mutex prevents attempts to dequeue frames if streaming
          // has stopped since we last checked
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          streaming = cameraInfo->isStreaming;
          if ( streaming ) {
            if ( p_dc1394_capture_dequeue ( cameraInfo->iidcHandle,
                DC1394_CAPTURE_POLICY_POLL, &cameraInfo->currentFrame )
                == DC1394_SUCCESS ) {
              haveFrame = ( cameraInfo->currentFrame != 0 ) ? 1 : 0;
            }
          }
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
                  cameraInfo->currentFrame;
              cameraInfo->frameCallbacks[ nextBuffer ].bufferLen =
                  cameraInfo->currentFrame->image_bytes;
              pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
              oaDLListAddToTail ( cameraInfo->callbackQueue,
                  &cameraInfo->frameCallbacks[ nextBuffer ]);
              cameraInfo->buffersFree--;
              cameraInfo->nextBuffer = ( nextBuffer + 1 ) %
                  cameraInfo->configuredBuffers;
              pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
              pthread_cond_broadcast ( &cameraInfo->callbackQueued );
            } else {
              usleep ( frameWait );
//            maxWaitTime -= frameWait;
            }
          }
//      } while ( !exitThread && streaming && !haveFrame && maxWaitTime > 0 );
      }
    }
  } while ( !exitThread );

  return 0;
}


static int
_processSetControl ( IIDC_STATE* cameraInfo, OA_COMMAND* command )
{
  oaControlValue	*val = command->commandData;
  int			control = command->controlId;
  int			found, iidcControl = 0;
  unsigned int		i;

  for ( i = 0, found = -1; found < 0 && i < numIIDCControls; i++ ) {
    if ( dc1394Controls[i].oaControl == OA_CAM_CTRL_MODE_NONAUTO( control )) {
      iidcControl = dc1394Controls[i].iidcControl;
      found = 1;
    }
  }

  if ( OA_CAM_CTRL_EXPOSURE_UNSCALED == control ||
      OA_CAM_CTRL_EXPOSURE_ABSOLUTE == control ||
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ) == control ||
      OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) == control ) {
    iidcControl = DC1394_FEATURE_SHUTTER;
  }

  if ( OA_CAM_CTRL_IS_AUTO ( control )) {
    uint32_t val_u32;
    if ( OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ) == control ||
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) == control ) {
      if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u32 = val->boolean;
      if ( val_u32 != 0 && val_u32 != 1 ) {
        fprintf ( stderr, "%s: control value out of range\n", __FUNCTION__ );
        return -OA_ERR_OUT_OF_RANGE;
      }
    } else {
      // anything here should be a boolean value
      if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_u32 = val->boolean;
    }
    if ( p_dc1394_feature_set_mode ( cameraInfo->iidcHandle, iidcControl,
        val_u32 ? DC1394_FEATURE_MODE_AUTO : DC1394_FEATURE_MODE_MANUAL ) !=
        DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_set_mode failed for control %d\n",
          __FUNCTION__, iidcControl );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_IS_ON_OFF ( control )) {
    uint32_t val_u32;
    // anything here should be a boolean value
    if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
          __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    val_u32 = val->boolean;
    if ( p_dc1394_feature_set_power ( cameraInfo->iidcHandle, iidcControl,
        val_u32 ? DC1394_ON : DC1394_OFF ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_set_power failed for control %d\n",
          __FUNCTION__, iidcControl );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  // We shouldn't allow white balance at this point as it only supports
  // auto on/off
  if ( OA_CAM_CTRL_WHITE_BALANCE == control ) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( OA_CAM_CTRL_BLUE_BALANCE == control || OA_CAM_CTRL_RED_BALANCE ==
      control ) {
    uint32_t val_u32;

    // control only supports a 32-bit unsigned value
    val_u32 = val->int32;
    if ( OA_CAM_CTRL_BLUE_BALANCE == control ) {
      cameraInfo->currentBlueBalance = val_u32;
    }
    if ( OA_CAM_CTRL_RED_BALANCE == control ) {
      cameraInfo->currentRedBalance = val_u32;
    }
    if ( p_dc1394_feature_whitebalance_set_value ( cameraInfo->iidcHandle,
        cameraInfo->currentBlueBalance, cameraInfo->currentRedBalance ) !=
        DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_whitebalance_set_value failed\n",
          __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( found >= 0 || OA_CAM_CTRL_EXPOSURE_UNSCALED == control ) {
    uint32_t val_u32;
    int64_t val_s64;
    // The value for dc1394_feature_set_value() is uint32_t, which we have
    // to map to an int64_t, so we need to throw an error if we don't have
    // an INT64 value here, or if it is negative
    if ( OA_CTRL_TYPE_INT64 != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where int64 expected "
          "for OA_CAM_CTRL_EXPOSURE\n", __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    val_s64 = val->int64;
    if ( val < 0 ) {
      fprintf ( stderr, "%s: invalid control value %d for exposure\n",
          __FUNCTION__, val->valueType );
      return -OA_ERR_OUT_OF_RANGE;
    }
    val_u32 = val_s64 & 0xffffffff;
    if ( p_dc1394_feature_set_value ( cameraInfo->iidcHandle, iidcControl,
        val_u32 ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_set_value failed for "
          "control %d\n", __FUNCTION__, control );
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
    decval = val_s64 / 1000000.0;
    if ( p_dc1394_feature_set_absolute_value ( cameraInfo->iidcHandle,
        iidcControl, decval ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_set_absolute_value %d failed\n",
          __FUNCTION__, control );
      return -OA_ERR_CAMERA_IO;
    }
    cameraInfo->currentAbsoluteExposure = val_s64;
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_COOLER == control ) {
    // This is a boolean
    // 0 -- swich the feature off
    // 1 -- swich the feature on, and enable auto mode if
    //      OA_CAM_CTRL_TEMP_SETPOINT is allowed

    if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where boolean expected "
          "for OA_CAM_CTRL_COOLER\n", __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }

    if ( val->boolean ) {
      if ( p_dc1394_feature_set_power ( cameraInfo->iidcHandle,
          DC1394_FEATURE_TEMPERATURE, DC1394_ON ) != DC1394_SUCCESS ) {
        fprintf ( stderr, "%s: dc1394_feature_set_power %d failed\n",
            __FUNCTION__, control );
        return -OA_ERR_CAMERA_IO;
      }
      if ( cameraInfo->haveSetpointCooling ) {
        if ( p_dc1394_feature_set_mode ( cameraInfo->iidcHandle,
            DC1394_FEATURE_TEMPERATURE, DC1394_FEATURE_MODE_AUTO ) !=
            DC1394_SUCCESS ) {
          fprintf ( stderr, "%s: dc1394_feature_set_mode fail for control %d\n",
              __FUNCTION__, iidcControl );
          return -OA_ERR_CAMERA_IO;
        }
      }
    } else {
      if ( p_dc1394_feature_set_power ( cameraInfo->iidcHandle,
          DC1394_FEATURE_TEMPERATURE, DC1394_OFF ) != DC1394_SUCCESS ) {
        fprintf ( stderr, "%s: dc1394_feature_set_power %d failed\n",
            __FUNCTION__, control );
        return -OA_ERR_CAMERA_IO;
      }
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_TEMP_SETPOINT == control ) {
    int32_t setpoint;

    if ( OA_CTRL_TYPE_INT32 != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where int32 expected "
          "for OA_CAM_CTRL_TEMP_SETPOINT\n", __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }

    // allegedly the temperature is in K, so we need to rebase from C
    // Looks like the measurement is actually 1/10th K.
    setpoint = val->int32 + 273.15;
    if ( p_dc1394_feature_temperature_set_value ( cameraInfo->iidcHandle,
        setpoint ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_set_value failed for "
          "control %d\n", __FUNCTION__, control );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_TRIGGER_ENABLE == control ) {
    if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where boolean expected "
          "for OA_CAM_CTRL_TRIGGER_ENABLE\n", __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }

    if ( p_dc1394_external_trigger_set_power ( cameraInfo->iidcHandle,
        val->boolean ? DC1394_ON : DC1394_OFF ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_external_trigger_set_power %d failed\n",
          __FUNCTION__, control );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_TRIGGER_MODE == control ) {
    if ( OA_CTRL_TYPE_MENU != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where boolean expected "
          "for OA_CAM_CTRL_TRIGGER_MENU\n", __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    
    if ( p_dc1394_external_trigger_set_mode ( cameraInfo->iidcHandle,
        val->menu ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_external_trigger_set_power %d failed\n",
          __FUNCTION__, control );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_TRIGGER_POLARITY == control ) {
    if ( OA_CTRL_TYPE_MENU != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where boolean expected "
          "for OA_CAM_CTRL_TRIGGER_POLARITY\n", __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }

    if ( p_dc1394_external_trigger_set_polarity ( cameraInfo->iidcHandle,
        val->menu ? DC1394_TRIGGER_ACTIVE_LOW : DC1394_TRIGGER_ACTIVE_HIGH )
        != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_external_trigger_set_polarity %d failed\n",
          __FUNCTION__, control );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_TRIGGER_DELAY_ENABLE == control ) {
    if ( OA_CTRL_TYPE_BOOLEAN != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where boolean expected "
          "for OA_CAM_CTRL_TRIGGER_DELAY_ENABLE\n", __FUNCTION__,
          val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }

    if ( p_dc1394_feature_set_power ( cameraInfo->iidcHandle,
        DC1394_FEATURE_TRIGGER_DELAY, val->boolean ? DC1394_ON :
        DC1394_OFF ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_set_power %d failed\n",
          __FUNCTION__, control );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_TRIGGER_DELAY == control ) {
    uint32_t val_u32;
    if ( OA_CTRL_TYPE_INT32 != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where int32 expected "
          "for OA_CAM_CTRL_TRIGGER_DELAY\n", __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    val_u32 = val->int32;
    if ( p_dc1394_feature_set_value ( cameraInfo->iidcHandle, iidcControl,
        val_u32 ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_set_value failed for "
          "control %d\n", __FUNCTION__, control );
      return -OA_ERR_CAMERA_IO;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_FRAME_FORMAT == control ) {
    uint32_t format;
    if ( OA_CTRL_TYPE_DISCRETE != val->valueType ) {
      fprintf ( stderr, "%s: invalid control type %d where discrete expected "
          "for OA_CAM_CTRL_FRAME_FORMAT\n", __FUNCTION__, val->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
    }
    format = val->discrete;
    switch ( format ) {
      case OA_PIX_FMT_GREY8:
        cameraInfo->currentCodec = DC1394_COLOR_CODING_MONO8;
        break;
      case OA_PIX_FMT_YUV411:
        cameraInfo->currentCodec = DC1394_COLOR_CODING_YUV411;
        break;
      case OA_PIX_FMT_YUYV:
        cameraInfo->currentCodec = DC1394_COLOR_CODING_YUV422;
        break;
      case OA_PIX_FMT_YUV444:
        cameraInfo->currentCodec = DC1394_COLOR_CODING_YUV444;
        break;
      case OA_PIX_FMT_RGB24:
        cameraInfo->currentCodec = DC1394_COLOR_CODING_RGB8;
        break;
      case OA_PIX_FMT_GREY16LE:
        cameraInfo->currentCodec = DC1394_COLOR_CODING_MONO16;
        break;
      case OA_PIX_FMT_RGB48LE:
        cameraInfo->currentCodec = DC1394_COLOR_CODING_RGB16;
        break;
      case OA_PIX_FMT_GBRG8:
        if ( cameraInfo->isTISColour ) {
          cameraInfo->currentCodec = DC1394_COLOR_CODING_MONO8;
        } else {
          cameraInfo->currentCodec = DC1394_COLOR_CODING_RAW8;
        }
        break;
      case OA_PIX_FMT_GBRG16LE:
        cameraInfo->currentCodec = DC1394_COLOR_CODING_RAW16;
        break;
    }
    return _doCameraConfig ( cameraInfo );
  }

  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );
  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( IIDC_STATE* cameraInfo, OA_COMMAND* command )
{
  int			found, iidcControl = 0;
  unsigned int		i;
  int			control = command->controlId;
  oaControlValue*	val = command->resultData;

  // We can handle these first as a special case
  if ( OA_CAM_CTRL_BLUE_BALANCE == control || OA_CAM_CTRL_RED_BALANCE ==
      control ) {
    if ( p_dc1394_feature_whitebalance_get_value ( cameraInfo->iidcHandle,
        &cameraInfo->currentBlueBalance, &cameraInfo->currentRedBalance ) !=
        DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_whitebalance_get_value failed\n",
          __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }
    val->valueType = OA_CTRL_TYPE_INT32;
    if ( OA_CAM_CTRL_BLUE_BALANCE == control ) {
      val->int32 = cameraInfo->currentBlueBalance;
    } else {
      val->int32 = cameraInfo->currentRedBalance;
    }
    return OA_ERR_NONE;
  }

  for ( i = 0, found = 0; !found && i < numIIDCControls; i++ ) {
    if ( dc1394Controls[i].oaControl == OA_CAM_CTRL_MODE_NONAUTO( control )) {
      iidcControl = dc1394Controls[i].iidcControl;
      found = 1;
    }
  }

  if ( found ) {
    if ( iidcControl != DC1394_FEATURE_SHUTTER ||
        OA_CAM_CTRL_EXPOSURE_UNSCALED == control ||
        OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ) == control ) {
      if ( !oaIsAuto ( control )) {
        uint32_t val_u32;
        val->valueType = OA_CTRL_TYPE_INT64;
        if ( p_dc1394_feature_get_value ( cameraInfo->iidcHandle, iidcControl,
            &val_u32 ) != DC1394_SUCCESS ) {
          fprintf ( stderr, "%s: dc1394_feature_get_value failed for "
              "control %d\n", __FUNCTION__, control );
          return -OA_ERR_CAMERA_IO;
        }
        val->int64 = val_u32;
      } else {
        dc1394feature_mode_t mode;

        if ( p_dc1394_feature_get_mode ( cameraInfo->iidcHandle, iidcControl,
            &mode ) != DC1394_SUCCESS ) {
          fprintf ( stderr, "%s: dc1394_feature_get failed for feature %d\n",
              __FUNCTION__, iidcControl );
          return -OA_ERR_CAMERA_IO;
        } else {
          val->valueType = OA_CTRL_TYPE_BOOLEAN;
          val->boolean = ( mode == DC1394_FEATURE_MODE_AUTO ) ? 1 : 0;
        }
      }
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_EXPOSURE_ABSOLUTE == control ) {
    // need to handle absolute exposure time separately
    float decval;
    val->valueType = OA_CTRL_TYPE_INT64;
    if ( p_dc1394_feature_get_absolute_value ( cameraInfo->iidcHandle,
        DC1394_FEATURE_SHUTTER, &decval ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_get_absolute_value %d failed\n",
          __FUNCTION__, iidcControl );
      return -OA_ERR_CAMERA_IO;
    } else {
      val->int64 = decval * 1000000.0;
    }
    return OA_ERR_NONE;
  }

  if ( OA_CAM_CTRL_TEMPERATURE == control || OA_CAM_CTRL_TEMP_SETPOINT ==
      control ) {
    uint32_t setpoint, current;
    if ( p_dc1394_feature_temperature_get_value ( cameraInfo->iidcHandle,
        &setpoint, &current ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_feature_get_temperature failed\n",
          __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }
    // allegedly the temperature is in K, so we need to rebase to C
    // Looks like the measurement is actually 1/10th K.
    if ( OA_CAM_CTRL_TEMP_SETPOINT == control ) {
      val->int32 = setpoint - 2731.5;
    } else {
      val->int32 = current - 2731.5;
    }
    val->valueType = OA_CTRL_TYPE_INT32;
    return OA_ERR_NONE;

  }


  fprintf ( stderr, "Unrecognised control %d in %s\n", control, __FUNCTION__ );
  return -OA_ERR_INVALID_CONTROL;
}


static int
_processSetResolution ( IIDC_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMESIZE*			size = command->commandData;

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;
  return _doCameraConfig ( cameraInfo );
}


static int
_processSetFrameInterval ( IIDC_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMERATE*                    rate = command->commandData;

  cameraInfo->frameRateNumerator = rate->numerator;
  cameraInfo->frameRateDenominator = rate->denominator;
  return _doCameraConfig ( cameraInfo );
}


static int
_doCameraConfig ( IIDC_STATE* cameraInfo )
{
  dc1394video_modes_t		videoModes;
  dc1394format7modeset_t	modeList;
  dc1394color_coding_t		codec;
  dc1394video_frame_t*		dummyFrame;
  dc1394framerates_t		framerates;
  unsigned int			thisOne = 0, requiredFrameRate, i, j;
  int				ret, matched, restart = 0;

  if ( cameraInfo->isStreaming ) {
    restart = 1;
    p_dc1394_video_set_transmission ( cameraInfo->iidcHandle, DC1394_OFF );
    usleep ( cameraInfo->currentAbsoluteExposure );
    do {
      p_dc1394_capture_dequeue ( cameraInfo->iidcHandle,
          DC1394_CAPTURE_POLICY_POLL, &dummyFrame );
      if ( dummyFrame ) {
        p_dc1394_capture_enqueue ( cameraInfo->iidcHandle, dummyFrame );
      }
    } while ( dummyFrame );
    p_dc1394_capture_stop ( cameraInfo->iidcHandle );
    cameraInfo->isStreaming = 0;
  }

  if ( cameraInfo->haveFormat7 ) {

    // Now we have to find a Format7 mode with the correct colour coding
    // and frame size

    if ( p_dc1394_format7_get_modeset ( cameraInfo->iidcHandle, &modeList ) !=
        DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_format7_get_modeset return error\n",
          __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }

    matched = 0;
    for ( i = 0; !matched && i < DC1394_VIDEO_MODE_FORMAT7_NUM; i++ ) {
      if ( modeList.mode[i].present ) {
        if ( modeList.mode[i].size_x == cameraInfo->xSize &&
            modeList.mode[i].size_y == cameraInfo->ySize ) {
          for ( j = 0; j < modeList.mode[i].color_codings.num; j++ ) {
            if ( modeList.mode[i].color_codings.codings[j] ==
                cameraInfo->currentCodec ) {
              thisOne = i + DC1394_VIDEO_MODE_FORMAT7_MIN;
              matched = 1;
            }
          }
        }
      }
    }

    if ( !matched ) {
      fprintf ( stderr, "%s: matching Format7 mode not found\n", __FUNCTION__ );
      return -OA_ERR_OUT_OF_RANGE;
    }

    if ( p_dc1394_video_set_mode ( cameraInfo->iidcHandle, thisOne ) !=
        DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: unable to set Format7 mode %d\n", __FUNCTION__,
          thisOne );
      return -OA_ERR_CAMERA_IO;
    }

    if ( p_dc1394_format7_set_roi ( cameraInfo->iidcHandle, thisOne,
        cameraInfo->currentCodec, DC1394_USE_MAX_AVAIL, 0, 0,
        cameraInfo->xSize, cameraInfo->ySize ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: unable to set mode %d, codec %d, roi %dx%d\n",
          __FUNCTION__, thisOne, cameraInfo->currentCodec, cameraInfo->xSize,
          cameraInfo->ySize );
      return -OA_ERR_CAMERA_IO;
    }
  } else {

    requiredFrameRate = 0;
    for ( i = 0; i < numIIDCFrameRates; i++ ) {
      if ( dc1394FrameRates[i].numerator == cameraInfo->frameRateNumerator &&
          dc1394FrameRates[i].denominator ==
          cameraInfo->frameRateDenominator ) {
        requiredFrameRate = dc1394FrameRates[i].iidcFrameRate;
      }
    }

    if ( !requiredFrameRate ) {
      fprintf ( stderr, "%s: no frame rate matching %d/%d found\n",
          __FUNCTION__, cameraInfo->frameRateNumerator,
          cameraInfo->frameRateDenominator );
      return -OA_ERR_CAMERA_IO;
    }

    if ( p_dc1394_video_get_supported_modes ( cameraInfo->iidcHandle,
        &videoModes ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_get_supported_modes failed",
          __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }

    thisOne = 0;
    for ( i = 0, matched = 0; !matched && i < videoModes.num; i++ ) {
      uint32_t w, h;

      if ( videoModes.modes[i] >= DC1394_VIDEO_MODE_FORMAT7_MIN &&
          videoModes.modes[i] <= DC1394_VIDEO_MODE_FORMAT7_MAX ) {
        continue;
      }

      if ( p_dc1394_get_image_size_from_video_mode ( cameraInfo->iidcHandle,
          videoModes.modes[i], &w, &h ) != DC1394_SUCCESS ) {
        fprintf ( stderr, "%s: dc1394_get_image_size_from_video_mode failed",
          __FUNCTION__ );
      } else {
        if ( w == cameraInfo->xSize && h == cameraInfo->ySize ) {
          if ( p_dc1394_get_color_coding_from_video_mode (
							cameraInfo->iidcHandle, videoModes.modes[i], &codec ) !=
							DC1394_SUCCESS ) {
            fprintf ( stderr, "%s: dc1394_get_color_coding_from_video_mode "
                "failed", __FUNCTION__ );
          } else {
            if ( codec == cameraInfo->currentCodec ) {
              if ( p_dc1394_video_get_supported_framerates (
                  cameraInfo->iidcHandle, videoModes.modes[i], &framerates )
                  != DC1394_SUCCESS ) {
                fprintf ( stderr, "%s: dc1394_video_get_supported_framerates "
                    "failed\n", __FUNCTION__ );
              } else {
                for ( j = 0; !matched && j < framerates.num; j++ ) {
                  if ( framerates.framerates[j] == requiredFrameRate ) {
                    thisOne = i;
                    matched = 1;
                  }
                }
              }
            }
          }
        }
      }
    }

    if ( !matched ) {
      fprintf ( stderr, "%s: can't find compatible video mode\n",
          __FUNCTION__ );
      return -OA_ERR_OUT_OF_RANGE;
    }

    if ( p_dc1394_video_set_mode ( cameraInfo->iidcHandle,
        videoModes.modes[ thisOne ]) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_set_mode failed\n", __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }

    if ( p_dc1394_video_set_framerate ( cameraInfo->iidcHandle,
        requiredFrameRate ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_set_framerate failed\n",
          __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }

  }

  if ( restart ) {
    if (( ret = p_dc1394_capture_setup ( cameraInfo->iidcHandle,
        OA_CAM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT )) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_capture_setup failed: %d\n",
          __FUNCTION__, ret );
      return -OA_ERR_CAMERA_IO;
    }

    cameraInfo->isStreaming = 1;
    if ( p_dc1394_video_set_transmission ( cameraInfo->iidcHandle,
        DC1394_ON ) != DC1394_SUCCESS ) {
      fprintf ( stderr, "%s: dc1394_video_set_transmission failed\n",
          __FUNCTION__ );
      return -OA_ERR_CAMERA_IO;
    }
  }
  return OA_ERR_NONE;
}


static int
_processStreamingStart ( IIDC_STATE* cameraInfo, OA_COMMAND* command )
{
  int		ret;
  CALLBACK*	cb = command->commandData;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

  if (( ret = p_dc1394_capture_setup ( cameraInfo->iidcHandle,
      OA_CAM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT )) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_capture_setup failed: %d\n", __FUNCTION__,
        ret );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if ( p_dc1394_video_set_transmission ( cameraInfo->iidcHandle, DC1394_ON ) !=
      DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_video_set_transmission failed\n",
        __FUNCTION__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}


static int
_processStreamingStop ( IIDC_STATE* cameraInfo, OA_COMMAND* command )
{
  int		queueEmpty;

  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  if ( p_dc1394_video_set_transmission ( cameraInfo->iidcHandle, DC1394_OFF )
      != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_video_set_transmission failed\n",
      __FUNCTION__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  // We wait here until the callback queue has drained otherwise
  // dc1394_capture_stop() could rip the image frame out from underneath
  // the callback

  queueEmpty = 0;
  do {
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    queueEmpty = ( OA_CAM_BUFFERS == cameraInfo->buffersFree ) ? 1 : 0;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
    if ( !queueEmpty ) {
      usleep ( 100 );
    }
  } while ( !queueEmpty );

  if ( p_dc1394_capture_stop ( cameraInfo->iidcHandle ) != DC1394_SUCCESS ) {
    fprintf ( stderr, "%s: dc1394_capture_stop failed\n", __FUNCTION__ );
    return -OA_ERR_SYSTEM_ERROR;
  }

  return OA_ERR_NONE;
}
