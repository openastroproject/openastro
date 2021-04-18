/*****************************************************************************
 *
 * Spincontroller.c -- Main camera controller thread
 *
 * Copyright 2021  James Fidell (james@openastroproject.org)
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
#include <sys/time.h>

#include <openastro/camera.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Spin.h"
#include "Spinstate.h"

static int	_processSetControl ( oaCamera*, OA_COMMAND* );
static int	_processGetControl ( SPINNAKER_STATE*, OA_COMMAND* );
static int	_processSetResolution ( SPINNAKER_STATE*, OA_COMMAND* );
static int	_processSetROI ( oaCamera*, OA_COMMAND* );
static int	_processStreamingStart ( SPINNAKER_STATE*, OA_COMMAND* );
static int	_processStreamingStop ( SPINNAKER_STATE*, OA_COMMAND* );
//static int	_setBinning ( SPINNAKER_STATE*, int );
//static int	_setFrameFormat ( SPINNAKER_STATE*, int );


void*
oacamSpinController ( void* param )
{
  oaCamera*		camera = param;
  SPINNAKER_STATE*	cameraInfo = camera->_private;
  OA_COMMAND*		command;
  int			exitThread = 0;
  int			resultCode;
  int			streaming = 0;

  do {
    pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
    exitThread = cameraInfo->stopControllerThread;
    pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    if ( exitThread ) {
      break;
    } else {
      pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
      // stop us busy-waiting
      streaming = ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) ? 1 : 0;
      if ( !streaming && oaDLListIsEmpty ( cameraInfo->commandQueue )) {
        pthread_cond_wait ( &cameraInfo->commandQueued,
            &cameraInfo->commandQueueMutex );
      }
      pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
    }
    do {
      command = oaDLListRemoveFromHead ( cameraInfo->commandQueue );
      if ( command ) {
        switch ( command->commandType ) {
          case OA_CMD_CONTROL_SET:
            resultCode = _processSetControl ( camera, command );
            break;
          case OA_CMD_CONTROL_GET:
            resultCode = _processGetControl ( cameraInfo, command );
            break;
          case OA_CMD_RESOLUTION_SET:
            resultCode = _processSetResolution ( cameraInfo, command );
            break;
          case OA_CMD_ROI_SET:
            resultCode = _processSetROI ( camera, command );
            break;
          case OA_CMD_START_STREAMING:
            resultCode = _processStreamingStart ( cameraInfo, command );
            break;
          case OA_CMD_STOP_STREAMING:
            resultCode = _processStreamingStop ( cameraInfo, command );
            break;
          default:
            fprintf ( stderr, "Invalid command type %d in controller\n",
                command->commandType );
            resultCode = -OA_ERR_INVALID_CONTROL;
            break;
        }
        if ( command->callback ) {
        } else {
          pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
          command->completed = 1;
          command->resultCode = resultCode;
          pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
          pthread_cond_broadcast ( &cameraInfo->commandComplete );
        }
      }
    } while ( command );

    if ( streaming ) {
    }
  } while ( !exitThread );

  return 0;
}


static int
_processSetControl ( oaCamera* camera, OA_COMMAND* command )
{
	oaLogError ( OA_LOG_CAMERA, "%s: not yet implemented", __func__ );
  return -OA_ERR_INVALID_CONTROL;
}


static int
_processGetControl ( SPINNAKER_STATE* cameraInfo, OA_COMMAND* command )
{
	oaLogError ( OA_LOG_CAMERA, "%s: not yet implemented", __func__ );
  return -OA_ERR_INVALID_CONTROL;
}



static int
_processSetResolution ( SPINNAKER_STATE* cameraInfo, OA_COMMAND* command )
{
  FRAMESIZE*			size = command->commandData;
  unsigned int		binMode, s, restart = 0;
  int							found;

  if ( size->x == cameraInfo->xSize && size->y == cameraInfo->ySize ) {
    return OA_ERR_NONE;
  }

  found = -1;
  binMode = cameraInfo->binMode;
  for ( s = 0; s < cameraInfo->frameSizes[ binMode ].numSizes && found < 0;
			s++ ) {
    if ( cameraInfo->frameSizes[ binMode ].sizes[ s ].x >= size->x &&
        cameraInfo->frameSizes[ binMode ].sizes[ s ].y >= size->y ) {
      found = s;
      break;
    }
  }

  if ( found < 0 ) {
    oaLogError ( OA_LOG_CAMERA, "%s: resolution %dx%d not found", __func__,
				size->x, size->y );
    return -OA_ERR_OUT_OF_RANGE;
  }

/*
  settings.width = size->x;
  settings.height = size->y;
  settings.offsetX = ( imageInfo.maxWidth - size->x ) / 2;
  settings.offsetY = ( imageInfo.maxHeight - size->y ) / 2;
*/

	oaLogError ( OA_LOG_CAMERA, "%s: implementation incomplete", __func__ );

  if ( cameraInfo->runMode == CAM_RUN_MODE_STREAMING ) {
    restart = 1;
/*
    _doStop ( cameraInfo );
*/
  }

  cameraInfo->xSize = size->x;
  cameraInfo->ySize = size->y;
  cameraInfo->imageBufferLength = size->x * size->y *
      cameraInfo->currentBytesPerPixel;

  if ( restart ) {
/*
    _doStart ( cameraInfo );
*/
  }
  return OA_ERR_NONE;
}


static int
_processSetROI ( oaCamera* camera, OA_COMMAND* command )
{
/*
  SPINNAKER_STATE*			cameraInfo = camera->_private;
  FRAMESIZE*			size = command->commandData;
  unsigned int			x, y;
  int				ret, restart = 0;
*/
  if (!( camera->features.flags & OA_CAM_FEATURE_ROI )) {
    return -OA_ERR_INVALID_CONTROL;
  }

	oaLogError ( OA_LOG_CAMERA, "%s: implementation incomplete", __func__ );

  return OA_ERR_NONE;
}


static int
_processStreamingStart ( SPINNAKER_STATE* cameraInfo, OA_COMMAND* command )
{
	oaLogError ( OA_LOG_CAMERA, "%s: not yet implemented", __func__ );
  return OA_ERR_NONE;
}


static int
_processStreamingStop ( SPINNAKER_STATE* cameraInfo, OA_COMMAND* command )
{
	oaLogError ( OA_LOG_CAMERA, "%s: not yet implemented", __func__ );
  return OA_ERR_NONE;
}

/*
static int
_setBinning ( SPINNAKER_STATE* cameraInfo, int binMode )
{
	oaLogError ( OA_LOG_CAMERA, "%s: not yet implemented", __func__ );
  return OA_ERR_NONE;
}


static int
_setFrameFormat ( SPINNAKER_STATE* cameraInfo, int format )
{
	oaLogError ( OA_LOG_CAMERA, "%s: not yet implemented", __func__ );
  return OA_ERR_NONE;
}
*/
