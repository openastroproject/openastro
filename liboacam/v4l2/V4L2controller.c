/*****************************************************************************
 *
 * V4L2controller.c -- Main camera controller thread
 *
 * Copyright 2015,2016,2017,2018,2019
 *     James Fidell (james@openastroproject.org)
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
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <sys/mman.h>
#include <libv4l2.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "V4L2.h"
#include "V4L2oacam.h"
#include "V4L2state.h"
#include "V4L2ioctl.h"
#include "V4L2cameras.h"


static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processGetControl ( oaCamera*, OA_COMMAND* );
static int	_processSetResolution ( V4L2_STATE*, OA_COMMAND* );
static int	_processStreamingStart ( V4L2_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( V4L2_STATE*, OA_COMMAND* );
static int	_processSetFrameInterval ( V4L2_STATE*, OA_COMMAND* );
static int	_processSetFrameFormat ( V4L2_STATE*, unsigned int,
		    OA_COMMAND* );
static int	_processGetMenuItem ( V4L2_STATE*, OA_COMMAND* );
static int	_setUserControl ( int, int, int );
static int	_getUserControl ( int, int );
static int	_setExtendedControl ( int, int, oaControlValue* );
static int	_getExtendedControl ( int, int, oaControlValue* );
static int	_doCameraConfig ( V4L2_STATE*, OA_COMMAND* );
static int	_doStart ( V4L2_STATE* );


void*
oacamV4L2controller ( void* param )
{
  oaCamera*		camera = param;
  V4L2_STATE*		cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode, streaming, r;
  int			frameWait, haveFrame;
  int			nextBuffer, buffersFree;
  struct timeval	tv;
  fd_set		fds;
  struct v4l2_buffer*	frame = 0;
  int			fd;

  // V4L2 cameras are a mixed bunch in terms of how they respond to
  // controls.  Some are happy if the controls are changed at any time
  // whilst others can fall in a heap if that happens.  Some also won't
  // accept changes of resolution or frame rate without restarting the
  // camera.

  // The simplest way to deal with this is to ignore queued controls
  // (with the exception of resolution changes and frame rate changes
  // until the camera is configured for streaming.  Once started the
  // queue can be handled as normal.

  // This also means that changes to resolution and frame rate occurring
  // whilst the camera is streaming wil require the camera to be stopped
  // and restarted.

  do {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    exitThread = cameraInfo->stopControllerThread;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( exitThread ) {
      break;
    } else {
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      // stop us busy-waiting
      if ( !cameraInfo->isStreaming &&
          oaDLListIsEmpty ( cameraInfo->commandQueue )) {
        pthread_cond_wait ( &cameraInfo->commandQueued,
            &cameraInfo->commandQueueMutex );
      }
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    }

    do {
      command = oaDLListRemoveFromHead ( cameraInfo->commandQueue );
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      streaming = cameraInfo->isStreaming;
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
      if ( command ) {
        // This is a bit cack.  Need a neater way to handle this
        if ( streaming || ( OA_CMD_RESOLUTION_SET == command->commandType ||
            OA_CMD_FRAME_INTERVAL_SET == command->commandType ||
            OA_CMD_START_STREAMING == command->commandType ||
            OA_CMD_MENU_ITEM_GET == command->commandType )) {
          switch ( command->commandType ) {
            case OA_CMD_CONTROL_SET:
              resultCode = _processSetControl ( camera, command );
              break;
            case OA_CMD_CONTROL_GET:
              resultCode = _processGetControl ( camera, command );
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
            case OA_CMD_MENU_ITEM_GET:
              resultCode = _processGetMenuItem ( cameraInfo, command );
              break;
            default:
              fprintf ( stderr, "Invalid command type %d in controller\n",
                  command->commandType );
              resultCode = -OA_ERR_INVALID_CONTROL;
              break;
          }
        } else {
          resultCode = -OA_ERR_IGNORED;
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

      if ( camera->features.flags & OA_CAM_FEATURE_FRAME_RATES ) {
        frameWait = 1000000.0 * cameraInfo->frameRateNumerator /
            cameraInfo->frameRateDenominator;
      } else {
        // FIX ME -- need to handle non-manual exposure modes here?
        if ( camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE )) {
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          frameWait = cameraInfo->currentAbsoluteExposure;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
        } else {
          // This is pretty arbitrary, but we don't really have much to
          // go on.  As the rates are in 100usec, we use 100usec.
          frameWait = 100;
        }
      }
      // pause between tests for frame here is no more than 1ms
      if ( frameWait > 1000 ) {
        frameWait = 1000;
      }

      pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
      buffersFree = cameraInfo->buffersFree;
      pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

      if ( buffersFree ) {
        nextBuffer = cameraInfo->nextBuffer;
        haveFrame = 0;
        pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
        fd = cameraInfo->fd;
        pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
        FD_ZERO ( &fds );
        FD_SET ( fd, &fds );
        tv.tv_sec = ( int ) ( frameWait / 1000000 );
        tv.tv_usec = ( frameWait % 1000000 );
        r = select ( cameraInfo->fd + 1, &fds, 0, 0, &tv );
        if ( r > 0 ) {
          // This mutex prevents attempts to dequeue frames if streaming
          // has stopped since we last checked
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          streaming = cameraInfo->isStreaming;
          if ( streaming ) {
            OA_CLEAR( cameraInfo->currentFrame[ nextBuffer ]);
            frame = &cameraInfo->currentFrame[ nextBuffer ];
            frame->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            frame->memory = V4L2_MEMORY_MMAP;
            frame->index = nextBuffer;
            if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_DQBUF, frame ) < 0 ) {
              perror ( "VIDIOC_DQBUF" );
            } else {
              haveFrame = 1;
            }
          }
          exitThread = cameraInfo->stopControllerThread;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
        }

        if ( !exitThread ) {
          if ( haveFrame ) {
            cameraInfo->frameCallbacks[ nextBuffer ].callbackType =
                OA_CALLBACK_NEW_FRAME;
            cameraInfo->frameCallbacks[ nextBuffer ].callback =
                cameraInfo->streamingCallback.callback;
            cameraInfo->frameCallbacks[ nextBuffer ].callbackArg =
                cameraInfo->streamingCallback.callbackArg;
            cameraInfo->frameCallbacks[ nextBuffer ].buffer = frame;
            cameraInfo->frameCallbacks[ nextBuffer ].bufferLen =
                frame->bytesused;
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
_processSetControl ( oaCamera* camera, OA_COMMAND* command )
{
  V4L2_STATE*		cameraInfo = camera->_private;
  oaControlValue	*valp = command->commandData;
  int32_t		val_s32;

  if ( !camera->OA_CAM_CTRL_TYPE( command->controlId )) {
    return -OA_ERR_INVALID_CONTROL;
  }
  if ( camera->OA_CAM_CTRL_TYPE( command->controlId ) != valp->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  // FIX ME -- make a map between control names to simplify this switch
  // statement

  switch ( command->controlId ) {

    case OA_CAM_CTRL_FRAME_FORMAT:
      val_s32 = valp->discrete;
      return _processSetFrameFormat ( cameraInfo, val_s32, command );
      break;

    case OA_CAM_CTRL_BRIGHTNESS:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_BRIGHTNESS, val_s32 );
      break;

    case OA_CAM_CTRL_CONTRAST:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_CONTRAST, val_s32 );
      break;

    case OA_CAM_CTRL_SATURATION:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_SATURATION, val_s32 );
      break;

    case OA_CAM_CTRL_HUE:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_HUE, val_s32 );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
      // These are the only two we currently allow in init()
      if ( camera->OA_CAM_CTRL_AUTO_TYPE( OA_CAM_CTRL_WHITE_BALANCE ) ==
          OA_CTRL_TYPE_MENU ) {
        val_s32 = valp->menu;
      } else {
        val_s32 = valp->boolean;
      }
      return _setUserControl ( cameraInfo->fd, V4L2_CID_AUTO_WHITE_BALANCE,
          val_s32 );
      break;

    case OA_CAM_CTRL_WHITE_BALANCE:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_DO_WHITE_BALANCE,
          val_s32 );
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_BLUE_BALANCE, val_s32 );
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_RED_BALANCE, val_s32 );
      break;

    case OA_CAM_CTRL_GAMMA:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_GAMMA, val_s32 );
      break;

    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_EXPOSURE, val_s32 );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
      val_s32 = valp->boolean;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_AUTOGAIN, val_s32 );
      break;

    case OA_CAM_CTRL_GAIN:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_GAIN, val_s32 );
      break;

    case OA_CAM_CTRL_HFLIP:
      val_s32 = valp->boolean;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_HFLIP, val_s32 );
      break;

    case OA_CAM_CTRL_VFLIP:
      val_s32 = valp->boolean;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_VFLIP, val_s32 );
      break;

    case OA_CAM_CTRL_POWER_LINE_FREQ:
      val_s32 = valp->menu;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_POWER_LINE_FREQUENCY,
          val_s32 );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ):
      val_s32 = valp->boolean;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_HUE_AUTO, val_s32 );
      break;

    case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd,
          V4L2_CID_WHITE_BALANCE_TEMPERATURE, val_s32 );
      break;

    case OA_CAM_CTRL_SHARPNESS:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_SHARPNESS, val_s32 );
      break;

    case OA_CAM_CTRL_BACKLIGHT_COMPENSATION:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd,
          V4L2_CID_BACKLIGHT_COMPENSATION, val_s32 );
      break;

    case OA_CAM_CTRL_CHROMA_AGC:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_CHROMA_AGC, val_s32 );
      break;

    case OA_CAM_CTRL_COLOUR_KILLER:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_COLOR_KILLER, val_s32 );
      break;

    case OA_CAM_CTRL_COLOURFX:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_COLORFX, val_s32 );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ):
      val_s32 = valp->boolean;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_BRIGHTNESS, val_s32 );
      break;

    case OA_CAM_CTRL_BAND_STOP_FILTER:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_BAND_STOP_FILTER,
          val_s32 );
      break;

    case OA_CAM_CTRL_ROTATE:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_ROTATE, val_s32 );
      break;

    case OA_CAM_CTRL_BG_COLOUR:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_BG_COLOR, val_s32 );
      break;

    case OA_CAM_CTRL_CHROMA_GAIN:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_CHROMA_GAIN, val_s32 );
      break;

    case OA_CAM_CTRL_MIN_BUFFERS_FOR_CAPTURE:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd,
          V4L2_CID_MIN_BUFFERS_FOR_CAPTURE, val_s32 );
      break;

#ifdef V4L2_CID_ALPHA_COMPONENT
    case OA_CAM_CTRL_ALPHA_COMPONENT:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_ALPHA_COMPONENT,
          val_s32 );
      break;
#endif

#ifdef V4L2_CID_COLORFX_CBCR
    case OA_CAM_CTRL_COLOURFX_CBCR:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, V4L2_CID_COLORFX_CBCR, val_s32 );
      break;
#endif

    // end of the standard V4L2 controls.  Now the extended ones

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_EXPOSURE_AUTO,
          valp );
      break;     

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    {
      // have to convert from usec to units of 100 usec here, but we don't
      // know which one we're using
      oaControlValue newVal;
      if (( newVal.valueType = valp->valueType == OA_CTRL_TYPE_INT32 )) {
        newVal.int32 = valp->int32 / 100;
        cameraInfo->currentAbsoluteExposure = valp->int32;
      } else {
        newVal.int64 = valp->int64 / 100;
        cameraInfo->currentAbsoluteExposure = valp->int64;
      }
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_EXPOSURE_ABSOLUTE,
        &newVal );
      break;
    }
    case OA_CAM_CTRL_PAN_RELATIVE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_PAN_RELATIVE,
        valp );
      break;

    case OA_CAM_CTRL_TILT_RELATIVE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_TILT_RELATIVE,
        valp );
      break;

    case OA_CAM_CTRL_PAN_RESET:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_PAN_RESET,
        valp );
      break;

    case OA_CAM_CTRL_TILT_RESET:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_TILT_RESET,
        valp );
      break;

    case OA_CAM_CTRL_PAN_ABSOLUTE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_PAN_ABSOLUTE,
        valp );
      break;

    case OA_CAM_CTRL_TILT_ABSOLUTE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_TILT_ABSOLUTE,
        valp );
      break;

    case OA_CAM_CTRL_FOCUS_ABSOLUTE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_FOCUS_ABSOLUTE,
        valp );
      break;

    case OA_CAM_CTRL_FOCUS_RELATIVE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_FOCUS_RELATIVE,
        valp );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_ABSOLUTE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_FOCUS_RELATIVE ):
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_FOCUS_AUTO,
        valp );
      break;

    case OA_CAM_CTRL_ZOOM_ABSOLUTE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_ZOOM_ABSOLUTE,
        valp );
      break;

    case OA_CAM_CTRL_PRIVACY_ENABLE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_PRIVACY,
        valp );
      break;

    case OA_CAM_CTRL_IRIS_ABSOLUTE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_IRIS_ABSOLUTE,
        valp );
      break;

    case OA_CAM_CTRL_IRIS_RELATIVE:
      return _setExtendedControl ( cameraInfo->fd, V4L2_CID_IRIS_RELATIVE,
        valp );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTOUR ):
      val_s32 = valp->boolean;
      return _setUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(autocontour),
          val_s32 );
      break;

    case OA_CAM_CTRL_CONTOUR:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(contour),
          val_s32 );
      break;

    case OA_CAM_CTRL_NOISE_REDUCTION:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(noise_reduction),
          val_s32 );
      break;

    case OA_CAM_CTRL_AUTO_WHITE_BALANCE_SPEED:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(awb_speed),
          val_s32 );
      break;

    case OA_CAM_CTRL_AUTO_WHITE_BALANCE_DELAY:
      val_s32 = valp->int32;
      return _setUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(awb_delay),
          val_s32 );
      break;

    case OA_CAM_CTRL_SAVE_USER:
      return _setUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(save_user), 1 );
      break;

    case OA_CAM_CTRL_RESTORE_USER:
      return _setUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(restore_user),
          1 );
      break;

    case OA_CAM_CTRL_RESTORE_FACTORY:
      return _setUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(restore_factory),
          1 );
      break;

    default:
      fprintf ( stderr, "Unrecognised control %d in %s\n", command->controlId,
          __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( oaCamera* camera, OA_COMMAND* command )
{
  V4L2_STATE*		cameraInfo = camera->_private;
  oaControlValue*	valp = command->resultData;

  switch ( command->controlId ) {

    case OA_CAM_CTRL_BRIGHTNESS:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_BRIGHTNESS );
      break;

    case OA_CAM_CTRL_CONTRAST:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_CONTRAST );
      break;

    case OA_CAM_CTRL_SATURATION:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_SATURATION );
      break;

    case OA_CAM_CTRL_HUE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_HUE );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          V4L2_CID_AUTO_WHITE_BALANCE );
      break;

    case OA_CAM_CTRL_WHITE_BALANCE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          V4L2_CID_DO_WHITE_BALANCE );
      break;

    case OA_CAM_CTRL_BLUE_BALANCE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_BLUE_BALANCE );
      break;

    case OA_CAM_CTRL_RED_BALANCE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_RED_BALANCE );
      break;

    case OA_CAM_CTRL_GAMMA:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_GAMMA );
      break;

    case OA_CAM_CTRL_EXPOSURE_UNSCALED:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_EXPOSURE );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_AUTOGAIN );
      break;

    case OA_CAM_CTRL_GAIN:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_GAIN );
      break;

    case OA_CAM_CTRL_HFLIP:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_HFLIP );
      break;

    case OA_CAM_CTRL_VFLIP:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_VFLIP );
      break;

    case OA_CAM_CTRL_POWER_LINE_FREQ:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          V4L2_CID_POWER_LINE_FREQUENCY );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_HUE ):
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_HUE_AUTO );
      break;

    case OA_CAM_CTRL_WHITE_BALANCE_TEMP:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          V4L2_CID_WHITE_BALANCE_TEMPERATURE );
      break;

    case OA_CAM_CTRL_SHARPNESS:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_SHARPNESS );
      break;

    case OA_CAM_CTRL_BACKLIGHT_COMPENSATION:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          V4L2_CID_BACKLIGHT_COMPENSATION );
      break;

    case OA_CAM_CTRL_CHROMA_AGC:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_CHROMA_AGC );
      break;

    case OA_CAM_CTRL_COLOUR_KILLER:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_COLOR_KILLER );
      break;

    case OA_CAM_CTRL_COLOURFX:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_COLORFX );
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ):
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_BRIGHTNESS );
      break;

    case OA_CAM_CTRL_BAND_STOP_FILTER:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          V4L2_CID_BAND_STOP_FILTER );
      break;

    case OA_CAM_CTRL_ROTATE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_ROTATE );
      break;

    case OA_CAM_CTRL_BG_COLOUR:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_BG_COLOR );
      break;

    case OA_CAM_CTRL_CHROMA_GAIN:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_CHROMA_GAIN );
      break;

    case OA_CAM_CTRL_MIN_BUFFERS_FOR_CAPTURE:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          V4L2_CID_MIN_BUFFERS_FOR_CAPTURE );
      break;

#ifdef V4L2_CID_ALPHA_COMPONENT
    case OA_CAM_CTRL_ALPHA_COMPONENT:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          V4L2_CID_ALPHA_COMPONENT );
      break;
#endif

#ifdef V4L2_CID_COLORFX_CBCR
    case OA_CAM_CTRL_COLOURFX_CBCR:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, V4L2_CID_COLORFX_CBCR );
      break;
#endif

    // end of the standard V4L2 controls.  Now the extended ones

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
      valp->valueType = camera->OA_CAM_CTRL_AUTO_TYPE( command->controlId );
      _getExtendedControl ( cameraInfo->fd, V4L2_CID_EXPOSURE_AUTO, valp );
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      valp->valueType =
          camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_EXPOSURE_ABSOLUTE );
      _getExtendedControl ( cameraInfo->fd, V4L2_CID_EXPOSURE_ABSOLUTE, valp );
      // convert 100 usec interval to 1 usec
      if ( valp->valueType == OA_CTRL_TYPE_INT32 ) {
        valp->int32 *= 100;
      } else {
        valp->int64 *= 100;
      }
      break;

    case OA_CAM_CTRL_PAN_RELATIVE:
      valp->valueType = camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_PAN_RELATIVE );
      _getExtendedControl ( cameraInfo->fd, V4L2_CID_PAN_RELATIVE, valp );
      break;

    case OA_CAM_CTRL_TILT_RELATIVE:
      valp->valueType = camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TILT_RELATIVE );
      _getExtendedControl ( cameraInfo->fd, V4L2_CID_TILT_RELATIVE, valp );
      break;

    case OA_CAM_CTRL_PAN_ABSOLUTE:
      valp->valueType = camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_PAN_ABSOLUTE );
      _getExtendedControl ( cameraInfo->fd, V4L2_CID_PAN_ABSOLUTE, valp );
      break;

    case OA_CAM_CTRL_TILT_ABSOLUTE:
      valp->valueType = camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_TILT_ABSOLUTE );
      _getExtendedControl ( cameraInfo->fd, V4L2_CID_TILT_ABSOLUTE, valp );
      break;

    case OA_CAM_CTRL_ZOOM_ABSOLUTE:
      valp->valueType = camera->OA_CAM_CTRL_TYPE( OA_CAM_CTRL_ZOOM_ABSOLUTE );
      _getExtendedControl ( cameraInfo->fd, V4L2_CID_ZOOM_ABSOLUTE, valp );
      break;

    case OA_CAM_CTRL_CONTOUR:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd, PWC_CID_CUSTOM(contour));
      break;

    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_CONTOUR ):
      valp->valueType = OA_CTRL_TYPE_BOOLEAN;
      valp->boolean = _getUserControl ( cameraInfo->fd,
          PWC_CID_CUSTOM(autocontour));
      break;

    case OA_CAM_CTRL_NOISE_REDUCTION:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          PWC_CID_CUSTOM(noise_reduction));
      break;

    case OA_CAM_CTRL_AUTO_WHITE_BALANCE_SPEED:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          PWC_CID_CUSTOM(awb_speed));
      break;

    case OA_CAM_CTRL_AUTO_WHITE_BALANCE_DELAY:
      valp->valueType = OA_CTRL_TYPE_INT32;
      valp->int32 = _getUserControl ( cameraInfo->fd,
          PWC_CID_CUSTOM(awb_delay));
      break;

    default:
      fprintf ( stderr, "Unrecognised control %d in %s\n", command->controlId,
          __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


static int
_processSetResolution ( V4L2_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMESIZE	*size = command->commandData;
  int		streaming;

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  streaming = cameraInfo->isStreaming;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  if ( streaming ) {
    return _doCameraConfig ( cameraInfo, command );
  }
  return OA_ERR_NONE;
}


static int
_processSetFrameFormat ( V4L2_STATE* cameraInfo, unsigned int format,
    OA_COMMAND* command )
{
  int           streaming;
  unsigned int	v4l2Format = 0;

  switch ( format ) {
    case OA_PIX_FMT_RGB24:
      v4l2Format = V4L2_PIX_FMT_RGB24;
      break;
    case OA_PIX_FMT_BGR24:
      v4l2Format = V4L2_PIX_FMT_BGR24;
      break;
    case OA_PIX_FMT_GREY8:
      v4l2Format = V4L2_PIX_FMT_GREY;
      break;
    case OA_PIX_FMT_GREY10:
      v4l2Format = V4L2_PIX_FMT_Y10;
      break;
    case OA_PIX_FMT_GREY12:
      v4l2Format = V4L2_PIX_FMT_Y12;
      break;
    case OA_PIX_FMT_GREY16LE:
      v4l2Format = V4L2_PIX_FMT_Y16;
      break;
#ifdef V4L2_PIX_FMT_Y16_BE
    case OA_PIX_FMT_GREY16BE:
      v4l2Format = V4L2_PIX_FMT_Y16_BE;
      break;
#endif
    case OA_PIX_FMT_YUYV:
      v4l2Format = V4L2_PIX_FMT_YUYV;
      break;
    case OA_PIX_FMT_UYVY:
      v4l2Format = V4L2_PIX_FMT_UYVY;
      break;
    case OA_PIX_FMT_YUV422P:
      v4l2Format = V4L2_PIX_FMT_YUV422P;
      break;
    case OA_PIX_FMT_YUV411P:
      v4l2Format = V4L2_PIX_FMT_YUV411P;
      break;
    case OA_PIX_FMT_YUV444:
      v4l2Format = V4L2_PIX_FMT_YUV444;
      break;
    case OA_PIX_FMT_YUV410:
      v4l2Format = V4L2_PIX_FMT_YUV410;
      break;
    case OA_PIX_FMT_YUV420P:
      v4l2Format = V4L2_PIX_FMT_YUV420;
      break;
    case OA_PIX_FMT_BGGR8:
      v4l2Format = V4L2_PIX_FMT_SBGGR8;
      break;
    case OA_PIX_FMT_RGGB8:
      v4l2Format = V4L2_PIX_FMT_SRGGB8;
      break;
    case OA_PIX_FMT_GBRG8:
      if ( cameraInfo->colourDxK ) {
        v4l2Format = V4L2_PIX_FMT_SBGGR8;
      } else {
        v4l2Format = V4L2_PIX_FMT_SGBRG8;
      }
      break;
    case OA_PIX_FMT_GRBG8:
      v4l2Format = V4L2_PIX_FMT_SGRBG8;
      break;
    case OA_PIX_FMT_BGGR10:
      v4l2Format = V4L2_PIX_FMT_SBGGR10;
      break;
    case OA_PIX_FMT_RGGB10:
      v4l2Format = V4L2_PIX_FMT_SRGGB10;
      break;
    case OA_PIX_FMT_GBRG10:
      v4l2Format = V4L2_PIX_FMT_SGBRG10;
      break;
    case OA_PIX_FMT_GRBG10:
      v4l2Format = V4L2_PIX_FMT_SGRBG10;
      break;
    case OA_PIX_FMT_BGGR12:
      v4l2Format = V4L2_PIX_FMT_SBGGR12;
      break;
    case OA_PIX_FMT_RGGB12:
      v4l2Format = V4L2_PIX_FMT_SRGGB12;
      break;
    case OA_PIX_FMT_GBRG12:
      v4l2Format = V4L2_PIX_FMT_SGBRG12;
      break;
    case OA_PIX_FMT_GRBG12:
      v4l2Format = V4L2_PIX_FMT_SGRBG12;
      break;
    default:
      fprintf ( stderr, "unhandled frame format '%s'\n",
          oaFrameFormats[ format ].name );
      break;
  }

  if ( v4l2Format ) {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    cameraInfo->currentV4L2Format = v4l2Format;
    cameraInfo->currentFrameFormat = format;
    streaming = cameraInfo->isStreaming;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( streaming ) {
      return _doCameraConfig ( cameraInfo, command );
    }
    return OA_ERR_NONE;
  }
  return -OA_ERR_OUT_OF_RANGE;
}


static int
_processSetFrameInterval ( V4L2_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMERATE*	rate = command->commandData;
  int		streaming;

  cameraInfo->frameRateNumerator = rate->numerator;
  cameraInfo->frameRateDenominator = rate->denominator;
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  streaming = cameraInfo->isStreaming;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  if ( streaming ) {
    return _doCameraConfig ( cameraInfo, command );
  }
  return OA_ERR_NONE;
}


static int
_setUserControl ( int fd, int id, int32_t value )
{
  struct v4l2_control        control;

  OA_CLEAR ( control );
  control.id = id;
  control.value = value;
  if ( v4l2ioctl ( fd, VIDIOC_S_CTRL, &control )) {
    perror ("VIDIOC_S_CTRL");
    return -OA_ERR_SYSTEM_ERROR;
  }
  return OA_ERR_NONE;
}


static int
_getUserControl ( int fd, int id )
{
  struct v4l2_control        control;

  OA_CLEAR ( control );
  control.id = id;
  if ( v4l2ioctl ( fd, VIDIOC_G_CTRL, &control )) {
    perror ("VIDIOC_G_CTRL");
    return -OA_ERR_SYSTEM_ERROR;
  }
  return control.value;
}


int
_setExtendedControl ( int fd, int id, oaControlValue* valp )
{
  struct v4l2_ext_control    extControl[1];
  struct v4l2_ext_controls   controls;

  OA_CLEAR ( controls );
  OA_CLEAR ( extControl );

  switch ( valp->valueType ) { 
    case OA_CTRL_TYPE_INT32:
      extControl[0].value = valp->int32;
      break;
    case OA_CTRL_TYPE_DISCRETE:
      extControl[0].value = valp->discrete;
      break;
    case OA_CTRL_TYPE_BOOLEAN:
      extControl[0].value = valp->boolean;
      break;
    case OA_CTRL_TYPE_MENU:
    case OA_CTRL_TYPE_DISC_MENU:
      extControl[0].value = valp->menu;
      break;
    case OA_CTRL_TYPE_INT64:
      extControl[0].value64 = valp->int64;
      break;
    case OA_CTRL_TYPE_BUTTON:
      extControl[0].value = 1;
      break;
    case OA_CTRL_TYPE_STRING:
      extControl[0].string = ( char* ) valp->string;
      break;
    default:
      fprintf ( stderr, "%s: unhandled value type %d\n", __FUNCTION__,
          valp->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
  }
  controls.ctrl_class = V4L2_CTRL_ID2CLASS ( id );
  // controls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
  controls.count = 1;
  controls.controls = extControl;
  extControl[0].id = id;
  if ( v4l2ioctl ( fd, VIDIOC_S_EXT_CTRLS, &controls )) {
    perror ("setExtendedControl: VIDIOC_S_EXT_CTRLS");
    return -OA_ERR_SYSTEM_ERROR;
  }
  return OA_ERR_NONE;
}


int
_getExtendedControl ( int fd, int id, oaControlValue* valp )
{
  struct v4l2_ext_control    extControl[1];
  struct v4l2_ext_controls   controls;

  OA_CLEAR ( controls );
  OA_CLEAR ( extControl );
  controls.ctrl_class = V4L2_CTRL_ID2CLASS ( id );
  controls.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
  controls.count = 1;
  controls.controls = extControl;
  extControl[0].id = id;
  if ( v4l2ioctl ( fd, VIDIOC_G_EXT_CTRLS, &controls )) {
    perror ("setExtendedControl: VIDIOC_G_EXT_CTRLS");
    return -OA_ERR_SYSTEM_ERROR;
  }

  switch ( valp->valueType ) {
    case OA_CTRL_TYPE_INT32:
      valp->int32 = extControl[0].value;
      break;
    case OA_CTRL_TYPE_DISCRETE:
      valp->discrete = extControl[0].value;
      break;
    case OA_CTRL_TYPE_BOOLEAN:
      valp->boolean = extControl[0].value;
      break;
    case OA_CTRL_TYPE_MENU:
      valp->menu = extControl[0].value;
      break;
    case OA_CTRL_TYPE_INT64:
      valp->int64 = extControl[0].value64;
      break;
    case OA_CTRL_TYPE_STRING:
      valp->string = extControl[0].string;
      break;
    default:
      fprintf ( stderr, "%s: unhandled value type %d\n", __FUNCTION__,
          valp->valueType );
      return -OA_ERR_INVALID_CONTROL_TYPE;
  }
  return OA_ERR_NONE;
}


static int
_doCameraConfig ( V4L2_STATE* cameraInfo, OA_COMMAND* command )
{
  int		r;

  if (( r = _processStreamingStop ( cameraInfo, command ))) {
    return r;
  }
  v4l2_close ( cameraInfo->fd );
  if (( cameraInfo->fd = v4l2_open ( cameraInfo->devicePath,
      O_RDWR | O_NONBLOCK, 0 )) < 0 ) {
    fprintf ( stderr, "cannot reopen video device '%s'\n",
        cameraInfo->devicePath );
    return -OA_ERR_SYSTEM_ERROR;
  }
  return _doStart ( cameraInfo );
}


static int
_processStreamingStart ( V4L2_STATE* cameraInfo, OA_COMMAND* command )
{
  CALLBACK*    			cb = command->commandData;

  if ( cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  cameraInfo->streamingCallback.callback = cb->callback;
  cameraInfo->streamingCallback.callbackArg = cb->callbackArg;

  return _doStart ( cameraInfo );
}


static int
_doStart ( V4L2_STATE* cameraInfo )
{
  struct v4l2_format		fmt;
  struct v4l2_buffer		buf;
  struct v4l2_requestbuffers	req;
  struct v4l2_streamparm	parm;
  enum v4l2_buf_type		type;
  unsigned int			m, n;

  OA_CLEAR ( fmt );
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = cameraInfo->xSize;
  fmt.fmt.pix.height = cameraInfo->ySize;
  fmt.fmt.pix.pixelformat = cameraInfo->currentV4L2Format;
  fmt.fmt.pix.field = V4L2_FIELD_NONE;
  if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_S_FMT, &fmt )) {
    perror ( "VIDIOC_S_FMT v4l2ioctl failed" );
    return -OA_ERR_CAMERA_IO;
  }

  if ( fmt.fmt.pix.pixelformat != cameraInfo->currentV4L2Format ) {
    fprintf ( stderr, "Can't get expected video format: %c%c%c%c\n",
        cameraInfo->currentV4L2Format & 0xff,
        cameraInfo->currentV4L2Format >> 8 & 0xff,
        cameraInfo->currentV4L2Format >> 16 & 0xff,
        cameraInfo->currentV4L2Format >> 24 );
    return -OA_ERR_CAMERA_IO;
  }

  if (( fmt.fmt.pix.width != cameraInfo->xSize ) || ( fmt.fmt.pix.height !=
      cameraInfo->ySize )) {
    fprintf ( stderr, "Requested image size %dx%d, offered as %dx%d\n",
        cameraInfo->xSize, cameraInfo->ySize, fmt.fmt.pix.width,
        fmt.fmt.pix.height );
    return -OA_ERR_OUT_OF_RANGE;
  }

  OA_CLEAR( parm );
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_G_PARM, &parm )) {
    if ( errno != EINVAL ) {
      perror ( "VIDIOC_G_PARM v4l2ioctl failed" );
      return -OA_ERR_CAMERA_IO;
    }
  }

  // FIX ME -- changing frameRates here may have unpleasant knock-on
  // effects

  if ( V4L2_CAP_TIMEPERFRAME == parm.parm.capture.capability ) {
    OA_CLEAR( parm );
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
    parm.parm.capture.timeperframe.numerator = cameraInfo->frameRateNumerator;
    parm.parm.capture.timeperframe.denominator =
        cameraInfo->frameRateDenominator;
//  camera->features.hasFrameRates = 1;
    if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_S_PARM, &parm )) {
      perror ( "VIDIOC_S_PARM v4l2ioctl failed" );
      return -OA_ERR_SYSTEM_ERROR;
    }
  }

  OA_CLEAR( req );
  req.count = OA_CAM_BUFFERS;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if ( v4l2ioctl( cameraInfo->fd, VIDIOC_REQBUFS, &req )) {
    perror ( "VIDIOC_REQBUFS v4l2ioctl failed" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  cameraInfo->nextBuffer = 0;
  cameraInfo->configuredBuffers = 0;
  cameraInfo->buffersFree = 0;
  if (!( cameraInfo->buffers = calloc( req.count, sizeof ( struct buffer )))) {
    fprintf ( stderr, "%s: calloc of transfer buffers failed\n", __FUNCTION__ );
    return -OA_ERR_MEM_ALLOC;
  }
  for ( n = 0; n < req.count; n++ ) {
    OA_CLEAR ( buf );
    buf.type = req.type;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n;
    if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_QUERYBUF, &buf )) {
      perror ( "VIDIOC_QUERYBUF v4l2ioctl failed" );
      free (( void* ) cameraInfo->buffers );
      return -OA_ERR_CAMERA_IO;
    }
    cameraInfo->buffers[ n ].length = buf.length;
    if ( MAP_FAILED == ( cameraInfo->buffers[ n ].start = v4l2_mmap ( 0,
        buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cameraInfo->fd,
        buf.m.offset ))) {
      perror ( "mmap" );
      if ( n ) {
        for ( m = 0; m < n; m++ ) {
          v4l2_munmap ( cameraInfo->buffers[ m ].start,
              cameraInfo->buffers[ m ].length );
        }
      }
      free (( void* ) cameraInfo->buffers );
      return -OA_ERR_SYSTEM_ERROR;
    }
    cameraInfo->configuredBuffers++;
  }
  cameraInfo->buffersFree = cameraInfo->configuredBuffers;

  for ( n = 0; n < req.count; n++ ) {
    OA_CLEAR( buf );
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n;
    if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_QBUF, &buf ) < 0 ) {
      perror ( "init VIDIOC_QBUF" );
      for ( m = 0; m < cameraInfo->configuredBuffers; m++ ) {
        v4l2_munmap ( cameraInfo->buffers[ m ].start,
            cameraInfo->buffers[ m ].length );
      }
      free (( void* ) cameraInfo->buffers );
      return -OA_ERR_SYSTEM_ERROR;
    }
  }

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_STREAMON, &type ) < 0 )  {
    if ( -ENOSPC == errno ) {
      fprintf ( stderr, "Insufficient bandwidth for camera on the USB bus\n" );
    }
    perror ( "VIDIOC_STREAMON" );
    for ( m = 0; m < cameraInfo->configuredBuffers; m++ ) {
      v4l2_munmap ( cameraInfo->buffers[ m ].start,
          cameraInfo->buffers[ m ].length );
    }
    free (( void* ) cameraInfo->buffers );
    return -OA_ERR_SYSTEM_ERROR;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 1;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  return OA_ERR_NONE;
}


static int
_processStreamingStop ( V4L2_STATE* cameraInfo, OA_COMMAND* command )
{
  int           	queueEmpty;
  unsigned int		n;
  enum v4l2_buf_type	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if ( !cameraInfo->isStreaming ) {
    return -OA_ERR_INVALID_COMMAND;
  }

  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  cameraInfo->isStreaming = 0;
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );

  if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_STREAMOFF, &type ) < 0 ) {
    perror ( "VIDIOC_STREAMOFF" );
  }

  // We wait here until the callback queue has drained otherwise unmapping
  // the buffers could rip the image frame out from underneath the callback

  queueEmpty = 0;
  do {
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    queueEmpty = ( OA_CAM_BUFFERS == cameraInfo->buffersFree ) ? 1 : 0;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
    if ( !queueEmpty ) {
      usleep ( 100 );
    }
  } while ( !queueEmpty );

  if ( cameraInfo->configuredBuffers ) {
    for ( n = 0; n < cameraInfo->configuredBuffers; n++ ) {
      v4l2_munmap ( cameraInfo->buffers[ n ].start,
          cameraInfo->buffers[ n ].length );
    }

    free (( void* ) cameraInfo->buffers );
  }
  cameraInfo->configuredBuffers = cameraInfo->buffersFree = 0;

  return OA_ERR_NONE;
}


static int
_processGetMenuItem ( V4L2_STATE* cameraInfo, OA_COMMAND* command )
{
  static char		buff[ V4L2_MAX_MENU_ITEM_LENGTH + 1 ];
  struct v4l2_querymenu	menuItem;
  int*			indexp;
  int			control, index;
  const char*		retStr;

  control = command->controlId;
  indexp = ( int* ) command->commandData;
  index = *indexp;
  retStr = buff;
  OA_CLEAR ( buff );

  // FIX ME -- need map of OA_CTRL to V4L2_CID values for this really
  if ( control != OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ) &&
      control != OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ) &&
      control != OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ) &&
      control != OA_CAM_CTRL_POWER_LINE_FREQ &&
      control != OA_CAM_CTRL_WHITE_BALANCE_PRESET ) {
    fprintf ( stderr, "%s: control not implemented\n", __FUNCTION__ );
    *buff = 0;
  } else {
    if ( OA_CAM_CTRL_WHITE_BALANCE_PRESET == control ) {
      if ( index >= 0 && index < OA_AWB_PRESET_LAST_P1 ) {
        retStr = oaCameraPresetAWBLabel[ index ];
      } else {
        retStr = "";
      }
    } else {
      OA_CLEAR( menuItem );
      switch ( control ) {
        case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_UNSCALED ):
        case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
          menuItem.id = V4L2_CID_EXPOSURE_AUTO;
          break;
        case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_WHITE_BALANCE ):
          menuItem.id = V4L2_CID_AUTO_WHITE_BALANCE;
          break;
        case OA_CAM_CTRL_POWER_LINE_FREQ:
          menuItem.id = V4L2_CID_POWER_LINE_FREQUENCY;
          break;
      }
      menuItem.index = index;
      if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_QUERYMENU, &menuItem )) {
        perror ("VIDIOC_QUERYMENU");
        fprintf ( stderr, "%s: control: %d, index %d\n", __FUNCTION__,
            menuItem.id, index );
        retStr = "";
      } else {
        strncpy ( buff, ( char* ) menuItem.name, V4L2_MAX_MENU_ITEM_LENGTH );
      }
    }
  }

  command->resultData = ( void* ) retStr;
  return OA_ERR_NONE;
}
