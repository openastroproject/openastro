/*****************************************************************************
 *
 * V4L2connect.c -- Initialise V4L2 cameras
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

#if HAVE_LIMITS_H
#include <limits.h>
#endif
#include <dirent.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <libv4l2.h>
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/sysctl.h>
#include <linux/limits.h>
#include <pthread.h>

#include <openastro/camera.h>
#include <openastro/util.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "V4L2.h"
#include "V4L2oacam.h"
#include "V4L2state.h"
#include "V4L2ioctl.h"
#include "V4L2camclass.h"
#include "V4L2cameras.h"


static void _V4L2InitFunctionPointers ( oaCamera* );


/**
 * Initialise a given camera device
 */

oaCamera*
oaV4L2InitCamera ( oaCameraDevice* device )
{
  oaCamera*			camera;
  struct v4l2_queryctrl 	ctrl;
  struct v4l2_capability	cap;
  struct v4l2_fmtdesc		formatDesc;
  struct v4l2_format		format;
  struct v4l2_frmsizeenum	fsize;
  struct v4l2_streamparm        parm;
  DEVICE_INFO*			devInfo;
  V4L2_STATE*			cameraInfo;
  COMMON_INFO*			commonInfo;
  int                   	j;
  uint32_t                 	id;


  if (!( camera = ( oaCamera* ) malloc ( sizeof ( oaCamera )))) {
    perror ( "malloc oaCamera failed" );
    return 0;
  }
  if (!( cameraInfo = ( V4L2_STATE* ) malloc ( sizeof ( V4L2_STATE )))) {
    free ( camera );
    perror ( "malloc V4L2_STATE failed" );
    return 0;
  }
  if (!( commonInfo = ( COMMON_INFO* ) malloc ( sizeof ( COMMON_INFO )))) {
    free ( cameraInfo );
    free ( camera );
    perror ( "malloc V4L2_STATE failed" );
    return 0;
  }
  OA_CLEAR ( *cameraInfo );
  OA_CLEAR ( *commonInfo );
  camera->_private = cameraInfo;
  camera->_common = commonInfo;

  camera->interface = device->interface;
  ( void ) strcpy ( camera->deviceName, device->deviceName );
  cameraInfo->initialised = 0;
  cameraInfo->fd = -1;
  cameraInfo->exposureMode = DEF_EXPOSURE_MODE;
  devInfo = device->_private;

  cameraInfo->colourDxK = 0;
  // FIX ME -- find out how to get the USB IDs for these cameras so that
  // can be used to identify them
  // I don't have all these cameras to test, but I'm guessing for the time
  // being that all the D[FB]K [34]1 cameras behave the same as the 21 models
  // The older DFK21 is actually labelled "DFx 21", so we allow that for all
  // the other models too just in case that's what they use
  if ( !strncmp ( camera->deviceName, "DFK 21", 6 ) ||
      !strncmp ( camera->deviceName, "DBK 21", 6 ) ||
      !strncmp ( camera->deviceName, "DFK 31", 6 ) ||
      !strncmp ( camera->deviceName, "DBK 31", 6 ) ||
      !strncmp ( camera->deviceName, "DFK 41", 6 ) ||
      !strncmp ( camera->deviceName, "DBK 41", 6 ) ||
      !strncmp ( camera->deviceName, "DFx 21", 6 ) ||
      !strncmp ( camera->deviceName, "DBx 21", 6 ) ||
      !strncmp ( camera->deviceName, "DFx 31", 6 ) ||
      !strncmp ( camera->deviceName, "DBx 31", 6 ) ||
      !strncmp ( camera->deviceName, "DFx 41", 6 ) ||
      !strncmp ( camera->deviceName, "DBx 41", 6 )) {
    cameraInfo->colourDxK = 1;
  }
  if ( v4l2isSPC900 ( device )) {
    cameraInfo->isSPC900 = 1;
  }

  // path name for device is /dev/video<device._devIndex>
  ( void ) snprintf ( cameraInfo->devicePath, PATH_MAX, "/dev/video%ld",
      devInfo->devIndex );
  if (( cameraInfo->fd = v4l2_open ( cameraInfo->devicePath,
      O_RDWR | O_NONBLOCK, 0 )) < 0 ) {
    fprintf ( stderr, "oaV4L2InitCamera: cannot open video device %s\n",
        cameraInfo->devicePath );
    v4l2_close ( cameraInfo->fd );
    free (( void * ) commonInfo );
    free (( void * ) cameraInfo );
    free (( void * ) camera );
    return 0;
  }

  // Now we can get the capabilites and make sure this is a capture device

  OA_CLEAR ( cap );
  if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_QUERYCAP, &cap )) {
    if ( EINVAL == errno ) {
      fprintf ( stderr, "%s is not a V4L2 device\n", camera->deviceName );
    } else {
      perror ( "VIDIOC_QUERYCAP" );
    }
    v4l2_close ( cameraInfo->fd );
    free (( void * ) commonInfo );
    free (( void * ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  if (!( cap.capabilities & V4L2_CAP_VIDEO_CAPTURE )) {
    fprintf ( stderr, "%s does not support video capture",
      camera->deviceName );
    v4l2_close ( cameraInfo->fd );
    free (( void * ) commonInfo );
    free (( void * ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  // FIX ME -- check for streaming?  V4L2_CAP_STREAMING

  // And now what controls the device supports

  // the "get next" ioctl doesn't seem to work reliably sometimes, so do
  // this the hard way

  // FIX ME -- use tables for these to avoid having so many cases in the
  // switch

  OA_CLEAR ( camera->controls );
  OA_CLEAR ( camera->features );
  _oaInitCameraFunctionPointers ( camera );
  _V4L2InitFunctionPointers ( camera );

  pthread_mutex_init ( &cameraInfo->commandQueueMutex, 0 );
  pthread_mutex_init ( &cameraInfo->callbackQueueMutex, 0 );
  pthread_cond_init ( &cameraInfo->callbackQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandQueued, 0 );
  pthread_cond_init ( &cameraInfo->commandComplete, 0 );
  cameraInfo->isStreaming = 0;

  for ( id = V4L2_CID_USER_BASE; id < V4L2_CID_LASTP1; id++ ) {
    OA_CLEAR ( ctrl );
    ctrl.id = id;

    if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_QUERYCTRL, &ctrl )) {
      // EINVAL means we don't have this one
      if ( EINVAL != errno ) {
        fprintf ( stderr, "VIDIOC_QUERYCTRL( %x ) failed, errno %d\n", id,
            errno );
        continue;
      }
    }

    // returning 0 as the type here is not helpful
    if ( !ctrl.type ) {
      continue;
    }

    // FIX ME -- it's a bit of a pain to work through these one at a
    // time, but without doing so we could end up with a big mess with
    // the gui controls

    switch ( id ) {

      case V4L2_CID_BRIGHTNESS:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_BRIGHTNESS ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_BRIGHTNESS ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_BRIGHTNESS ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_BRIGHTNESS ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_BRIGHTNESS ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "brightness is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_CONTRAST:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_CONTRAST ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_CONTRAST ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_CONTRAST ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_CONTRAST ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_CONTRAST ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "contrast is not INTEGER (%d)\n", ctrl.type );
          }
        } 
        break;

      case V4L2_CID_SATURATION:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_SATURATION ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_SATURATION ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_SATURATION ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_SATURATION ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_SATURATION ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "saturation is not INTEGER (%d)\n", ctrl.type );
          }
        } 
        break;

      case V4L2_CID_HUE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_HUE ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_HUE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_HUE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_HUE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_HUE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "hue is not INTEGER (%d)\n", ctrl.type );
          }
        } 
        break;

      case V4L2_CID_AUDIO_VOLUME:
      case V4L2_CID_AUDIO_BALANCE:
      case V4L2_CID_AUDIO_BASS:
      case V4L2_CID_AUDIO_TREBLE:
      case V4L2_CID_AUDIO_MUTE:
      case V4L2_CID_AUDIO_LOUDNESS:
      case V4L2_CID_BLACK_LEVEL:
        break;

      case V4L2_CID_AUTO_WHITE_BALANCE:
        cameraInfo->haveWhiteBalanceManual = 0;
        cameraInfo->autoWhiteBalanceOff = 0;
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] =
              OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] = 0;
          commonInfo->max[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] = 1;
          commonInfo->step[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] = 1;
          commonInfo->def[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] =
              ctrl.default_value;
          cameraInfo->haveWhiteBalanceManual = 1;
        } else {

          // Now we're into a world of pain.  Known cameras are special-cased
          // in the setControl etc. code, but otherwise we make a best guess
          // at the auto and manual values here

          int foundManual = 0;
          int manualValue = 0;
          if ( cameraInfo->isSPC900 ) {
            if ( V4L2_CTRL_TYPE_MENU == ctrl.type ) {
              foundManual = 1;
              manualValue = 3;
            } else {
              fprintf ( stderr, "SPC900 AWB control type not handled (%d)\n",
                  ctrl.type );
            }
          } else {
            if ( V4L2_CTRL_TYPE_MENU == ctrl.type ) {
              int m;
              struct v4l2_querymenu menuItem;
              for ( m = ctrl.minimum; m <= ctrl.maximum; m++ ) {
                OA_CLEAR( menuItem );
                menuItem.id = V4L2_CID_AUTO_WHITE_BALANCE;
                menuItem.index = m;
                if ( !v4l2ioctl ( cameraInfo->fd, VIDIOC_QUERYMENU,
                    &menuItem )) {
                  if ( strstr ( menuItem.name, "manual" ) ||
                        strstr ( menuItem.name, "Manual" ) ||
                        strstr ( menuItem.name, "MANUAL")) {
                    foundManual++;
                    manualValue = m;
                  }
                }
              }
            }
          }
          if ( 1 == foundManual ) {
            camera->controls[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] =
                OA_CTRL_TYPE_MENU;
            commonInfo->min[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] = ctrl.minimum;
            commonInfo->max[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] = ctrl.maximum;
            commonInfo->step[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] = ctrl.step;
            commonInfo->def[ OA_CAM_CTRL_AUTO_WHITE_BALANCE ] =
                ctrl.default_value;
            cameraInfo->autoWhiteBalanceOff = manualValue;
            cameraInfo->haveWhiteBalanceManual = 1;
          } else {
            fprintf ( stderr, "AWB control type not handled (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_DO_WHITE_BALANCE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_WHITE_BALANCE ] =
              OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_WHITE_BALANCE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_WHITE_BALANCE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_WHITE_BALANCE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_WHITE_BALANCE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "white balance is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_RED_BALANCE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_RED_BALANCE ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_RED_BALANCE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_RED_BALANCE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_RED_BALANCE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_RED_BALANCE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "red balance is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_BLUE_BALANCE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_BLUE_BALANCE ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_BLUE_BALANCE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_BLUE_BALANCE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_BLUE_BALANCE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_BLUE_BALANCE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "blue balance is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_GAMMA:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_GAMMA ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_GAMMA ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_GAMMA ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_GAMMA ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_GAMMA ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "gamma is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_EXPOSURE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_EXPOSURE ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_EXPOSURE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_EXPOSURE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_EXPOSURE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_EXPOSURE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "exposure is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_AUTOGAIN:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_AUTO_GAIN ] = OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_AUTO_GAIN ] = 0;
          commonInfo->max[ OA_CAM_CTRL_AUTO_GAIN ] = 1;
          commonInfo->step[ OA_CAM_CTRL_AUTO_GAIN ] = 1;
          commonInfo->def[ OA_CAM_CTRL_AUTO_GAIN ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "Auto Gain is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_GAIN:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_GAIN ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_GAIN ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_GAIN ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_GAIN ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_GAIN ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "exposure is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_HFLIP:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_HFLIP ] = OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_HFLIP ] = 0;
          commonInfo->max[ OA_CAM_CTRL_HFLIP ] = 1;
          commonInfo->step[ OA_CAM_CTRL_HFLIP ] = 1;
          commonInfo->def[ OA_CAM_CTRL_HFLIP ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "HFLIP is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_VFLIP:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_VFLIP ] = OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_VFLIP ] = 0;
          commonInfo->max[ OA_CAM_CTRL_VFLIP ] = 1;
          commonInfo->step[ OA_CAM_CTRL_VFLIP ] = 1;
          commonInfo->def[ OA_CAM_CTRL_VFLIP ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "VFLIP is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

#ifdef V4L2_CID_HCENTER
      case V4L2_CID_HCENTER:
#endif
#ifdef V4L2_CID_VCENTER
      case V4L2_CID_VCENTER:
#endif
      case V4L2_CID_POWER_LINE_FREQUENCY:
        break;

      case V4L2_CID_HUE_AUTO:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_HUE_AUTO ] = OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_HUE_AUTO ] = 0;
          commonInfo->max[ OA_CAM_CTRL_HUE_AUTO ] = 1;
          commonInfo->step[ OA_CAM_CTRL_HUE_AUTO ] = 1;
          commonInfo->def[ OA_CAM_CTRL_HUE_AUTO ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "HUE_AUTO is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_WHITE_BALANCE_TEMP ] =
              OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_WHITE_BALANCE_TEMP ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_WHITE_BALANCE_TEMP ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_WHITE_BALANCE_TEMP ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_WHITE_BALANCE_TEMP ] =
              ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "white bal temp is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_SHARPNESS:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_SHARPNESS ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_SHARPNESS ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_SHARPNESS ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_SHARPNESS ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_SHARPNESS ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "sharpness is not INTEGER (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_BACKLIGHT_COMPENSATION:
      case V4L2_CID_CHROMA_AGC:
      case V4L2_CID_COLOR_KILLER:
      case V4L2_CID_COLORFX:
        break;

      case V4L2_CID_AUTOBRIGHTNESS:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_AUTO_BRIGHTNESS ] =
              OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_AUTO_BRIGHTNESS ] = 0;
          commonInfo->max[ OA_CAM_CTRL_AUTO_BRIGHTNESS ] = 1;
          commonInfo->step[ OA_CAM_CTRL_AUTO_BRIGHTNESS ] = 1;
          commonInfo->def[ OA_CAM_CTRL_AUTO_BRIGHTNESS ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "HUE_AUTO is not BOOLEAN (%d)\n", ctrl.type );
          }
        }
        break;

      case V4L2_CID_BAND_STOP_FILTER:
      case V4L2_CID_ROTATE:
      case V4L2_CID_BG_COLOR:
      case V4L2_CID_CHROMA_GAIN:
      case V4L2_CID_ILLUMINATORS_1:
      case V4L2_CID_ILLUMINATORS_2:
      case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE: // FIX ME -- use this one?
      case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:
        break;
