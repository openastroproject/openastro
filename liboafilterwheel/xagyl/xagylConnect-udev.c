/*****************************************************************************
 *
 * xagylInit-udev.c -- Initialise Xagyl filter wheels (udev)
 *
 * Copyright 2014,2015,2017,2018 James Fidell (james@openastroproject.org)
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
#include <hidapi.h>
#include <pthread.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "unimplemented.h"
#include "xagylfw.h"


static void _xagylInitFunctionPointers ( oaFilterWheel* );
static int  _getNumSlots ( oaFilterWheel* );

/**
 * Initialise a given filter wheel device
 */

oaFilterWheel*
oaXagylInitFilterWheel ( oaFilterWheelDevice* device )
{
  oaFilterWheel*	wheel;
  int			fwDesc;
  struct termios	tio;
  DEVICE_INFO*		devInfo;
  PRIVATE_INFO*		privateInfo;

  devInfo = device->_private;

  if (!( wheel = ( oaFilterWheel* ) malloc ( sizeof ( oaFilterWheel )))) {
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }
  if (!( privateInfo = ( PRIVATE_INFO* ) malloc ( sizeof ( PRIVATE_INFO )))) {
    free (( void* ) wheel );
    perror ( "malloc oaFilterWheel failed" );
    return 0;
  }

  OA_CLEAR ( *wheel );
  OA_CLEAR ( *privateInfo );
  // OA_CLEAR ( wheel->features );
  OA_CLEAR ( wheel->controls );

  wheel->_private = privateInfo;

  pthread_mutex_init ( &privateInfo->ioMutex, 0 );

  ( void ) strcpy ( wheel->deviceName, device->deviceName );
  ( void ) strcpy ( privateInfo->devicePath, devInfo->sysPath );

  privateInfo->initialised = 0;
  privateInfo->index = -1;

  if (( fwDesc = open ( devInfo->sysPath, O_RDWR | O_NOCTTY )) < 0 ) {
    fprintf ( stderr, "Can't open %s read-write, errno = %d\n",
        devInfo->sysPath, errno );
    free (( void* ) wheel );
    free (( void* ) privateInfo );
    return 0;
  }

  if ( ioctl ( fwDesc, TIOCEXCL )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( fwDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't get lock on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    free (( void* ) wheel );
    free (( void* ) privateInfo );
    return 0;
  }

  if ( tcgetattr ( fwDesc, &tio )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( fwDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't get termio on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    free (( void* ) wheel );
    free (( void* ) privateInfo );
    return 0;
  }

  tio.c_iflag = IXON | IGNPAR | BRKINT;
  tio.c_oflag = 0;
  tio.c_cflag = CLOCAL | CREAD | CS8 | B9600;
  tio.c_lflag = IEXTEN | ECHOKE | ECHOCTL | ECHOK | ECHOE;

  if ( tcsetattr ( fwDesc, TCSAFLUSH, &tio )) {
    int errnoCopy = errno;
    errno = 0;
    while (( close ( fwDesc ) < 0 ) && EINTR == errno );
    fprintf ( stderr, "%s: can't set termio on %s, errno = %d\n", __FUNCTION__,
        devInfo->sysPath, errnoCopy );
    free (( void* ) wheel );
    free (( void* ) privateInfo );
    return 0;
  }
  tcflush ( fwDesc, TCIFLUSH );

  wheel->interface = device->interface;
  privateInfo->fd = fwDesc;
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
      oafwXagylcontroller, ( void* ) wheel )) {
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

  oaXagylWheelWarmReset ( privateInfo, 0 );

  if (( wheel->numSlots = _getNumSlots ( wheel )) < 1 ) {
    fprintf ( stderr, "%s: invalid number of slots in filter wheel %s\n",
        __FUNCTION__, devInfo->sysPath );
    free (( void* ) wheel );
    free (( void* ) privateInfo );
    return 0;
  }

  _oaInitFilterWheelFunctionPointers ( wheel );
  _xagylInitFunctionPointers ( wheel );

  wheel->controls [ OA_FW_CTRL_MOVE_ABSOLUTE_ASYNC ] = OA_CTRL_TYPE_INT32;
  wheel->controls [ OA_FW_CTRL_SPEED ] = OA_CTRL_TYPE_INT32;
  privateInfo->currentSpeed = XAGYL_DEFAULT_SPEED;
  wheel->controls [ OA_FW_CTRL_WARM_RESET ] = OA_CTRL_TYPE_BUTTON;
  wheel->controls [ OA_FW_CTRL_COLD_RESET ] = OA_CTRL_TYPE_BUTTON;

  return wheel;
}


static void
_xagylInitFunctionPointers ( oaFilterWheel* wheel )
{
  wheel->funcs.initWheel = oaXagylInitFilterWheel;
  wheel->funcs.closeWheel = oaXagylWheelClose;
  wheel->funcs.readControl = oaWheelReadControl;
  wheel->funcs.setControl = oaWheelSetControl;
  // wheel->funcs.testControl = XXXX;
}


int
_getNumSlots ( oaFilterWheel* wheel )
{
  PRIVATE_INFO*		privateInfo = wheel->_private;
  char			buffer[50];
  int			numRead, numSlots;

  pthread_mutex_lock ( &privateInfo->ioMutex );

  tcflush ( privateInfo->fd, TCIFLUSH );

  if ( _xagylWheelWrite ( privateInfo->fd, "i8", 2 )) {
    fprintf ( stderr, "%s: write error on i8 command\n", __FUNCTION__ );
    return 0;
  }

  numRead = _xagylWheelRead ( privateInfo->fd, buffer, 50 );
  pthread_mutex_unlock ( &privateInfo->ioMutex );

  if ( numRead > 0 ) {
    if ( strncmp ( buffer, "FilterSlots ", 12 )) {
      fprintf ( stderr, "%s: failed to match expecting string 'FilterSlots '"
           ", got '%40s'\n", __FUNCTION__, buffer );
      return 0;
    }
    if ( sscanf ( buffer, "FilterSlots %d", &numSlots ) != 1 ) {
      fprintf ( stderr, "%s: Failed to match number of slots in '%s'\n",
          __FUNCTION__, buffer );
      return 0;
    }
    return numSlots;
  }

  fprintf ( stderr, "%s: no data read from wheel interface\n",
      __FUNCTION__ );
  return 0;
}


int
oaXagylWheelClose ( oaFilterWheel* wheel )
{
  PRIVATE_INFO*		privateInfo = wheel->_private;

  privateInfo->initialised = 0;
  return close ( privateInfo->fd );
}
