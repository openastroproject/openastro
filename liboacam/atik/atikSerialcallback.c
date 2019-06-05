/*****************************************************************************
 *
 * atikSerialcallback.c -- Thread for handling callbacks to user code
 *
 * Copyright 2015,2016,2019 James Fidell (james@openastroproject.org)
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
#include <libusb-1.0/libusb.h>

#include <openastro/camera.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "atikSerial.h"
#include "atikSerialoacam.h"
#include "atikSerialstate.h"


void*
oacamAtikSerialcallbackHandler ( void* param )
{
  oaCamera*		camera = param;
  AtikSerial_STATE*	cameraInfo = camera->_private;
  int			exitThread = 0;
  CALLBACK*		callback;
  void*			(*callbackFunc)( void*, void*, int, void* );

  do {
    pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
    exitThread = cameraInfo->stopCallbackThread;
    pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );

    if ( exitThread ) {
      break;
    } else {
      // try to prevent busy-waiting
      if ( oaDLListIsEmpty ( cameraInfo->callbackQueue )) {
        pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
        pthread_cond_wait ( &cameraInfo->callbackQueued,
            &cameraInfo->callbackQueueMutex );
        pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
      }
    }

    callback = oaDLListRemoveFromHead ( cameraInfo->callbackQueue );
    if ( callback ) {
      switch ( callback->callbackType ) {
        case OA_CALLBACK_NEW_FRAME:
          callbackFunc = callback->callback;
          callbackFunc ( callback->callbackArg, callback->buffer,
              callback->bufferLen, 0 );
          // We can only requeue frames if we're still streaming
          pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
          cameraInfo->buffersFree++;
          pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
          break;
        default:
          fprintf ( stderr, "unexpected callback type %d\n",
              callback->callbackType );
          break;
      }
    }
  } while ( 1 );

  return 0;
}
