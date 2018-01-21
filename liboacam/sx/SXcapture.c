/*****************************************************************************
 *
 * SXcapture.c -- capture functions for Starlight Xpress cameras
 *
 * Copyright 2014 James Fidell (james@openastroproject.org)
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

#include "SXoacam.h"
#include "oacamprivate.h"

#define cameraState             camera->_sx

int
oaSXCameraReadFrame ( oaCamera* camera, void** bufferPtr )
{
  oacamDebugMsg ( DEBUG_CAM_CMD, "SX: command: %s()\n", __FUNCTION__ );

  int haveData = 0;
  int bufferToRead;
  struct timespec waitUntil;
  struct timeval now;
  unsigned long frameSec = 0;
  unsigned long frameNanoSec = 0;

  frameNanoSec = cameraState.currentExposure * 1000000;
  frameSec = cameraState.currentExposure / 1000 + 5;

  pthread_mutex_lock ( &cameraState.captureMutex );
  while ( !cameraState.haveDataToRead ) {

    gettimeofday ( &now, 0 );
    waitUntil.tv_sec = now.tv_sec + frameSec;
    if (( now.tv_usec * 1000 + frameNanoSec ) > 1000000000 ) {
      waitUntil.tv_sec++;
    }
    waitUntil.tv_nsec = ( now.tv_usec * 1000 + frameNanoSec ) % 1000000000;

    pthread_cond_timedwait ( &cameraState.frameAvailable,
        &cameraState.captureMutex, &waitUntil );
  }
  haveData = cameraState.haveDataToRead;
  bufferToRead = cameraState.nextBufferToRead;
  cameraState.haveDataToRead = 0;
  pthread_mutex_unlock ( &cameraState.captureMutex );

  *bufferPtr = haveData ? cameraState.buffers[ bufferToRead ].start : 0;
  return ( haveData ? cameraState.imageBufferLength : 0 );
}


int
oaSXCameraStartReadFrame ( oaCamera* camera, unsigned int frameTime )
{
  oacamDebugMsg ( DEBUG_CAM_CMD, "SX: command: %s ( %d )\n",
      __FUNCTION__, frameTime );

  return OA_ERR_NONE;
}


int
oaSXCameraFinishReadFrame ( oaCamera* camera )
{
  oacamDebugMsg ( DEBUG_CAM_CMD, "SX: command: %s()\n", __FUNCTION__ );

  return OA_ERR_NONE;
}
