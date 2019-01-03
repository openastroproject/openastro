/*****************************************************************************
 *
 * genericCallback.c -- Thread for handling callbacks to user code
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

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "unimplemented.h"


void*
oafwCallbackHandler ( void* param )
{
  oaFilterWheel*	wheel = param;
  PRIVATE_INFO*		wheelInfo = wheel->_private;
  int			exitThread = 0;
  CALLBACK*		callback;
  // void*			(*callbackFunc)( void*, void*, int);

  do {
    pthread_mutex_lock ( &wheelInfo->callbackQueueMutex );
    exitThread = wheelInfo->stopCallbackThread;
    pthread_mutex_unlock ( &wheelInfo->callbackQueueMutex );

    if ( exitThread ) {
      break;
    } else {
      // try to prevent busy-waiting
      if ( oaDLListIsEmpty ( wheelInfo->callbackQueue )) {
        pthread_mutex_lock ( &wheelInfo->callbackQueueMutex );
        pthread_cond_wait ( &wheelInfo->callbackQueued,
            &wheelInfo->callbackQueueMutex );
        pthread_mutex_unlock ( &wheelInfo->callbackQueueMutex );
      }
    }

    callback = oaDLListRemoveFromHead ( wheelInfo->callbackQueue );
    if ( callback ) {
      switch ( callback->callbackType ) {
        default:
          fprintf ( stderr, "unexpected callback type %d\n",
              callback->callbackType );
          break;
      }
    }
  } while ( 1 );

  return 0;
}
