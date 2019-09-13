/*****************************************************************************
 *
 * QHYcontrol.c -- control functions for QHY cameras
 *
 * Copyright 2014,2015,2017,2019 James Fidell (james@openastroproject.org)
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

#include "oacamprivate.h"
#include "QHY.h"
#include "QHYoacam.h"
#include "QHYstate.h"


int
oaQHYCameraSetControl ( oaCamera* camera, int control, oaControlValue* val,
  int dontWait )
{
  OA_COMMAND	command;
  QHY_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;
  int		retval = OA_ERR_NONE;

  // Could do more validation here, but it's a bit messy to do here
  // and in the controller too.

  if ( camera->OA_CAM_CTRL_TYPE( control ) == OA_CTRL_TYPE_INT32 &&
      val->valueType == OA_CTRL_TYPE_INT32 && (
      val->int32 < commonInfo->OA_CAM_CTRL_MIN( control ) ||
      val->int32 > commonInfo->OA_CAM_CTRL_MAX( control ))) {
    return -OA_ERR_OUT_OF_RANGE;
  }

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
oaQHYCameraReadControl ( oaCamera* camera, int control, oaControlValue* val )
{
  OA_COMMAND    command;
  QHY_STATE*	cameraInfo = camera->_private;
  int           retval;

  // Could do more validation here, but it's a bit messy to do here
  // and in the controller too.

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    fprintf ( stderr, "Unimplemented control %d in %s\n", control,
        __FUNCTION__ );
    return -OA_ERR_INVALID_CONTROL;
  }

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

/*
int
oaQHYCameraSetFrameInterval ( oaCamera* camera, int numerator,
    int denominator )
{
  FRAMERATE	r;
  OA_COMMAND	command;
  QHY_STATE*	cameraInfo = camera->_private;
  int		retval;

  OA_CLEAR ( command );
  command.commandType = OA_CMD_FRAME_INTERVAL_SET;
  r.numerator = numerator;
  r.denominator = denominator;
  command.commandData = &r;
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
*/

int
oaQHYCameraSetResolution ( oaCamera* camera, int x, int y )
{
  FRAMESIZE	s;
  OA_COMMAND	command;
  QHY_STATE*	cameraInfo = camera->_private;
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
oaQHYCameraStartStreaming ( oaCamera* camera,
    void* (*callback)(void*, void*, int, void* ), void* callbackArg )
{
  OA_COMMAND    command;
  CALLBACK      callbackData;
  int           retval;
  QHY_STATE*  cameraInfo = camera->_private;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY: control: %s ( %p )\n",
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
oaQHYCameraIsStreaming ( oaCamera* camera )
{
  QHY_STATE*  cameraInfo = camera->_private;

  return ( cameraInfo->isStreaming );
}


int
oaQHYCameraStopStreaming ( oaCamera* camera )
{
  OA_COMMAND    command;
  int           retval;
  QHY_STATE*   cameraInfo = camera->_private;

  oacamDebugMsg ( DEBUG_CAM_CTRL, "QHY: control: %s()\n", __FUNCTION__ );

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

/*
const char*
oaQHYCameraGetMenuString ( oaCamera* camera, int control, int index )
{
  if ( control != OA_CAM_CTRL_AUTO_EXPOSURE ) {
    fprintf ( stderr, "%s: control not implemented\n", __FUNCTION__ );
    return "";
  }

  switch ( index ) {
    case 1:
      return "Manual";
      break;
    case 2:
      return "Auto";
      break;
    case 4:
      return "Shutter Priority";
      break;
    case 8:
      return "Aperture Priority";
      break;
  }

  return "Unknown";
}
*/
