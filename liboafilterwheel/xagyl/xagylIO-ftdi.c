/*****************************************************************************
 *
 * xagylIO-ftdi.c -- Xagyl filter wheel IO routines (libftdi)
 *
 * Copyright 2014,2015,2017 James Fidell (james@openastroproject.org)
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
#include <hidapi.h>
#include <libftdi1/ftdi.h>

#include <openastro/util.h>
#include <openastro/filterwheel.h>

#include "oafwprivate.h"
#include "xagylfw.h"


int
_xagylWheelWrite ( struct ftdi_context* ftdiContext, const char* buffer,
    int len )
{
  if ( ftdi_write_data ( ftdiContext, (const unsigned char*) buffer,
      len ) != len ) {
    return -1;
  }
  return 0;
}


int
_xagylWheelRead ( struct ftdi_context* ftdiContext, char* buffer, int maxlen )
{
  unsigned char *p = (unsigned char*) buffer;
  int done = 0, len = 1, tries, bytes;

  // libftdi1 doesn't have a blocking read, so for the time being we
  // loop for up to ten seconds (actually 100 tries) attempting to read
  // from the target device with a 100ms delay between each attempt.  Any
  // valid read resets the loop timer.
  // It's a bit scabby, but it appears to do the job for the time being.
  //
  // FIX ME
  // Should really replace this with select() or similar
  do {
    tries = 0;
    do {
      if (!( bytes = ftdi_read_data ( ftdiContext, p, 1 ))) {
        tries++;
        usleep ( 100000 );
      }
    } while ( !bytes && tries < 100 );
    if ( bytes != 1 ) {
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
oaXagylGetWheelSpeed ( PRIVATE_INFO* wheelInfo, unsigned int* speed )
{
  pthread_mutex_lock ( &wheelInfo->ioMutex );

  char buffer[50];
  int numRead;

  if ( _xagylWheelWrite ( wheelInfo->ftdiContext, "i4", 2 )) {
    fprintf ( stderr, "%s: write error on command '%s'\n", __FUNCTION__,
        buffer );
    pthread_mutex_unlock ( &wheelInfo->ioMutex );
    return -1;
  }

  numRead = _xagylWheelRead ( wheelInfo->ftdiContext, buffer, 50 );
  if ( numRead > 0 ) {
    if ( strncmp ( buffer, "MaxSpeed ", 9 )) {
      fprintf ( stderr, "%s: '%s' failed to match expected string\n",
          __FUNCTION__, buffer );
      pthread_mutex_unlock ( &wheelInfo->ioMutex );
      return -1;
    }
    if ( sscanf ( buffer, "MaxSpeed %d%%\n", speed ) != 1 ) {
      buffer[ numRead ] = 0;
      fprintf ( stderr, "%s: wrong number of items(1) scanned from '%s'\n",
          __FUNCTION__, buffer );
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


int
oaXagylSetWheelSpeed ( PRIVATE_INFO* wheelInfo, unsigned int speed,
    int nodelay )
{
  // FIX ME -- make this actually work
  return 0;
}


int
oaXagylMoveTo ( PRIVATE_INFO* wheelInfo, int slot, int nodelay )
{
  pthread_mutex_lock ( &wheelInfo->ioMutex );

  if ( nodelay ) {
    fprintf ( stderr, "%s: ignoring nodelay\n", __FUNCTION__ );
  }

  char buffer[50];
  char expected[50];
  int numRead;

  snprintf ( buffer, 3, "g%d", slot );
  snprintf ( expected, 3, "P%d", slot );

  if ( _xagylWheelWrite ( wheelInfo->ftdiContext, buffer, 2 )) {
    fprintf ( stderr, "%s: write error on command '%s'\n", __FUNCTION__,
        buffer );
    pthread_mutex_unlock ( &wheelInfo->ioMutex );
    return -1;
  }

  numRead = _xagylWheelRead ( wheelInfo->ftdiContext, buffer, 50 );
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


int
oaXagylWheelDoReset ( PRIVATE_INFO* wheelInfo, const char* cmd, int nodelay )
{
  /*
   * After sending "r0" or "r1" here there appears to be some variation
   * in output.
   * eg.
   *
   * Xagyl FW5125V2
   * FW 2.1.7
   * Initializing
   * Calibrating
   * P1
   *
   * Xagyl FW5125V1
   * FW 2.2.0
   * Initializing
   * Calibrating
   * RightCal -2
   * Found Sensor
   * Found Null
   * Found Midrange 514
   * Done
   * P1
   *
   * Restart
   * FW5125V2
   * FW 3.3.1
   * Initializing
   * P1
   *
   * I think it makes sense just to wait for "P1\n" to be returned by the
   * wheel to decide when the reset is complete
   */

  pthread_mutex_lock ( &wheelInfo->ioMutex );

  if ( nodelay ) {
    fprintf ( stderr, "%s: reset ignoring nodelay\n", __FUNCTION__ );
  }

  const char* expectedWords[] = { "Restart", "Xagyl", "FW", "Initializing",
      "Calibrating", "RightCal", "Found", "\n", 0
  };
  char buffer[50];
  int done = 0, error = 0, ret = 0, numRead;

  if ( _xagylWheelWrite ( wheelInfo->ftdiContext, cmd, 2 )) {
    fprintf ( stderr, "%s: write error on reset '%s' command\n", __FUNCTION__,
        cmd );
    pthread_mutex_unlock ( &wheelInfo->ioMutex );
    return -1;
  }

  do {
    if (( numRead = _xagylWheelRead ( wheelInfo->ftdiContext, buffer,
        50 )) < 0 ) {
      fprintf ( stderr, "%s: no data read from wheel interface\n",
          __FUNCTION__ );
      pthread_mutex_unlock ( &wheelInfo->ioMutex );
      return -1;
    }

    if ( numRead >= 5 && !strncmp ( buffer, "ERROR", 5 )) {
      done = error = 1;
    } else {
      if ( numRead == 3 && !strncmp ( buffer, "P1\n", 3 )) {
        done = 1;
      } else {
        int i = 0, found = 0;
        while ( expectedWords[i] && !found ) {
          if ( !strncmp ( buffer, expectedWords[i],
              strlen ( expectedWords[i] ))) {
            found = 1;
          }
          i++;
        }
        if ( !found ) {
          buffer [ numRead ] = 0;
          fprintf ( stderr, "%s: unexpected string '%s' (len %d) read\n",
              __FUNCTION__, buffer, numRead );
        }
      }
    }
  } while ( !done );

  if ( error ) {
    buffer[ numRead ] = 0;
    fprintf ( stderr, "%s: wheel returned error '%s'\n", __FUNCTION__,
        buffer );
    ret = -1;
  }

  pthread_mutex_unlock ( &wheelInfo->ioMutex );

  return ret;
}
