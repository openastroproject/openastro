/*****************************************************************************
 *
 * zwoConnect.c -- Initialise ZWO filter wheels
 *
 * Copyright 2018,2019 James Fidell (james@openastroproject.org)
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

#include <EFW_filter.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "zwofw.h"
#include "zwofwprivate.h"


static void _zwoInitFunctionPointers ( oaFilterWheel* );

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
  EFW_ERROR_CODE      err;

  devInfo = device->_private;

  if (( err = p_EFWGetID ( devInfo->devIndex, &wheelInfo.ID )) !=
			EFW_SUCCESS ) {
    fprintf ( stderr, "%s: EFWGetID returns error %d\n", __FUNCTION__, err );
    return 0;
  }
  if (( err = p_EFWOpen ( wheelInfo.ID )) != EFW_SUCCESS ) {
    fprintf ( stderr, "%s: EFWOpen returns error %d\n", __FUNCTION__, err );
    return 0;
  }
  if (( err = p_EFWGetProperty ( wheelInfo.ID, &wheelInfo )) != EFW_SUCCESS ) {
    fprintf ( stderr, "%s: EFWGetProperty returns error %d\n",
        __FUNCTION__, err );
    return 0;
  }

  if (!( wheel = ( oaFilterWheel* ) malloc ( sizeof ( oaFilterWheel )))) {
    p_EFWClose ( wheelInfo.ID );
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }
  if (!( privateInfo = ( PRIVATE_INFO* ) malloc ( sizeof ( PRIVATE_INFO )))) {
    p_EFWClose ( wheelInfo.ID );
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
  privateInfo->index = wheelInfo.ID;
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
      oafwCallbackHandler, ( void* ) wheel )) {

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
  wheel->funcs.setControl = oaWheelSetControl;
  wheel->funcs.readControl = oaWheelReadControl;
  // wheel->funcs.testControl = XXXX;
}


int
oaZWOWheelClose ( oaFilterWheel* wheel )
{
  PRIVATE_INFO*		privateInfo;

  privateInfo = wheel->_private;
  p_EFWClose ( privateInfo->index );
  privateInfo->initialised = 0;
  return 0;
}
