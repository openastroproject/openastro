/*****************************************************************************
 *
 * V4L2callback.c -- Thread for handling callbacks to user code
 *
 * Copyright 2015,2019,2021 James Fidell (james@openastroproject.org)
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
#include "unimplemented.h"
#include "V4L2.h"
#include "V4L2oacam.h"
#include "V4L2state.h"
#include "V4L2ioctl.h"


void*
oacamV4L2callbackHandler ( void* param )
{
  oaCamera*		camera = param;
  V4L2_STATE*		cameraInfo = camera->_private;
  int			exitThread = 0, streaming;
  CALLBACK*		callback;
  void*			(*callbackFunc)( void*, void*, int, void* );
  struct v4l2_buffer*	frameData;

	oaLogInfo ( OA_LOG_CAMERA, "%s: thread started", __func__ );

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
          frameData = callback->buffer;
          callbackFunc ( callback->callbackArg,
              cameraInfo->buffers[ frameData->index ].start,
              callback->bufferLen, 0 );
          // We can only requeue frames if we're still streaming
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          streaming = ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) ? 1 : 0;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
          if ( streaming ) {
            if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_QBUF, frameData )) {
							oaLogError ( OA_LOG_CAMERA, "%s: VIDIOC_DQBUF failed", __func__ );
            }
          }
          pthread_mutex_lock ( &cameraInfo->callbackQueueMutex );
          cameraInfo->buffersFree++;
          pthread_mutex_unlock ( &cameraInfo->callbackQueueMutex );
          break;
        default:
					oaLogWarning ( OA_LOG_CAMERA, "%s: unexpected callback type %d",
              __func__, callback->callbackType );
          break;
      }
    }
  } while ( 1 );

	oaLogInfo ( OA_LOG_CAMERA, "%s: exiting thread", __func__ );

  return 0;
}
