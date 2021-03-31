/*****************************************************************************
 *
 * V4L2oacam.c -- main entrypoint for V4L2 Cameras
 *
 * Copyright 2013,2014,2015,2016,2019,2020,2021
 *   James Fidell (james@openastroproject.org)
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
#include <openastro/util.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <linux/limits.h>
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <dirent.h>
#include <ctype.h>
#include <linux/videodev2.h>
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
  DIR*											dirp;
  struct dirent*						entry;
  char											nameFile[ PATH_MAX+1 ];
  char											sysPath[ PATH_MAX+1 ];
  char											devicePath[ PATH_MAX+1 ];
  FILE*											fp;
  char											name[ OA_MAX_NAME_LEN+1 ];
  unsigned int							numFound = 0, index;
  int												ret;
  int												fd;
  oaCameraDevice*					  dev;
  DEVICE_INFO*							_private;
	struct v4l2_capability		cap;

	oaLogInfo ( OA_LOG_CAMERA, "%s ( %p, %ld, %d ): entered", __func__,
			deviceList, featureFlags, flags );

  if ( access ( SYS_V4L_PATH, X_OK )) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't access %s", __func__,
				SYS_V4L_PATH );
    return 0;
  }

  if ( 0 == ( dirp = opendir ( SYS_V4L_PATH ))) {
		oaLogError ( OA_LOG_CAMERA, "%s: Can't open %s", __func__,
				SYS_V4L_PATH );
    return -OA_ERR_SYSTEM_ERROR;
  }

  while (( entry = readdir ( dirp ))) {
    if ( !strncmp ( entry->d_name, "video", 5 )) {
      // we need a numeric portion for the index
      if ( !isdigit ( entry->d_name[5] )) {
        closedir ( dirp );
				oaLogError ( OA_LOG_CAMERA, "%s: %s doesn't look like video{\\d+}",
						__func__, entry->d_name );
        return -OA_ERR_SYSTEM_ERROR;
      }
      index = atoi ( entry->d_name+5 );
      
      ( void ) snprintf ( sysPath, PATH_MAX, "%s/%s", SYS_V4L_PATH,
          entry->d_name );
      ( void ) strcpy ( nameFile, sysPath );
      ( void ) strncat ( nameFile, "/name", PATH_MAX );
      if (!( fp = fopen ( nameFile, "r" ))) {
        closedir ( dirp );
				oaLogError ( OA_LOG_CAMERA, "%s: failed to open %s", __func__,
						nameFile );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if ( !fgets ( name, OA_MAX_NAME_LEN, fp )) {
        closedir ( dirp );
        fclose ( fp );
				oaLogError ( OA_LOG_CAMERA, "%s: failed to read %s", __func__,
						nameFile );
        return -OA_ERR_SYSTEM_ERROR;
      }
      fclose ( fp );
      // remove terminating LF
      name[ strlen ( name ) - 1] = 0;

			// path name for device is /dev/video<index>
			( void ) snprintf ( devicePath, PATH_MAX, "/dev/video%d", index );
			if (( fd = v4l2_open ( devicePath, O_RDWR | O_NONBLOCK, 0 )) < 0 ) {
				oaLogError ( OA_LOG_CAMERA, "%s: v4l2_open failed on %s", __func__,
						devicePath );
				// carry on through the list of device we've found
				continue;
			}

			// Now get the capabilites and make sure this is a capture device

			OA_CLEAR ( cap );
			if ( -1 == v4l2_ioctl ( fd, VIDIOC_QUERYCAP, &cap )) {
				if ( EINVAL == errno ) {
					oaLogWarning ( OA_LOG_CAMERA, "%s: %s is not a V4L2 device",
							__func__, devicePath );
				} else {
					oaLogWarning ( OA_LOG_CAMERA, "%s: VIDIOC_QUERYCAP failed on %s",
							__func__, devicePath );
				}
				v4l2_close ( fd );
				continue;
			}
			v4l2_close ( fd );

			if (!( cap.device_caps & V4L2_CAP_VIDEO_CAPTURE )) {
				continue;
			}

			if (!( cap.device_caps & V4L2_CAP_STREAMING )) {
				continue;
			}

			// Very ugly, but I've not been able to find a more appropriate way
			// of ignoring these devices yet
			if (!strncmp ( "bcm2835-isp-", name, 12 )) {
				continue;
			}

      // now we can drop the data into the list
      if (!( dev = malloc ( sizeof ( oaCameraDevice )))) {
        closedir ( dirp );
				oaLogError ( OA_LOG_CAMERA,
						"%s: Failed to allocate memory for oaCameraDevice", __func__ );
        return -OA_ERR_MEM_ALLOC;
      }
      if (!( _private = malloc ( sizeof ( DEVICE_INFO )))) {
				oaLogError ( OA_LOG_CAMERA,
						"%s: Failed to allocate memory for DEVICE_INFO", __func__ );
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
      ( void ) strncpy ( _private->sysPath, sysPath, PATH_MAX );
      if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
	      oaLogError ( OA_LOG_CAMERA, "%s: _oaCheckCameraArraySize() failed",
						__func__ );
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

	oaLogInfo ( OA_LOG_CAMERA, "%s: exiting.  Found %d cameras", __func__,
			numFound );

  return numFound;
}

#endif /* HAVE_LIBV4L2 */
