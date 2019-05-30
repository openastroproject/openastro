/*****************************************************************************
 *
 * zwoController.c -- ZWO filter wheel controller thread
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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
#include <errno.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include <EFW_filter.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "zwofw.h"
#include "zwofwprivate.h"


static int	_processSetControl ( PRIVATE_INFO*, OA_COMMAND* );
static int	_processGetControl ( PRIVATE_INFO*, OA_COMMAND* );
static int	_oaZWOMoveTo ( PRIVATE_INFO*, int );


void*
oafwZWOcontroller ( void* param )
{
  oaFilterWheel*	wheel = param;
  PRIVATE_INFO*		wheelInfo = wheel->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode;

  do {
    pthread_mutex_lock ( &wheelInfo->commandQueueMutex );
    exitThread = wheelInfo->stopControllerThread;
    pthread_mutex_unlock ( &wheelInfo->commandQueueMutex );
    if ( exitThread ) {
      break;
    } else {
      pthread_mutex_lock ( &wheelInfo->commandQueueMutex );
      // stop us busy-waiting
      if ( oaDLListIsEmpty ( wheelInfo->commandQueue )) {
        pthread_cond_wait ( &wheelInfo->commandQueued,
            &wheelInfo->commandQueueMutex );
      }
      pthread_mutex_unlock ( &wheelInfo->commandQueueMutex );
    }
    do {
      command = oaDLListRemoveFromHead ( wheelInfo->commandQueue );
      if ( command ) {
        switch ( command->commandType ) {
          case OA_CMD_CONTROL_SET:
            resultCode = _processSetControl ( wheelInfo, command );
            break;
          case OA_CMD_CONTROL_GET:
            resultCode = _processGetControl ( wheelInfo, command );
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
          pthread_mutex_lock ( &wheelInfo->commandQueueMutex );
          command->completed = 1;
          command->resultCode = resultCode;
          pthread_mutex_unlock ( &wheelInfo->commandQueueMutex );
          pthread_cond_broadcast ( &wheelInfo->commandComplete );
        }
      }
    } while ( command );

  } while ( !exitThread );

  return 0;
}


static int
_processSetControl ( PRIVATE_INFO* wheelInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  oaControlValue*	val = command->commandData;

  oafwDebugMsg ( DEBUG_CAM_CTRL, "ZWO: control: %s ( %d, ? )\n",
      __FUNCTION__, control );

  switch ( control ) {

    case OA_FW_CTRL_MOVE_ABSOLUTE_ASYNC:
    {
      int	slot;

      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      slot = val->int32;
      _oaZWOMoveTo ( wheelInfo, slot );
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


static int
_processGetControl ( PRIVATE_INFO* cameraInfo, OA_COMMAND* command )
{
  int			control = command->controlId;
  // oaControlValue*	val = command->resultData;

  oafwDebugMsg ( DEBUG_CAM_CTRL, "ZWO: control: %s ( %d )\n",
      __FUNCTION__, control );

  fprintf ( stderr,
      "Unrecognised control %d in %s\n", control, __FUNCTION__ );
  return -OA_ERR_INVALID_CONTROL;
}


static int
_oaZWOMoveTo ( PRIVATE_INFO* wheelInfo, int slot )
{
  pthread_mutex_lock ( &wheelInfo->ioMutex );
  // FIX ME -- check for errors
  p_EFWSetPosition ( wheelInfo->index, slot - 1 );
  pthread_mutex_unlock ( &wheelInfo->ioMutex );

  return 0;
}
