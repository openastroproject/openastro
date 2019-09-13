/*****************************************************************************
 *
 * GP2controller.c -- Main camera controller thread
 *
 * Copyright 2019
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

#include "oacamprivate.h"
#include "unimplemented.h"
#include "GP2oacam.h"
#include "GP2state.h"
#include "GP2private.h"


static int	_processGetControl ( oaCamera*, OA_COMMAND* );
static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processExposureSetup ( oaCamera*, OA_COMMAND* );
static int	_processAbortExposure ( oaCamera* );
static int	_startExposure ( oaCamera* );
static int	_setWidgetValue ( GP2_STATE*, CameraWidget*, const void* );
static int	_handleCompletedExposure ( GP2_STATE* );


void*
oacamGP2controller ( void* param )
{
  oaCamera*		camera = param;
  GP2_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode;
  int			exposurePending = 0;
  int			exposureInProgress;
  time_t	exposureStartTime;

  do {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    exitThread = cameraInfo->stopControllerThread;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( exitThread ) {
      break;
    } else {
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      // stop us busy-waiting
      exposurePending = cameraInfo->exposurePending;
      exposureInProgress = cameraInfo->exposureInProgress;
      exposureStartTime = cameraInfo->exposureStartTime;
      if ( !exposurePending && !exposureInProgress &&
					oaDLListIsEmpty ( cameraInfo->commandQueue )) {
        pthread_cond_wait ( &cameraInfo->commandQueued,
            &cameraInfo->commandQueueMutex );
      }
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    }
    do {
      command = oaDLListRemoveFromHead ( cameraInfo->commandQueue );
      if ( command ) {
        switch ( command->commandType ) {
          case OA_CMD_CONTROL_GET:
            resultCode = _processGetControl ( camera, command );
            break;
          case OA_CMD_CONTROL_SET:
            resultCode = _processSetControl ( camera, command );
            break;
          case OA_CMD_START_EXPOSURE:
            resultCode = _processExposureSetup ( camera, command );
            break;
          case OA_CMD_ABORT_EXPOSURE:
            resultCode = _processAbortExposure ( camera );
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

		if ( exposurePending ) {
			time_t now = time(0);
			if ( now > exposureStartTime ) {
				( void ) _startExposure ( camera );
			}
		} else {
			if ( exposureInProgress ) {
				_handleCompletedExposure ( cameraInfo );
			}
		}

  } while ( !exitThread );

  return 0;
}


static int
_processGetControl ( oaCamera* camera, OA_COMMAND* command )
{
  int							control = command->controlId;
  oaControlValue*	valp = command->resultData;
  GP2_STATE*			cameraInfo = camera->_private;
	CameraWidget*		widget = 0;
	const char**		options = 0;
	int							numOptions, i, found, val;
	const char*			currOption;

	if ( control == OA_CAM_CTRL_MIRROR_LOCKUP ) {
		valp->valueType = OA_CTRL_TYPE_BOOLEAN;
		valp->boolean =
				cameraInfo->customFuncStr[ cameraInfo->mirrorLockupPos ] - '0';
		return OA_ERR_NONE;
	}

	if ( control == OA_CAM_CTRL_BATTERY_LEVEL ) {
		if ( p_gp_widget_get_value ( cameraInfo->batteryLevel, &currOption ) !=
				GP_OK ) {
			fprintf ( stderr, "Failed to read value of control %d in %s\n",
					control, __FUNCTION__ );
			return -OA_ERR_CAMERA_IO;
		}
		if ( sscanf ( currOption, "%d%%", &val ) != 1 ) {
			fprintf ( stderr, "Don't recognise data '%s' in battery level\n",
					currOption );
			val = 0;
		}
		valp->valueType = OA_CTRL_TYPE_INT32;
		valp->int32 = val;
		return OA_ERR_NONE;
	}

	switch ( control ) {
		case OA_CAM_CTRL_WHITE_BALANCE:
			widget = cameraInfo->whiteBalance;
			options = cameraInfo->whiteBalanceOptions;
			numOptions = cameraInfo->numWBOptions;
			break;

		case OA_CAM_CTRL_ISO:
			widget = cameraInfo->iso;
			options = cameraInfo->isoOptions;
			numOptions = cameraInfo->numIsoOptions;
			break;

		case OA_CAM_CTRL_SHUTTER_SPEED:
			widget = cameraInfo->shutterSpeed;
			options = cameraInfo->shutterSpeedOptions;
			numOptions = cameraInfo->numShutterSpeedOptions;
			break;

		case OA_CAM_CTRL_FRAME_FORMAT:
			widget = cameraInfo->frameFormat;
			options = cameraInfo->frameFormatOptions;
			numOptions = cameraInfo->numFrameFormatOptions;
			break;

		default:
			fprintf ( stderr, "Unrecognised control %d in %s\n", control,
          __FUNCTION__ );
			return -OA_ERR_INVALID_CONTROL;
			break;
	}

	// Populate the options if we don't already have them
	if ( !options ) {
		( void ) oaGP2CameraGetMenuString ( camera, control, 0 );
	}

	if ( p_gp_widget_get_value ( widget, &currOption ) != GP_OK ) {
		fprintf ( stderr, "Failed to read value of control %d in %s\n",
				control, __FUNCTION__ );
		return -OA_ERR_CAMERA_IO;
	}

	found = 0;
	for ( i = 0; i < numOptions && !found; i++ ) {
		if ( !strcmp ( currOption, options[i] )) {
			valp->valueType = OA_CTRL_TYPE_MENU;
			valp->int32 = i;
			found = 1;
		}
	}
	if ( !found ) {
		fprintf ( stderr, "Failed to match value of control %d in %s [%s]\n",
				control, __FUNCTION__, currOption );
		return -OA_ERR_CAMERA_IO;
	}

	if ( control == OA_CAM_CTRL_FRAME_FORMAT ) {
		valp->valueType = OA_CTRL_TYPE_DISC_MENU;
		if ( valp->int32 == cameraInfo->jpegOption ) {
			valp->int32 = OA_PIX_FMT_JPEG8;
		} else {
			switch ( cameraInfo->manufacturer ) {
				case CAMERA_MANUF_CANON:
					fprintf ( stderr, "Returning Canon CR2 format, but may be CR3\n" );
					valp->int32 = OA_PIX_FMT_CANON_CR2;
					break;
				case CAMERA_MANUF_NIKON:
					valp->int32 = OA_PIX_FMT_NIKON_NEF;
					break;
				default:
					fprintf ( stderr, "Unknown raw camera format\n" );
					break;
			}
		}
	}

	return OA_ERR_NONE;
}


static int
_processSetControl ( oaCamera* camera, OA_COMMAND* command )
{
  oaControlValue*	valp = command->commandData;
  int							control = command->controlId;
  GP2_STATE*			cameraInfo = camera->_private;
	CameraWidget*		widget = 0;
	const char**		options = 0;
	int							numOptions;
	int							newVal;
	const void*			valuePointer;

	if ( control == OA_CAM_CTRL_MIRROR_LOCKUP ) {
		newVal = valp->boolean;
		cameraInfo->customFuncStr[ cameraInfo->mirrorLockupPos ] =
				newVal ? '1' : '0';
		valuePointer = cameraInfo->customFuncStr;
	} else {
		switch ( control ) {
			case OA_CAM_CTRL_WHITE_BALANCE:
				widget = cameraInfo->whiteBalance;
				options = cameraInfo->whiteBalanceOptions;
				numOptions = cameraInfo->numWBOptions;
				break;

			case OA_CAM_CTRL_ISO:
				widget = cameraInfo->iso;
				options = cameraInfo->isoOptions;
				numOptions = cameraInfo->numIsoOptions;
				break;

			case OA_CAM_CTRL_SHUTTER_SPEED:
				widget = cameraInfo->shutterSpeed;
				options = cameraInfo->shutterSpeedOptions;
				numOptions = cameraInfo->numShutterSpeedOptions;
				break;

			case OA_CAM_CTRL_FRAME_FORMAT:
				widget = cameraInfo->frameFormat;
				options = cameraInfo->frameFormatOptions;
				numOptions = cameraInfo->numFrameFormatOptions;
				break;

			default:
				fprintf ( stderr, "Unrecognised control %d in %s\n", control,
          __FUNCTION__ );
				return -OA_ERR_INVALID_CONTROL;
				break;
		}

		newVal = valp->menu;
		if ( control == OA_CAM_CTRL_FRAME_FORMAT ) {
			switch ( newVal ) {
				case OA_PIX_FMT_CANON_CR2:
				case OA_PIX_FMT_CANON_CR3:
				case OA_PIX_FMT_NIKON_NEF:
					newVal = cameraInfo->rawOption;
					break;
				case OA_PIX_FMT_JPEG8:
					newVal = cameraInfo->jpegOption;
					break;
				default:
					return -OA_ERR_OUT_OF_RANGE;
					break;
			}
		} else {
			if ( newVal < 0 || newVal >= numOptions ) {
				return -OA_ERR_OUT_OF_RANGE;
			}
		}

		// Populate the options if we don't already have them
		if ( !options ) {
			( void ) oaGP2CameraGetMenuString ( camera, control, 0 );
		}

		valuePointer = options[newVal];
	}

	return _setWidgetValue ( cameraInfo, widget, valuePointer );
}


int
_setWidgetValue ( GP2_STATE* cameraInfo, CameraWidget* widget,
		const void* value )
{
	int			ret;

	if ( p_gp_widget_set_value ( widget, value ) != GP_OK ) {
		fprintf ( stderr, "Failed to set value of control in %s\n", __FUNCTION__ );
		return -OA_ERR_CAMERA_IO;
	}

	if (( ret = p_gp_camera_set_config ( cameraInfo->handle,
			cameraInfo->rootWidget, cameraInfo->ctx )) != GP_OK ) {
		fprintf ( stderr, "Failed to write config to camera in %s, error %d\n",
				__FUNCTION__, ret );
		return -OA_ERR_CAMERA_IO;
	}

	cameraInfo->captureEnabled = 0;

	return OA_ERR_NONE;
}


static int
_processExposureSetup ( oaCamera* camera, OA_COMMAND* command )
{
  GP2_STATE*	cameraInfo = camera->_private;
  CALLBACK*		cb = command->commandData;
	int					ret;

  if ( cameraInfo->exposurePending ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->exposureCallback.callback = cb->callback;
  cameraInfo->exposureCallback.callbackArg = cb->callbackArg;
	cameraInfo->exposureStartTime = *(( time_t* ) command->commandArgs );

	if ( cameraInfo->manufacturer == CAMERA_MANUF_CANON && cameraInfo->capture ) {
		int			onOff = 1;
		if (( ret = _setWidgetValue ( cameraInfo, cameraInfo->capture,
				&onOff )) != GP_OK ) {
			fprintf ( stderr, "setting capture toggle on failed with error %d\n",
					ret );
			return -OA_ERR_CAMERA_IO;
		}
	}
	cameraInfo->captureEnabled = 1;

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->exposurePending = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  return OA_ERR_NONE;
}


static int
_startExposure ( oaCamera* camera )
{
  GP2_STATE*	cameraInfo = camera->_private;
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->exposurePending = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

	p_gp_camera_trigger_capture ( cameraInfo->handle, cameraInfo->ctx );

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->exposureInProgress = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

	return OA_ERR_NONE;
}


static int
_handleCompletedExposure ( GP2_STATE* cameraInfo )
{
	CameraEventType			eventType;
	CameraFilePath*			filePath;
	CameraFile*					file;
	int									ret;
	int									buffersFree, nextBuffer;
	void*								data;
	void*								ptr;
	const char*					imageBuffer;
	unsigned long				size;
	const char*					mimeType;

	if (( ret = p_gp_camera_wait_for_event ( cameraInfo->handle, 100,
			&eventType, &data, cameraInfo->ctx )) != GP_OK ) {
		fprintf ( stderr, "wait for event returns error %d\n", ret );
		return -OA_ERR_CAMERA_IO;
	}

	if ( eventType != GP_EVENT_FILE_ADDED ) {
		switch ( eventType ) {
			case GP_EVENT_CAPTURE_COMPLETE:
			case GP_EVENT_UNKNOWN:
			case GP_EVENT_TIMEOUT:
				break;
			case GP_EVENT_FOLDER_ADDED:
				free ( data );
				break;
			default:
				fprintf ( stderr, "%s: unexpected event type %d returned\n",
						__FUNCTION__, eventType );
				break;
		}
		return OA_ERR_NONE;
	}

	if ( cameraInfo->abortExposure ) {
		pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
		cameraInfo->abortExposure = 0;
		pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
		return OA_ERR_NONE;
	}

	filePath = data;

	( void ) p_gp_file_new ( &file );

	if (( ret = p_gp_camera_file_get ( cameraInfo->handle, filePath->folder,
			filePath->name, GP_FILE_TYPE_NORMAL, file, cameraInfo->ctx )) != GP_OK ) {
		fprintf ( stderr, "gp_camera_file_get %s/%s failed with error %d\n",
				filePath->folder, filePath->name, ret );
		( void ) p_gp_file_free ( file );
		return -OA_ERR_CAMERA_IO;
	}

	if (( ret = p_gp_file_get_data_and_size ( file, &imageBuffer,
			&size )) != GP_OK ) {
		fprintf ( stderr, "gp_file_get_data_and_size failed with error %d\n", ret );
		( void ) p_gp_file_free ( file );
		return -OA_ERR_CAMERA_IO;
	}

	if (( ret = p_gp_file_get_mime_type ( file, &mimeType )) != GP_OK ) {
		fprintf ( stderr, "gp_file_get_mime_type failed with error %d\n", ret );
		( void ) p_gp_file_free ( file );
		return -OA_ERR_CAMERA_IO;
	}

	pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
	buffersFree = cameraInfo->buffersFree;
	pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

	if ( buffersFree && size > 0 ) {
		nextBuffer = cameraInfo->nextBuffer;
		if ( size > cameraInfo->currentBufferLength[ nextBuffer ]) {
			if ( cameraInfo->currentBufferLength[ nextBuffer ] == 0 ) {
				ptr = malloc ( size );
			} else {
				ptr = realloc ( cameraInfo->buffers[ nextBuffer ].start, size );
			}
			if ( !ptr ) {
				fprintf ( stderr, "failed to make bigger buffer for camera frame\n" );
				return -OA_ERR_MEM_ALLOC;
			}
			cameraInfo->buffers[ nextBuffer ].start = ptr;
			cameraInfo->currentBufferLength[ nextBuffer ] = size;
		}

		( void ) memcpy ( cameraInfo->buffers[ nextBuffer ].start, imageBuffer,
				size );
		cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
				OA_CALLBACK_NEW_FRAME;
		cameraInfo->frameCallbacks[ nextBuffer ].callback =
				cameraInfo->exposureCallback.callback;
		cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
				cameraInfo->exposureCallback.callbackArg;
		cameraInfo->frameCallbacks[ nextBuffer ].buffer =
				cameraInfo->buffers[ nextBuffer ].start;
		cameraInfo->frameCallbacks[ nextBuffer ].bufferLen = size;
		pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
		oaDLListAddToTail ( cameraInfo->callbackQueue,
				&cameraInfo->frameCallbacks[ nextBuffer ]);
		cameraInfo->buffersFree--;
		cameraInfo->nextBuffer = ( nextBuffer + 1 ) % cameraInfo->configuredBuffers;		pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
		pthread_cond_broadcast ( &cameraInfo->callbackQueued );
	}

	p_gp_file_free ( file );

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->exposureInProgress = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

	return OA_ERR_NONE;
}


static int
_processAbortExposure ( oaCamera* camera )
{
  GP2_STATE*	cameraInfo = camera->_private;
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->exposurePending = 0;
  cameraInfo->abortExposure = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

	return OA_ERR_NONE;
}


