/*****************************************************************************
 *
 * V4L2oacam.c -- main entrypoint for V4L2 Cameras
 *
 * Copyright 2013,2014,2015,2016 James Fidell (james@openastroproject.org)
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

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <linux/limits.h>
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#include <dirent.h>
#include <ctype.h>
#include <libv4l2.h>

#include "unimplemented.h"
#include "oacamprivate.h"
#include "V4L2oacam.h"

/**
 * Cycle through the sys filesystem looking for V4L devices and grab
 * their names
 */

int
oaV4L2GetCameras ( CAMERA_LIST* deviceList, unsigned long featureFlags,
		int flags )
{
  DIR*			dirp;
  struct dirent*	entry;
  char			nameFile[ PATH_MAX ];
  char			sysPath[ PATH_MAX ];
  FILE*			fp;
  char			name[ OA_MAX_NAME_LEN+1 ];
  unsigned int		numFound = 0, index;
  int			ret;
  oaCameraDevice*       dev;
  DEVICE_INFO*		_private;

  if ( access ( SYS_V4L_PATH, X_OK )) {
    return 0;
  }

  if ( 0 == ( dirp = opendir ( SYS_V4L_PATH ))) {
    return -OA_ERR_SYSTEM_ERROR;
  }

  while (( entry = readdir ( dirp ))) {
    if ( !strncmp ( entry->d_name, "video", 5 )) {
      // we need a numeric portion for the index
      if ( !isdigit ( entry->d_name[5] )) {
        closedir ( dirp );
        return -OA_ERR_SYSTEM_ERROR;
      }
      index = atoi ( entry->d_name+5 );
      
      ( void ) snprintf ( sysPath, PATH_MAX-1, "%s/%s", SYS_V4L_PATH,
          entry->d_name );
      ( void ) snprintf ( nameFile, PATH_MAX-1, "%s/name", sysPath );
      if (!( fp = fopen ( nameFile, "r" ))) {
        closedir ( dirp );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if ( !fgets ( name, OA_MAX_NAME_LEN, fp )) {
        closedir ( dirp );
        fclose ( fp );
        return -OA_ERR_SYSTEM_ERROR;
      }
      // remove terminating LF
      fclose ( fp );
      name[ strlen ( name ) - 1] = 0;

      // now we can drop the data into the list
      if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
        closedir ( dirp );
        return -OA_ERR_MEM_ALLOC;
      }
      if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
        ( void ) free (( void* ) dev );
        closedir ( dirp );
        return -OA_ERR_MEM_ALLOC;
      }
      _oaInitCameraDeviceFunctionPointers ( dev );
      dev->interface = OA_CAM_IF_V4L2;
      ( void ) strncpy ( dev->deviceName, name, OA_MAX_NAME_LEN );
      _private->devIndex = index;
      dev->_private = _private;
      dev->initCamera = oaV4L2InitCamera;
      dev->hasLoadableFirmware = 0;
      ( void ) strncpy ( _private->sysPath, sysPath, PATH_MAX );
      if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
        closedir ( dirp );
        ( void ) free (( void* ) dev );
        ( void ) free (( void* ) _private );
        return ret;
      }
      deviceList->cameraList[ deviceList->numCameras++ ] = dev;
      numFound++;
    }
  }
  closedir ( dirp );

  return numFound;
}

#endif /* HAVE_LIBV4L2 */