#ifdef V4L2_CID_ALPHA_COMPONENT
      case V4L2_CID_ALPHA_COMPONENT:
        break;
#endif
#ifdef V4L2_CID_COLORFX_CBCR
      case V4L2_CID_COLORFX_CBCR:
        break;
#endif
    }
  }

  for ( id = V4L2_CID_CAMERA_CLASS_BASE; id < V4L2_CAMERA_CLASS_LASTP1;
      id++ ) {
    OA_CLEAR ( ctrl );
    ctrl.id = id;

    if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_QUERYCTRL, &ctrl )) {
      if ( EINVAL != errno ) {
        fprintf ( stderr, "VIDIOC_QUERYCTRL( %x ) failed, errno %d\n", id,
            errno );
        continue;
      }
    }

    // returning 0 as the type here is not helpful
    if ( !ctrl.type ) {
      continue;
    }

    // FIX ME -- it's a bit of a pain to work through these one at a
    // time, but without doing so we could end up with a big mess with
    // the gui controls

    switch ( id ) {

      case V4L2_CID_EXPOSURE_AUTO:
        if ( V4L2_CTRL_TYPE_MENU == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_AUTO_EXPOSURE ] = OA_CTRL_TYPE_MENU;
          commonInfo->min[ OA_CAM_CTRL_AUTO_EXPOSURE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_AUTO_EXPOSURE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_AUTO_EXPOSURE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_AUTO_EXPOSURE ] = ctrl.default_value;
        } else {
          if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
            if ( ctrl.minimum != 0 ) {
              fprintf ( stderr, "AUTO_EXPOSURE control type is BOOLEAN, but "
                  "minimum value is %d\n", ctrl.minimum );
            } else {
              camera->controls[ OA_CAM_CTRL_AUTO_EXPOSURE ] =
                  OA_CTRL_TYPE_BOOLEAN;
              commonInfo->min[ OA_CAM_CTRL_AUTO_EXPOSURE ] = ctrl.minimum;
              commonInfo->max[ OA_CAM_CTRL_AUTO_EXPOSURE ] = ctrl.maximum;
              commonInfo->step[ OA_CAM_CTRL_AUTO_EXPOSURE ] = ctrl.step;
              commonInfo->def[ OA_CAM_CTRL_AUTO_EXPOSURE ] = ctrl.default_value;
            }
          } else {
            fprintf ( stderr, "AUTO_EXPOSURE control type is not MENU (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_EXPOSURE_ABSOLUTE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          // convert 100 usec intervals to 1 usec
          // FIX ME -- This leaves us with a problem where the maximum
          // exposure > INT_MAX usec.  Use a temporary hack to work
          // around it
          camera->controls[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] =
              OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = ctrl.minimum * 100;
          long max = INT_MAX / 100;
          if ( ctrl.maximum > max ) {
            ctrl.maximum = max;
          }
          commonInfo->max[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = ctrl.maximum * 100;
          commonInfo->step[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = ctrl.step * 100;
          commonInfo->def[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = ctrl.default_value
              * 100;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "absolute exposure is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_PAN_RELATIVE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_PAN_RELATIVE ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_PAN_RELATIVE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_PAN_RELATIVE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_PAN_RELATIVE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_PAN_RELATIVE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "pan relative is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_TILT_RELATIVE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_TILT_RELATIVE ] =
              OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_TILT_RELATIVE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_TILT_RELATIVE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_TILT_RELATIVE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_TILT_RELATIVE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "tilt relative is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_PAN_RESET:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_PAN_RESET ] = OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_PAN_RESET ] = 0;
          commonInfo->max[ OA_CAM_CTRL_PAN_RESET ] = 1;
          commonInfo->step[ OA_CAM_CTRL_PAN_RESET ] = 1;
          commonInfo->def[ OA_CAM_CTRL_PAN_RESET ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "pan reset is not BOOLEAN (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_TILT_RESET:
        if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_TILT_RESET ] = OA_CTRL_TYPE_BOOLEAN;
          commonInfo->min[ OA_CAM_CTRL_TILT_RESET ] = 0;
          commonInfo->max[ OA_CAM_CTRL_TILT_RESET ] = 1;
          commonInfo->step[ OA_CAM_CTRL_TILT_RESET ] = 1;
          commonInfo->def[ OA_CAM_CTRL_TILT_RESET ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "tilt reset is not BOOLEAN (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_PAN_ABSOLUTE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_PAN_ABSOLUTE ] = OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_PAN_ABSOLUTE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_PAN_ABSOLUTE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_PAN_ABSOLUTE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_PAN_ABSOLUTE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "pan absolute is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_TILT_ABSOLUTE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_TILT_ABSOLUTE ] =
              OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_TILT_ABSOLUTE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_TILT_ABSOLUTE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_TILT_ABSOLUTE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_TILT_ABSOLUTE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "tilt absolute is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_FOCUS_ABSOLUTE:
      case V4L2_CID_FOCUS_RELATIVE:
      case V4L2_CID_FOCUS_AUTO:
        break;

      case V4L2_CID_ZOOM_ABSOLUTE:
        if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
          camera->controls[ OA_CAM_CTRL_ZOOM_ABSOLUTE ] =
              OA_CTRL_TYPE_INT32;
          commonInfo->min[ OA_CAM_CTRL_ZOOM_ABSOLUTE ] = ctrl.minimum;
          commonInfo->max[ OA_CAM_CTRL_ZOOM_ABSOLUTE ] = ctrl.maximum;
          commonInfo->step[ OA_CAM_CTRL_ZOOM_ABSOLUTE ] = ctrl.step;
          commonInfo->def[ OA_CAM_CTRL_ZOOM_ABSOLUTE ] = ctrl.default_value;
        } else {
          if ( ctrl.type ) {
            fprintf ( stderr, "zoom absolute is not INTEGER (%d)\n",
                ctrl.type );
          }
        }
        break;

      case V4L2_CID_ZOOM_CONTINUOUS:
      case V4L2_CID_PRIVACY:
      case V4L2_CID_IRIS_ABSOLUTE:
      case V4L2_CID_IRIS_RELATIVE:
#ifdef V4L2_CID_AUTO_EXPOSURE_BIAS
      case V4L2_CID_AUTO_EXPOSURE_BIAS:
#endif
#ifdef V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE
      case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
#endif
#ifdef V4L2_CID_WIDE_DYNAMIC_RANGE
      case V4L2_CID_WIDE_DYNAMIC_RANGE:
#endif
#ifdef V4L2_CID_IMAGE_STABILIZATION
      case V4L2_CID_IMAGE_STABILIZATION:
#endif
#ifdef V4L2_CID_ISO_SENSITIVITY
      case V4L2_CID_ISO_SENSITIVITY:
#endif
#ifdef V4L2_CID_ISO_SENSITIVITY_AUTO
      case V4L2_CID_ISO_SENSITIVITY_AUTO:
#endif
#ifdef V4L2_CID_EXPOSURE_METERING
      case V4L2_CID_EXPOSURE_METERING:
#endif
#ifdef V4L2_CID_SCENE_MODE
      case V4L2_CID_SCENE_MODE:
#endif
#ifdef V4L2_CID_3A_LOCK
      case V4L2_CID_3A_LOCK:
#endif
#ifdef V4L2_CID_AUTO_FOCUS_START
      case V4L2_CID_AUTO_FOCUS_START:
#endif
#ifdef V4L2_CID_AUTO_FOCUS_STOP
      case V4L2_CID_AUTO_FOCUS_STOP:
#endif
#ifdef V4L2_CID_AUTO_FOCUS_STATUS
      case V4L2_CID_AUTO_FOCUS_STATUS:
#endif
#ifdef V4L2_CID_AUTO_FOCUS_RANGE
      case V4L2_CID_AUTO_FOCUS_RANGE:
#endif
        fprintf ( stderr, "currently unsupported V4L2 control 0x%x\n", id );
        break;
    }
  }

  for ( id = V4L2_CID_PRIVATE_BASE; errno != EINVAL; id++ ) {
    OA_CLEAR ( ctrl );
    ctrl.id = id;

    if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_QUERYCTRL, &ctrl )) {
      if ( EINVAL != errno ) {
        fprintf ( stderr, "VIDIOC_QUERYCTRL( %x ) failed, errno %d\n", id,
            errno );
      }
      continue;
    }

    // returning 0 as the type here is not helpful
    if ( !ctrl.type ) {
      fprintf ( stderr, "query private control 0x%x returns control type 0\n",
          id );
      continue;
    }

    fprintf ( stderr, "This %s camera has a private control of type %d\n",
        camera->deviceName, ctrl.type );
    fprintf ( stderr, "The description is: '%s'\n", ctrl.name );
  }

  // And finally, these little bundles of joy are the private SPC900
  // controls

  if ( cameraInfo->isSPC900 ) {
    for ( id = PWC_CID_CUSTOM(autocontour);
        id <= PWC_CID_CUSTOM(restore_factory); id++ ) {
      OA_CLEAR ( ctrl );
      ctrl.id = id;

      if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_QUERYCTRL, &ctrl )) {
        // EINVAL means we don't have this one
        if ( EINVAL != errno ) {
          fprintf ( stderr, "PWC VIDIOC_QUERYCTRL( %x ) failed, errno %d\n",
              id, errno );
          continue;
        }
      }

      // FIX ME -- it's a bit of a pain to work through these one at a
      // time, but without doing so we could end up with a big mess with
      // the gui controls

      switch ( id ) {

        case PWC_CID_CUSTOM(autocontour):
          if ( V4L2_CTRL_TYPE_BOOLEAN == ctrl.type ) {
            camera->controls[ OA_CAM_CTRL_AUTO_CONTOUR ] =
                OA_CTRL_TYPE_BOOLEAN;
            commonInfo->min[ OA_CAM_CTRL_AUTO_CONTOUR ] = ctrl.minimum;
            commonInfo->max[ OA_CAM_CTRL_AUTO_CONTOUR ] = ctrl.maximum;
            commonInfo->step[ OA_CAM_CTRL_AUTO_CONTOUR ] = ctrl.step;
            commonInfo->def[ OA_CAM_CTRL_AUTO_CONTOUR ] = ctrl.default_value;
          } else {
            if ( ctrl.type ) {
              fprintf ( stderr, "autocontour is not boolean (%d)\n",
                  ctrl.type );
            }
          }
          break;

        case PWC_CID_CUSTOM(contour):
          if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
            camera->controls[ OA_CAM_CTRL_CONTOUR ] = OA_CTRL_TYPE_INT32;
            commonInfo->min[ OA_CAM_CTRL_CONTOUR ] = ctrl.minimum;
            commonInfo->max[ OA_CAM_CTRL_CONTOUR ] = ctrl.maximum;
            commonInfo->step[ OA_CAM_CTRL_CONTOUR ] = ctrl.step;
            commonInfo->def[ OA_CAM_CTRL_CONTOUR ] = ctrl.default_value;
          } else {
            if ( ctrl.type ) {
              fprintf ( stderr, "contour is not INTEGER (%d)\n", ctrl.type );
            }
          }
          break;

        case PWC_CID_CUSTOM(noise_reduction):
          if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
            camera->controls[ OA_CAM_CTRL_NOISE_REDUCTION ] =
                OA_CTRL_TYPE_INT32;
            commonInfo->min[ OA_CAM_CTRL_NOISE_REDUCTION ] = ctrl.minimum;
            commonInfo->max[ OA_CAM_CTRL_NOISE_REDUCTION ] = ctrl.maximum;
            commonInfo->step[ OA_CAM_CTRL_NOISE_REDUCTION ] = ctrl.step;
            commonInfo->def[ OA_CAM_CTRL_NOISE_REDUCTION ] = ctrl.default_value;
          } else {
            if ( ctrl.type ) {
              fprintf ( stderr, "noise reduction is not INTEGER (%d)\n",
                  ctrl.type );
            }
          }
          break;

        case PWC_CID_CUSTOM(awb_speed):
          if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
            camera->controls[ OA_CAM_CTRL_AUTO_WB_SPEED ] =
                OA_CTRL_TYPE_INT32;
            commonInfo->min[ OA_CAM_CTRL_AUTO_WB_SPEED ] = ctrl.minimum;
            commonInfo->max[ OA_CAM_CTRL_AUTO_WB_SPEED ] = ctrl.maximum;
            commonInfo->step[ OA_CAM_CTRL_AUTO_WB_SPEED ] = ctrl.step;
            commonInfo->def[ OA_CAM_CTRL_AUTO_WB_SPEED ] = ctrl.default_value;
          } else {
            if ( ctrl.type ) {
              fprintf ( stderr, "awb speed is not INTEGER (%d)\n", ctrl.type );
            }
          }
          break;

        case PWC_CID_CUSTOM(awb_delay):
          if ( V4L2_CTRL_TYPE_INTEGER == ctrl.type ) {
            camera->controls[ OA_CAM_CTRL_AUTO_WB_DELAY ] =
                OA_CTRL_TYPE_INT32;
            commonInfo->min[ OA_CAM_CTRL_AUTO_WB_DELAY ] = ctrl.minimum;
            commonInfo->max[ OA_CAM_CTRL_AUTO_WB_DELAY ] = ctrl.maximum;
            commonInfo->step[ OA_CAM_CTRL_AUTO_WB_DELAY ] = ctrl.step;
            commonInfo->def[ OA_CAM_CTRL_AUTO_WB_DELAY ] = ctrl.default_value;
          } else {
            if ( ctrl.type ) {
              fprintf ( stderr, "awb delay is not INTEGER (%d)\n", ctrl.type );
            }
          }
          break;

        case PWC_CID_CUSTOM(save_user):
          if ( V4L2_CTRL_TYPE_BUTTON == ctrl.type ) {
            camera->controls[ OA_CAM_CTRL_SAVE_USER ] = OA_CTRL_TYPE_BUTTON;
            commonInfo->min[ OA_CAM_CTRL_SAVE_USER ] = 1;
            commonInfo->max[ OA_CAM_CTRL_SAVE_USER ] = 1;
            commonInfo->step[ OA_CAM_CTRL_SAVE_USER ] = 1;
            commonInfo->def[ OA_CAM_CTRL_SAVE_USER ] = 1;
          } else {
            if ( ctrl.type ) {
              fprintf ( stderr, "save user is not button (%d)\n",
                  ctrl.type );
            }
          }
          break;

        case PWC_CID_CUSTOM(restore_user):
          if ( V4L2_CTRL_TYPE_BUTTON == ctrl.type ) {
            camera->controls[ OA_CAM_CTRL_RESTORE_USER ] =
                OA_CTRL_TYPE_BUTTON;
            commonInfo->min[ OA_CAM_CTRL_RESTORE_USER ] = 1;
            commonInfo->max[ OA_CAM_CTRL_RESTORE_USER ] = 1;
            commonInfo->step[ OA_CAM_CTRL_RESTORE_USER ] = 1;
            commonInfo->def[ OA_CAM_CTRL_RESTORE_USER ] = 1;
          } else {
            if ( ctrl.type ) {
              fprintf ( stderr, "restore user is not button (%d)\n",
                  ctrl.type );
            }
          }
          break;

        case PWC_CID_CUSTOM(restore_factory):
          if ( V4L2_CTRL_TYPE_BUTTON == ctrl.type ) {
            camera->controls[ OA_CAM_CTRL_RESTORE_FACTORY ] =
                OA_CTRL_TYPE_BUTTON;
            commonInfo->min[ OA_CAM_CTRL_RESTORE_FACTORY ] = 1;
            commonInfo->max[ OA_CAM_CTRL_RESTORE_FACTORY ] = 1;
            commonInfo->step[ OA_CAM_CTRL_RESTORE_FACTORY ] = 1;
            commonInfo->def[ OA_CAM_CTRL_RESTORE_FACTORY ] = 1;
          } else {
            if ( ctrl.type ) {
              fprintf ( stderr, "restore factory is not button (%d)\n",
                  ctrl.type );
            }
          }
          break;
      }
    }
  }

  // Ok, now we need to find out what frame formats are supported and
  // which one we want to use

  cameraInfo->videoRGB24 = cameraInfo->videoYUYV = 0;
  cameraInfo->videoGrey16 = cameraInfo->videoYUV420 = 0;
  cameraInfo->videoGrey = 0;
  cameraInfo->mosaicFormat = 0;
  cameraInfo->videoCurrent = 0;

  camera->features.rawMode = camera->features.demosaicMode = 0;
  camera->features.hasReset = 1;
  if ( cameraInfo->isSPC900 ) {
    camera->features.pixelSizeX = 5600;
    camera->features.pixelSizeY = 5600;
  }

  int bestGreyscale = 0;
  int haveNonEmulatedColour = 0;
  int preferredFound = 0;
  for ( id = 0;; id++ ) {
    OA_CLEAR ( formatDesc );
    formatDesc.index = id;
    formatDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_ENUM_FMT, &formatDesc )) {
      if ( EINVAL != errno) {
        perror("VIDIOC_ENUM_FORMAT");
        continue;
      }
      break;
    }

    // The DxK[234]1 cameras have to be handled separately here as they
    // don't actually appear to allow all the format options to be set that
    // they claim to support and lie about the CFA layout (it is GBRG).
    if ( cameraInfo->colourDxK ) {
      switch ( formatDesc.pixelformat ) {
        case V4L2_PIX_FMT_SBGGR8:
          cameraInfo->videoCurrent = V4L2_PIX_FMT_SBGGR8;
          cameraInfo->mosaicFormat = formatDesc.pixelformat;
          camera->features.rawMode = 1;
          preferredFound = 1;
          if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
            haveNonEmulatedColour = 1;
          }
          break;
        default:
          break;
      }

    } else {

      switch ( formatDesc.pixelformat ) {

        case V4L2_PIX_FMT_RGB24:
          cameraInfo->videoCurrent = V4L2_PIX_FMT_RGB24;
          cameraInfo->videoRGB24 = 1;
          camera->features.demosaicMode = 1;
          preferredFound = 1;
          if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
            haveNonEmulatedColour = 1;
          }
          break;

        case V4L2_PIX_FMT_SBGGR8:
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGRBG8:
          if ( !preferredFound ) {
            cameraInfo->videoCurrent = formatDesc.pixelformat;
            cameraInfo->mosaicFormat = formatDesc.pixelformat;
            camera->features.rawMode = 1;
            preferredFound = 1;
            if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
              haveNonEmulatedColour = 1;
            }
          }
          break;

        case V4L2_PIX_FMT_YUV420:
          if ( !preferredFound ) {
            cameraInfo->videoCurrent = V4L2_PIX_FMT_YUV420;
            cameraInfo->videoYUV420 = 1;
            camera->features.demosaicMode = 1;
            preferredFound = 1;
            if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
              haveNonEmulatedColour = 1;
            }
          }
          break;

        case V4L2_PIX_FMT_YUYV:
          if ( !preferredFound ) {
            cameraInfo->videoCurrent = V4L2_PIX_FMT_YUYV;
            cameraInfo->videoYUYV = 1;
            camera->features.demosaicMode = 1;
            preferredFound = 1;
            if (!( formatDesc.flags & V4L2_FMT_FLAG_EMULATED )) {
              haveNonEmulatedColour = 1;
            }
          }
          break;

        case V4L2_PIX_FMT_Y16:
          bestGreyscale = V4L2_PIX_FMT_Y16;
          if ( !preferredFound ) {
            cameraInfo->videoCurrent = V4L2_PIX_FMT_Y16;
            cameraInfo->videoGrey16 = 1;
            preferredFound = 1;
          }
          break;

        case V4L2_PIX_FMT_GREY:
          if ( !bestGreyscale ) {
            bestGreyscale = V4L2_PIX_FMT_GREY;
          }
          if ( !cameraInfo->videoCurrent ) {
            cameraInfo->videoCurrent = V4L2_PIX_FMT_GREY;
            cameraInfo->videoGrey = 1;
            preferredFound = 1;
          }
          break;
      }
    }
  }

  // if the only colour modes are emulated then we look for a suitable
  // greyscale mode

  // FIX ME -- this isn't going to work.  Try again.

  if ( !haveNonEmulatedColour ) {
    switch ( bestGreyscale ) {
      case V4L2_PIX_FMT_Y16:
        cameraInfo->videoCurrent = V4L2_PIX_FMT_Y16;
        cameraInfo->videoGrey16 = 1;
        cameraInfo->videoGrey = 0;
        cameraInfo->videoRGB24 = cameraInfo->videoYUYV = 0;
        cameraInfo->videoYUV420 = 0;
      case V4L2_PIX_FMT_GREY:
        cameraInfo->videoCurrent = V4L2_PIX_FMT_GREY;
        cameraInfo->videoGrey = 1;
        cameraInfo->videoGrey16 = 0;
        cameraInfo->videoRGB24 = cameraInfo->videoYUYV = 0;
        cameraInfo->videoYUV420 = 0;
    }
  }

  if ( !cameraInfo->videoCurrent ) {
    fprintf ( stderr, "No suitable video format found on %s",
        camera->deviceName );
    v4l2_close ( cameraInfo->fd );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  // Put the camera into the current video mode.  Ignore the frame size
  // for now.  That will have to be sorted later by the caller

  OA_CLEAR ( format );
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.width = 1;
  format.fmt.pix.height = 1;
  format.fmt.pix.pixelformat = cameraInfo->videoCurrent;
  format.fmt.pix.field = V4L2_FIELD_NONE;
  if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_S_FMT, &format )) {
    perror ( "VIDIOC_S_FMT xioctl failed" );
    v4l2_close ( cameraInfo->fd );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  if ( format.fmt.pix.pixelformat != cameraInfo->videoCurrent ) {
    fprintf ( stderr, "Can't set required video format in %s.\n",
        __FUNCTION__);
    v4l2_close ( cameraInfo->fd );
    free (( void* ) commonInfo );
    free (( void* ) cameraInfo );
    free (( void* ) camera );
    return 0;
  }

  cameraInfo->frameSizes[1].numSizes = 0;
  cameraInfo->frameSizes[1].sizes = 0;

  j = 0;
  while ( 1 ) {
    OA_CLEAR ( fsize );
    fsize.index = j;
    fsize.pixel_format = cameraInfo->videoCurrent;
    if ( -1 == v4l2ioctl ( cameraInfo->fd, VIDIOC_ENUM_FRAMESIZES, &fsize )) {
      if ( EINVAL == errno) {
        break;
      } else {
        perror("VIDIOC_ENUM_FRAMESIZES failed");
      }
    }
    // FIX ME -- we can't handle mixed frame types here
    if ( V4L2_FRMSIZE_TYPE_DISCRETE == fsize.type ) {
      if (!(  cameraInfo->frameSizes[1].sizes = realloc (
          cameraInfo->frameSizes[1].sizes, ( j+1 ) * sizeof ( FRAMESIZE )))) {
        v4l2_close ( cameraInfo->fd );
        free (( void* ) commonInfo );
        free (( void* ) cameraInfo );
        free (( void* ) camera );
        return 0;
      }
      cameraInfo->frameSizes[1].sizes[j].x = fsize.discrete.width;
      cameraInfo->frameSizes[1].sizes[j].y = fsize.discrete.height;
    } else {
      fprintf ( stderr, "Can't handle framesizing type %d\n", fsize.type );
    }
    j++;
  }
  cameraInfo->frameSizes[1].numSizes = j;

  camera->features.frameRates = 0;

  OA_CLEAR( parm );
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_G_PARM, &parm )) {
    if ( errno != EINVAL ) {
      perror ( "VIDIOC_G_PARM v4l2ioctl failed" );
      v4l2_close ( cameraInfo->fd );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
  }
  if ( V4L2_CAP_TIMEPERFRAME == parm.parm.capture.capability ) {
    OA_CLEAR( parm );
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = 1;
    camera->features.frameRates = 1;
    if ( v4l2ioctl ( cameraInfo->fd, VIDIOC_S_PARM, &parm )) {
      perror ( "VIDIOC_S_PARM v4l2ioctl failed" );
      v4l2_close ( cameraInfo->fd );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      free (( void* ) commonInfo );
      free (( void* ) cameraInfo );
      free (( void* ) camera );
      return 0;
    }
  }

  cameraInfo->stopControllerThread = cameraInfo->stopCallbackThread = 0;
  cameraInfo->commandQueue = oaDLListCreate();
  cameraInfo->callbackQueue = oaDLListCreate();

  if ( pthread_create ( &( cameraInfo->controllerThread ), 0,
      oacamV4L2controller, ( void* ) camera )) {
    v4l2_close ( cameraInfo->fd );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }
  if ( pthread_create ( &( cameraInfo->callbackThread ), 0,
      oacamV4L2callbackHandler, ( void* ) camera )) {

    void* dummy;
    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );
    v4l2_close ( cameraInfo->fd );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) camera->_common );
    free (( void* ) camera->_private );
    free (( void* ) camera );
    oaDLListDelete ( cameraInfo->commandQueue, 0 );
    oaDLListDelete ( cameraInfo->callbackQueue, 0 );
    return 0;
  }

  cameraInfo->isStreaming = 0;
  cameraInfo->initialised = 1;

  return camera;
}


