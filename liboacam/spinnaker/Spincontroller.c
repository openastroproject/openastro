/*****************************************************************************
 *
 * Spincontroller.c -- Main camera controller thread
 *
 * Copyright 2021  James Fidell (james@openastroproject.org)
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

#include <spinc/SpinnakerC.h>
#include <pthread.h>
#include <sys/time.h>

#include <openastro/camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Spin.h"
#include "Spinstate.h"

static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processGetControl ( SPINNAKER_STATE*, OA_COMMAND* );
static int	_processSetResolution ( SPINNAKER_STATE*, OA_COMMAND* );
static int	_processSetROI ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( SPINNAKER_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( SPINNAKER_STATE*, OA_COMMAND* );
//static int	_setBinning ( SPINNAKER_STATE*, int );
static int	_getEnumValue ( spinNodeHandle, size_t* );
static int	_getCustomEnumValue ( spinNodeHandle, int64_t* );
static int	_doStart ( SPINNAKER_STATE* );
static int	_doStop ( SPINNAKER_STATE* );
static int	_doFrameFormat ( oaCamera*, int );
static int	_configureEvents ( SPINNAKER_STATE* );
static int	_unconfigureEvents ( SPINNAKER_STATE* );


void*
oacamSpinController ( void* param )
{
  oaCamera*		camera = param;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
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
            oaLogError ( OA_LOG_CAMERA,
								"%s: Invalid command type %d in controller", __func__,
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
    }
  } while ( !exitThread );

  return 0;
}


void
_SpinFrameCallback ( spinImage imageData, void *ptr )
{
	spinImageStatus		status;
	bool8_t						imageComplete;
	void*							frame;
  SPINNAKER_STATE*	cameraInfo = ptr;
  int								buffersFree, nextBuffer;
  size_t						dataLength;

	if (( *p_spinImageIsIncomplete )( imageData, &imageComplete ) !=
			SPINNAKER_ERR_SUCCESS ) {
		if (( *p_spinImageGetStatus )( imageData, &status ) !=
				SPINNAKER_ERR_SUCCESS ) {
			status = IMAGE_UNKNOWN_ERROR;
		}
		oaLogError ( OA_LOG_CAMERA, "%s: incomplete image; status %d, ignoring",
				__func__, status );
		// FIX ME -- add dropped count?
		return;
	}

	if (( *p_spinImageGetData )( imageData, &frame ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: failed to get data for image", __func__ );
		return;
	}

	if (( *p_spinImageGetValidPayloadSize )( imageData, &dataLength ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: failed to get data length for image",
				__func__ );
		return;
	}

  pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
  buffersFree = cameraInfo->buffersFree;
  pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

  if ( buffersFree ) {
    nextBuffer = cameraInfo->nextBuffer;
    ( void ) memcpy ( cameraInfo->buffers[ nextBuffer ].start, frame,
        dataLength );
    cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
        OA_CALLBACK_NEW_FRAME;
    cameraInfo->frameCallbacks[ nextBuffer ].callback =
        cameraInfo->streamingCallback.callback;
    cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
        cameraInfo->streamingCallback.callbackArg;
    cameraInfo->frameCallbacks[ nextBuffer ].buffer =
        cameraInfo->buffers[ nextBuffer ].start;
    cameraInfo->frameCallbacks[ nextBuffer ].metadata = 0;
        // &( cameraInfo->metadataBuffers[ nextBuffer ]);
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
	int								control = command->controlId;
	oaControlValue*		val = command->commandData;
	size_t						enumValue;
	bool8_t						newBool;
	double						newFloat;
	int64_t						newInt;
	SPINNAKER_STATE*	cameraInfo = camera->_private;
	spinError					err;
	int								restart;

	switch ( control ) {
		case OA_CAM_CTRL_GAIN:
			newFloat = val->int32 * ( cameraInfo->maxFloatGain -
					cameraInfo->minFloatGain ) / 400.0 + cameraInfo->minFloatGain;
			if (( *p_spinFloatSetValue )( cameraInfo->gain, newFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set current gain value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_GAMMA:
			newFloat = val->int32 * ( cameraInfo->maxFloatGamma -
					cameraInfo->minFloatGamma ) / 100.0 + cameraInfo->minFloatGamma;
			if (( *p_spinFloatSetValue )( cameraInfo->gamma, newFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set current gamma value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_HUE:
			newFloat = val->int32 * ( cameraInfo->maxFloatHue -
					cameraInfo->minFloatHue ) / 100.0 + cameraInfo->minFloatHue;
			if (( *p_spinFloatSetValue )( cameraInfo->hue, newFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set current hue value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_SATURATION:
			newFloat = val->int32 * ( cameraInfo->maxFloatSaturation -
					cameraInfo->minFloatSaturation ) / 100.0 +
					cameraInfo->minFloatSaturation;
			if (( *p_spinFloatSetValue )( cameraInfo->saturation, newFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set current saturation value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_SHARPNESS:
			if (( *p_spinIntegerSetValue )( cameraInfo->sharpness, val->int32 ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set current sharpness value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_BLACKLEVEL:
			newFloat = val->int32 * ( cameraInfo->maxFloatBlacklevel -
					cameraInfo->minFloatBlacklevel ) / 100.0 +
					cameraInfo->minFloatBlacklevel;
			if (( *p_spinFloatSetValue )( cameraInfo->blackLevel, newFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set current blacklevel value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
			newFloat = val->int64;
			if (( *p_spinFloatSetValue )( cameraInfo->exposure, newFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set current exposure value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
			enumValue = val->boolean ? GainAuto_Continuous : GainAuto_Off;
			if (( *p_spinEnumerationSetEnumValue )( cameraInfo->autoGain,
					enumValue ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set auto gain value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_GAMMA ):
			newBool = val->boolean ? True : False;
			if (( *p_spinBooleanSetValue )( cameraInfo->gammaEnabled, newBool ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set gamma enabled", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ):
			newInt = val->boolean ? 2 : 0;
			if (( *p_spinEnumerationSetIntValue )( cameraInfo->autoHue, newInt ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set auto hue value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_HUE ):
			newBool = val->boolean ? True : False;
			if (( *p_spinBooleanSetValue )( cameraInfo->hueEnabled, newBool ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set hue enabled", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_SATURATION ):
			newInt = val->boolean ? 2 : 0;
			if (( *p_spinEnumerationSetIntValue )( cameraInfo->autoSaturation,
					newInt ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set auto saturation value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_SATURATION ):
			newBool = val->boolean ? True : False;
			if (( *p_spinBooleanSetValue )( cameraInfo->saturationEnabled,
					newBool ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set saturation enabled",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_SHARPNESS ):
			newInt = val->boolean ? 2 : 0;
			if (( *p_spinEnumerationSetIntValue )( cameraInfo->autoSharpness,
					newInt ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set auto sharpness value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_SHARPNESS ):
			newBool = val->boolean ? True : False;
			if (( *p_spinBooleanSetValue )( cameraInfo->sharpnessEnabled,
					newBool ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set sharpness enabled",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLACKLEVEL ):
			enumValue = val->boolean ? BlackLevelAuto_Continuous : BlackLevelAuto_Off;
			if (( *p_spinEnumerationSetEnumValue )( cameraInfo->autoBlackLevel,
					enumValue ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set auto black level value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_BLACKLEVEL ):
			newBool = val->boolean ? True : False;
			if (( *p_spinBooleanSetValue )( cameraInfo->blackLevelEnabled,
					newBool ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set black level enabled",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
			enumValue = val->boolean ? BalanceWhiteAuto_Continuous :
					BalanceWhiteAuto_Off;
			if (( *p_spinEnumerationSetEnumValue )( cameraInfo->autoWhiteBalance,
					enumValue ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set auto white balance value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
			enumValue = val->boolean ? ExposureAuto_Continuous : ExposureAuto_Off;
			if (( *p_spinEnumerationSetEnumValue )( cameraInfo->autoExposure,
					enumValue ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set auto exposure value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_FRAME_FORMAT:
			if ( OA_CTRL_TYPE_DISCRETE != val->valueType ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: invalid control type %d where discrete expected", __func__,
						val->valueType );
				return -OA_ERR_INVALID_CONTROL_TYPE;
			}
			return _doFrameFormat ( camera, val->discrete );
			break;

		case OA_CAM_CTRL_HFLIP:
			restart = 0;
			if ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) {
				restart = 1;
				_doStop ( cameraInfo );
			}
			newBool = val->boolean ? True : False;
			if (( err = ( *p_spinBooleanSetValue )( cameraInfo->flipX, newBool )) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set reverse x, error %d",
						__func__, err );
				return -OA_ERR_SYSTEM_ERROR;
			}
			if ( restart ) {
				_doStart ( cameraInfo );
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_TRIGGER_DELAY_ENABLE:
			newBool = val->boolean ? True : False;
			if (( *p_spinBooleanSetValue )( cameraInfo->triggerDelayEnabled,
					newBool ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't set trigger delay enable",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_TRIGGER_MODE:
			// For Spinnaker this either turns off overlap, or enables it on
			// readout.  We can only set this if the trigger is enabled though.
			// If it isn't we'll have to do it when the trigger is actually
			// enabled.

			newInt = val->menu;
			switch ( newInt ) {
				case 0: // off
					cameraInfo->currentOverlapMode = TriggerOverlap_Off;
					break;
				case 1: // readout
					cameraInfo->currentOverlapMode = TriggerOverlap_ReadOut;
					break;
				default:
					return -OA_ERR_INVALID_CONTROL;
					break;
			}
			if ( cameraInfo->triggerEnabled ) {
				if (( *p_spinEnumerationSetEnumValue )( cameraInfo->triggerOverlap,
						cameraInfo->currentOverlapMode ) != SPINNAKER_ERR_SUCCESS ) {
					oaLogError ( OA_LOG_CAMERA, "%s: Can't set overlap value", __func__ );
					return -OA_ERR_SYSTEM_ERROR;
				}
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_TRIGGER_POLARITY:
			// Everything except falling edge and rising edge are ignored here.
			newInt = val->menu;
			switch ( newInt ) {
				case 0: // falling edge
					enumValue = TriggerActivation_FallingEdge;
					break;
				case 1: // rising edge
					enumValue = TriggerActivation_RisingEdge;
					break;
				default:
					return -OA_ERR_INVALID_CONTROL;
					break;
			}
			if (( *p_spinEnumerationSetEnumValue )( cameraInfo->triggerActivation,
					enumValue ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't set trigger activation value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_BINNING:
			oaLogError ( OA_LOG_CAMERA, "%s: Unhandled control %d", __func__,
					control );
			break;

		default:
			oaLogError ( OA_LOG_CAMERA, "%s: Unrecognised control %d", __func__,
					control );
			break;
	}

  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( SPINNAKER_STATE* cameraInfo, OA_COMMAND* command )
{
	int								control = command->controlId;
	int64_t						currInt;
	double						currFloat;
	oaControlValue*		val = command->resultData;
	size_t						enumValue;
	int								err;
	bool8_t						currBool;

	switch ( control ) {
		case OA_CAM_CTRL_GAIN:
			if (( *p_spinFloatGetValue )( cameraInfo->gain, &currFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current gain value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			// Potentially temporarily, convert this to a range from 0 to 400
			currInt = ( currFloat - cameraInfo->minFloatGain ) * 400.0 /
				( cameraInfo->maxFloatGain - cameraInfo->minFloatGain );
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = currInt;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_GAMMA:
			if (( *p_spinFloatGetValue )( cameraInfo->gamma, &currFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current gamma value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			// Potentially temporarily, convert this to a range from 0 to 100
			currInt = ( currFloat - cameraInfo->minFloatGamma ) * 100.0 /
				( cameraInfo->maxFloatGamma - cameraInfo->minFloatGamma );
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = currInt;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_HUE:
			if (( *p_spinFloatGetValue )( cameraInfo->hue, &currFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current hue value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			// Potentially temporarily, convert this to a range from 0 to 100
			currInt = ( currFloat - cameraInfo->minFloatHue ) * 100.0 /
				( cameraInfo->maxFloatHue - cameraInfo->minFloatHue );
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = currInt;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_SATURATION:
			if (( *p_spinFloatGetValue )( cameraInfo->saturation, &currFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current saturation value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			// Potentially temporarily, convert this to a range from 0 to 100
			currInt = ( currFloat - cameraInfo->minFloatSaturation ) * 100.0 /
				( cameraInfo->maxFloatSaturation - cameraInfo->minFloatSaturation );
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = currInt;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_SHARPNESS:
			if (( *p_spinIntegerGetValue )( cameraInfo->sharpness, &currInt ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current sharpness value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = currInt;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_BLACKLEVEL:
			if (( *p_spinFloatGetValue )( cameraInfo->blackLevel, &currFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current blacklevel value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			// Potentially temporarily, convert this to a range from 0 to 100
			currInt = ( currFloat - cameraInfo->minFloatBlacklevel ) * 100.0 /
				( cameraInfo->maxFloatBlacklevel - cameraInfo->minFloatBlacklevel );
			val->valueType = OA_CTRL_TYPE_INT32;
			val->int32 = currInt;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
			if (( *p_spinFloatGetValue )( cameraInfo->exposure, &currFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current exposure value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_INT64;
			val->int64 = currFloat;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_TEMPERATURE:
			if (( *p_spinFloatGetValue )( cameraInfo->temperature, &currFloat ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current exposure value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_READONLY;
			val->int32 = currFloat * 10;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
			if (( err = _getEnumValue ( cameraInfo->autoGain, &enumValue )) !=
					OA_ERR_NONE ) {
				return err;
			}
			switch ( enumValue ) {
				case GainAuto_Off:
					val->boolean = 0;
					break;
				case GainAuto_Continuous:
					val->boolean = 1;
					break;
				default:
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Unhandled value '%d' for auto gain", __func__, enumValue );
					break;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_GAMMA ):
			if (( *p_spinBooleanGetValue )( cameraInfo->gammaEnabled, &currBool ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current gamma enabled value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			val->boolean = currBool ? 1 : 0;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ):
			if (( err = _getCustomEnumValue ( cameraInfo->autoHue, &currInt )) !=
					OA_ERR_NONE ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current auto hue value", __func__ );
				return err;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			switch ( currInt ) {
				case 0:
					val->boolean = 0;
					break;
				case 2:
					val->boolean = 1;
					break;
				default:
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Unhandled value '%d' for auto hue", __func__,
							currInt );
					break;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_HUE ):
			if (( *p_spinBooleanGetValue )( cameraInfo->hueEnabled, &currBool ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current hue enabled value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			val->boolean = currBool ? 1 : 0;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_SATURATION ):
			if (( err = _getCustomEnumValue ( cameraInfo->autoSaturation,
					&currInt )) != OA_ERR_NONE ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current auto saturation value", __func__ );
				return err;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			switch ( currInt ) {
				case 0:
					val->boolean = 0;
					break;
				case 2:
					val->boolean = 1;
					break;
				default:
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Unhandled value '%d' for auto saturation", __func__,
							currInt );
					break;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_SATURATION ):
			if (( *p_spinBooleanGetValue )( cameraInfo->saturationEnabled,
					&currBool ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current saturation enabled value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			val->boolean = currBool ? 1 : 0;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_SHARPNESS ):
			if (( err = _getCustomEnumValue ( cameraInfo->autoSharpness,
					&currInt )) != OA_ERR_NONE ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current auto sharpness value", __func__ );
				return err;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			switch ( currInt ) {
				case 0:
					val->boolean = 0;
					break;
				case 2:
					val->boolean = 1;
					break;
				default:
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Unhandled value '%d' for auto sharpness", __func__,
							currInt );
					break;
			}
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_SHARPNESS ):
			if (( *p_spinBooleanGetValue )( cameraInfo->sharpnessEnabled,
					&currBool ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current sharpness enabled value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			val->boolean = currBool ? 1 : 0;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLACKLEVEL ):
			if (( err = _getEnumValue ( cameraInfo->autoBlackLevel, &enumValue )) !=
					OA_ERR_NONE ) {
				return err;
			}
			switch ( enumValue ) {
				case BlackLevelAuto_Off:
					val->boolean = 0;
					break;
				case BlackLevelAuto_Continuous:
					val->boolean = 1;
					break;
				default:
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Unhandled value '%d' for auto black level", __func__,
							enumValue );
					break;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_ON_OFF( OA_CAM_CTRL_BLACKLEVEL ):
			if (( *p_spinBooleanGetValue )( cameraInfo->blackLevelEnabled,
					&currBool ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current black level enabled value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			val->boolean = currBool ? 1 : 0;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
			if (( err = _getEnumValue ( cameraInfo->autoWhiteBalance,
					&enumValue )) != OA_ERR_NONE ) {
				return err;
			}
			switch ( enumValue ) {
				case BalanceWhiteAuto_Off:
					val->boolean = 0;
					break;
				case BalanceWhiteAuto_Continuous:
					val->boolean = 1;
					break;
				default:
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Unhandled value '%d' for auto black level", __func__,
							enumValue );
					break;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
			if (( err = _getEnumValue ( cameraInfo->autoExposure,
					&enumValue )) != OA_ERR_NONE ) {
				return err;
			}
			switch ( enumValue ) {
				case ExposureAuto_Off:
					val->boolean = 0;
					break;
				case ExposureAuto_Continuous:
					val->boolean = 1;
					break;
				default:
					oaLogWarning ( OA_LOG_CAMERA,
							"%s: Unhandled value '%d' for auto black level", __func__,
							enumValue );
					break;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_HFLIP:
			if (( *p_spinBooleanGetValue )( cameraInfo->flipX, &currBool ) !=
					SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA, "%s: Can't get current reverse X value",
						__func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			val->boolean = currBool ? 1 : 0;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_TRIGGER_DELAY_ENABLE:
			if (( *p_spinBooleanGetValue )( cameraInfo->triggerDelayEnabled,
					&currBool ) != SPINNAKER_ERR_SUCCESS ) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Can't get current trigger delay enabled value", __func__ );
				return -OA_ERR_SYSTEM_ERROR;
			}
			val->valueType = OA_CTRL_TYPE_BOOLEAN;
			val->boolean = currBool ? 1 : 0;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_TRIGGER_MODE:
			val->valueType = OA_CTRL_TYPE_MENU;
			val->menu = cameraInfo->currentOverlapMode;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_TRIGGER_POLARITY:
			if (( err = _getEnumValue ( cameraInfo->triggerActivation,
					&enumValue )) != OA_ERR_NONE ) {
				return err;
			}
			// Everything except falling edge and rising edge are ignored here.
			switch ( enumValue ) {
				case TriggerActivation_FallingEdge:
					val->menu = 0;
					break;
				case TriggerActivation_RisingEdge:
					val->menu = 1;
					break;
				default:
					return -OA_ERR_INVALID_CONTROL;
					break;
			}
			val->valueType = OA_CTRL_TYPE_MENU;
			return OA_ERR_NONE;
			break;

		case OA_CAM_CTRL_BINNING:
			oaLogError ( OA_LOG_CAMERA, "%s: Unhandled control %d", __func__,
					control );
			break;

		default:
			oaLogError ( OA_LOG_CAMERA, "%s: Unrecognised control %d", __func__,
					control );
			break;
	}

  return -OA_ERR_INVALID_CONTROL;
}


static int
_getEnumValue ( spinNodeHandle node, size_t* value )
{
	spinNodeHandle		enumHandle;
	spinError					err;

	if (( *p_spinEnumerationGetCurrentEntry )( node, &enumHandle ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum current entry", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( err = ( *p_spinEnumerationEntryGetEnumValue )( enumHandle, value )) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum current value, error %d",
				__func__, err );
		return -OA_ERR_SYSTEM_ERROR;
	}
	return OA_ERR_NONE;
}


static int
_getCustomEnumValue ( spinNodeHandle node, int64_t* value )
{
	spinNodeHandle		enumHandle;
	spinError					err;

	if (( *p_spinEnumerationGetCurrentEntry )( node, &enumHandle ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum current entry", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( err = ( *p_spinEnumerationEntryGetIntValue )( enumHandle,
			value )) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't get enum integer value, error %d",
				__func__, err );
		return -OA_ERR_SYSTEM_ERROR;
	}
	return OA_ERR_NONE;
}


static int
_processSetResolution ( SPINNAKER_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMESIZE*			size = command->commandData;
  unsigned int		binMode, s, restart = 0;
  int							found;

  if ( size->x == cameraInfo->xSize && size->y == cameraInfo->ySize ) {
    return OA_ERR_NONE;
  }

  found = -1;
  binMode = cameraInfo->binMode;
  for ( s = 0; s < cameraInfo->frameSizes[ binMode ].numSizes && found < 0;
			s++ ) {
    if ( cameraInfo->frameSizes[ binMode ].sizes[ s ].x >= size->x &&
        cameraInfo->frameSizes[ binMode ].sizes[ s ].y >= size->y ) {
      found = s;
      break;
    }
  }

  if ( found < 0 ) {
    oaLogError ( OA_LOG_CAMERA, "%s: resolution %dx%d not found", __func__,
				size->x, size->y );
    return -OA_ERR_OUT_OF_RANGE;
  }

/*
  settings.width = size->x;
  settings.height = size->y;
  settings.offsetX = ( imageInfo.maxWidth - size->x ) / 2;
  settings.offsetY = ( imageInfo.maxHeight - size->y ) / 2;
*/

	oaLogError ( OA_LOG_CAMERA, "%s: implementation incomplete", __func__ );

  if ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) {
    restart = 1;
/*
    _doStop ( cameraInfo );
*/
  }

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;
  cameraInfo->imageBufferLength = size->x * size->y *
      cameraInfo->currentBytesPerPixel;

  if ( restart ) {
/*
    _doStart ( cameraInfo );
*/
  }
  return OA_ERR_NONE;
}


