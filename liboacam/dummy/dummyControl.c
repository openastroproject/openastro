/*****************************************************************************
 *
 * dummyControl.c -- control functions for dummy cameras
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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
#include <openastro/camera.h>

#include <pthread.h>

#include "oacamprivate.h"
#include "dummyoacam.h"
#include "dummystate.h"


int
oaDummyCameraReadControl ( oaCamera* camera, int control,
    oaControlValue* val )
{
  OA_COMMAND    command;
  DUMMY_STATE*  cameraInfo = camera->_private;
  int     retval;

  // Could do more validation here, but it's a bit messy to do here
  // and in the controller too.

  OA_CLEAR ( command );
  command.commandType = OA_CMD_CONTROL_GET;
  command.controlId = control;
  command.resultData = val;

  cameraInfo = camera->_private;
  oaDLListAddToTail ( cameraInfo->commandQueue, &command );
  pthread_cond_broadcast ( &cameraInfo->commandQueued );
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &cameraInfo->commandComplete,
        &cameraInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaDummyCameraTestControl ( oaCamera* camera, int control,
    oaControlValue* val )
{
  int32_t	val_s32;
  DUMMY_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "%s: ( %d, ? )\n", __FUNCTION__, control );

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->OA_CAM_CTRL_TYPE( control ) != val->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  switch ( control ) {
    case OA_CAM_CTRL_GAIN:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    // case OA_CAM_CTRL_GAMMA:
    case OA_CAM_CTRL_BRIGHTNESS:
      val_s32 = val->int32;
      if ( val_s32 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val_s32 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val_s32 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_BINNING:
    {
      int i = 0;

      val_s32 = val->discrete;
      while ( cameraInfo->binModes[i] && i < 16 ) {
        if ( val_s32 == cameraInfo->binModes[i] ) {
          return OA_ERR_NONE;
        }
        i++;
      }
      break;
    }

    // This lot are all boolean, so we'll take any value
    case OA_CAM_CTRL_HFLIP:
    case OA_CAM_CTRL_VFLIP:
      return OA_ERR_NONE;
      break;

    default:
      // If we reach here it's because we don't recognise the control
      fprintf ( stderr, "Unrecognised control %d in oaDummyCameraSetControl\n",
          control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  // And if we reach here it's because the value wasn't valid
  return -OA_ERR_OUT_OF_RANGE;
}


int
oaDummyCameraSetControl ( oaCamera* camera, int control, oaControlValue* val,
    int dontWait )
{
  OA_COMMAND	command;
  DUMMY_STATE*	cameraInfo;
  int		retval = OA_ERR_NONE;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "%s: ( %d, ? )\n", __FUNCTION__, control );

  switch ( control ) {

    case OA_CAM_CTRL_BRIGHTNESS:
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;

			/*
    case OA_CAM_CTRL_GAMMA:
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;
			*/

    case OA_CAM_CTRL_GAIN:
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;

    case OA_CAM_CTRL_BINNING:
      if ( val->valueType != OA_CTRL_TYPE_DISCRETE ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;

    case OA_CAM_CTRL_HFLIP:
      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;

    case OA_CAM_CTRL_VFLIP:
      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;

    case OA_CAM_CTRL_FRAME_FORMAT:
      if ( val->valueType != OA_CTRL_TYPE_DISCRETE ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;

			/*
    case OA_CAM_CTRL_COOLER:
      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;
			*/

			/*
    case OA_CAM_CTRL_TEMP_SETPOINT:
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      break;
			*/

    default:
      fprintf ( stderr, "Unrecognised control %d in %s\n", control,
          __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  // Ok, at this point we have a new control setting that we think is good

  OA_CLEAR ( command );
  command.commandType = OA_CMD_CONTROL_SET;
  command.controlId = control;
  command.commandData = val;

  cameraInfo = camera->_private;
  oaDLListAddToTail ( cameraInfo->commandQueue, &command );
  pthread_cond_broadcast ( &cameraInfo->commandQueued );
  if ( !dontWait ) {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    while ( !command.completed ) {
      pthread_cond_wait ( &cameraInfo->commandComplete,
          &cameraInfo->commandQueueMutex );
    }
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    retval = command.resultCode;
  }

  return retval;
}


int
oaDummyCameraSetResolution ( oaCamera* camera, int x, int y )
{
  FRAMESIZE	s;
  OA_COMMAND	command;
  DUMMY_STATE*	cameraInfo = camera->_private;
  int		retval;

  OA_CLEAR ( command );
  command.commandType = OA_CMD_RESOLUTION_SET;
  s.x = x;
  s.y = y;
  command.commandData = &s;
  cameraInfo = camera->_private;
  oaDLListAddToTail ( cameraInfo->commandQueue, &command );
  pthread_cond_broadcast ( &cameraInfo->commandQueued );
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &cameraInfo->commandComplete,
        &cameraInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaDummyCameraStartStreaming ( oaCamera* camera,
    void* (*callback)(void*, void*, int, void* ), void* callbackArg )
{
  OA_COMMAND	command;
  CALLBACK	callbackData;
  int		retval;
  DUMMY_STATE*	cameraInfo = camera->_private;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "dummy: control: %s ( %p )\n",
      __FUNCTION__, callback );

  OA_CLEAR ( command );
  callbackData.callback = callback;
  callbackData.callbackArg = callbackArg;
  command.commandType = OA_CMD_START_STREAMING;
  command.commandData = ( void* ) &callbackData;

  oaDLListAddToTail ( cameraInfo->commandQueue, &command );
  pthread_cond_broadcast ( &cameraInfo->commandQueued );
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &cameraInfo->commandComplete,
        &cameraInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaDummyCameraIsStreaming ( oaCamera* camera )
{
  DUMMY_STATE*	cameraInfo = camera->_private;

  return ( cameraInfo->isStreaming );
}


int
oaDummyCameraStopStreaming ( oaCamera* camera )
{
  OA_COMMAND    command;
  int           retval;
  DUMMY_STATE*  cameraInfo = camera->_private;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "dummy: control: %s()\n", __FUNCTION__ );

  OA_CLEAR ( command );
  command.commandType = OA_CMD_STOP_STREAMING;

  oaDLListAddToTail ( cameraInfo->commandQueue, &command );
  pthread_cond_broadcast ( &cameraInfo->commandQueued );
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &cameraInfo->commandComplete,
        &cameraInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}