static void
_V4L2InitFunctionPointers ( oaCamera* camera )
{
  camera->funcs.initCamera = oaV4L2InitCamera;
  camera->funcs.closeCamera = oaV4L2CloseCamera;

  camera->funcs.readControl = oaV4L2CameraReadControl;
  camera->funcs.setControl = oaV4L2CameraSetControl;
  camera->funcs.testControl = oaV4L2CameraTestControl;
  camera->funcs.getControlRange = oaV4L2CameraGetControlRange;

  camera->funcs.startStreaming = oaV4L2CameraStartStreaming;
  camera->funcs.stopStreaming = oaV4L2CameraStopStreaming;
  camera->funcs.isStreaming = oaV4L2CameraIsStreaming;

  camera->funcs.setResolution = oaV4L2CameraSetResolution;

  // camera->funcs.resetCamera = oaV4L2CameraReset;

  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = oaIsAuto;

  camera->funcs.enumerateFrameSizes = oaV4L2CameraGetFrameSizes;
  camera->funcs.getFramePixelFormat = oaV4L2CameraGetFramePixelFormat;
  camera->funcs.enumerateFrameRates = oaV4L2CameraGetFrameRates;
  camera->funcs.setFrameInterval = oaV4L2CameraSetFrameInterval;

  camera->funcs.getAutoWBManualSetting = oaV4L2CameraGetAutoWBManualSetting;

  camera->funcs.getMenuString = oaV4L2CameraGetMenuString;
}


