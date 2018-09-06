/*****************************************************************************
 *
 * zwoInit.c -- Initialise ZWO filter wheels
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

#include <errno.h>
#include <pthread.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

// This bit of nastiness is because the ZWO SDK uses this type without
// it being defined
typedef uint8_t bool;
#include <EFW_filter.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "zwofw.h"


static void _zwoInitFunctionPointers ( oaFilterWheel* );
static int  _getNumSlots ( oaFilterWheel* );

/**
 * Initialise a given filter wheel device
 */

oaFilterWheel*
oaZWOInitFilterWheel ( oaFilterWheelDevice* device )
{
  oaFilterWheel*			wheel;
  DEVICE_INFO*				devInfo;
  PRIVATE_INFO*				privateInfo;
  EFW_INFO            wheelInfo;

  devInfo = device->_private;

  wheelInfo.ID = devInfo->devIndex;
  EFWGetProperty ( wheelInfo.ID, &wheelInfo );

  if ( EFWOpen ( devInfo->devIndex ) != EFW_SUCCESS ) {
    perror ( "failed to open filter wheel" );
    return 0;
  }

  if (!( wheel = ( oaFilterWheel* ) malloc ( sizeof ( oaFilterWheel )))) {
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }
  if (!( privateInfo = ( PRIVATE_INFO* ) malloc ( sizeof ( PRIVATE_INFO )))) {
    ( void ) free (( void* ) wheel );
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }

  OA_CLEAR ( *wheel );
  OA_CLEAR ( *privateInfo );
  OA_CLEAR ( wheel->controls );
  // OA_CLEAR ( wheel->features );

  wheel->_private = privateInfo;

  pthread_mutex_init ( &privateInfo->ioMutex, 0 );

  ( void ) strcpy ( wheel->deviceName, device->deviceName );

  _oaInitFilterWheelFunctionPointers ( wheel );
  _zwoInitFunctionPointers ( wheel );

  privateInfo->initialised = 0;
  privateInfo->index = -1;

  wheel->interface = device->interface;
  privateInfo->index = devInfo->devIndex;
  privateInfo->wheelType = devInfo->devType;
  privateInfo->currentPosition = 1;

  pthread_mutex_init ( &privateInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &privateInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &privateInfo->callbackQueued, 0 );
  pthread_cond_init ( &privateInfo->commandQueued, 0 );
  pthread_cond_init ( &privateInfo->commandComplete, 0 );

  privateInfo->stopControllerThread = privateInfo->stopCallbackThread = 0;
  privateInfo->commandQueue = oaDLListCreate();
  privateInfo->callbackQueue = oaDLListCreate();
  if ( pthread_create ( &( privateInfo->controllerThread ), 0,
      oafwZWOcontroller, ( void* ) wheel )) {
    free (( void* ) wheel->_private );
    free (( void* ) wheel );
    oaDLListDelete ( privateInfo->commandQueue, 0 );
    oaDLListDelete ( privateInfo->callbackQueue, 0 );
    return 0;
  }

  if ( pthread_create ( &( privateInfo->callbackThread ), 0,
      oafwZWOcallbackHandler, ( void* ) wheel )) {

    void* dummy;
    privateInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &privateInfo->commandQueued );
    pthread_join ( privateInfo->controllerThread, &dummy );
    free (( void* ) wheel->_private );
    free (( void* ) wheel );
    oaDLListDelete ( privateInfo->commandQueue, 0 );
    oaDLListDelete ( privateInfo->callbackQueue, 0 );
    return 0;
  }

  wheel->numSlots = wheelInfo.slotNum;
  wheel->controls [ OA_FW_CTRL_MOVE_ABSOLUTE_ASYNC ] = OA_CTRL_TYPE_INT32;
  return wheel;
}


static void
_zwoInitFunctionPointers ( oaFilterWheel* wheel )
{
  wheel->funcs.initWheel = oaZWOInitFilterWheel;
  wheel->funcs.closeWheel = oaZWOWheelClose;
  wheel->funcs.readControl = oaZWOWheelReadControl;
  wheel->funcs.setControl = oaZWOWheelSetControl;
  // wheel->funcs.testControl = XXXX;
}


int
_getNumSlots ( oaFilterWheel* wheel )
{
  fprintf ( stderr, "guessing at number of slots" );
  return 5;
}


int
oaZWOWheelClose ( oaFilterWheel* wheel )
{
  PRIVATE_INFO*		privateInfo;

  privateInfo = wheel->_private;
  EFWClose ( privateInfo->index );
  privateInfo->initialised = 0;
  return 0;
}
