/*****************************************************************************
 *
 * atikSerialIO-udev.c -- Atik serial camera IO routines (libudev)
 *
 * Copyright 2014,2016 James Fidell (james@openastroproject.org)
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
#include <openastro/camera.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "atikSerialoacam.h"


int
_atikUdevSerialCamWrite ( AtikSerial_STATE* cameraInfo,
    const unsigned char* buffer, int len )
{
  int fd = cameraInfo->fd;

  if ( write ( fd, buffer, len ) != len ) {
    return -OA_ERR_CAMERA_IO;
  }
  return OA_ERR_NONE;
}


int
_atikUdevSerialCamRead ( AtikSerial_STATE* cameraInfo,
    unsigned char* buffer, int maxlen )
{
  int fd = cameraInfo->fd;
  int len = 0;
  char *p = buffer;

  do {
    if ( read ( fd, p, 1 ) != 1 ) {
      if ( len ) {
        return len;
      }
      return -OA_ERR_CAMERA_IO;
    }
    p++;
    len++;
  } while ( len < maxlen );
  return len;
}

int
_atikUdevSerialCamReadToZero ( AtikSerial_STATE* cameraInfo,
    unsigned char* buffer, unsigned int max )
{
  int fd = cameraInfo->fd;
  char *p = buffer;
  int done = 0;
  unsigned int len = 0;

  do {
    if ( read ( fd, p, 1 ) != 1 ) {
      return -OA_ERR_CAMERA_IO;
    }
    if ( !*p++ ) {
      done = 1;
    }
    len++;
  } while ( !done && len < max );

  if ( !done )
    return -1;

  return len;
}


int
_atikUdevSerialCamReadBlock ( AtikSerial_STATE* cameraInfo,
    unsigned char* buffer, int len )
{
  int fd = cameraInfo->fd;
  return ( read ( fd, buffer, len ));
}