int
oaV4L2CloseCamera ( oaCamera* camera )
{
  V4L2_STATE*	cameraInfo;
  void*		dummy;

  if ( camera ) {

    cameraInfo = camera->_private;

    cameraInfo->stopControllerThread = 1;
    pthread_cond_broadcast ( &cameraInfo->commandQueued );
    pthread_join ( cameraInfo->controllerThread, &dummy );

    cameraInfo->stopCallbackThread = 1;
    pthread_cond_broadcast ( &cameraInfo->callbackQueued );
    pthread_join ( cameraInfo->callbackThread, &dummy );

    if ( cameraInfo->fd >= 0 ) {
      v4l2_close ( cameraInfo->fd );
    }
    if ( cameraInfo->frameRates.numRates ) {
     free (( void* ) cameraInfo->frameRates.rates );
    }
    if ( cameraInfo->frameSizes[1].numSizes ) {
      free (( void* ) cameraInfo->frameSizes[1].sizes );
    }

    oaDLListDelete ( cameraInfo->commandQueue, 1 );
    oaDLListDelete ( cameraInfo->callbackQueue, 1 );

    free (( void* ) camera->_common );
    free (( void* ) cameraInfo );
    free (( void* ) camera );

  } else {
   return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}

#endif /* HAVE_LIBV4L2 */