static int
_processSetROI ( oaCamera* camera, OA_COMMAND* command )
{
/*
  SPINNAKER_STATE*			cameraInfo = camera->_private;
  FRAMESIZE*			size = command->commandData;
  unsigned int			x, y;
  int				ret, restart = 0;
*/
  if (!( camera->features.flags & OA_CAM_FEATURE_ROI )) {
    return -OA_ERR_INVALID_CONTROL;
  }

	oaLogError ( OA_LOG_CAMERA, "%s: implementation incomplete", __func__ );

  return OA_ERR_NONE;
}


static int
_processStreamingStart ( SPINNAKER_STATE* cameraInfo, OA_COMMAND* command )
{
  CALLBACK*		cb = command->commandData;

  if ( cameraInfo->runMode != CAM_RUN_MODE_STOPPED ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

  cameraInfo->imageBufferLength = cameraInfo->xSize * cameraInfo->ySize *
      cameraInfo->currentBytesPerPixel;

	_configureEvents ( cameraInfo );

	if (( *p_spinEnumerationSetEnumValue )( cameraInfo->acquisitionMode,
			AcquisitionMode_Continuous ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't set continuous acquisition mode",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

  return _doStart ( cameraInfo );
}


static int
_doStart ( SPINNAKER_STATE* cameraInfo )
{
	if (( *p_spinCameraBeginAcquisition )( cameraInfo->cameraHandle ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: begin acquisition failed", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->runMode = CAM_RUN_MODE_STREAMING;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  return OA_ERR_NONE;
}


static int
_configureEvents ( SPINNAKER_STATE* cameraInfo )
{
#if HAVE_LIBSPINNAKER_V1
	cameraInfo->imageEvent = 0;

	if (( *p_spinImageEventCreate )( &( cameraInfo->imageEvent ),
			_SpinFrameCallback, cameraInfo ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to create image event",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if (( *p_spinCameraRegisterImageEvent )( cameraInfo->cameraHandle,
			cameraInfo->imageEvent ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to register image event",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
#else
	cameraInfo->eventHandler = 0;

	if (( *p_spinImageEventHandlerCreate )( &( cameraInfo->eventHandler ),
			_SpinFrameCallback, cameraInfo ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to create image event handler",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if (( *p_spinCameraRegisterImageEventHandler )( cameraInfo->cameraHandle,
			cameraInfo->eventHandler ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to register event handler",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
#endif

	return OA_ERR_NONE;
}


static int
_processStreamingStop ( SPINNAKER_STATE* cameraInfo, OA_COMMAND* command )
{
  if ( cameraInfo->runMode != CAM_RUN_MODE_STREAMING ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  _doStop ( cameraInfo );
	return _unconfigureEvents ( cameraInfo );
}


static int
_doStop ( SPINNAKER_STATE* cameraInfo )
{
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->runMode = CAM_RUN_MODE_STOPPED;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

	if (( *p_spinCameraEndAcquisition )( cameraInfo->cameraHandle ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: end acquisition failed", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

  return OA_ERR_NONE;
}


static int
_unconfigureEvents ( SPINNAKER_STATE* cameraInfo )
{
#if HAVE_LIBSPINNAKER_V1
	if (( *p_spinCameraUnregisterImageEvent )( cameraInfo->cameraHandle,
			cameraInfo->imageEvent ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to unregister image event",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if (( *p_spinImageEventDestroy )( cameraInfo->imageEvent ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to destroy image event",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
#else
	if (( *p_spinCameraUnregisterImageEventHandler )( cameraInfo->cameraHandle,
			cameraInfo->eventHandler ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to unregister image event handler",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

	if (( *p_spinImageEventHandlerDestroy )( cameraInfo->eventHandler ) !=
			SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Unable to destroy event handler",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
#endif
	return OA_ERR_NONE;
}


/*
static int
_setBinning ( SPINNAKER_STATE* cameraInfo, int binMode )
{
	oaLogError ( OA_LOG_CAMERA, "%s: not yet implemented", __func__ );
  return OA_ERR_NONE;
}
*/


static int
_doFrameFormat ( oaCamera* camera, int format )
{
  SPINNAKER_STATE*	cameraInfo = camera->_private;
	int								i = 0, found = 0, restart = 0;
	size_t						newFormat;

	if ( !camera->frameFormats[ format ] ) {
		return OA_ERR_UNSUPPORTED_FORMAT;
	}

	for ( i = 0; !found && i <= NUM_SPIN_FORMATS; i++ ) {
		if ( _spinFormatMap[i] == format ) {
			found = 1;
			newFormat = i;
		}
	}

	if ( !found ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Format %d not found in map", __func__,
				format );
		return OA_ERR_UNSUPPORTED_FORMAT;
	}

  if ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) {
    restart = 1;
    _doStop ( cameraInfo );
  }

	if (( *p_spinEnumerationSetEnumValue )( cameraInfo->pixelFormat,
			newFormat ) != SPINNAKER_ERR_SUCCESS ) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't set format %d", __func__,
				newFormat );
		return -OA_ERR_SYSTEM_ERROR;
	}

  cameraInfo->currentFrameFormat = format;
  cameraInfo->currentBytesPerPixel = oaFrameFormats[ format ].bytesPerPixel;
  cameraInfo->imageBufferLength = cameraInfo->xSize * cameraInfo->ySize *
    cameraInfo->currentBytesPerPixel;

  if ( restart ) {
    _doStart ( cameraInfo );
  }

  return OA_ERR_NONE;
}
