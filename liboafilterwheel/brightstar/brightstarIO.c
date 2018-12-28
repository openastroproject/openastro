/*****************************************************************************
 *
 * brightstarIO.c -- Brightstar filter wheel IO routines
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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <hidapi.h>
#include <pthread.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "brightstarfw.h"

#define	wheelState		wheel->_brightstar


int
_brightstarWheelWrite ( int fd, const char* buffer, int len )
{
  if ( write ( fd, buffer, len ) != len ) {
    return -1;
  }
  return 0;
}


int
_brightstarWheelRead ( int fd, char* buffer, int maxlen )
{
  char *p = buffer;
  int done = 0, len = 1;

  do {
    if ( read ( fd, p, 1 ) != 1 ) {
      return -1;
    }
    if ( *p != '\r' ) {
      if ( *p++ == '\n' ) {
        done = 1;
      }
      len++;
    }
  } while ( !done && len < maxlen );
  *p = 0;

  return len - 1;
}


int
oaBrightstarMoveTo ( PRIVATE_INFO* wheelInfo, int slot, int nodelay )
{
  pthread_mutex_lock ( &wheelInfo->ioMutex );

  if ( nodelay ) {
    fprintf ( stderr, "%s: ignoring nodelay\n", __FUNCTION__ );
  }

  char buffer[50];
  char expected[50];
  int numRead;

  snprintf ( buffer, 6, "G%d\r\n\n", slot );
  snprintf ( expected, 3, "P%d", slot );

  tcflush ( wheelInfo->fd, TCIFLUSH );

  if ( _brightstarWheelWrite ( wheelInfo->fd, buffer, 2 )) {
    fprintf ( stderr, "%s: write error on command '%s'\n", __FUNCTION__,
        buffer );
    pthread_mutex_unlock ( &wheelInfo->ioMutex );
    return -1;
  }

  numRead = _brightstarWheelRead ( wheelInfo->fd, buffer, 50 );
  if ( numRead > 0 ) {
    if ( strncmp ( buffer, expected, 2 )) {
      fprintf ( stderr, "%s: '%s' failed to match expecting string '%s'\n",
          __FUNCTION__, buffer, expected );
      pthread_mutex_unlock ( &wheelInfo->ioMutex );
      return -1;
    }
  } else {
    fprintf ( stderr, "%s: no data read from wheel interface\n",
        __FUNCTION__ );
    pthread_mutex_unlock ( &wheelInfo->ioMutex );
    return -1;
  }

  pthread_mutex_unlock ( &wheelInfo->ioMutex );

  return 0;
}
