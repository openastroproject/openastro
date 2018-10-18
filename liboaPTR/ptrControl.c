/*****************************************************************************
 *
 * ptrControl.c -- PTR device control functions
 *
 * Copyright 2015,2016,2017,2018 James Fidell (james@openastroproject.org)
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

#include <errno.h>
#include <pthread.h>

#include <openastro/util.h>
#include <openastro/timer.h>
#include <openastro/ptr/controls.h>

#include "oaptrprivate.h"
#include "unimplemented.h"
#include "ptr.h"


int
oaPTRSetControl ( oaPTR* device, int control, oaControlValue* val )
{
  OA_COMMAND	command;
  PRIVATE_INFO*	privateInfo = device->_private;
  int		retval;

  OA_CLEAR ( command );
  command.commandType = OA_CMD_CONTROL_SET;
  command.controlId = control;
  command.commandData = val;

  oaDLListAddToTail ( privateInfo->commandQueue, &command );
  pthread_cond_broadcast ( &privateInfo->commandQueued );
  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &privateInfo->commandComplete,
        &privateInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaPTRReadControl ( oaPTR* device, int control, oaControlValue* val )
{
  OA_COMMAND	command;
  PRIVATE_INFO*	privateInfo = device->_private;
  int		retval;

  // Could do more validation here, but it's a bit messy to do here
  // and in the controller too.

  OA_CLEAR ( command );
  command.commandType = OA_CMD_CONTROL_GET;
  command.controlId = control;
  command.resultData = val;

  oaDLListAddToTail ( privateInfo->commandQueue, &command );
  pthread_cond_broadcast ( &privateInfo->commandQueued );
  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &privateInfo->commandComplete,
        &privateInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaPTRReset ( oaPTR* device )
{
  OA_COMMAND    command;
  PRIVATE_INFO* privateInfo = device->_private;
  int           retval;

  OA_CLEAR ( command );
  command.commandType = OA_CMD_RESET;

  oaDLListAddToTail ( privateInfo->commandQueue, &command );
  pthread_cond_broadcast ( &privateInfo->commandQueued );
  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &privateInfo->commandComplete,
        &privateInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaPTRStart ( oaPTR* device )
{
  OA_COMMAND    command;
  PRIVATE_INFO* privateInfo = device->_private;
  int           retval;

  OA_CLEAR ( command );
  command.commandType = OA_CMD_START;

  oaDLListAddToTail ( privateInfo->commandQueue, &command );
  pthread_cond_broadcast ( &privateInfo->commandQueued );
  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &privateInfo->commandComplete,
        &privateInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaPTRStop ( oaPTR* device )
{
  OA_COMMAND    command;
  PRIVATE_INFO* privateInfo = device->_private;
  int           retval;

  OA_CLEAR ( command );
  command.commandType = OA_CMD_STOP;

  oaDLListAddToTail ( privateInfo->commandQueue, &command );
  pthread_cond_broadcast ( &privateInfo->commandQueued );
  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &privateInfo->commandComplete,
        &privateInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaPTRIsRunning ( oaPTR* device )
{
  int r;

  PRIVATE_INFO* privateInfo = device->_private;

  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  r = privateInfo->isRunning;
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  return r;
}


int
oaPTRGetTimestamp ( oaPTR* device, int timestampWait, oaTimerStamp* val )
{
  OA_COMMAND    command;
  PRIVATE_INFO* privateInfo = device->_private;
  int           retval;

  // Could do more validation here, but it's a bit messy to do here
  // and in the controller too.

  OA_CLEAR ( command );
  command.commandType = OA_CMD_DATA_GET;
  command.commandData = &timestampWait;
  command.resultData = val;

  oaDLListAddToTail ( privateInfo->commandQueue, &command );
  pthread_cond_broadcast ( &privateInfo->commandQueued );
  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &privateInfo->commandComplete,
        &privateInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaPTRReadGPS ( oaPTR* device, double* data )
{
  OA_COMMAND    command;
  PRIVATE_INFO* privateInfo = device->_private;
  int           retval;

  // Could do more validation here, but it's a bit messy to do here
  // and in the controller too.

  OA_CLEAR ( command );
  command.commandType = OA_CMD_GPS_GET;
  command.resultData = data;

  oaDLListAddToTail ( privateInfo->commandQueue, &command );
  pthread_cond_broadcast ( &privateInfo->commandQueued );
  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &privateInfo->commandComplete,
        &privateInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}


int
oaPTRReadCachedGPS ( oaPTR* device, double* data )
{
  OA_COMMAND    command;
  PRIVATE_INFO* privateInfo = device->_private;
  int           retval;

  // Could do more validation here, but it's a bit messy to do here
  // and in the controller too.

  OA_CLEAR ( command );
  command.commandType = OA_CMD_GPS_CACHE_GET;
  command.resultData = data;

  oaDLListAddToTail ( privateInfo->commandQueue, &command );
  pthread_cond_broadcast ( &privateInfo->commandQueued );
  pthread_mutex_lock ( &privateInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &privateInfo->commandComplete,
        &privateInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &privateInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}
