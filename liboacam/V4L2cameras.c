/*****************************************************************************
 *
 * V4L2cameras.c -- camera checking functions
 *
 * Copyright 2013,2014,2015 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBV4L2

#include <openastro/camera.h>

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <linux/videodev2.h>
#include <linux/sysctl.h>
#include <libv4l2.h>
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#include <string.h>

#include "V4L2cameras.h"

int
v4l2isSPC900 ( oaCameraDevice* device )
{
  return ( strstr ( device->deviceName, "spc 900" ) ||
      strstr ( device->deviceName, "SPC 900" )) ? 1 : 0;
/*
  The best way to do this might be to walk the udev tree, assuming we're
  on Linux

  char  realSysPath [ PATH_MAX+1 ];
  char  idFile [ PATH_MAX+1 ];
  char  vid[ 20 ];
  char  pid[ 20 ];
  FILE *fp;

  ( void ) realpath ( device->sysPath, realSysPath );
  ( void ) snprintf ( idFile, PATH_MAX, "%s/../../../idVendor",
      realSysPath );
  if ( !access ( idFile, R_OK )) {
    fprintf ( stderr, "Can't read file %s\n", idFile );
    return 0;
  }

  if (!( fp = fopen ( idFile, "r" ))) {
    fprintf ( stderr, "fopen of %s failed\n", idFile );
    return 0;
  }

  if ( !fgets ( vid, 16, fp )) {
    fclose ( fp );
    fprintf ( stderr, "fgets from %s failed\n", idFile );
    return 0;
  }
  fclose ( fp );

  if ( strcmp ( vid, "0471" )) {
fprintf ( stderr, "SPC900 vid test fails\n" );
    return 0;
  }

  ( void ) snprintf ( idFile, PATH_MAX, "%s/../../../idProduct",
      realSysPath );
  if ( !access ( idFile, R_OK )) {
    fprintf ( stderr, "Can't read file %s\n", idFile );
    return 0;
  }

  if (!( fp = fopen ( idFile, "r" ))) {
    fprintf ( stderr, "fopen of %s failed\n", idFile );
    return 0;
  }

  if ( !fgets ( pid, 16, fp )) {
    fclose ( fp );
    fprintf ( stderr, "fgets from %s failed\n", idFile );
    return 0;
  }
  fclose ( fp );

  if ( strcmp ( vid, "0329" )) {
fprintf ( stderr, "SPC900 pid test fails\n" );
    return 0;
  }

fprintf ( stderr, "SPC900 connected\n" );
  return -1;
*/
}

#endif /* HAVE_LIBV4L2 */
