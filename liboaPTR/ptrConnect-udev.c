/*****************************************************************************
 *
 * ptrInit-udev.c -- Initialise PTR device (udev)
 *
 * Copyright 2015,2016 James Fidell (james@openastroproject.org)
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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>

#include <openastro/util.h>
#include <openastro/ptr.h>
#include <openastro/timer.h>

#include "oaptrprivate.h"
#include "unimplemented.h"
#include "ptr.h"


static void _ptrInitFunctionPointers ( oaPTR* );


/**
 * Initialise a PTR device
 */

oaPTR*
oaPTRInit ( oaPTRDevice* device )
{
  oaPTR*		ptr = 0;
  int			ptrDesc;
  struct termios	tio;
  DEVICE_INFO*		devInfo;
  PRIVATE_INFO*		privateInfo;
  COMMON_INFO*		commonInfo;

  devInfo = device->_private;

  if (!( ptr = ( oaPTR* ) malloc ( sizeof ( oaPTR )))) {
    perror ( "malloc oaPTR failed" );
    return 0;
  }
  if (!( privateInfo = ( PRIVATE_INFO* ) malloc ( sizeof ( PRIVATE_INFO )))) {
    free (( void* ) ptr );
    perror ( "malloc oaPTR failed" );
    return 0;
  }
  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    free (( void* ) privateInfo );
    free (( void* ) ptr );
    perror ( "malloc COMMON_INFO failed" );
    return 0;
  }

  OA_CLEAR ( *ptr );
  OA_CLEAR ( *privateInfo );
  OA_CLEAR ( *commonInfo );
  OA_CLEAR ( ptr->controls );

  ptr->_private = privateInfo;
  ptr->_common = commonInfo;

  pthread_mutex_init ( &privateInfo->ioMutex, 0 );

  ( void ) strcpy ( ptr->deviceName, device->deviceName );
  ( void ) strcpy ( privateInfo->devicePath, devInfo->sysPath );

  privateInfo->initialised = 0;
  privateInfo->index = -1;

  if (( ptrDesc = open ( devInfo->sysPath, O_RDWR | O_NOCTTY )) < 0 ) {
    fprintf ( stderr, "Can't open %s read-write, errno = %d\n",
        devInfo->sysPath, errno );
    free (( void* ) ptr );
    free (( void* ) privateInfo );
    free (( void* ) commonInfo );
    return 0;
  }

  if ( ioctl ( ptrDesc, TIOCEXCL )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( ptrDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't get lock on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    free (( void* ) ptr );
    free (( void* ) privateInfo );
    free (( void* ) commonInfo );
    return 0;
  }

  if ( tcgetattr ( ptrDesc, &tio )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( ptrDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't get termio on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    free (( void* ) ptr );
    free (( void* ) privateInfo );
    free (( void* ) commonInfo );
    return 0;
  }

  tio.c_iflag = IGNBRK | IGNPAR | CS8;
  tio.c_oflag |= CS8;
  tio.c_oflag &= ~( ONLRET | ONOCR );
  tio.c_lflag &= ~ICANON;
  tio.c_cc[VMIN] = 1;
  tio.c_cc[VTIME] = 4;
  tio.c_cflag &= ~PARENB; // no parity
  tio.c_cflag &= ~CSTOPB; // 1 stop bit
  cfsetispeed ( &tio, B38400 );
  cfsetospeed ( &tio, B38400 );
  tcflush ( ptrDesc, TCIFLUSH );

  if ( tcsetattr ( ptrDesc, TCSANOW, &tio )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( ptrDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't set termio on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    free (( void* ) ptr );
    free (( void* ) privateInfo );
    free (( void* ) commonInfo );
    return 0;
  }

  privateInfo->fd = ptrDesc;
  privateInfo->isRunning = 0;
  privateInfo->index = devInfo->devIndex;

  pthread_mutex_init ( &privateInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &privateInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &privateInfo->callbackQueued, 0 );
  pthread_cond_init ( &privateInfo->commandQueued, 0 );
  pthread_cond_init ( &privateInfo->commandComplete, 0 );

  privateInfo->stopControllerThread = privateInfo->stopCallbackThread = 0;
  privateInfo->commandQueue = oaDLListCreate();
  privateInfo->callbackQueue = oaDLListCreate();
  if ( pthread_create ( &( privateInfo->controllerThread ), 0,
      oaPTRcontroller, ( void* ) ptr )) {
    free (( void* ) ptr->_private );
    free (( void* ) commonInfo );
    free (( void* ) ptr );
    oaDLListDelete ( privateInfo->commandQueue, 0 );
    oaDLListDelete ( privateInfo->callbackQueue, 0 );
    return 0;
  }

  if ( pthread_create ( &( privateInfo->callbackThread ), 0,
      oaPTRcallbackHandler, ( void* ) ptr )) {

    void* dummy;
    privateInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &privateInfo->commandQueued );
    pthread_join ( privateInfo->controllerThread, &dummy );
    free (( void* ) ptr->_private );
    free (( void* ) commonInfo );
    free (( void* ) ptr );
    oaDLListDelete ( privateInfo->commandQueue, 0 );
    oaDLListDelete ( privateInfo->callbackQueue, 0 );
    return 0;
  }

  _oaInitPTRFunctionPointers ( ptr );

  ptr->controls [ OA_TIMER_CTRL_SYNC ] = OA_CTRL_TYPE_BUTTON;
  ptr->controls [ OA_TIMER_CTRL_NMEA ] = OA_CTRL_TYPE_BUTTON;
  ptr->controls [ OA_TIMER_CTRL_STATUS ] = OA_CTRL_TYPE_BUTTON;

  ptr->controls [ OA_TIMER_CTRL_COUNT ] = OA_CTRL_TYPE_INT32;
  commonInfo->min [ OA_TIMER_CTRL_COUNT ] = 1;
  commonInfo->max [ OA_TIMER_CTRL_COUNT ] = 999999;
  commonInfo->step [ OA_TIMER_CTRL_COUNT ] = 1;
  commonInfo->def [ OA_TIMER_CTRL_COUNT ] = 1;
  privateInfo->requestedCount = 1;
  
  ptr->controls [ OA_TIMER_CTRL_INTERVAL ] = OA_CTRL_TYPE_INT32;
  commonInfo->min [ OA_TIMER_CTRL_INTERVAL ] = 1;
  commonInfo->max [ OA_TIMER_CTRL_INTERVAL ] = 999999;
  commonInfo->step [ OA_TIMER_CTRL_INTERVAL ] = 1;
  commonInfo->def [ OA_TIMER_CTRL_INTERVAL ] = 1;
  privateInfo->requestedInterval = 1;

  ptr->controls [ OA_TIMER_CTRL_MODE ] = OA_CTRL_TYPE_MENU;
  commonInfo->min [ OA_TIMER_CTRL_MODE ] = OA_TIMER_MODE_TRIGGER;
  commonInfo->max [ OA_TIMER_CTRL_MODE ] = OA_TIMER_MODE_STROBE;
  commonInfo->step [ OA_TIMER_CTRL_MODE ] = 1;
  commonInfo->def [ OA_TIMER_CTRL_MODE ] = OA_TIMER_MODE_TRIGGER;
  privateInfo->requestedMode = OA_TIMER_MODE_TRIGGER;

  _ptrInitFunctionPointers ( ptr );

  return ptr;
}


static void
_ptrInitFunctionPointers ( oaPTR* ptr )
{
  ptr->funcs.init = oaPTRInit;
  ptr->funcs.close = oaPTRClose;
  ptr->funcs.reset = oaPTRReset;
  ptr->funcs.start = oaPTRStart;
  ptr->funcs.stop = oaPTRStop;
  ptr->funcs.isRunning = oaPTRIsRunning;
  ptr->funcs.readControl = oaPTRReadControl;
  ptr->funcs.setControl = oaPTRSetControl;
  ptr->funcs.readTimestamp = oaPTRGetTimestamp;
fprintf ( stderr, "%s: need control range function\n", __FUNCTION__ );
}


int
oaPTRClose ( oaPTR* device )
{
  PRIVATE_INFO*		privateInfo;
  void*			dummy;

  if ( device ) {
    privateInfo = device->_private;
    privateInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &privateInfo->commandQueued );
    pthread_join ( privateInfo->controllerThread, &dummy );
    privateInfo->initialised = 0;

    close ( privateInfo->fd );
    return OA_ERR_NONE;
  }

  return -OA_ERR_INVALID_TIMER;
}
