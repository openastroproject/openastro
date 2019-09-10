/*****************************************************************************
 *
 * oldQHY6.c -- Old QHY6 camera interface
 *
 * Copyright 2014,2015,2018,2019 James Fidell (james@openastroproject.org)
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
#include <openastro/camera.h>
#include <openastro/errno.h>

#include <libusb-1.0/libusb.h>

#include "oacamprivate.h"
#include "oldQHY6.h"
#include "QHYusb.h"
#include "QHYoacam.h"
#include "QHYstate.h"

#define OLD_QHY6_SENSOR_WIDTH	796
#define OLD_QHY6_SENSOR_HEIGHT	596
#define OFFSET_X		115
#define VBE			1

#define	DEFAULT_SPEED		0
#define	DEFAULT_GAIN		31
#define DEFAULT_EXPOSURE	500

#define USB_TIMEOUT		5000

#define AMP_MODE_OFF		0
#define AMP_MODE_ON		1
#define AMP_MODE_AUTO		2


static void	_QHY6InitFunctionPointers ( oaCamera* );
static void     _recalculateSizes ( oaCamera* );
/*
static int      _setParameters ( oaCamera* );
static void     _startExposure ( oaCamera* );
static int      _readExposure ( oaCamera*, void*, unsigned int );
static void*    _capture ( void* );


static int      setControl ( oaCamera*, int, oaControlValue* );
static int      getFramePixelFormat ( oaCamera*, int );
*/
static const FRAMESIZES* getFrameSizes ( oaCamera* );
/*
static int      getControlRange ( oaCamera*, int, int64_t*, int64_t*,
			int64_t*, int64_t* );
*/
static int      closeCamera ( oaCamera* );

/**
 * Initialise a given camera device
 */

int
_oldQHY6InitCamera ( oaCamera* camera )
{
  int		i, j;
  QHY_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;

  OA_CLEAR ( camera->controls );
  OA_CLEAR ( camera->features );
  _QHY6InitFunctionPointers ( camera );

  camera->controls[ OA_CAM_CTRL_GAIN ] = OA_CTRL_TYPE_INT32;
  commonInfo->min[ OA_CAM_CTRL_GAIN ] = 0;
  commonInfo->max[ OA_CAM_CTRL_GAIN ] = 63;
  commonInfo->step[ OA_CAM_CTRL_GAIN ] = 1;
  commonInfo->def[ OA_CAM_CTRL_GAIN ] = DEFAULT_GAIN;

  camera->controls[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = OA_CTRL_TYPE_INT64;
  commonInfo->min[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = 0;
  commonInfo->max[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = 0xffffffff;
  commonInfo->step[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = 1;
  commonInfo->def[ OA_CAM_CTRL_EXPOSURE_ABSOLUTE ] = DEFAULT_EXPOSURE * 1000;

  camera->controls[ OA_CAM_CTRL_HIGHSPEED ] = OA_CTRL_TYPE_BOOLEAN;
  commonInfo->min[ OA_CAM_CTRL_HIGHSPEED ] = 0;
  commonInfo->max[ OA_CAM_CTRL_HIGHSPEED ] = 1;
  commonInfo->step[ OA_CAM_CTRL_HIGHSPEED ] = 1;
  commonInfo->def[ OA_CAM_CTRL_HIGHSPEED ] = DEFAULT_SPEED;
  cameraInfo->currentHighSpeed = DEFAULT_SPEED;

  // FIX ME -- Binning disabled as it's not clear how some of it works
  // camera->controls[ OA_CAM_CTRL_BINNING ] = OA_CTRL_TYPE_DISCRETE;

  camera->controls[ OA_CAM_CTRL_DROPPED ] = OA_CTRL_TYPE_READONLY;
  camera->controls[ OA_CAM_CTRL_DROPPED_RESET ] = OA_CTRL_TYPE_BUTTON;

  cameraInfo->maxResolutionX = OLD_QHY6_SENSOR_WIDTH;
  cameraInfo->maxResolutionY = OLD_QHY6_SENSOR_HEIGHT;

  cameraInfo->videoRGB24 = cameraInfo->videoGrey = 0;
  cameraInfo->videoGrey16 = 1;
  cameraInfo->videoCurrent = OA_PIX_FMT_GREY16BE;

  for ( i = 1; i <= OA_MAX_BINNING; i++ ) {
    cameraInfo->frameSizes[i].numSizes = 0;
    cameraInfo->frameSizes[i].sizes = 0;
  }
  if (!( cameraInfo->frameSizes[1].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    return -OA_ERR_MEM_ALLOC;
  }
  if (!( cameraInfo->frameSizes[2].sizes =
      ( FRAMESIZE* ) malloc ( sizeof ( FRAMESIZE )))) {
    fprintf ( stderr, "%s: malloc ( FRAMESIZE ) failed\n", __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    return -OA_ERR_MEM_ALLOC;
  }
  cameraInfo->frameSizes[1].sizes[0].x = cameraInfo->maxResolutionX;
  cameraInfo->frameSizes[1].sizes[0].y = cameraInfo->maxResolutionY;
  cameraInfo->frameSizes[1].numSizes = 1;

  cameraInfo->binMode = OA_BIN_MODE_NONE;

  cameraInfo->frameSizes[2].sizes[0].x = OLD_QHY6_SENSOR_WIDTH / 2;
  cameraInfo->frameSizes[2].sizes[0].y = OLD_QHY6_SENSOR_HEIGHT / 2;
  cameraInfo->frameSizes[2].numSizes = 1;

  cameraInfo->xSize = cameraInfo->maxResolutionX;
  cameraInfo->ySize = cameraInfo->maxResolutionY;
  cameraInfo->xOffset = OFFSET_X;

  cameraInfo->buffers = 0;
  cameraInfo->configuredBuffers = 0;

  cameraInfo->topOffset = cameraInfo->bottomOffset = 0;

  _recalculateSizes ( camera );
  if (!( cameraInfo->xferBuffer = malloc ( cameraInfo->captureLength ))) {
    fprintf ( stderr, "malloc of transfer buffer failed in %s\n",
        __FUNCTION__ );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    return -OA_ERR_MEM_ALLOC;
  }

  cameraInfo->currentGain = commonInfo->def[ OA_CAM_CTRL_GAIN ];
  cameraInfo->currentExposure = DEFAULT_EXPOSURE;
  cameraInfo->correctedExposureTime = DEFAULT_EXPOSURE - DEFAULT_EXPOSURE / 10;

  // FIX ME -- need to add amp on/off/auto when I understand what it does
  cameraInfo->requestedAmpMode = AMP_MODE_AUTO;
  if ( cameraInfo->correctedExposureTime > 550 &&
      AMP_MODE_AUTO == cameraInfo->requestedAmpMode ) {
    cameraInfo->currentAmpMode = AMP_MODE_OFF;
  } else {
    cameraInfo->currentAmpMode = cameraInfo->requestedAmpMode ?
        AMP_MODE_OFF : AMP_MODE_ON;
  }
  if ( AMP_MODE_OFF == cameraInfo->currentAmpMode ) {
    if ( cameraInfo->correctedExposureTime > 500 ) {
      cameraInfo->correctedExposureTime -= 500;
    } else {
      cameraInfo->correctedExposureTime = 1;
    }
  }

  cameraInfo->imageBufferLength = cameraInfo->maxResolutionX *
      cameraInfo->maxResolutionY * 2;
  cameraInfo->buffers = calloc ( OA_CAM_BUFFERS, sizeof ( struct QHYbuffer ));
  for ( i = 0; i < OA_CAM_BUFFERS; i++ ) {
    void* m = malloc ( cameraInfo->imageBufferLength );
    if ( m ) {
      cameraInfo->buffers[i].start = m;
      cameraInfo->configuredBuffers++;
    } else {
      fprintf ( stderr, "%s malloc failed\n", __FUNCTION__ );
      if ( i ) {
        for ( j = 0; j < i; j++ ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
      free ( cameraInfo->xferBuffer );
      free (( void* ) cameraInfo->frameSizes[1].sizes );
      free (( void* ) cameraInfo->frameSizes[2].sizes );
      return -OA_ERR_MEM_ALLOC;
    }
  }

  return OA_ERR_NONE;
}


/*
static int
_setParameters ( oaCamera* camera )
{
  unsigned char buff[24];
  unsigned int xSizex2 = cameraInfo->xSize * 2;

  OA_CLEAR ( buff );

  buff[0] = cameraInfo->currentGain;
  buff[1] = ( cameraInfo->correctedExposureTime >> 16 ) & 0xff;
  buff[2] = ( cameraInfo->correctedExposureTime >> 8 ) & 0xff;
  buff[3] = cameraInfo->correctedExposureTime & 0xff;
  buff[4] = ( cameraInfo->binMode == OA_BIN_MODE_2x2 ? 0x4c : 0x04 ) |
      ( cameraInfo->currentHighSpeed ? 0x80 : 0 );
  buff[5] = ( cameraInfo->currentAmpMode == AMP_MODE_OFF ) ? 0x40 : 0;
  buff[6] = ( cameraInfo->binMode == OA_BIN_MODE_2x2 ) ? 2 : ( VBE - 1 );
  buff[7] = cameraInfo->xOffset;
  buff[8] = 0xac;
  buff[9] = 0xac;
  buff[14] = ( xSizex2 >> 8 ) & 0xff;
  buff[15] = xSizex2 & 0xff;
  buff[16] = ( cameraInfo->ySize >> 8 ) & 0xff;
  buff[17] = cameraInfo->ySize & 0xff;
  buff[18] = ( cameraInfo->transferPadding >> 8 ) & 0xff;
  buff[19] = cameraInfo->transferPadding & 0xff;
  buff[20] = 0;
  buff[23] = 0xac;

  _usbControlMsg ( camera, 0x42, QHY_REQ_SET_REGISTERS, 0, 0, buff,
      sizeof ( buff ), USB_TIMEOUT );
  return OA_ERR_NONE;
}


static void
_startExposure ( oaCamera* camera )
{
  unsigned char buff[8] = "DEADBEEF";
  // FIX ME -- more magic numbers
  buff[0] = 0;
  buff[1] = 100;
  _usbControlMsg ( camera, 0x42, QHY_REQ_BEGIN_VIDEO, 0, 0, buff, 2,
      USB_TIMEOUT );
}


static int
_readExposure ( oaCamera* camera, void* buffer, unsigned int readLength )
{
  int ret;
  unsigned int readSize;

  // references to droppedFrames here could be mutexed

  ret = _usbBulkTransfer ( camera, QHY_BULK_ENDP_IN, buffer, readLength,
      &readSize, USB_TIMEOUT );
  if ( ret ) {
    cameraInfo->droppedFrames++;
    return ret;
  }
  if ( readSize != readLength ) {
    fprintf ( stderr, "readExposure: USB bulk transfer was short. %d != %d\n",
        readSize, readLength );
    cameraInfo->droppedFrames++;
    return -OA_ERR_CAMERA_IO;
  }
  return OA_ERR_NONE;
}


static int
setControl ( oaCamera* camera, int control, oaControlValue* val )
{
  int32_t	val_s64;
  int		updateSettings = 0;

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      if ( val->valueType != OA_CTRL_TYPE_INT32 ) {
        fprintf ( stderr, "%s: invalid control type %d where int32 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentGain = val->int64;
      updateSettings = 1;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    {
      uint64_t val_u64;

      if ( val->valueType != OA_CTRL_TYPE_INT64 ) {
        fprintf ( stderr, "%s: invalid control type %d where int64 expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      val_s64 = val->int64;
      val_s64 /= 1000;
      if ( val_s64 < 1 ) { val_s64 = 1; }
      val_u64 = ( uint64_t ) val_s64;
      cameraInfo->currentExposure = val_u64;
      cameraInfo->correctedExposureTime = val_u64 - val_u64 / 10;
      if ( cameraInfo->correctedExposureTime > 550 &&
          AMP_MODE_AUTO == cameraInfo->requestedAmpMode ) {
        cameraInfo->currentAmpMode = 0;
      } else {
        cameraInfo->currentAmpMode = cameraInfo->requestedAmpMode ? 0 : 1;
      }
      if ( AMP_MODE_OFF == cameraInfo->currentAmpMode ) {
        if ( cameraInfo->correctedExposureTime > 500 ) {
          cameraInfo->correctedExposureTime -= 500;
        } else {
          cameraInfo->correctedExposureTime = 1;
        }
      }
      updateSettings = 1;
      break;
    }

#if 0
    // FIX ME -- binning currently disabled
    case OA_CAM_CTRL_BINNING:
      if ( val->valueType != OA_CTRL_TYPE_DISCRETE ) {
        fprintf ( stderr, "%s: invalid control type %d where discrete "
            "expected\n", __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      switch ( val->discrete ) {
        case OA_BIN_MODE_NONE:
        case OA_BIN_MODE_2x2:
          pthread_mutex_lock ( &cameraInfo->captureMutex );
          cameraInfo->binMode = val->discrete;
          // _recalculateSizes ( camera );
          pthread_mutex_unlock ( &cameraInfo->captureMutex );
          // don't need to update the settings here as a resolution
          // change should be forthcoming
          break;
        default:
          return -OA_ERR_OUT_OF_RANGE;
          break;
      }
      break;
#endif

    case OA_CAM_CTRL_HIGHSPEED:
      if ( val->valueType != OA_CTRL_TYPE_BOOLEAN ) {
        fprintf ( stderr, "%s: invalid control type %d where bool expected\n",
            __FUNCTION__, val->valueType );
        return -OA_ERR_INVALID_CONTROL_TYPE;
      }
      cameraInfo->currentHighSpeed = val->boolean;
      updateSettings = 1;
      break;

    case OA_CAM_CTRL_DROPPED_RESET:
      // droppedFrames could be mutexed
      cameraInfo->droppedFrames = 0;
      break;

    default:
      fprintf ( stderr, "oldQHY6: %s not yet implemented for control %d\n",
          __FUNCTION__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  if ( updateSettings ) {
    if ( cameraInfo->captureThreadStarted ) {
      pthread_mutex_lock ( &cameraInfo->captureMutex );
      cameraInfo->doSetParameters = 1;
      pthread_mutex_unlock ( &cameraInfo->captureMutex );
    } else {
      _setParameters ( camera );
    }
  }

  return OA_ERR_NONE;
}


static int
readControl ( oaCamera* camera, int control, oaControlValue* val )
{
  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      val->valueType = OA_CTRL_TYPE_INT32;
      val->int32 = cameraInfo->currentGain;
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      val->valueType = OA_CTRL_TYPE_INT64;
      val->int64 = cameraInfo->currentExposure * 1000;
      break;

    case OA_CAM_CTRL_DROPPED:
      val->valueType = OA_CTRL_TYPE_READONLY;
      val->readonly = cameraInfo->droppedFrames;
      break;

    default:
      fprintf ( stderr,
          "Unrecognised control %d in oldQHY6:%s\n", control, __FUNCTION__ );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }
  return OA_ERR_NONE;
}


static int
testControl ( oaCamera* camera, int control, oaControlValue* val )
{
  if ( !camera->controls [ control ] ) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->controls [ control ] != val->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  switch ( control ) {

    case OA_CAM_CTRL_GAIN:
      if ( val->int32 >= camera->min[ control ] &&
          val->int32 <= camera->max[ control ] &&
          ( 0 == ( val->int32 - camera->min[ control ]) %
          camera->step[ control ])) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
      if ( val->int64 >= camera->min[ control ] &&
          val->int64 <= camera->max[ control ] &&
          ( 0 == ( val->int64 - camera->min[ control ]) %
          camera->step[ control ])) {
        return OA_ERR_NONE;
      }
      break;

#if 0
    // FIX ME -- binning currently disabled
    case OA_CAM_CTRL_BINNING:
      if ( OA_CTRL_TYPE_DISCRETE == val->valueType &&
          ( OA_BIN_MODE_NONE == val->discrete || OA_BIN_MODE_2x2 ==
          val->discrete )) {
        return OA_ERR_NONE;
      }
      break;
#endif

    default:
      fprintf ( stderr, "oldQHY6: %s not yet implemented for control %d\n",
          __FUNCTION__, control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  // And if we reach here it's because the value wasn't valid
  return -OA_ERR_OUT_OF_RANGE;
}


static int
getFramePixelFormat ( oaCamera* camera, int depth )
{
  if ( depth > 0 && depth != 16 ) {
    return -OA_ERR_INVALID_BIT_DEPTH;
  }
  return OA_PIX_FMT_GREY16BE;
}
*/

const static FRAMESIZES*
getFrameSizes ( oaCamera* camera )
{
  QHY_STATE*    cameraInfo = camera->_private;

  if ( cameraInfo->binMode == OA_BIN_MODE_2x2 ) {
    return &cameraInfo->frameSizes[2];
  }
  return &cameraInfo->frameSizes[1];
}


/*
static int
stop ( oaCamera* camera )
{
  if ( !cameraInfo->captureThreadStarted ) {
    return OA_ERR_NONE;
  }

  pthread_mutex_lock ( &cameraInfo->captureMutex );
  cameraInfo->captureThreadExit = 1;
  pthread_mutex_unlock ( &cameraInfo->captureMutex );
  pthread_join ( cameraInfo->captureThread, 0 );
  cameraInfo->captureThreadStarted = 0;
  return OA_ERR_NONE;
}


static int
start ( oaCamera* camera, START_PARMS* params )
{
  cameraInfo->captureThreadStarted = 0;
  cameraInfo->captureThreadExit = 0;
  cameraInfo->xSize = params->size.x;
  cameraInfo->ySize = params->size.y;

  _recalculateSizes ( camera );
  _setParameters ( camera );
  cameraInfo->imageBufferLength = cameraInfo->frameSize;
  pthread_create ( &cameraInfo->captureThread, 0, &_capture, camera );
  cameraInfo->captureThreadStarted = 1;
  return OA_ERR_NONE;
}


static void*
_capture ( void* param )
{
  oaCamera*      camera = param;
  int            quitThread, nextBuffer, doSetParameters, rowBytes;
  unsigned int   i;
  unsigned char* evenSrc;
  unsigned char* oddSrc;
  unsigned char* s1;
  unsigned char* s2;
  unsigned char* t;

  pthread_mutex_lock ( &cameraInfo->captureMutex );
  cameraInfo->haveDataToRead = 0;
  cameraInfo->nextBufferToRead = -1;
  cameraInfo->doSetParameters = 0;
  pthread_mutex_unlock ( &cameraInfo->captureMutex );
  nextBuffer = 0;

  // see the comments further down for more information about these
  evenSrc = cameraInfo->xferBuffer;
  oddSrc = cameraInfo->xferBuffer + cameraInfo->frameSize / 2;
  rowBytes = cameraInfo->xSize * 2;

  while ( 1 ) {
    pthread_mutex_lock ( &cameraInfo->captureMutex );
    quitThread = cameraInfo->captureThreadExit;
    if ( nextBuffer == cameraInfo->nextBufferToRead ) {
      nextBuffer = ( nextBuffer + 1 ) % cameraInfo->configuredBuffers;
    }
    pthread_mutex_unlock ( &cameraInfo->captureMutex );
    if ( quitThread ) {
      break;
    }

    _startExposure ( camera );
    usleep ( cameraInfo->currentExposure );
    _readExposure ( camera, cameraInfo->xferBuffer,
        cameraInfo->captureLength );

    // Now the frame has to be unpacked.  There are two "half-frames"
    // in the buffer, one of odd-numbered scanlines and one of even

    s1 = oddSrc;
    s2 = evenSrc;
    t = cameraInfo->buffers[ nextBuffer ].start;
    for ( i = 0; i < cameraInfo->ySize / 2; i++ ) {
      memcpy ( t, s1, rowBytes );
      t += rowBytes;
      s1 += rowBytes;
      memcpy ( t, s2, rowBytes );
      t += rowBytes;
      s2 += rowBytes;
    }

    pthread_mutex_lock ( &cameraInfo->captureMutex );
    cameraInfo->nextBufferToRead = nextBuffer;
    cameraInfo->haveDataToRead = 1;
    doSetParameters = cameraInfo->doSetParameters;
    cameraInfo->doSetParameters = 0;
    pthread_cond_signal ( &cameraInfo->frameAvailable );
    pthread_mutex_unlock ( &cameraInfo->captureMutex );
    nextBuffer = ( nextBuffer + 1 ) % cameraInfo->configuredBuffers;

    if ( doSetParameters ) {
      _setParameters ( camera );
    }
  }
  return OA_ERR_NONE;
}


static int
getControlRange ( oaCamera* camera, int control, int64_t* min, int64_t* max,
    int64_t* step, int64_t* def )
{
  if ( !camera->controls[ control ] ) {
    return -OA_ERR_INVALID_CONTROL;
  }

  *min = camera->min[ control ];
  *max = camera->max[ control ];
  *step = camera->step[ control ];
  *def = camera->def[ control ];
  return OA_ERR_NONE;
}


static int
startReadFrame ( oaCamera* camera, unsigned int frameTime )
{
  cameraInfo->frameTime = frameTime;
  return OA_ERR_NONE;
}


static int
readFrame ( oaCamera* camera, void** buffer )
{
  int haveData = 0;
  int bufferToRead;
  struct timespec waitUntil;
  struct timeval now;
  unsigned long frameSec = 0;
  unsigned long frameNanoSec = 0;

  frameNanoSec = cameraInfo->frameTime * 1000000;
  frameSec = cameraInfo->frameTime / 1000 + 5;

  pthread_mutex_lock ( &cameraInfo->captureMutex );
  while ( !cameraInfo->haveDataToRead ) {

    gettimeofday ( &now, 0 );
    waitUntil.tv_sec = now.tv_sec + frameSec;
    if (( now.tv_usec * 1000 + frameNanoSec ) > 1000000000 ) {
      waitUntil.tv_sec++;
    }
    waitUntil.tv_nsec = ( now.tv_usec * 1000 + frameNanoSec ) % 1000000000;

    pthread_cond_timedwait ( &cameraInfo->frameAvailable,
        &cameraInfo->captureMutex, &waitUntil );
  }
  haveData = cameraInfo->haveDataToRead;
  bufferToRead = cameraInfo->nextBufferToRead;
  cameraInfo->haveDataToRead = 0;
  pthread_mutex_unlock ( &cameraInfo->captureMutex );

  *buffer = haveData ? cameraInfo->buffers[ bufferToRead ].start : 0;
  return ( haveData ? cameraInfo->imageBufferLength : 0 );
}


static int
finishReadFrame ( oaCamera* camera )
{
  return OA_ERR_NONE;
}
*/

static int
closeCamera ( oaCamera* camera )
{
  int		j;
  QHY_STATE*	cameraInfo;

  if ( camera ) {

    cameraInfo = camera->_private;
    libusb_release_interface ( cameraInfo->usbHandle, 0 );
    libusb_close ( cameraInfo->usbHandle );
    libusb_exit ( cameraInfo->usbContext );

    if ( cameraInfo->buffers ) {
      for ( j = 0; j < OA_CAM_BUFFERS; j++ ) {
        if ( cameraInfo->buffers[j].start ) {
          free (( void* ) cameraInfo->buffers[j].start );
        }
      }
    }
    free (( void* ) cameraInfo->xferBuffer );
    free (( void* ) cameraInfo->buffers );
    free (( void* ) cameraInfo->frameSizes[1].sizes );
    free (( void* ) cameraInfo->frameSizes[2].sizes );
    free (( void* ) cameraInfo );
    free (( void* ) camera->_common );
    free (( void* ) camera );
  } else {
   return -OA_ERR_INVALID_CAMERA;
  }
  return OA_ERR_NONE;
}


static void
_recalculateSizes ( oaCamera* camera )
{
  QHY_STATE*	cameraInfo;

  cameraInfo = camera->_private;
  cameraInfo->frameSize = cameraInfo->xSize * cameraInfo->ySize * 2;
  cameraInfo->captureLength = (( cameraInfo->frameSize >> 10 ) + 1 ) << 10;
  cameraInfo->transferPadding = cameraInfo->captureLength -
      cameraInfo->frameSize;
}


static void
_QHY6InitFunctionPointers ( oaCamera* camera )
{
  // Set by QHYinit()
  // camera->funcs.initCamera = oaQHYInitCamera;
  camera->funcs.closeCamera = closeCamera;

/*
  // camera->funcs.resetCamera = reset;
  // camera->funcs.startCapture = start;
  camera->funcs.startCaptureExtended = start;
  camera->funcs.stopCapture = stop;
  // stick with the device versions of these
  // camera->funcs.hasLoadableFirmware = _cameraHasLoadableFirmware;
  // camera->funcs.isFirmwareLoaded = _cameraIsFirmwareLoaded;
  // camera->funcs.loadFirmware = _cameraLoadFirmware;
  camera->funcs.getControlRange = getControlRange;
  camera->funcs.readControl = readControl;
  camera->funcs.setControl = setControl;
  camera->funcs.testControl = testControl;
*/
  camera->funcs.hasAuto = oacamHasAuto;
  // camera->funcs.isAuto = _isAuto;
/*
  // camera->funcs.setControlMulti = _setControlMulti;
  camera->funcs.startReadFrame = startReadFrame;
  camera->funcs.readFrame = readFrame;
  camera->funcs.finishReadFrame = finishReadFrame;
*/
  camera->funcs.enumerateFrameSizes = getFrameSizes;
/*
  camera->funcs.getFramePixelFormat = getFramePixelFormat;
  // camera->funcs.testROISize = testROISize;
  // camera->funcs.getMenuString = _getMenuString;
  // camera->funcs.getAutoWBManualSetting = _getAutoWBManualSetting;
  // camera->funcs.hasFixedFrameRates = _hasFixedFrameRates;
*/
}

